/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "timer.h"
#include "queue.h"

/* Utils includes. */
#include "cmd_interpreter.h"
#include "dsh_parser.h"
#ifdef AIR_MW_SUPPORT
#include "mw_cmd_parser.h"
#include "mw_utils.h"
#include "lwip/apps/sntp.h"
#endif
#include "spinorwrite.h"
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include <lwip/ip.h>
#include "lwip/dhcp.h"
#ifdef AIR_SUPPORT_IPV6
#include "lwip/priv/nd6_priv.h"
#endif
#include "ip4_addr_util.h"


#if LWIP_DNS
#include "lwip/dns.h"
#include "lwip/prot/dns.h"
#include "lwip/api.h"
#endif /* LWIP_DNS */

#include "lwip/apps/tftp_client.h"

#define CMD_FIX_BACKSPACE_FOR_UI

typedef void(*uart_input_callback)(char c);
extern int outbyte(int c);
extern void uart_register_input_cb(uart_input_callback cb);
#ifdef AIR_MW_SUPPORT
extern int g_local_timezone;
#endif

#define CMD_SH_MAX_INPUT        (256)
#define CMD_SH_MAX_OUTPUT       (256)
#define CMD_SH_ERR(fmt, ...) do             \
    {                                       \
        printf("[FAIL]"fmt, ##__VA_ARGS__); \
    }while(0)

#if LWIP_DNS
#define DNS_LOCAL_HOSTLIST_MAX_NAMELEN (3)
#define NUMBEROF_DNS_ANS_IPADDRS (5)
#endif /* LWIP_DNS */

#define IPV4_STR_SIZE        (16)
#define IPV6_STR_SIZE        (40)
#ifdef AIR_SUPPORT_IPV6
#define IPV6_STATE_SIZE      (8)
#endif
#define IPV4_TO_STR(__buf__,__ipv4__)    \
                        sprintf(__buf__, "%d.%d.%d.%d",   \
                        ((__ipv4__)&0xFF000000)>>24,((__ipv4__)&0x00FF0000)>>16,    \
                        ((__ipv4__)&0x0000FF00)>>8, ((__ipv4__)&0x000000FF))


GDMPSRAM_DATA QueueHandle_t g_cmd_queue = NULL;
GDMPSRAM_DATA CMD_PRIVILEGE_MODE_TYPE_T g_privilege_mode = CMD_PRIVILEGE_MODE_EXEC;

typedef enum
{
    CMD_STATE_IDLE = 0,
    CMD_STATE_KEYING,
} CMD_STATE_T;

CMD_STATE_T g_cmd_state = CMD_STATE_IDLE;

static portBASE_TYPE task_stats_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE run_time_stats_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE proc_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE port_stats_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE memory_read_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE memory_write_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE memory_dump_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE cpu_reg_dump_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE kill_wdog_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE show_interrupts_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE xmodem_rcv_cmd(signed char *buf, size_t len, const signed char * input);
#if LWIP_UDP
static portBASE_TYPE tftp_get_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE tftp_put_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE upgrade_cmd(signed char *buf, size_t len, const signed char * input);
#endif

static portBASE_TYPE pdma_init_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE pdma_debug_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE lwip_stats_cmd(signed char *buf, size_t len, const signed char * input);
//static portBASE_TYPE httpclient_start_cmd(signed char *buf, size_t len, const signed char * input);
//static portBASE_TYPE sshd_cmd(signed char *buf, size_t len, const signed char * input);
#ifdef AIR_WEBSOCKET_DEMO
static portBASE_TYPE httpclient_ws_cmd(signed char *buf, size_t len, const signed char * input);
#endif

/* for en8851 sdk build in freeRTOS */
/*SDK session start*/
static portBASE_TYPE diag_shell_cmd(signed char *buf, size_t len, const signed char * input);
/*SDK session end*/
static portBASE_TYPE peripheral_shell_cmd(signed char *buf, size_t len, const signed char * input);
#ifdef AIR_MW_SUPPORT
static portBASE_TYPE mw_shell_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE sysmac_set_cmd(signed char *buf, size_t len, const signed char * input);
#endif
static portBASE_TYPE timer_dump_cmd(signed char *buf, size_t len, const signed char * input);

static portBASE_TYPE debug_shell_cmd(signed char *buf, size_t len, const signed char * input);
#if LWIP_UDP
static portBASE_TYPE tftp_fw_upgrade_cmd(signed char *buf, size_t len, const signed char * input);
#endif
static portBASE_TYPE reset_cmd(signed char *buf, size_t len, const signed char * input);

static portBASE_TYPE ip_get_cmd(signed char *buf, size_t len, const signed char * input);

#ifdef AIR_SUPPORT_SNTP
static portBASE_TYPE sntp_off_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE sntp_on_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE sntp_get_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE sntp_set_cmd(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE localtime_get(signed char *buf, size_t len, const signed char * input);
static portBASE_TYPE localtime_set(signed char *buf, size_t len, const signed char * input);
#endif

static portBASE_TYPE
_ip_set_cmd(
    signed char *ptr_buf,
    size_t len,
    const signed char *ptr_input);

#if LWIP_DNS
static portBASE_TYPE
_dns_cmd(signed char *buf,
    size_t len,
    const signed char *input);
#endif /* LWIP_DNS */


static const CMD_LINE_INPUT_T sysCmds[] = {
    {"task-stats",      "task-stats: Displays a table showing the state of each FreeRTOS task\r\n",                             task_stats_cmd,       0},
    {"run-time-stats",  "run-time-stats: Displays a table showing how much processing time each FreeRTOS task has used\r\n",    run_time_stats_cmd,   0},
    {"sdk",             "sdk: enter sdk diag shell mode\r\n",                                                                   diag_shell_cmd,       0},
    {"peripheral",      "peripheral: enter peripheral shell mode\r\n",                                                          peripheral_shell_cmd, 0},
#ifdef AIR_MW_SUPPORT
    {"mw",              "mw: enter magic wand debug shell mode\r\n",                                                            mw_shell_cmd,         0},
#endif
    {"debug",           "debug: enter debug shell mode\r\n",                                                                    debug_shell_cmd,      0},
#if LWIP_UDP
    {"tftp-fw-upgrade", "tftp-fw-upgrade <server-ip> <firmware-name>: upgrade image using TFTP\r\n",                            tftp_fw_upgrade_cmd,  0},
#endif
    {"reset",           "reset: reset system\r\n",                                                                              reset_cmd,            0},
    {NULL,              NULL,                                                                                                   NULL,                 0}
};

static const CMD_LINE_INPUT_T debugCmds[] = {
    {"echo", "echo: compatiable sif read/write command\r\n", proc_cmd, 0},
    {"port-stats", "port-stats <port>: display port number\r\n", port_stats_cmd, 0},
    {"memrl", "memrl address: read 4 bytes from register or memory at the address\r\n", memory_read_cmd, 1},
    {"memwl", "memwl address value: write 4-byte value to register or memory at the address\r\n", memory_write_cmd, 2},
    {"memory", "memory address len: dump registers or memory for range (address)~(address+len)\r\n", memory_dump_cmd, 2},
    {"cpu_reg_dump", "cpu_reg_dump: dump CPU registers for debug\r\n", cpu_reg_dump_cmd, 0},
    {"kill_wdog", "kill_wdog: kill watchdog\r\n", kill_wdog_cmd, 0},
    {"show_interrupts", "show_interrupts: show interrupts' counters\r\n", show_interrupts_cmd, 0},
#ifndef SRAM_SHRINK___XMODEM_CMD_REMOVE
    {"xmodem_rcv", "xmodem_rcv [dst_addr]: recv xmodem data to specify addr\r\n", xmodem_rcv_cmd, 0},
#endif
#if LWIP_UDP
    {"tftp_get", "tftp_get <ip> <name> [dst_addr]: tftp get file\r\n", tftp_get_cmd, 0},
    {"tftp_put", "tftp_put <ip> <name> <src addr> <src data size>: tftp put file\r\n", tftp_put_cmd, 4},
    {"upgrade", "upgrade <type> <mode> [ip] [name]: upgrade image\r\n", upgrade_cmd, 0},
#endif

    /* PDMA test cmds */
    {"pdma_init", "pdma_init: init pdma driver\r\n", pdma_init_cmd, 0},
    {"pdma_debug", "pdma_debug: enable/disable pdma debug\r\n", pdma_debug_cmd, 0},
    {"lwip_stats", "lwip_stats: display lwip statistics\r\n", lwip_stats_cmd, 0},
    //{"httpclient_start", "httpclient_start: start httpclient\r\n", httpclient_start_cmd, 0},
    //{"sshd", "sshd: start sshd\r\n", sshd_cmd, 0},
#ifdef AIR_WEBSOCKET_DEMO
    {"wsclient", "wsclient <url>: start websocket client connect to url\r\n", httpclient_ws_cmd, 0},
#endif

    /* IP cmds */
    {"ip_get" ,  "ip_get: Show IP address\r\n", ip_get_cmd, 0},
    {"ip_set" ,  "ip_set: Set IP config. Write ip, netmask, gw and dns server in dot-deciaml notation\r\n", _ip_set_cmd, 0},
#ifdef AIR_MW_SUPPORT
    /* Set system mac */
    {"sysmac_set" ,  "sysmac_set <mac_addr>\r\n", sysmac_set_cmd, 0},
#endif
    {"timer-dump" ,  "timer-dump <threshold>\r\n", timer_dump_cmd, 0},
#if LWIP_DNS
    {"dnsquery" ,  "dnsquery [-4, -6] <hostname> : resolve hostname to IP address.\r\n", _dns_cmd, 0},
#endif /* LWIP_DNS */
#ifdef AIR_SUPPORT_SNTP
    /*SNTP test time cmds*/
    {"sntp_on" , "sntp_on : Start sntp client\r\n", sntp_on_cmd, 0},
    {"sntp_off" , "sntp_off : Stop sntp client\r\n", sntp_off_cmd, 0},
    {"sntp_set", "sntp_set <idx> <IP>.<IP>.<IP>.<IP> <timezone>: Set sntp client info\r\n", sntp_set_cmd, 0},
    {"sntp_get", "sntp_get <idx>: Get sntp client info\r\n", sntp_get_cmd, 0},
    {"localtime_set", "localtime_set <yy>:<mm>:<dd> <hh>:<mm>:<ss> : Set localtime info\r\n", localtime_set, 0},
    {"localtime_get", "localtime_get: Get localtime info\r\n", localtime_get, 0},
#endif
    {NULL, NULL, NULL, 0}
};

static void cmd_prompt(void)
{
    switch(g_privilege_mode)
    {
        case CMD_PRIVILEGE_MODE_EXEC:
            printf("\r\n#");
            break;
        case CMD_PRIVILEGE_MODE_SDK:
            printf("\r\nsdk#");
            break;
        case CMD_PRIVILEGE_MODE_PERIPHERAL:
            printf("\r\nperipheral#");
            break;
#ifdef AIR_MW_SUPPORT
        case CMD_PRIVILEGE_MODE_MW:
            printf("\r\nmw#");
            break;
#endif
        case CMD_PRIVILEGE_MODE_DEBUG:
            printf("\r\ndebug#");
            break;
        default:
            printf("\r\n#");
            break;
    }
}

static portBASE_TYPE task_stats_cmd(signed char *buf, size_t len, const signed char * input)
{
    vTaskList();

    xPortModuleMallocDumpInfo();

    printf("\ntotal heap size : %7d Bytes\n\n", configTOTAL_HEAP_SIZE);
    printf("heap used size  : %7d Bytes\n",
        xPortGetHeapUsableBytes() - xPortGetFreeHeapSize());

    printf("free heap size  : %7d Bytes (%d %%)\n",
        xPortGetFreeHeapSize(), (xPortGetFreeHeapSize()*100)/xPortGetHeapUsableBytes());
    printf("free heap size  : %7d Bytes (%d %%, Minimum ever)\n",
        xPortGetMinimumEverFreeHeapSize(),
        (xPortGetMinimumEverFreeHeapSize()*100)/xPortGetHeapUsableBytes());

#ifdef NON_FIXED_HEAP_SIZE_DBG
    extern uint32_t ucHeap_malloc_cnt(void);
    extern uint32_t ucHeap_free_cnt(void);
    printf("     malloc cnt : %7d\n", ucHeap_malloc_cnt());
    printf("       free cnt : %7d\n", ucHeap_free_cnt());
#endif

    return pdFALSE;
}

uint8_t bMonitorCpuLoading = 0;
static portBASE_TYPE run_time_stats_cmd(signed char *buf, size_t len, const signed char * input)
{
    char cmd[32];
    int tmp = 0;
    sscanf(input, "%s %d", cmd, &tmp);
    bMonitorCpuLoading = tmp;

    airTaskGetRunTimeStats(1);

    return pdFALSE;
}

static portBASE_TYPE port_stats_cmd(signed char *buf, size_t len, const signed char * input)
{
    char cmd[32];
    int port = -1;

    sscanf(input, "%s %d", cmd, &port);
    if ((port < 0) || (port > 28))
    {
        printf("Error: Invalid port number!\r\n");
        return pdFALSE;
    }

    printf("Port%d:\r\n", port);
    printf("Tx Drop Packet      : %d\r\n", io_read32((0x10214000 + 0x200 * port)));
    printf("Tx CRC Error        : %d\r\n", io_read32((0x10214004 + 0x200 * port)));
    printf("Tx Unicast Packet   : %d\r\n", io_read32((0x10214008 + 0x200 * port)));
    printf("Tx Multicast Packet : %d\r\n", io_read32((0x1021400c + 0x200 * port)));
    printf("Tx Broadcast Packet : %d\r\n", io_read32((0x10214010 + 0x200 * port)));
    printf("Tx Collision Event  : %d\r\n", io_read32((0x10214014 + 0x200 * port)));
    printf("Tx Pause Packet     : %d\r\n", io_read32((0x1021402c + 0x200 * port)));
    printf("Tx 64    Packet     : %d\r\n", io_read32((0x10214030 + 0x200 * port)));
    printf("Tx 65-   Packet     : %d\r\n", io_read32((0x10214034 + 0x200 * port)));
    printf("Tx 128-  Packet     : %d\r\n", io_read32((0x10214038 + 0x200 * port)));
    printf("Tx 256-  Packet     : %d\r\n", io_read32((0x1021403c + 0x200 * port)));
    printf("Tx 512-  Packet     : %d\r\n", io_read32((0x10214040 + 0x200 * port)));
    printf("Tx 1024- Packet     : %d\r\n", io_read32((0x10214044 + 0x200 * port)));
    printf("Tx 1519- Packet     : %d\r\n", io_read32((0x10214048 + 0x200 * port)));
    printf("Rx Drop Packet      : %d\r\n", io_read32((0x10214080 + 0x200 * port)));
    printf("Rx Filtering Packet : %d\r\n", io_read32((0x10214084 + 0x200 * port)));
    printf("Rx Unicast Packet   : %d\r\n", io_read32((0x10214088 + 0x200 * port)));
    printf("Rx Multicast Packet : %d\r\n", io_read32((0x1021408c + 0x200 * port)));
    printf("Rx Broadcast Packet : %d\r\n", io_read32((0x10214090 + 0x200 * port)));
    printf("Rx Alignment Error  : %d\r\n", io_read32((0x10214094 + 0x200 * port)));
    printf("Rx CRC Error        : %d\r\n", io_read32((0x10214098 + 0x200 * port)));
    printf("Rx Undersize Error  : %d\r\n", io_read32((0x1021409c + 0x200 * port)));
    printf("Rx Fragment Error   : %d\r\n", io_read32((0x102140a0 + 0x200 * port)));
    printf("Rx Oversize Error   : %d\r\n", io_read32((0x102140a4 + 0x200 * port)));
    printf("Rx Jabber Error     : %d\r\n", io_read32((0x102140a8 + 0x200 * port)));
    printf("Rx Pause Packet     : %d\r\n", io_read32((0x102140ac + 0x200 * port)));
    printf("Rx 64    Packet     : %d\r\n", io_read32((0x102140B0 + 0x200 * port)));
    printf("Rx 65-   Packet     : %d\r\n", io_read32((0x102140B4 + 0x200 * port)));
    printf("Rx 128-  Packet     : %d\r\n", io_read32((0x102140B8 + 0x200 * port)));
    printf("Rx 256-  Packet     : %d\r\n", io_read32((0x102140Bc + 0x200 * port)));
    printf("Rx 512-  Packet     : %d\r\n", io_read32((0x102140C0 + 0x200 * port)));
    printf("Rx 1024- Packet     : %d\r\n", io_read32((0x102140C4 + 0x200 * port)));
    printf("Rx 1519- Packet     : %d\r\n", io_read32((0x102140C8 + 0x200 * port)));
    return pdFALSE;
}

static portBASE_TYPE proc_cmd(signed char *buf, size_t len, const signed char * input)
{
    char cmd[32];
    char str[32];
    unsigned int addr = 0;
    unsigned int val = 0;
    int ret;

    ret = sscanf(input, "%s %x %x %s", cmd, &addr, &val, str) ;
    if (ret == 2)
    {
        ret = sscanf(input, "%s %x %s", cmd, &addr, str);
        if (ret == 3)
        {
            //printf("cmd %s, proc %s, addr 0x%x\n", cmd, str, addr);
        }
        if(isDmemAddr(addr) || isRegAddr(addr))
        {
            val = io_read32(addr);
        }
        else
        {
            errAddrInfo(addr);
            return pdFALSE;
        }
        printf("reg %x: 0x%x\r\n", addr, val);
    }
    else if (ret == 4)
    {
        //printf("cmd %s, proc %s, addr 0x%x, val 0x%x\n", cmd, str, addr, val);
        if(isDmemAddr(addr) || isRegAddr(addr))
        {
            io_write32(addr, val);
        }
        else
        {
            errAddrInfo(addr);
            return pdFALSE;
        }
        printf("reg %x: 0x%x\r\n", addr, val);
    }

    return pdFALSE;
}

static portBASE_TYPE memory_read_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32];
    unsigned int addr;

    sscanf(input, "%s %x", str, &addr) ;

    doSysMemrl(addr);

    return pdFALSE;
}

