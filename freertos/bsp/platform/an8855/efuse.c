/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include <FreeRTOS.h>

#include <platform.h>

typedef enum{
    EFUSE_STATUS_SUCCESS = 0,
    EFUSE_STATUS_BUSY,
    EFUSE_STATUS_FAIL
} EFUSE_STATUS_T;

#define CR_EFUSE_BASE               (0x1000A400)

#define CR_EFUSE_CON1               (0x00)
#define CR_EFUSE_CON2               (0x04)
#define CR_EFUSE_BPKEY              (0x08)
#define CR_EFUSE_TMST               (0x14)
#define CR_EFUSE_TMRD               (0x18)
#define CR_EFUSE_TMPG               (0x1C)
#define CR_EFUSE_TMHD               (0x20)
#define CR_EFUSE_MCTL               (0x24)

#define CR_EFUSE_DATA00             (0x100)
#define CR_EFUSE_DATA01             (0x104)
#define CR_EFUSE_DATA02             (0x108)
#define CR_EFUSE_DATA03             (0x10C)
#define CR_EFUSE_DATA04             (0x110)
#define CR_EFUSE_DATA05             (0x114)
#define CR_EFUSE_DATA06             (0x118)
#define CR_EFUSE_DATA07             (0x11C)
#define CR_EFUSE_DATA08             (0x120)
#define CR_EFUSE_DATA09             (0x124)
#define CR_EFUSE_DATA10             (0x128)
#define CR_EFUSE_DATA11             (0x12C)
#define CR_EFUSE_DATA12             (0x130)
#define CR_EFUSE_DATA13             (0x134)
#define CR_EFUSE_DATA14             (0x138)
#define CR_EFUSE_DATA15             (0x13C)
#define CR_EFUSE_DATA16             (0x140)
#define CR_EFUSE_DATA17             (0x144)
#define CR_EFUSE_DATA18             (0x148)
#define CR_EFUSE_DATA19             (0x14C)
#define CR_EFUSE_DATA20             (0x150)
#define CR_EFUSE_DATA21             (0x154)
#define CR_EFUSE_DATA22             (0x158)
#define CR_EFUSE_DATA23             (0x15C)
#define CR_EFUSE_DATA24             (0x160)
#define CR_EFUSE_DATA25             (0x164)
#define CR_EFUSE_DATA26             (0x168)
#define CR_EFUSE_DATA27             (0x16C)
#define CR_EFUSE_DATA28             (0x170)
#define CR_EFUSE_DATA29             (0x174)
#define CR_EFUSE_DATA30             (0x178)
#define CR_EFUSE_DATA31             (0x17C)
#define CR_EFUSE_DATA32             (0x180)
#define CR_EFUSE_DATA33             (0x184)
#define CR_EFUSE_DATA34             (0x188)
#define CR_EFUSE_DATA35             (0x18C)
#define CR_EFUSE_DATA36             (0x190)
#define CR_EFUSE_DATA37             (0x194)
#define CR_EFUSE_DATA38             (0x198)
#define CR_EFUSE_DATA39             (0x19C)
#define CR_EFUSE_DATA40             (0x1A0)
#define CR_EFUSE_DATA41             (0x1A4)
#define CR_EFUSE_DATA42             (0x1A8)
#define CR_EFUSE_DATA43             (0x1AC)
#define CR_EFUSE_DATA44             (0x1B0)
#define CR_EFUSE_DATA45             (0x1B4)
#define CR_EFUSE_DATA46             (0x1B8)
#define CR_EFUSE_DATA47             (0x1BC)
#define CR_EFUSE_DATA48             (0x1C0)
#define CR_EFUSE_DATA49             (0x1C4)
#define CR_EFUSE_SDATA0             (0x1C8)
#define CR_EFUSE_SDATA0_SPR         (0x1CC)
#define CR_EFUSE_SDATA1             (0x1D0)
#define CR_EFUSE_SDATA1_SPR         (0x1D4)
#define CR_EFUSE_MDATA0             (0x1D8)
#define CR_EFUSE_MDATA1             (0x1DC)
#define CR_EFUSE_MDATA2             (0x1E0)
#define CR_EFUSE_MDATA3             (0x1E4)
#define CR_EFUSE_MDATA4             (0x1E8)
#define CR_EFUSE_MDATA5             (0x1EC)
#define CR_EFUSE_MDATA6             (0x1F0)
#define CR_EFUSE_MDATA7             (0x1F4)

