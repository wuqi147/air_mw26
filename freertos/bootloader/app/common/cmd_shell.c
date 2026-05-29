/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Kernel includes. */
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "cache.h"

/* Utils includes. */
#include "cmd_interpreter.h"
#include "bootmenu.h"
#include "spinorwrite.h"
#include "air_ver.h"
#include "util.h"
#include "uart.h"
#include "timer.h"
#include "sif.h"
#include "gpio.h"
#include "cmd_func.h"
#include "../include/ansi.h"
#if defined(AIR_SUPPORT_CPU_PROCESS)
#include "interrupt.h"
#endif

extern int doSysMemrl(unsigned int addr);
extern int XModemReceive(unsigned int flash_addr);
extern int doSysMemwl(unsigned int addr, unsigned int value);
extern int doSysMemory(unsigned int addr, unsigned int len);
extern int doSysMemrl_raw(unsigned int addr);
extern int doSysMemwl_raw(unsigned int addr, unsigned int value);
extern int doSysMemory_raw(unsigned int addr, unsigned int len);

static portBASE_TYPE memory_read_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE memory_write_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE memory_dump_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE memory_read32_raw_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE memory_write32_raw_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE raw_memory_dump_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE kill_wdog_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE xmodem_rcv_cmd(char *buf, size_t len, const char *input);
#if defined(AIR_8851_SUPPORT)
static portBASE_TYPE sif_write_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE sif_read_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE gpio_reqeust_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE gpio_free_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE gpio_set_direction_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE gpio_get_direction_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE gpio_set_data_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE gpio_get_data_cmd(char *buf, size_t len, const char *input);
#endif /* AIR_8851_SUPPORT */
static portBASE_TYPE upgrade_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE boot_rtos_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE version_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE reset_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE flash_write_cfg_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE flash_read_cfg_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE flash_erase_partition_cmd(char *buf, size_t len, const char *input);
static portBASE_TYPE flash_show_partition_cmd(char *buf, size_t len, const char *input);
#ifdef  FLASH_AVL_TEST
static portBASE_TYPE spinor_avl_test_cmd(char *buf, size_t len, const char *input);
#endif
#ifdef AIR_SUPPORT_CPU_PROCESS
static portBASE_TYPE port_stats_cmd(char *buf, size_t len, const char * input);
static void clear_udf_rule();
#endif

static void cmd_output(char *buf, uint32_t len);
static void cmd_queue_recv_task(void *pvParameters);
void create_queue_recv_task(void);

QueueHandle_t g_cmd_queue = NULL;

#ifdef AIR_SUPPORT_CPU_PROCESS
extern TaskHandle_t g_xMacRcvHandle;
#endif