static portBASE_TYPE memory_write_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32];
    unsigned int addr, val;

    sscanf(input, "%s %x %x", str, &addr, &val) ;

    doSysMemwl(addr, val);

    return pdFALSE;
}

static portBASE_TYPE memory_dump_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32];
    unsigned int addr, length;

    sscanf(input, "%s %x %x", str, &addr, &length) ;

    doSysMemory(addr, length);

    return pdFALSE;
}

static portBASE_TYPE cpu_reg_dump_cmd(signed char *buf, size_t len, const signed char * input)
{
    cpu_reg_dump();

    return pdFALSE;
}

static portBASE_TYPE kill_wdog_cmd(signed char *buf, size_t len, const signed char * input)
{
    wdog_kill();

    return pdFALSE;
}

static portBASE_TYPE show_interrupts_cmd(signed char *buf, size_t len, const signed char * input)
{
    show_interrupts();

    return pdFALSE;
}

static portBASE_TYPE pdma_init_cmd(signed char *buf, size_t len, const signed char * input)
{
    pdma_init();

    return pdFALSE;
}

static portBASE_TYPE diag_shell_cmd(signed char *buf, size_t len, const signed char * input)
{
    g_privilege_mode = CMD_PRIVILEGE_MODE_SDK;
    return pdFALSE;
}