#define CR_EFUSE_MDATA0_SPR         (0x0FC)
#define CR_EFUSE_MDATA1_SPR         (0x0F8)
#define CR_EFUSE_MDATA2_SPR         (0x0F4)
#define CR_EFUSE_MDATA3_SPR         (0x0F0)
#define CR_ECC                      (0x0EC)
#define CR_CLOCK                    (0x0E8)
#define CR_EFUSE_MDATA4_SPR         (0x0E4)
#define CR_EFUSE_MDATA5_SPR         (0x0E0)
#define CR_EFUSE_MDATA6_SPR         (0x0DC)
#define CR_EFUSE_MDATA7_SPR         (0x0D8)
#define CR_MLOCK                    (0x0D4)



#define MAX_DELAY                   (0x10000000)  /* (1073741823 * 6) / 900M ~= 1.78s */
#define MAX_RETRIES                 (100000)

#define EFUSE_BPKEY                 (0xa07923b6)
#define EFUSE_VLD                   (0x1 << 0)
#define EFUSE_BUSY                  (0x1 << 8)
#define EFUSE_RELD                  (0x1 << 16)
#define EFUSE_AUTOECC               (0x1 << 0)
#define EFUSE_AUTOECC_ONLY          (0x1 << 8)
#define EFUSE_SECC                  (0x1 << 0)
#define EFUSE_MECC                  (0x1 << 1)

void efuse_bpkey_write(unsigned int enable)
{
    int con1 = CR_EFUSE_BASE + CR_EFUSE_CON1;
    int bpkey = CR_EFUSE_BASE + CR_EFUSE_BPKEY;

    while((io_read32(con1) & EFUSE_BUSY));

    if (1 == enable)
    {
        io_write32(bpkey ,EFUSE_BPKEY);
    }
    else if (0 == enable)
    {
        io_write32(bpkey ,0);
    }
}

void efuse_reload(void)
{
    int con1 = CR_EFUSE_BASE + CR_EFUSE_CON1;

    while((io_read32(con1) & EFUSE_BUSY));
    io_write32(con1 ,EFUSE_RELD);
    while(!(io_read32(con1) & EFUSE_VLD));
}

unsigned int efuse_read_word(unsigned int word_offset)
{
    int con1 = CR_EFUSE_BASE + CR_EFUSE_CON1;
    int addr = CR_EFUSE_BASE + CR_EFUSE_DATA00 + (0x4 * word_offset);

    while(!(io_read32(con1) & EFUSE_VLD));

    return io_read32(addr);
}

void efuse_write_word(unsigned int word_offset, unsigned int value)
{
    int con1 = CR_EFUSE_BASE + CR_EFUSE_CON1;
    int addr = CR_EFUSE_BASE + CR_EFUSE_DATA00 + (0x4 * word_offset);

    while(!(io_read32(con1) & EFUSE_VLD));

    io_write32(addr, value);

    while((io_read32(con1) & EFUSE_BUSY));
}

void efuse_read_pkgid(void)
{
    unsigned char pkgid =0, remark_pkgid = 0;


    printf("pkgid: %x, remark_pkgid: %x\r\n", pkgid, remark_pkgid);
}

int efuse_write_pkgid(unsigned char id, unsigned char remark)
{

}

static unsigned int data[16] = {
    0xFF0055AA, 0x00FFAA55, 0x55AA00FF, 0x55AA00FF,
    0x55AAFF00, 0xAA5500FF, 0x00FF55AA, 0xFF00AA55,
    0x00FFAA55, 0xFF0055AA, 0x55AA00FF, 0x55AA00FF,
    0xAA5500FF, 0x55AAFF00, 0xFF00AA55, 0x00FF55AA,
};

static int efuse_write_and_compare(unsigned int mode, unsigned int data_pattern, unsigned int check_data)
{
	unsigned int i = 0;
	unsigned int read_data = 0;
	unsigned int err = 0;

	for(i = 0; i < 62; i++)
	{
        if (mode == 2)
        {
            data_pattern = data[i%16];
        }

        if (mode != 4)
            efuse_bpkey_write(1);
        efuse_write_word(i, data_pattern);
        efuse_bpkey_write(0);
	}

	efuse_reload();

	for(i = 0; i < 62; i++)
	{
	    read_data = efuse_read_word(i);
        if (mode == 2)
        {
            check_data = data[i%16];
        }

        if ( read_data != check_data)
        {
            printf("efuse_test failed: mode %d i %d read_data[%x] data_pattern[%x] check_data[%x]\r\n",
                    mode, i, read_data, data_pattern, check_data);
            err = -1;
        }
	}

	return err;
}