static const CMD_LINE_INPUT_T sysCmds[] = {
    {"memrl",           "memrl <addr-hex>: read 4-byte data from memory, i.e. memrl 0x1000500c\r\n",                       memory_read_cmd,        1},
    {"memwl",           "memwl <addr-hex> <val-hex>: write 4-byte value to memory, i.e. memwl 0x1000500c 0x12345678\r\n",  memory_write_cmd,       2},
    {"memory",          "memory <addr-hex> <len-hex>: dump memory for a range, i.e. memory 0x1000500c 0x10\r\n",           memory_dump_cmd,        2},
    {"raw_memrl",       "raw_memrl address: read 4 bytes from register or memory at the address\r\n",                      memory_read32_raw_cmd,  1},
    {"raw_memwl",       "raw_memwl address value: write 4-byte value to register or memory at the address\r\n",            memory_write32_raw_cmd, 2},
    {"raw_memory",      "raw_memory address len: dump registers or memory for range (address)~(address+len)\r\n",          raw_memory_dump_cmd,    2},
    {"kill_wdog",       "kill_wdog: kill watchdog\r\n",                                                                    kill_wdog_cmd,          0},
    {"xmodem_rcv",      "xmodem_rcv [dst_addr]: recv xmodem data to specify addr\r\n",                                     xmodem_rcv_cmd,         0},
#if defined(AIR_8851_SUPPORT)
    {"sif-read",        "sif-read <bus> <slave_id> <address>: read 4 bytes data from 4 bytes address \r\n",                sif_read_cmd,           3},
    {"sif-write",       "sif-write <bus> <slave_id> <address> <data>: write 4 bytes data to 4 bytes address\r\n",          sif_write_cmd,          4},
    {"gpio-request",    "gpio-request <pin>: set the pin to gpio mode\r\n",                                                gpio_reqeust_cmd,       1},
    {"gpio-free",       "gpio-free <pin>: clear the pin to disable gpio mode\r\n",                                         gpio_free_cmd,          1},
    {"gpio-set-dir",    "gpio-set-dir <pin> <direction>: set the gpio pin direction (1: output, 0: input)\r\n",            gpio_set_direction_cmd, 2},
    {"gpio-get-dir",    "gpio-get-dir <pin>: get the gpio pin direction\r\n",                                              gpio_get_direction_cmd, 1},
    {"gpio-set-data",   "gpio-set-data <pin> <data>: set the gpio pin data (1: high, 0:low)\r\n",                          gpio_set_data_cmd,      2},
    {"gpio-get-data",   "gpio-get-data <pin>: get the gpio pin data\r\n",                                                  gpio_get_data_cmd,      1},
#endif /* AIR_8851_SUPPORT */
    {"upgrade",         "upgrade fw-type= { rtos | bootloader }: load rtos(.img)/bootloader(.bin) to upgrade firmware by xmodem\r\n",   upgrade_cmd, 1},
    {"boot-rtos",       "boot-rtos: boot from rtos\r\n",                                                                   boot_rtos_cmd,          0},
    {"ver",             "ver: bootloader version\r\n",                                                                     version_cmd,            0},
    {"reset",           "reset: reset system\r\n",                                                                         reset_cmd,              0},
    {"flash-write-cfg", "flash-write-cfg addr=<HEX> data-len=<UINT> data=<HEX>: write configuration into flash\r\n",       flash_write_cfg_cmd,    3},
    {"flash-read-cfg",  "flash-read-cfg addr=<HEX> data-len=<UINT>: read configuration from flash\r\n",                    flash_read_cfg_cmd,     2},
    {"flash-erase-partition",  "flash-erase-partition: erase flash all partitions except for bootloader and reset\r\n",    flash_erase_partition_cmd, 0},
    {"flash-show-partition",  "flash-show-partition: get flash all partitions information\r\n",                            flash_show_partition_cmd, 0},
#ifdef  FLASH_AVL_TEST
    {"spinor_avl_test", "spinor_avl_test : write and readback flash available test\r\n",                                   spinor_avl_test_cmd,    3},
#endif
#if defined(AIR_SUPPORT_CPU_PROCESS)
    {"port-stats",      "port-stats <port>: display port number\r\n",                                                      port_stats_cmd,         0},
#endif
    {NULL, NULL, NULL, 0}
};

static portBASE_TYPE memory_read_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int addr;
    sscanf(input, "%s %x", str, &addr) ;
    doSysMemrl(addr);
    return pdFALSE;
}

static portBASE_TYPE memory_write_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int addr, val;
    sscanf(input, "%s %x %x", str, &addr, &val) ;
    doSysMemwl(addr, val);
    return pdFALSE;
}

static portBASE_TYPE memory_dump_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int addr, length;
    sscanf(input, "%s %x %x", str, &addr, &length) ;
    doSysMemory(addr, length);
    return pdFALSE;
}

static portBASE_TYPE memory_read32_raw_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int addr;
    sscanf(input, "%s %x", str, &addr) ;
    doSysMemrl_raw(addr);
    return pdFALSE;
}

static portBASE_TYPE memory_write32_raw_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int addr, val;
    sscanf(input, "%s %x %x", str, &addr, &val) ;
    doSysMemwl_raw(addr, val);
    return pdFALSE;
}

static portBASE_TYPE raw_memory_dump_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int addr, length;
    sscanf(input, "%s %x %x", str, &addr, &length) ;
    doSysMemory_raw(addr, length);
    return pdFALSE;
}

static portBASE_TYPE kill_wdog_cmd(char *buf, size_t len, const char *input)
{
    wdog_kill();
    return pdFALSE;
}