static portBASE_TYPE peripheral_shell_cmd(signed char *buf, size_t len, const signed char * input)
{
    g_privilege_mode = CMD_PRIVILEGE_MODE_PERIPHERAL;
    return pdFALSE;
}
#ifdef AIR_MW_SUPPORT
static portBASE_TYPE mw_shell_cmd(signed char *buf, size_t len, const signed char * input)
{
    g_privilege_mode = CMD_PRIVILEGE_MODE_MW;
    return pdFALSE;
}
#endif

static portBASE_TYPE debug_shell_cmd(signed char *buf, size_t len, const signed char * input)
{
    g_privilege_mode = CMD_PRIVILEGE_MODE_DEBUG;
    return pdFALSE;
}

static portBASE_TYPE reset_cmd(signed char *buf, size_t len, const signed char * input)
{
    air_chipscu_resetSystem(0);
    return pdFALSE;
}

/*SDK session end*/

static portBASE_TYPE pdma_debug_cmd(signed char *buf, size_t len, const signed char * input)
{
    pdma_debug(input);

    return pdFALSE;
}

#ifndef SRAM_SHRINK___XMODEM_CMD_REMOVE
static portBASE_TYPE xmodem_rcv_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32];
    unsigned int addr = ~0UL;
    int recvSize = 0;

    sscanf(input, "%s %x", str, &addr);

    //disable UART isr
    io_write32(0x1000a004, 0);

    recvSize = XModemReceive(addr);

    //enable UART isr
    io_write32(0x1000a004, 1);

    printf("\nReceived %d bytes!\n", recvSize);

    return pdFALSE;
}
#endif