static int efuse_meec_write_and_compare(unsigned int mode, unsigned int data_pattern, unsigned int check_data)
{
	unsigned int i = 0;
	unsigned int read_data = 0;
	unsigned int con1 = CR_EFUSE_BASE + CR_EFUSE_CON1;
	unsigned int con2 = CR_EFUSE_BASE + CR_EFUSE_CON2;
	unsigned int err = 0;

	for(i = 54; i < 62; i++)
	{
        efuse_bpkey_write(1);
        efuse_write_word(i, (data_pattern | 0x1));
        io_write32(con2, (EFUSE_AUTOECC_ONLY));
        while((io_read32(con1) & EFUSE_BUSY));
        efuse_write_word(i, (data_pattern));
        io_write32(con2, (EFUSE_AUTOECC));
        while((io_read32(con1) & EFUSE_BUSY));
        io_write32((CR_EFUSE_BASE + CR_ECC), EFUSE_MECC);
        efuse_bpkey_write(0);
	}

	efuse_reload();

	for(i = 54; i < 62; i++)
	{
	    read_data = efuse_read_word(i);

        if ( read_data != check_data)
        {
            printf("efuse_test failed: mode %d i %d read_data[%x] data_pattern[%x] check_data[%x]\r\n",
                    mode, i, read_data, data_pattern, check_data);
            err = -1;
        }
	}

	return err;
}

static int efuse_seec_write_and_compare(unsigned int mode, unsigned int data_pattern, unsigned int check_data)
{
	unsigned int i = 0;
	unsigned int read_data = 0;
	unsigned int con1 = CR_EFUSE_BASE + CR_EFUSE_CON1;
	unsigned int con2 = CR_EFUSE_BASE + CR_EFUSE_CON2;
	unsigned int err = 0;

	for(i = 50; i < 54; i+=2)
	{

        efuse_bpkey_write(1);
        efuse_write_word(i, (data_pattern));
        io_write32(con2, (EFUSE_AUTOECC_ONLY));
        while((io_read32(con1) & EFUSE_BUSY));
        efuse_write_word(i, (data_pattern | 0x1));
        io_write32(con2, (EFUSE_AUTOECC));
        while((io_read32(con1) & EFUSE_BUSY));
        io_write32((CR_EFUSE_BASE + CR_ECC), EFUSE_SECC);
        efuse_bpkey_write(0);
	}

	efuse_reload();

	for(i = 50; i < 54; i+=2)
	{
	    read_data = efuse_read_word(i);

        if ( read_data != check_data)
        {
            printf("efuse_test failed: mode %d i %d read_data[%x] data_pattern[%x] check_data[%x]\r\n",
                    mode, i, read_data, data_pattern, check_data);
            err = -1;
        }
	}

	return err;
}

