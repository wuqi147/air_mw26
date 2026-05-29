#include "FreeRTOS.h"
#include "spinorwrite.h"
#include "timer.h"

#define SOH         0x01
#define STX         0x02
#define EOT         0x04
#define ACK         0x06
#define NAK         0x15
#define CAN         0x18
#define CTRLZ       0x1A

#define XMODEM_TIMEOUT_DELAY    100
#define XMODEM_RETRY_LIMIT      32

#define XMODEM_ERROR_REMOTECANCEL   -1
#define XMODEM_ERROR_OUTOFSYNC      -2
#define XMODEM_ERROR_RETRYEXCEED    -3
#define XMODEM_ERROR_UNEXPECTEDDATA -4

#define XMODEM_BUFFER_SIZE      1024

#define XMODEM_WRITE_FLASH_BUFFER   4096

extern void serial_outc(char c);
extern char serial_inc(void);
extern int serial_tstc(void);
extern int tmpCpuTimerCnt;


unsigned char xmbuf[XMODEM_BUFFER_SIZE+6];

unsigned short crc_xmodem_update(unsigned short crc, unsigned char data)
{
    int i;

    crc = crc ^ ((unsigned short)data << 8);
    for (i=0; i<8; i++)
    {
        if(crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
    }

    return crc;
}

int xmodemCrcCheck(int crcflag, const unsigned char *buffer, int size)
{
    // crcflag=0 - do regular checksum
    // crcflag=1 - do CRC checksum

    if(crcflag)
    {
        unsigned short crc=0;
        unsigned short pktcrc = (buffer[size]<<8)+buffer[size+1];
        // do CRC checksum
        while(size--)
            crc = crc_xmodem_update(crc, *buffer++);
        // check checksum against packet
        if(crc == pktcrc)
            return 1;
    }
    else
    {
        int i;
        unsigned char cksum = 0;
        // do regular checksum
        for(i=0; i<size; ++i)
        {
            cksum += buffer[i];
        }
        // check checksum against packet
        if(cksum == buffer[size])
            return 1;
    }

    return 0;
}

int xmodemInTime(int timeout_10ms)
{
    int timeout = tmpCpuTimerCnt + timeout_10ms;

    while(serial_tstc() == 0)
    {
        /* Check timeout */
        if(tmpCpuTimerCnt >= timeout)
        {
            return -1;
        }
    }

    return (serial_inc() & 0xff);
}

void xmodemInFlush(void)
{
    while(xmodemInTime(XMODEM_TIMEOUT_DELAY) >= 0);
}

int xmodemReceive(void)
{
    // create xmodem buffer
    // 1024b for Xmodem 1K
    // 128 bytes for Xmodem std.
    // + 5b header/crc + NULL
    unsigned char seqnum=1;     // xmodem sequence number starts at 1
    unsigned short pktsize=128; // default packet size is 128 bytes
    unsigned char response='C'; // solicit a connection with CRC enabled
    char retry=XMODEM_RETRY_LIMIT;
    unsigned char crcflag=0;
    unsigned long totalbytes=0;
    int i,c;

    while(retry > 0)
    {
        air_wdog_kick();

        // solicit a connection/packet
        serial_outc(response);
        // wait for start of packet
        if( (c = xmodemInTime(XMODEM_TIMEOUT_DELAY)) >= 0)
        {
            switch(c)
            {
                case SOH:
                    pktsize = 128;
                    break;
                #if(XMODEM_BUFFER_SIZE>=1024)
                case STX:
                    pktsize = 1024;
                    break;
                #endif
                case EOT:
                    xmodemInFlush();
                    serial_outc(ACK);
                    // completed transmission normally
                    WriteLastBuffer();
                    return totalbytes;
                case CAN:
                    if((c = xmodemInTime(XMODEM_TIMEOUT_DELAY)) == CAN)
                    {
                        xmodemInFlush();
                        serial_outc(ACK);
                        // transaction cancelled by remote node
                        return XMODEM_ERROR_REMOTECANCEL;
                    }
                default:
                    return XMODEM_ERROR_UNEXPECTEDDATA;
            }
        }
        else
        {
            // timed out, try again
            // no need to flush because receive buffer is already empty
            retry--;
            //response = NAK;
            continue;
        }

        // check if CRC mode was accepted
        if(response == 'C') crcflag = 1;
        // got SOH/STX, add it to processing buffer
        xmbuf[0] = c;
        // try to get rest of packet
        for(i=0; i<(pktsize+crcflag+4-1); i++)
        {
            if((c = xmodemInTime(XMODEM_TIMEOUT_DELAY)) >= 0)
            {
                xmbuf[1+i] = c;
            }
            else
            {
                // timed out, try again
                retry--;
                xmodemInFlush();
                response = NAK;
                break;
            }
        }
        // packet was too small, retry
        if(i<(pktsize+crcflag+4-1))
            continue;

        // got whole packet
        // check validity of packet
        if( (xmbuf[1] == (unsigned char)(~xmbuf[2])) &&     // sequence number was transmitted w/o error
            xmodemCrcCheck(crcflag, &xmbuf[3], pktsize) )   // packet is not corrupt
        {
            // is this the packet we were waiting for?
            if(xmbuf[1] == seqnum)
            {
                // write/deliver data
                WriteBuffer(&xmbuf[3], pktsize);
                totalbytes += pktsize;
                // next sequence number
                seqnum++;
                // reset retries
                retry = XMODEM_RETRY_LIMIT;
                // reply with ACK
                response = ACK;
                continue;
            }
            else if(xmbuf[1] == (unsigned char)(seqnum-1))
            {
                // this is a retransmission of the last packet
                // ACK and move on
                response = ACK;
                continue;
            }
            else
            {
                // we are completely out of sync
                // cancel transmission
                xmodemInFlush();
                serial_outc(CAN);
                serial_outc(CAN);
                serial_outc(CAN);
                return XMODEM_ERROR_OUTOFSYNC;
            }
        }
        else
        {
            // packet was corrupt
            // NAK it and try again
            retry--;
            xmodemInFlush();
            response = NAK;
            continue;
        }
    }

    // exceeded retry count
    xmodemInFlush();
    serial_outc(CAN);
    serial_outc(CAN);
    serial_outc(CAN);
    return XMODEM_ERROR_RETRYEXCEED;
}

int XModemReceive(unsigned int flash_addr)
{
    int result = 0;
    unsigned int i = 0;
    unsigned int temp_src = 0;
    unsigned int temp_dst =0;
    unsigned int temp_size = 0;

    WriteBufferInit((unsigned char *) TempSystemBase);

    result = xmodemReceive();

    if (result > 0)
    {
        /* Update raw image to special flash address */
        if (flash_addr != ~0UL)
        {
            temp_src = TempSystemBase;
            temp_dst = flash_addr;
            temp_size = result;

            while (temp_size)
            {
                if (temp_size > SPI_NOR_SECTOR_SIZE)
                    i = SPI_NOR_SECTOR_SIZE;
                else
                    i = temp_size;

                spinor_write(temp_src, temp_dst, i);
                temp_src += i;
                temp_dst += i;
                temp_size -= i;
            }
        }
    }
    return result;
}