#if LWIP_UDP
static portBASE_TYPE tftp_get_cmd(signed char *buf, size_t len, const signed char * input)
{
    char *str = NULL;
    char *ip = NULL;
    char *name = NULL;
    char *tmp = NULL;
    char *save = NULL;
    unsigned int addr = ~0UL;
    int recvSize = 0;
    int ret;

    str = strtok_r(input, " ", &save);
    ip = strtok_r(NULL, " ", &save);
    name = strtok_r(NULL, " ", &save);
    tmp = strtok_r(NULL, " ", &save);

    if ((ip != NULL) && (name != NULL))
    {
        if (tmp != NULL)
            sscanf(tmp, "%x", &addr);

        ret = tftp_get_file(ip, name, addr, &recvSize);
        printf("\nReceived %d bytes!\n", recvSize);
        if(ret)
            CMD_SH_ERR("tftp get fail, rc = %d\n", ret);
        else
            printf("tftp get success!\n");
    }
    else
    {
        CMD_SH_ERR("Illegal argument!\r\n");
    }

    return pdFALSE;
}

static portBASE_TYPE upgrade_cmd(signed char *buf, size_t len, const signed char * input)
{
    char *str = NULL;
    char *type = NULL;
    char *mode = NULL;
    char *ip = NULL;
    char *name = NULL;
    char *save = NULL;
    unsigned int addr = ~0UL;
    int recvSize = 0;
    int ret;

    str = strtok_r(input, " ", &save);
    type = strtok_r(NULL, " ", &save);
    mode = strtok_r(NULL, " ", &save);

    if ((type != NULL) && (mode != NULL))
    {
        if (strncmp(type, "firmware", 8) == 0)
        {
#ifndef SRAM_SHRINK___XMODEM_CMD_REMOVE
            if (strncmp(mode, "xmdm", 4) == 0)
            {
                //disable UART isr
                io_write32(0x1000a004, 0);

                recvSize = XModemReceive(addr);

                //enable UART isr
                io_write32(0x1000a004, 1);
            }
            else
#endif
            if (strncmp(mode, "tftp", 4) == 0)
            {
                ip = strtok_r(NULL, " ", &save);
                name = strtok_r(NULL, " ", &save);

                if ((ip != NULL) && (name != NULL))
                {
                    ret = tftp_get_file(ip, name, addr, &recvSize);
                }
                else
                {
                    CMD_SH_ERR("Illegal argument!\r\n");
                    return pdFALSE;
                }
            }
            else
            {
                CMD_SH_ERR("Illegal mode!\r\n");
                return pdFALSE;
            }

            printf("\nReceived %d bytes!\n", recvSize);

            /* Chechk image CRC and prepare fw upgrade */
            if (crc_check((unsigned char *) TempSystemBase) == 0 && ret == TFTP_CLIENT_SUCCESS)
            {
                update_upgrade_flag(1);
            }
            else
            {
                CMD_SH_ERR("tftp upgrade image fail, rc = %d\r\n", ret);
            }
        }
        else
        {
            CMD_SH_ERR("Illegal type!\r\n");
        }
    }
    else
    {
        CMD_SH_ERR("Illegal argument!\r\n");
    }

    return pdFALSE;
}
#endif

#ifdef AIR_WEBSOCKET_DEMO
extern void HttpWS_TestTask( void *pvParameters );
static portBASE_TYPE httpclient_ws_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32] = { 0 }, addr[50] = { 0 };

    sscanf((char*)input, "%s %s", str, addr);

    xTaskCreate(HttpWS_TestTask,
            "WS",
            2048,
            addr,
            (tskIDLE_PRIORITY + 5),
            NULL );

    return pdFALSE;
}
#endif

#if LWIP_UDP
static portBASE_TYPE tftp_fw_upgrade_cmd(signed char *buf, size_t len, const signed char * input)
{
    char *str = NULL;
    char *ip = NULL;
    char *name = NULL;
    char *save = NULL;
    unsigned int addr = ~0UL;
    int recvSize = 0;
    int ret;

    str = strtok_r(input, " ", &save);
    ip = strtok_r(NULL, " ", &save);
    name = strtok_r(NULL, " ", &save);

    if ((ip != NULL) && (name != NULL))
    {
        ret = tftp_get_file(ip, name, addr, &recvSize);
        printf("\nReceived %d bytes!\n", recvSize);

        /* Chechk image CRC and prepare fw upgrade */
        if (crc_check((unsigned char *) TempSystemBase) == 0 && ret == TFTP_CLIENT_SUCCESS)
        {
            update_upgrade_flag(1);
        }
        else
        {
            CMD_SH_ERR("tftp fw upgrade failed, rc = %d\n", ret);
        }
    }
    else
    {
        CMD_SH_ERR("Illegal argument!\r\n");
    }

    return pdFALSE;
}
#endif