int efuse_test(unsigned int mode)
{
    unsigned int data_pattern = 0;
    unsigned int check_data = 0;
    unsigned int reset = 0;

    switch(mode)
    {
        case 1:
        {
            check_data = data_pattern = 0xFFFFFFFF;
            /* Write 0xFFFFFFFF to data and read back to check */
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            data_pattern = 0x0;
            /* Write 0 to data and read back to check */
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 2:
        {
            /* Write special pattern to data and read back to check */
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 3:
        {
            data_pattern = 0x12345678;
            check_data = 0x12345678;
            /* Write 0xFFFFFFFF to data and read back to check */
            if (efuse_meec_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }
            data_pattern = 0x12345678;
            check_data = 0x12345679;
            if (efuse_seec_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 4:
        {
            data_pattern = 0xFFFF0000;
            check_data = 0x0;
            /* Write 0xFFFFFFFF to data and read back to check */
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            data_pattern = 0x0000FFFF;
            /* Write 0 to data and read back to check */
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 5:
        {
            if (io_read32(CR_EFUSE_BASE + CR_MLOCK) != 0x1F)
            {
                efuse_bpkey_write(1);
                io_write32((CR_EFUSE_BASE + CR_MLOCK), 0x1F);
                while((io_read32(CR_EFUSE_BASE + CR_EFUSE_CON1) & EFUSE_BUSY));
                efuse_bpkey_write(0);
                reset = 1;
                printf("mode %d step 1: CR_MLOCK=0x1F\r\n", mode);
            }

            if (io_read32(CR_EFUSE_BASE + CR_CLOCK) != 0x3)
            {
                efuse_bpkey_write(1);
                io_write32((CR_EFUSE_BASE + CR_CLOCK), 0x3);
                while((io_read32(CR_EFUSE_BASE + CR_EFUSE_CON1) & EFUSE_BUSY));
                efuse_bpkey_write(0);
                reset = 1;
                printf("mode %d step 1: CR_CLOCK=0x3\r\n", mode);
            }

            if (reset == 1)
            {
                printf("mode %d step 1: reset\r\n", mode);
                io_write32(0X100050C0, 0x80000000);
            }
            data_pattern = 0xFFFFFFFF;
            check_data = 0x0;
            /* Write 0xFFFFFFFF to data and read back to check */
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 6:
        {
            if (io_read32(CR_EFUSE_BASE + CR_MLOCK) != 0x20)
            {
                efuse_bpkey_write(1);
                io_write32((CR_EFUSE_BASE + CR_MLOCK), 0x20);
                while((io_read32(CR_EFUSE_BASE + CR_EFUSE_CON1) & EFUSE_BUSY));
                efuse_bpkey_write(0);
                reset = 1;
                printf("mode %d step 1: CR_MLOCK=0x20\r\n", mode);
            }

            if (io_read32(CR_EFUSE_BASE + CR_CLOCK) != 0x8)
            {
                efuse_bpkey_write(1);
                io_write32((CR_EFUSE_BASE + CR_CLOCK), 0x8);
                while((io_read32(CR_EFUSE_BASE + CR_EFUSE_CON1) & EFUSE_BUSY));
                efuse_bpkey_write(0);
                reset = 1;
                printf("mode %d step 1: CR_CLOCK=0x8\r\n", mode);
            }

            if (reset == 1)
            {
                printf("mode %d step 1: reset\r\n", mode);
                io_write32(0X100050C0, 0x80000000);
            }

            if (io_read32(0x10005014) != mode)
            {
                efuse_bpkey_write(1);
                io_write32((CR_EFUSE_BASE + CR_MLOCK), 0x1F);
                while((io_read32(CR_EFUSE_BASE + CR_EFUSE_CON1) & EFUSE_BUSY));
                io_write32((CR_EFUSE_BASE + CR_CLOCK), 0x3);
                while((io_read32(CR_EFUSE_BASE + CR_EFUSE_CON1) & EFUSE_BUSY));
                efuse_bpkey_write(0);
                printf("mode %d step 2: CR_MLOCK=0x1F\r\n", mode);
                printf("mode %d step 2: CR_CLOCK=0x3\r\n", mode);
                printf("mode %d step 2: reset\r\n", mode);
                io_write32(0X10005014, mode);
                io_write32(0X100050C0, 0x80000000);
            }
            check_data = data_pattern = 0xFFFFFFFF;
            /* Write 0xFFFFFFFF to data and read back to check */
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 7:
        {
            if (!(io_read32(CR_EFUSE_BASE + CR_CLOCK) & 0x4))
            {
                efuse_bpkey_write(1);
                io_write32((CR_EFUSE_BASE + CR_CLOCK), 0x4);
                while((io_read32(CR_EFUSE_BASE + CR_EFUSE_CON1) & EFUSE_BUSY));
                efuse_bpkey_write(0);
                reset = 1;
                printf("mode %d step 1: CR_CLOCK=0x4\r\n", mode);
            }

            if (reset == 1)
            {
                printf("mode %d step 1: reset\r\n", mode);
                io_write32(0X100050C0, 0x80000000);
            }
            data_pattern = 0x12345678;
            check_data = 0x12345679;
            /* Write 0xFFFFFFFF to data and read back to check */
            if (efuse_meec_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }
            data_pattern = 0x12345678;
            check_data = 0x12345678;
            if (efuse_seec_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 8:
        {
            check_data = data_pattern = 0xFFFF;
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            data_pattern = 0xFFFF0000;
            check_data = 0xFFFFFFFF;
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 9:
        {
            check_data = data_pattern = 0xAAAAAAAA;
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            data_pattern = 0x55555555;
            check_data = 0xFFFFFFFF;
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;

        case 10:
        {
            check_data = data_pattern = 0x55555555;
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            data_pattern = 0xAAAAAAAA;
            check_data = 0xFFFFFFFF;
            if (efuse_write_and_compare(mode, data_pattern, check_data))
            {
                return -1;
            }

            printf("efuse_test success: mode %d\r\n", mode);
        }
        break;
    }

    return 0;
}

void efuse_init(void)
{
    uint32_t retry = 0;
    while((retry < MAX_RETRIES) && (0 == (io_read32(CR_EFUSE_BASE + CR_EFUSE_CON1) & EFUSE_VLD)))
    {
        ++retry;
    }

    /* TODO: how to handle error? if error_no is returned? */
    if (retry == MAX_RETRIES)
        printf("efuse init fail\n");

    return;
}

uint8_t is_efuse_empty(void)
{
    return 0 == get_sku_id();
}