static portBASE_TYPE xmodem_rcv_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int addr = ~0UL;
    int recvSize = 0;
    sscanf(input, "%s %x", str, &addr);
    /*disable UART isr*/
    io_write32(0x1000a004, 0);
    recvSize = XModemReceive(addr);
    /*enable UART isr*/
    io_write32(0x1000a004, 1);
    printf("\nReceived %d bytes!\n", recvSize);
    return pdFALSE;
}

#ifdef AIR_SUPPORT_CPU_PROCESS
static void clear_udf_rule()
{
    /* clear UDF rule */
    io_write32(0x10200208, 0x0);
    io_write32(0x1020020c, 0x0);
    io_write32(0x10200210, 0x0);
    io_write32(0x10200200, 0x9000000f);
}
#endif
static portBASE_TYPE upgrade_cmd(char *buf, size_t len, const char *input)
{
    char *type = NULL;
    char *type_val = NULL;
    unsigned int addr = TempSystemBase;
    int recvSize = 0;
    const char *d = " ";
    const char *d1 = "=";
    strtok((char*)input, d);
    type = strtok(NULL, d1);
    type_val = strtok(NULL, d);

    if ((type == NULL) ||
        (type_val == NULL) ||
        (strcmp(type, "fw-type") != 0) ||
        (strcmp(type_val, "rtos") != 0 && strcmp(type_val, "bootloader") != 0))
    {
        printf("syntax error:\r\n");
        printf("upgrade fw-type={ rtos | bootloader }\r\n");
        return pdFALSE;
    }

    if (strcmp(type_val, "rtos") == 0)
    {
#ifdef AIR_SUPPORT_CPU_PROCESS
        /* Disable PDMA */
        /* [TODO : AN8855] ACE*/
        unsigned int rg;
        unregister_isr(IRQ_PDMA);
        rg = io_read32(0x100050c4);
        io_write32(0x100050c4, (rg | (1L << 20)));
        io_write32(0x100050c4, (rg & ~(1L << 20)));
        /* clear UDF rule */
        clear_udf_rule();
        /* Disable Mac Rcv */
        vTaskSuspend(g_xMacRcvHandle);
#endif
        /*disable UART isr*/
        io_write32(0x1000a004, 0);
        recvSize = XModemReceive(~0UL);
        /*enable UART isr*/
        io_write32(0x1000a004, 1);
        printf("\nReceived %d bytes!\n", recvSize);
        /*Chechk image CRC and prepare fw upgrade*/
        if (crc_check((unsigned char *) addr) == 0)
        {
            update_upgrade_flag(1);
        }
        else
        {
            printf("FW CRC32 is mismatch\r\n");
        }
    }
    else if(strcmp(type_val, "bootloader") == 0)
    {
#ifdef AIR_SUPPORT_CPU_PROCESS
        /* Disable PDMA */
        /* [TODO : AN8855] ACE*/
        unsigned int rg;
        unregister_isr(IRQ_PDMA);
        rg = io_read32(0x100050c4);
        io_write32(0x100050c4, (rg | (1L << 20)));
        io_write32(0x100050c4, (rg & ~(1L << 20)));
        /* clear UDF rule */
        clear_udf_rule();
        /* Disable Mac Rcv */
        vTaskSuspend(g_xMacRcvHandle);
#endif
        /*disable UART isr*/
        io_write32(0x1000a004, 0);
        recvSize = XModemReceive(~0UL);
        /*enable UART isr*/
        io_write32(0x1000a004, 1);
        spinor_write_bootloader(recvSize);
    }

    return pdFALSE;
}

static portBASE_TYPE boot_rtos_cmd(char *buf, size_t len, const char * input)
{
#ifdef AIR_SUPPORT_CPU_PROCESS
    /* Disable PDMA */
    /* [TODO : AN8855] ACE*/
    unsigned int rg;
    unregister_isr(IRQ_PDMA);
    rg = io_read32(0x100050c4);
    io_write32(0x100050c4, (rg | (1L << 20)));
    io_write32(0x100050c4, (rg & ~(1L << 20)));
    /* clear UDF rule */
    clear_udf_rule();
#endif

    __nds32__gie_dis();
    __asm__ __volatile__("mfsr  $r1, $mr8\n\t"
                            "li    $r2, ~0x03\n\t"
                            "and   $r1, $r1, $r2\n\t"
                            "mtsr  $r1, $mr8\n\t"
                            "isb\n\t"
                        );
    nds32_icache_flush();
    nds32_dcache_invalidate();
    __asm__ __volatile__("movi55 $fp,#0x0");
    __asm__ __volatile__("movi55 $gp,#0x0");
    __asm__ __volatile__("movi55 $lp,#0x0");
    __asm__ __volatile__("movi55 $sp,#0x0");
    __asm__ __volatile__("sethi $r0, #0x20020");

    __asm__ __volatile__("jr5 $r0");
    return pdFALSE;
}