static portBASE_TYPE lwip_stats_cmd(signed char *buf, size_t len, const signed char * input)
{
    stats_display();

    printf("[MemTrace]: xMinimumEverFreeBytesRemaining %d\n", xPortGetMinimumEverFreeHeapSize());
    printf("[MemTrace]: xFreeBytesRemaining    %d\n", xPortGetFreeHeapSize());

    return pdFALSE;
}

static portBASE_TYPE ip_get_cmd(signed char *buf, size_t len, const signed char * input)
{
    char   ip_str[IPV4_STR_SIZE];
#ifdef AIR_SUPPORT_IPV6
    char ipv6_state[IPV6_STATE_SIZE];
    u8_t i = 0, addr_state;
    u8_t glb_ipv6_exist = 0;
#endif
    struct netif *xNetIf = NULL;
#if LWIP_DNS
    ip_addr_t *ptr_dns = dns_getserver(0);
#endif /* LWIP_DNS */

    NETIF_FOREACH(xNetIf)
    {
        memset(ip_str, 0, IPV4_STR_SIZE);
        printf("\n  Interface Name: %c%c%d\n", xNetIf->name[0], xNetIf->name[1], xNetIf->num);
        IPV4_TO_STR(ip_str, PP_HTONL(ip_addr_get_ip4_u32(&xNetIf->ip_addr)));
        printf("  IPv4 Address  : %s\n", ip_str);
#ifdef AIR_SUPPORT_IPV6
        printf("  Link-local IPv6 Address  : %s/64 (Auto)\n", (UI8_T *)ipaddr_ntoa((const ip_addr_t *)netif_ip6_addr(xNetIf, 0)));
        printf("                             (preferred lifetime: forever; valid lifetime: forever)\n");
        for (i = 1; i < LWIP_IPV6_NUM_ADDRESSES; ++i)
        {
            if (0 == ip_addr_isany(&(xNetIf->ip6_addr[i])))
            {
                addr_state = netif_ip6_addr_state(xNetIf, i);
                if (0 != ip6_addr_isvalid(addr_state))
                {
                    memcpy(ipv6_state, IP6_STATE_AUTO, sizeof(IP6_STATE_AUTO));
                }
                else
                {
                    memcpy(ipv6_state, IP6_STATE_INVAILD, sizeof(IP6_STATE_INVAILD));
                }
                if (0 == glb_ipv6_exist)
                {
                    printf("  Global IPv6 Address(es)  : %s/64 (%s)\n", (UI8_T *)ipaddr_ntoa((const ip_addr_t *)netif_ip6_addr(xNetIf, i)), ipv6_state);
#if LWIP_IPV6_ADDRESS_LIFETIMES
                    printf("                             (preferred lifetime: %dsec; valid lifetime: %dsec)\n",
                           netif_ip6_addr_pref_life(xNetIf, i), netif_ip6_addr_valid_life(xNetIf, i));
#endif
                    glb_ipv6_exist = 1;
                }
                else
                {
                    printf("                             %s/64 (%s)\n", (UI8_T *)ipaddr_ntoa((const ip_addr_t *)netif_ip6_addr(xNetIf, i)), ipv6_state);
#if LWIP_IPV6_ADDRESS_LIFETIMES
                    printf("                             (preferred lifetime: %dsec; valid lifetime: %dsec)\n",
                           netif_ip6_addr_pref_life(xNetIf, i), netif_ip6_addr_valid_life(xNetIf, i));
#endif
                }
            }
        }
        if (0 == glb_ipv6_exist)
        {
            printf("  Global IPv6 Address(es)  : None\n");
        }
#endif
        IPV4_TO_STR(ip_str, PP_HTONL(ip_addr_get_ip4_u32(&xNetIf->netmask)));
        printf("  Net Mask      : %s\n", ip_str);
        IPV4_TO_STR(ip_str, PP_HTONL(ip_addr_get_ip4_u32(&xNetIf->gw)));
        printf("  Gateway       : ");
#ifdef AIR_SUPPORT_IPV6
        i = nd6_select_router(NULL, xNetIf);
        if ((i >= 0) && (i < LWIP_ND6_NUM_ROUTERS) && (NULL != default_router_list[i].neighbor_entry))
        {
            printf("%s\n", (UI8_T *)ip6addr_ntoa(ip_2_ip6((const ip_addr_t *)&(default_router_list[i].neighbor_entry->next_hop_address))));
            printf("                  %s\n", ip_str);
        }
        else
#endif
        {
            printf("%s\n", ip_str);
        }
#if LWIP_DNS
        IPV4_TO_STR(ip_str, PP_HTONL(ip_addr_get_ip4_u32(ptr_dns)));
        printf("  Dns           : %s\n", ip_str);
#endif /* LWIP_DNS */
    }

    return pdFALSE;
}


static BOOL_T
_cmd_ip4_addr_isbroadcast(
    ip4_addr_t *ptr_ip,
    ip4_addr_t *ptr_netmask)
{
    u32_t addr;

    if ((NULL == ptr_ip) || (NULL == ptr_netmask))
    {
        return FALSE;
    }

    addr = ip4_addr_get_u32(ptr_ip);

    /* all ones (broadcast) or all zeroes (old skool broadcast) */
    if ((~addr == IPADDR_ANY) || (addr == IPADDR_ANY))
    {
        return TRUE;
    }
    else if ((addr & (~ip4_addr_get_u32(ptr_netmask))) ==
             (IPADDR_BROADCAST & (~ip4_addr_get_u32(ptr_netmask))))
    {
        /* => network broadcast address */
        return TRUE;
    }

    return FALSE;
}

#if LWIP_UDP
static portBASE_TYPE tftp_put_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32];
    char ipaddr[32];
    char fname[32];
    unsigned int src_addr;
    unsigned int src_data_size;
    int ret;
    sscanf(input, "%s %s %s %x %d", str, ipaddr, fname, &src_addr, &src_data_size);
    ret = tftp_put_file(ipaddr, fname, (unsigned char *)src_addr, src_data_size);
    printf("\n");
    if(ret == TFTP_CLIENT_SUCCESS)
        printf("tftp put file success\n", ret);
    else
        CMD_SH_ERR("tftp put file fail, rc = %d\n", ret);
    return FALSE;
}
#endif

static portBASE_TYPE
_ip_set_cmd(
    signed char *ptr_buf,
    size_t len,
    const signed char *ptr_input)
{
    char *ptr_str = NULL, *ptr_ip = NULL, *ptr_netmask = NULL, *ptr_gw = NULL, *save = NULL;
    ip4_addr_t ip, netmask, gw;
#if LWIP_DNS
    char *ptr_dns = NULL;
    ip_addr_t dns;
#endif /*LWIP_DNS*/
    struct netif *ptr_netif = netif_find_default();

    if ((NULL == ptr_input) || (NULL == ptr_netif))
    {
        return pdFAIL;
    }

    ptr_str = strtok_r(ptr_input, " ", &save);
    ptr_ip = strtok_r(NULL, " ", &save);
    ptr_netmask = strtok_r(NULL, " ", &save);
    ptr_gw = strtok_r(NULL, " ", &save);
#if LWIP_DNS
    ptr_dns = strtok_r(NULL, " ", &save);
#endif

    if (((NULL == ptr_ip) || (0 == ip4addr_aton(ptr_ip, &ip))) ||
        ((NULL == ptr_netmask) || (0 == ip4addr_aton(ptr_netmask, &netmask))) ||
        ((NULL == ptr_gw) || (0 == ip4addr_aton(ptr_gw, &gw)))
#if LWIP_DNS
         || ((NULL == ptr_dns) || (0 == ipaddr_aton(ptr_dns, &dns)))
#endif
       )
    {
        if (NULL != ptr_ip)
        {
            printf("Addresses are not all in dot-deciaml notation.\n");
        }
        return pdFAIL;
    }

    if (FALSE == ip4_addr_netmask_isValid(&netmask))
    {
        printf("Invalid netmask.\n");
        return pdFAIL;
    }

    if (FALSE == ip4_addr_ipAddr_isValid(&ip, &netmask))
    {
        printf("Invalid IP address.\n");
        return pdFAIL;
    }

    if (FALSE == ip4_addr_gateway_isValid(&gw, &netmask))
    {
        printf("Invalid gateway address.\n");
        return pdFAIL;
    }

#if LWIP_DNS
    if (FALSE == ip4_addr_dnsSrvAddr_isValid(&dns))
    {
        printf("Invalid DNS server address.\n");
        return pdFAIL;
    }
#endif /*LWIP_DNS*/

#if LWIP_DNS
    dns_setserver(0, &dns);
#endif /*LWIP_DNS*/
    netif_set_addr(ptr_netif, &ip, &netmask, &gw);

    printf("ip_set OK\n");
    return pdTRUE;
}

#ifdef AIR_MW_SUPPORT
static portBASE_TYPE sysmac_set_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32], str_mac[32], str_tmp[3];
    char *ptr_str = str_mac;
    unsigned char mac[6];
    unsigned char i = 0;

    sscanf((char*)input, "%s %s", str, str_mac);

    if (strlen(str_mac) != 12)
    {
        printf("Invalid length of MAC.\n");
        return pdFALSE;
    }
    printf("\nSet new MAC =");
    for (i = 0; i < 6; i++ )
    {
        strncpy(str_tmp, ptr_str, 2);
        ptr_str+=2;
        mac[i] = strtoul(str_tmp, NULL, 16);
        printf(" %02X", mac[i]);
    }
    printf("\n");
    update_mac_addr(mac);

    return pdFALSE;
}
#endif

extern void timer_dump_dbg(unsigned int threshold);
static portBASE_TYPE timer_dump_cmd(signed char *buf, size_t len, const signed char * input)
{
    char cmd[32];
    int tmp = 0;
    sscanf(input, "%s %d", cmd, &tmp);

    timer_dump_dbg(tmp);

    return pdFALSE;
}