static portBASE_TYPE version_cmd(char *buf, size_t len, const char *input)
{
    printf("Version: %s\n", AIR_VER_BOOTLOADER);
    return pdFALSE;
}


static portBASE_TYPE reset_cmd(char *buf, size_t len, const char *input)
{
    io_write32(0x10005010, 0);
    io_write32(0x100050c0, 0x80000000);

    return pdFALSE;
}

#if defined(AIR_8851_SUPPORT)
static portBASE_TYPE sif_write_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int bus, slave_id, addr, data;
    SIF_PARAM_T sif_param;
    sscanf((char*)input, "%s %x %x %x %x", str, &bus, &slave_id, &addr, &data);

    if((bus > 1))
    {
        printf("Syntax error\n");
        return pdFALSE;
    }

    sif_param.bus = bus;
    sif_param.slave_id = slave_id;
    sif_param.addr = addr;
    sif_param.ptr_data = &data;

    if(sif_write(&sif_param))
    {
        printf("write fail, please check the arguments, master channel and the reset pin !\n");
    }
    else
    {
        printf("Done\n");
    }

    return pdFALSE;
}

static portBASE_TYPE sif_read_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int bus, slave_id, addr, data;
    SIF_PARAM_T sif_param;
    sscanf((char*)input, "%s %x %x %x", str, &bus, &slave_id, &addr);

    if((bus > 1))
    {
        printf("Syntax error\n");
        return pdFALSE;
    }

    sif_param.bus = bus;
    sif_param.slave_id = slave_id;
    sif_param.addr = addr;
    sif_param.ptr_data = &data;

    if(sif_read(&sif_param))
    {
        printf("read fail, please check the arguments, master channel and the reset pin !\n");
    }
    else
    {
        printf("data = 0x%x\n", data);
    }

    return pdFALSE;
}

static portBASE_TYPE gpio_reqeust_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int pin;
    sscanf((char*)input, "%s %u", str, &pin);

    gpio_request(pin);

    return pdFALSE;
}

static portBASE_TYPE gpio_free_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int pin;
    sscanf((char*)input, "%s %u", str, &pin);

    gpio_free(pin);

    return pdFALSE;
}

static portBASE_TYPE gpio_set_direction_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int pin, direction;
    sscanf((char*)input, "%s %u %u", str, &pin, &direction);

    gpio_set_direction(pin, direction);

    return pdFALSE;
}

static portBASE_TYPE gpio_get_direction_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int pin, direction;
    sscanf((char*)input, "%s %u", str, &pin);

    gpio_get_direction(pin, &direction);
    printf("pin %u, direction = %s\n", pin, (direction) ? "output" : "input");

    return pdFALSE;
}

static portBASE_TYPE gpio_set_data_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int pin, data;
    sscanf((char*)input, "%s %u %u", str, &pin, &data);

    gpio_set_data(pin, data);
    return pdFALSE;
}

static portBASE_TYPE gpio_get_data_cmd(char *buf, size_t len, const char *input)
{
    char str[32];
    unsigned int pin, data;
    sscanf((char*)input, "%s %u", str, &pin);

    gpio_get_data(pin, &data);
    printf("pin %u, data = %s\n", pin, (data) ? "high" : "low");
    return pdFALSE;
}
#endif /* AIR_8851_SUPPORT */

static portBASE_TYPE flash_write_cfg_cmd(char *buf, size_t len, const char *input)
{
    if(E_SYNTAX_OK != write_flash_data(input))
    {
        printf("syntax error:\n");
        printf("flash-write-cfg addr=<HEX> data-len=<UINT> data=<HEX>\n");
    }
    return pdFALSE;
}