#if LWIP_DNS
static portBASE_TYPE
_dns_cmd(signed char *buf,
    size_t len,
    const signed char *input)
{
    err_t err;
    char   ip_str[IPV6_STR_SIZE];
    char cmd[32], str_hostname[DNS_LOCAL_HOSTLIST_MAX_NAMELEN];
    uint8_t space_count = 0;
    char addr_type[8];
    uint8_t addr_type_u8 = 0;
    uint8_t i = 0;
    uint8_t is_ipv6 = 0;
    uint8_t is_url = 0;
    uint8_t is_ipv6_url = 0;
    ip_addr_t addr;
    ip_addr_t addr_arry[NUMBEROF_DNS_ANS_IPADDRS] = {0};
    signed char *input_scan = input;
    u8_t addr_num = NUMBEROF_DNS_ANS_IPADDRS;

    while (0 != *input_scan)
    {
        if (' ' == *input_scan)
        {
            space_count++;
        }
        else if('-' == *input_scan)
        {
            addr_type_u8 = (input_scan[1] - '0');
        }
        else if(':' == *input_scan)
        {
            is_ipv6 = 1;
        }
        else if('9' > *input_scan)
        {
            is_url = 1;
        }
        input_scan++;
    }

    if((2 < space_count) || (0 == space_count))
    {
        printf("Invalid argument.\n");
        return pdFAIL;
    }

    if (NULL != strstr(input, "ipv6"))
    {
#if !LWIP_IPV6
            printf("IPV6 not support.\n");
            return pdFAIL;
#endif /* LWIP_IPV6 */
        if (4 == addr_type_u8)
        {
            printf("Invalid argument.\n");
            return pdFAIL;
        }
            is_ipv6_url = 1;
    }

    if (0 == addr_type_u8)
    {
        sscanf((char *)input, "%s %s", cmd, str_hostname);

        if (1 == is_ipv6)
        {
#if !LWIP_IPV6
            printf("IPV6 not support.\n");
            return pdFAIL;
#endif /* LWIP_IPV6 */
            printf("%s\n", str_hostname);
            return pdFAIL;
        }

        if (0 == is_url)
        {
            printf("%s\n", str_hostname);
            return pdFAIL;
        }

        if("" == str_hostname)
        {
            printf("Invalid argument.\n");
            return pdFAIL;
        }

        if (1 == is_ipv6_url)
        {
#if LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR
            err = netconn_gethostbyname_addrtype_ext(str_hostname, addr_arry, &addr_num, NETCONN_DNS_IPV6);
#else /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
            err = netconn_gethostbyname_addrtype(str_hostname, addr_arry, NETCONN_DNS_IPV6);
#endif /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
            if (err != ERR_OK)
            {
                LWIP_DEBUGF(DNS_DEBUG, ("lwip_gethostbyname(%s) failed, err=%d\n", str_hostname, err));
                printf("Fail! hostname:%s err:%d\n", str_hostname, err);
                return pdFAIL;
            }
            else
            {
#if LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR
                for (i = 0; i < addr_num; i++)
#endif /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
                {
                    printf("%s\n", (UI8_T *)ipaddr_ntoa(addr_arry + i));
                }
                return pdFALSE;
            }
        }
#if LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR
        err = netconn_gethostbyname_ext(str_hostname, addr_arry, &addr_num);
#else /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
        err = netconn_gethostbyname(str_hostname, addr_arry);
#endif /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
        if (err != ERR_OK)
        {
            LWIP_DEBUGF(DNS_DEBUG, ("lwip_gethostbyname(%s) failed, err=%d\n", str_hostname, err));
            printf("Fail! hostname:%s err:%d\n", str_hostname, err);
            return pdFAIL;
        }
        else
        {
#if LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR
            for (i = 0; i < addr_num; i++)
#endif /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
            {
                printf("%s\n", (UI8_T *)ipaddr_ntoa(addr_arry + i));
            }
            return pdFALSE;
        }
    }
    else if ((6 == addr_type_u8) || (4 == addr_type_u8))
    {
        sscanf((char *)input, "%s %s %s", cmd, addr_type, str_hostname);

        if (1 == is_ipv6)
        {
#if !LWIP_IPV6
            printf("IPV6 not support.\n");
            return pdFAIL;
#endif /* LWIP_IPV6 */
            if (4 == addr_type_u8)
            {
                printf("Invalid argument.\n");
                return pdFAIL;
            }
            printf("%s\n", str_hostname);
            return pdFAIL;
        }

        if (0 == is_url)
        {
            printf("%s\n", str_hostname);
            return pdFAIL;
        }

        u8_t type = NETCONN_DNS_IPV4_IPV6;
        if (4 == addr_type_u8)
        {
            type = NETCONN_DNS_IPV4;
            if(1 == is_ipv6)
            {
                printf("Fail to resolve.\n");
                return pdFAIL;
            }
#if !LWIP_IPV4
            printf("IPV4 not support.\n");
            return pdFAIL;
#endif /* LWIP_IPV4 */
        }
        else if (6 == addr_type_u8)
        {
            type = NETCONN_DNS_IPV6;
#if !LWIP_IPV6
            printf("IPV6 not support.\n");
            return pdFAIL;
#endif /* LWIP_IPV6 */
        }
        else
        {
            printf("Invalid argument.\n");
            return pdFAIL;
        }
#if LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR
        err = netconn_gethostbyname_addrtype_ext(str_hostname, addr_arry, &addr_num, type);
#else /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
        err = netconn_gethostbyname_addrtype(str_hostname, addr_arry, type);
#endif /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
        if (err != ERR_OK)
        {
            LWIP_DEBUGF(DNS_DEBUG, ("lwip_gethostbyname(%s) failed, err=%d\n", str_hostname, err));
            printf("Fail! hostname:%s type:%d err:%d\n",str_hostname, type, err);
            return pdFAIL;
        }
        else
        {
#if LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR
            for (i = 0; i < addr_num; i++)
#endif /* LWIP_DNS_SUPPORT_MULTIPLE_IP_ADDR */
            {
                printf("%s\n", (UI8_T *)ipaddr_ntoa(addr_arry + i));
            }
            return pdFALSE;
        }
    }
    else
    {
        printf("Invalid argument.\n");
        return pdFAIL;
    }
    return pdFALSE;
}
#endif /* LWIP_DNS */

#ifdef AIR_SUPPORT_SNTP
#ifdef AIR_SUPPORT_CLI
extern portBASE_TYPE sntp_on;
#else
portBASE_TYPE sntp_on = pdFALSE;
#endif
static portBASE_TYPE sntp_on_cmd(signed char *buf, size_t len, const signed char *input)
{
    ip4_addr_t ip4;

    if (pdTRUE == sntp_on)
    {
        printf("SNTP is already enable\n");
        return pdFALSE;
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL);/* only spt SNTP_OPMODE_POLL yet, listen spt later*/
    sntp_init();
    /*Init sntp server*/
#if LWIP_IPV4 && LWIP_IPV6
    ip4 = sntp_servers[sntp_current_server].addr.u_addr.ip4;
#else
    ip4 = sntp_servers[sntp_current_server].addr;
#endif
    ip4_addr_set_u32(&ip4, AIR_DEFAULT_IP);
    sntp_on = pdTRUE;
    return pdFALSE;
}

static portBASE_TYPE sntp_off_cmd(signed char *buf, size_t len, const signed char * input)
{
    sntp_stop();
    sntp_on = pdFALSE;
    return pdFALSE;
}

static portBASE_TYPE sntp_set_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32];
    unsigned int idx = 0;
    unsigned int ip0, ip1, ip2, ip3;
    int loc = 0;
    ip4_addr_t sntp_ip;

    sscanf((char*)input, "%s %u %u.%u.%u.%u %d", str, &idx, &ip0, &ip1, &ip2, &ip3, &loc);
    if(SNTP_MAX_SERVERS <= idx)
    {
        printf("Invalid NTP server idx %u, max idx is %u, please input again\n", idx , LWIP_DHCP_MAX_NTP_SERVERS);
        return pdFALSE;
    }
    g_local_timezone = loc;
    IP4_ADDR(&sntp_ip, ip0, ip1, ip2, ip3);
    sntp_setserver(idx, &sntp_ip);
    return pdFALSE;
}

static portBASE_TYPE sntp_get_cmd(signed char *buf, size_t len, const signed char * input)
{
    char str[32];
    unsigned int idx = 0;

    const ip4_addr_t *sntp_ip;
    sscanf((char *)input, "%s %u", str, &idx);
    if(SNTP_MAX_SERVERS < idx)
    {
        printf("Invalid NTP server idx %u, max idx is %u, please input again\n", idx , LWIP_DHCP_MAX_NTP_SERVERS);
        return pdFALSE;
    }
    sntp_ip = sntp_getserver(idx);
    printf("sntp %u IP is %s \n", idx, ip_ntoa(sntp_ip));
    return pdFALSE;
}

static portBASE_TYPE localtime_set(signed char *buf, size_t len, const signed char * input)
{
    char str[32];
    unsigned int day = 0, month = 0, year = 0;
    unsigned int hour = 0, minute = 0, second = 0;
    MW_DATE_T date;
    sscanf((char *)input, "%s %u:%u:%u %u:%u:%u", str, &year, &month, &day, &hour, &minute, &second);
    if ((12 < month) || (31 < day) || (24 <= hour) || (60 <= minute) || (60 <= second))
    {
        printf("Input out of range, plz try again\n");
        return pdFAIL;
    }
    date.year = year;
    date.month = month;
    date.day = day;
    date.hours = hour;
    date.minutes = minute;
    date.seconds = second;
    sys_mgmt_setTimes(date);

    return pdFALSE;
}