static portBASE_TYPE flash_read_cfg_cmd(char *buf, size_t len, const char *input)
{
    if(E_SYNTAX_OK != read_flash_data(input))
    {
        printf("syntax error:\n");
        printf("flash-read-cfg addr=<HEX> data-len=<UINT>\n");
    }
    return pdFALSE;
}

static portBASE_TYPE flash_erase_partition_cmd(char *buf, size_t len, const char *input)
{
#define ROUND_UP(x,n) (((x)+((n-1))) & ~((n)-1))
    unsigned int erase_cur_addr, progress_index, tmp_progress_index = ~0U;
    unsigned int erase_start_addr = BootConfigBase;
#ifdef FW_SIGNATURE
    unsigned int erase_end_addr = FwSignatureBase + FwSignatureSize;
#else
    unsigned int erase_end_addr = TempSystemBase + TempSystemSize;
#endif
    unsigned int erase_total_size = ROUND_UP((erase_end_addr - erase_start_addr), SPI_NOR_SECTOR_SIZE);
    const char *label = "|/-\\";

    for(erase_cur_addr=erase_start_addr; erase_cur_addr<erase_start_addr+erase_total_size; erase_cur_addr+=SPI_NOR_SECTOR_SIZE)
    {
        air_wdog_kick();
        memset((unsigned char *)g_flash_buf, 0xff, SPI_NOR_SECTOR_SIZE);
        if(SPI_SUCCESS != spinor_write_sector((unsigned int)g_flash_buf, erase_cur_addr, SPI_NOR_SECTOR_SIZE))
        {
            printf("\nErase flash partition fail\n");
            return pdFALSE;
        }
        progress_index = ((erase_cur_addr-erase_start_addr)*100)/erase_total_size;
        if(tmp_progress_index != progress_index)
        {
            printf(ANSI_EL_ALL);
            printf("[%c] %d%%\r", label[progress_index%4], (((erase_cur_addr-erase_start_addr)*100)/erase_total_size)+1);
            tmp_progress_index = progress_index;
        }
    }
    printf("\nErase flash partition success\n");
    /*after erase flash success, then reset. Let the flash_config set to 0*/
    io_write32(0x10005010, 0);
    io_write32(0x100050c0, 0x80000000);
    return pdFALSE;
}

static portBASE_TYPE flash_show_partition_cmd(char *buf, size_t len, const char *input)
{
#define PARTITION_LEVEL (0)
#define PARTITION_SUB_LEVEL ((PARTITION_LEVEL)+2)
#define PARTITION_ADDR_RANGE  "0x%08x-0x%08x"
#define PARTITION_ADDR        "0x%08x"
#define PARTITION_SIZE        "%d bytes = %d.%d MB"
#define PARTITION_DEC         "%d"
#define PARTITION_STR         "%s"
#define PRINT_FLASH_PARTITION(__indent__, __name__, __fmt__, ...)   \
do{                                                                 \
    int i;                                                          \
    if(__indent__)                                                  \
    {                                                               \
        for(i = 0; i < __indent__; i++)                             \
        {                                                           \
            printf(" ");                                            \
        }                                                           \
        printf("|--");                                              \
    }                                                               \
    printf("%-16s", __name__":");                                   \
    printf(__fmt__, __VA_ARGS__);                                   \
    printf("\n");                                                   \
} while(0)
    unsigned int main_img_len = VPint(BootConfigBase + offsetof(flash_config_t, main_img_len));
#if DUAL_IMAGE
    unsigned int backup_img_len = VPint(BootConfigBase + offsetof(flash_config_t, backup_img_len));
#endif
    unsigned int magic_num = VPint(TempSystemBase + offsetof(image_header_t, magic_num));
    unsigned int temp_img_header_size = VPint(TempSystemBase + offsetof(image_header_t, img_offset));
    unsigned int temp_img_len = VPint(TempSystemBase + offsetof(image_header_t, img_len));
    unsigned int upgrade_flag = VPint(BootConfigBase + offsetof(flash_config_t, upgrade_flag));
    unsigned int backup_flag = VPint(BootConfigBase + offsetof(flash_config_t, backup_flag));
    unsigned int rollback_flag = VPint(BootConfigBase + offsetof(flash_config_t, rollback_flag));
    unsigned int status_flag = VPint(BootConfigBase + offsetof(flash_config_t, status_flag));
    unsigned int temp_upgrade_flag = VPint(BootConfigBase + offsetof(flash_config_t, upgrade_flag));
    unsigned int temp_backup_flag = VPint(BootConfigBase + offsetof(flash_config_t, backup_flag));
    unsigned int temp_rollback_flag = VPint(BootConfigBase + offsetof(flash_config_t, rollback_flag));
    unsigned int temp_status_flag = VPint(BootConfigBase + offsetof(flash_config_t, status_flag));
    PRINT_FLASH_PARTITION(PARTITION_LEVEL, "Bootloader", PARTITION_ADDR_RANGE, BootLoaderBase, BootLoaderBase+BootLoaderSize);
    PRINT_FLASH_PARTITION(PARTITION_LEVEL, "Boot config", PARTITION_ADDR_RANGE, BootConfigBase, BootConfigBase+BootConfigSize);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "upgrade flag", PARTITION_DEC, upgrade_flag);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "backup flag", PARTITION_DEC, backup_flag);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "rollback flag", PARTITION_DEC, rollback_flag);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "status flag", PARTITION_DEC, status_flag);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "upgrade flag(Temp)", PARTITION_DEC, temp_upgrade_flag);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "backup flag(Temp)", PARTITION_DEC, temp_backup_flag);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "rollback flag(Temp)", PARTITION_DEC, temp_rollback_flag);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "status flag(Temp)", PARTITION_DEC, temp_status_flag);
    PRINT_FLASH_PARTITION(PARTITION_LEVEL, "Manufacture", PARTITION_ADDR_RANGE, ManufactureBase, ManufactureBase+ManufactureSize);
    PRINT_FLASH_PARTITION(PARTITION_LEVEL, "Main image", PARTITION_ADDR_RANGE, MainSystemBase, MainSystemBase+MainSystemSize);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "partition size", PARTITION_SIZE, MainSystemSize, MainSystemSize/S_1M, ((MainSystemSize%S_1M)/S_1K));
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "data size", PARTITION_SIZE, main_img_len, main_img_len/S_1M, ((main_img_len%S_1M)/S_1K));
#if DUAL_IMAGE
    PRINT_FLASH_PARTITION(PARTITION_LEVEL, "Backup image", PARTITION_ADDR_RANGE, BackupSystemBase, BackupSystemBase+BackupSystemSize);
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "partition size", PARTITION_SIZE, BackupSystemSize, BackupSystemSize/S_1M, ((BackupSystemSize%S_1M)/S_1K));
    PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "data size", PARTITION_SIZE, backup_img_len, backup_img_len/S_1M, ((backup_img_len%S_1M)/S_1K));
#endif
    PRINT_FLASH_PARTITION(PARTITION_LEVEL, "Switch config", PARTITION_ADDR_RANGE, SystemConfigBase, SystemConfigBase+SystemConfigSize);
    PRINT_FLASH_PARTITION(PARTITION_LEVEL, "Temp image", PARTITION_ADDR_RANGE, TempSystemBase, TempSystemBase+TempSystemSize);
    if(strncmp((const char *)&magic_num, FW_MAGIC_NUM, sizeof(magic_num)) == 0)
    {
        PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "partition size", PARTITION_SIZE, TempSystemSize, TempSystemSize/S_1M, ((TempSystemSize%S_1M)/S_1K));
        PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "header size", PARTITION_SIZE, temp_img_header_size, temp_img_header_size/S_1M, ((temp_img_header_size%S_1M)/S_1K));
        PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "data size", PARTITION_SIZE, temp_img_len, temp_img_len/S_1M, ((temp_img_len%S_1M)/S_1K));
    }
    else
    {
        PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "partition size", PARTITION_STR, "(not found)");
        PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "header size", PARTITION_STR, "(not found)");
        PRINT_FLASH_PARTITION(PARTITION_SUB_LEVEL, "data size", PARTITION_STR, "(not found)");
    }