static portBASE_TYPE localtime_get(signed char *buf, size_t len, const signed char * input)
{
    UI64_T time = 0;
    MW_DATE_T date;

    sys_mgmt_getTimes(&time, &date);

    printf("local time is %u:%u:%u %u:%u:%u \n", date.year, date.month, date.day, date.hours, date.minutes, date.seconds);
    return pdFALSE;
}
#endif

void cmd_output(char * buf, uint32_t len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        outbyte((int)buf[i]);
    }
}

void vTimerCallback_Top ( TimerHandle_t xTimer )
{
    uint32_t cpu_loading = airTaskGetRunTimeStats(0);
    if( (g_cmd_state == CMD_STATE_IDLE) && bMonitorCpuLoading )
    {
        printf(" Run %lu%%\n", cpu_loading);
    }
}

void setup_timeStats_update_interval(uint32_t interval_ms)
{
    TimerHandle_t xTimers = xTimerCreate( "top",
                                          ( interval_ms/portTICK_PERIOD_MS ),
                                          pdTRUE,
                                          NULL,
                                          vTimerCallback_Top);
    xTimerStart( xTimers, portMAX_DELAY);
}

static void cmd_shell_input(char c)
{
    if (g_cmd_queue != NULL)
    {
        xQueueSendFromISR(g_cmd_queue, &c, NULL);
    }
}

static void cmd_queue_recv_task(void *pvParameters)
{
    uint32_t value;

    // TODO: improve this
    GDMPSRAM_BSS static char input[CMD_SH_MAX_INPUT], output[CMD_SH_MAX_OUTPUT];

    unsigned char input_char;
    signed int input_index;
    portBASE_TYPE ret;
    int i;

    uart_register_input_cb(cmd_shell_input);

    g_cmd_queue = xQueueCreate(128, sizeof(unsigned char), "cmd");

    if(g_cmd_queue == NULL)
    {
        printf("\nError: xQueueCreate failed !\n");
        return;
    }

    cmd_register(&sysCmds, CMD_PRIVILEGE_MODE_EXEC);
    cmd_register(&debugCmds, CMD_PRIVILEGE_MODE_DEBUG);

    memset(input, 0x00, CMD_SH_MAX_INPUT);
    memset(output, 0x00, CMD_SH_MAX_OUTPUT);
    cmd_prompt();

    setup_timeStats_update_interval(1000);

    input_index = 0;
    for( ;; )
    {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h.  It will not use any CPU time while it is in the
        Blocked state. */
        xQueueReceive(g_cmd_queue, &input_char, portMAX_DELAY);

#ifdef CMD_FIX_BACKSPACE_FOR_UI
        if( input_char == '\b' )
        {
            if( input_index > 0 )
            {
                serial_outc(input_char);

                char temp[]=" \b";
                cmd_output(temp, strlen(temp));
            }
        }
        else
#endif
        {
            serial_outc(input_char);
        }

        if ((input_char == '\n') || (input_char == '\r'))
        {
            // must before cmd_process() so monito-cpu-loading can show when cmd is processing
            g_cmd_state = CMD_STATE_IDLE;
#ifdef AIR_SUPPORT_REMOTE_DEBUG
            printf("\r\n", strlen("\r\n"));
#else
            cmd_output("\r\n", strlen("\r\n"));
#endif
            if (    !strncmp(input, "exit", strlen("exit")) &&
                    (strlen("exit") == input_index) &&
                    (   (CMD_PRIVILEGE_MODE_SDK == g_privilege_mode) ||
                        (CMD_PRIVILEGE_MODE_PERIPHERAL == g_privilege_mode) ||
#ifdef AIR_MW_SUPPORT
                        (CMD_PRIVILEGE_MODE_MW == g_privilege_mode) ||
#endif
                        (CMD_PRIVILEGE_MODE_DEBUG == g_privilege_mode)
                    )
                )
            {
                g_privilege_mode = CMD_PRIVILEGE_MODE_EXEC;
            }
            else
            {
                if (CMD_PRIVILEGE_MODE_SDK == g_privilege_mode)
                {
                    dsh_parseString(input);
                }
                else if (CMD_PRIVILEGE_MODE_PERIPHERAL == g_privilege_mode)
                {
                    dsh_peripheral_parseString(input);
                }
#ifdef AIR_MW_SUPPORT
                else if (CMD_PRIVILEGE_MODE_MW == g_privilege_mode)
                {
                    mw_cmd_parseString(input);
                }
#endif
                else
                {
                    do
                    {
                        ret = cmd_process(input, output, CMD_SH_MAX_OUTPUT, g_privilege_mode);
#ifdef AIR_SUPPORT_REMOTE_DEBUG
                        printf("%s", output);
#else
                        cmd_output(output, strlen(output));
#endif
                    } while( ret != pdFALSE );
                }
            }

            input_index = 0;
            memset(input, 0x00, CMD_SH_MAX_INPUT);
            memset(output, 0x00, CMD_SH_MAX_OUTPUT);
            cmd_prompt();
        }
        else
        {
            if( input_char == '\b' )
            {
                /* Backspace was pressed.  Erase the last  character in the string - if any. */
                if( input_index > 0 )
                {
                    input_index--;
                    input[input_index] = '\0';
                }
            }
            else
            {
                /* A character was entered.  Add it to the string entered so far.  When a \n is entered the complete string will be passed to the command interpreter. */
                if( input_index < sizeof(input) )
                {
                    input[input_index] = input_char;
                    input_index++;
                }
            }

            if(input_index)
            {
                g_cmd_state = CMD_STATE_KEYING;
            }
        }
    }
}

void create_queue_recv_task(void)
{
    xTaskCreate(cmd_queue_recv_task,            /* The function that implements the task. */
                "CMD",                          /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                configCMD_STACK_SIZE,           /* The size of the stack to allocate to the task. */
                NULL,                           /* The parameter passed to the task - not used in this simple case. */
                (tskIDLE_PRIORITY + 3),         /* The priority assigned to the task. */
                NULL );                         /* The task handle is not required, so NULL is passed. */

    return;
}

QueueHandle_t cmd_queue_get()
{
    return g_cmd_queue;
}