#ifdef FW_SIGNATURE
    PRINT_FLASH_PARTITION(PARTITION_LEVEL, "FW Signature", PARTITION_ADDR_RANGE, FwSignatureBase, FwSignatureBase+FwSignatureSize);
#endif
    return pdFALSE;
}

#ifdef  FLASH_AVL_TEST
static portBASE_TYPE spinor_avl_test_cmd(char *buf, size_t len, const char *input)
{
    unsigned int capacity, flash_dst_addr, run_time;
    char str[32], option;
    SPI_AVL_TEST_T rtn_code;
    sscanf(input, "%s %c %x %u", str, &option, &capacity, &run_time);
    /*
     * option 'q' : test the last 4 sector in flash
     * option 'w' : test the middle and the last 4 sector in flash
     */
    if(option == 'q')
    {
        flash_dst_addr = (capacity + 1 - (4*SPI_NOR_SECTOR_SIZE));
        rtn_code = spinor_write_sector_test(flash_dst_addr, run_time);
    }
    else if(option == 'w')
    {
        flash_dst_addr = ((capacity + 1) >> 1) - (2*SPI_NOR_SECTOR_SIZE);
        rtn_code = spinor_write_sector_test(flash_dst_addr, run_time);
        if(rtn_code == SPI_AVL_TEST_FAIL)
        {
            return pdFALSE;
        }
        flash_dst_addr = (capacity + 1 - (4*SPI_NOR_SECTOR_SIZE));
        rtn_code = spinor_write_sector_test(flash_dst_addr, run_time);
    }
    return pdFALSE;
}
#endif

#if defined(AIR_SUPPORT_CPU_PROCESS)
static portBASE_TYPE port_stats_cmd(char *buf, size_t len, const char * input)
{
    char cmd[32];
    int port = -1;

    sscanf((char*)input, "%s %d", cmd, &port);
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
#endif

static void cmd_output(char * buf, uint32_t len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        outbyte((int)buf[i]);
    }
}

static void cmd_queue_recv_task( void *pvParameters )
{
    uint32_t value;
    static char input[128], output[128];
    signed char input_char = 0, input_index = 0;
    portBASE_TYPE ret;
    int i;

    g_cmd_queue = xQueueCreate(128, sizeof(uint32_t));
    if(g_cmd_queue == NULL) {
        printf("\nError: xQueueCreate failed !\n");
        return;
    }

    /* register_commands */
    for(i=0; sysCmds[i].cmd !=NULL; i++){
        cmd_register(&sysCmds[i]);
    }

    memset(input, 0x00, sizeof(input));
    memset(output, 0x00, sizeof(output));

    switch(get_boot_menu_select())
    {
        case BOOTMENU_ENTRY_XMODEM_UPGRADE_BOOT_SYSTEM:
            memcpy(input, "upgrade fw-type=rtos", strlen("upgrade fw-type=rtos"));
            cmd_process(input, output, sizeof(output));
            /* fall through to next case on purpose */
        case BOOTMENU_ENTRY_BOOT_SYSTEM_VIA_FLASH:
            memcpy(input, "boot-rtos", strlen("boot-rtos"));
            cmd_process(input, output, sizeof(output));
            break;
        default:
            break;
    };

    cmd_output("\r\n#", strlen("\r\n#"));

    for( ;; )
    {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h.  It will not use any CPU time while it is in the
        Blocked state. */
        xQueueReceive(g_cmd_queue, &value, portMAX_DELAY);
        input_char = (signed char)value;
        serial_outc(input_char);

        if ((input_char == '\n') || (input_char == '\r'))
        {
            //serial_outc('Z');
            cmd_output("\r\n", strlen("\r\n"));
            do
            {
                ret = cmd_process(input, output, sizeof(output));
                cmd_output(output, strlen(output));
            } while( ret != pdFALSE );

            input_index = 0;
            memset(input, 0x00, sizeof(input));
            memset(output, 0x00, sizeof(output));
            cmd_output("\r\n#", strlen("\r\n#"));
        }
        else
        {
            if( input_char == '\r' )
            {
                /* Ignore the character. */
                //serial_outc('Y');
            }
            else if( input_char == '\b' )
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
