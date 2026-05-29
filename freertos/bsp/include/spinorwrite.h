#ifndef __NDS32_SPINORWRITE_H__
#define __NDS32_SPINORWRITE_H__

#include "fwheader.h"

#define MAC_ADDRESS_LEN (6)
typedef struct
{
    unsigned char addr[MAC_ADDRESS_LEN];
    unsigned short reserved;
} flash_mac_t;

/*****************************************************************************
 * Flash Config
 ****************************************************************************/
typedef struct flash_config
{
    uint32_t    upgrade_flag;   /* set to 1 to notice bootloader need to upgrade */
    uint32_t    backup_flag;    /* set to 1 to notice bootloader backuped */
    uint32_t    rollback_flag;  /* set to 1 to notice bootloader need to rollback */
    uint32_t    status_flag;    /* set to 1 to notice fw upgrade failed */
    version_t   main_img_ver;   /* main image version */
    uint32_t    main_img_len;   /* main image length */
    uint32_t    main_img_crc;   /* main image crc checksum */
    version_t   backup_img_ver; /* backup image version */
    uint32_t    backup_img_len; /* backup image length */
    uint32_t    backup_img_crc; /* backup image crc checksum */
    uint32_t    reserved[54];   /* Reserved to 256B  */
} flash_config_t;

/*****************************************************************************
 * SPI driver API's error code
 ****************************************************************************/
typedef enum{
    SPI_SUCCESS = 0,
    SPI_ERR_INIT = 1,
    SPI_ERR_WR_SIZE = 2,
    SPI_ERR_ILLEGAL_ADDR = 3,
    SPI_ERR_ILLEGAL_INS = 4,
    SPI_ERR_INVAL = 5
}SPI_ERR_T;

typedef enum{
    SPI_OP_WRITE = 0,
    SPI_OP_UPBOOT = 1,
    SPI_OP_INS_READ = 2,
    SPI_OP_INS_WRITE = 3
}SPI_OP_MODE_T;

/*****************************************************************************
 * SPI Driver memory layout
 ****************************************************************************/
#define SPI_NOR_SECTOR_SIZE         (4096)
#define SPI_NOR_PARAM_BASE          (0x80037C00)
#define SPI_NOR_PARAM_FPPTR         (SPI_NOR_PARAM_BASE + 0x00)
#define SPI_NOR_PARAM_SPPTR         (SPI_NOR_PARAM_BASE + 0x04)
#define SPI_NOR_PARAM_PCPTR         (SPI_NOR_PARAM_BASE + 0x08)
#define SPI_NOR_PARAM_GPPTR         (SPI_NOR_PARAM_BASE + 0x0C)
#define SPI_NOR_PARAM_SRC_ADDR      (SPI_NOR_PARAM_BASE + 0x50)
#define SPI_NOR_PARAM_DST_ADDR      (SPI_NOR_PARAM_BASE + 0x54)
#define SPI_NOR_PARAM_SIZE          (SPI_NOR_PARAM_BASE + 0x58)
#define SPI_NOR_PARAM_ERR_CODE      (SPI_NOR_PARAM_BASE + 0x5C)
#define SPI_NOR_PARAM_OP_MODE       (SPI_NOR_PARAM_BASE + 0x60)
#define SPI_NOR_PARAM_INS           (SPI_NOR_PARAM_BASE + 0x64)
#define SPI_NOR_PARAM_REG_DATA_LEN  (SPI_NOR_PARAM_BASE + 0x68)
#define SPI_NOR_PARAM_DRIVER_ADDR   (SPI_NOR_PARAM_BASE + 0x100)

#define UPDATE_PARAMETER        SPI_NOR_PARAM_SRC_ADDR
#define UPIMAGE_MAGIC           0x746e6365
#define MAX_CODE_SIZE           4096
#define FLASH_WRITE_CODE_OFFSET 8

#define WRITE_FLASH_BUFFER_SIZE S_4K

#define FW_CRC_MATCH         (0)
#define FW_MAGIC_MISMATCH   (-1)
#define FW_CRC_MISMATCH     (-2)

#define FW_UPGRADE_NONPROCESS   (0)
#define FW_UPGRADE_PROCESS      (1)
#define FW_UPGRADE_SUCCESS      (2)
#define FW_UPGRADE_FAILED       (-1)
#define FW_BACKUP_FAILED        (-2)
#define FW_BOOT_FAILED          (-3)

SPI_ERR_T spinor_write_init(void);
SPI_ERR_T spinor_write(uint32_t src_addr, uint32_t flash_dst_addr, uint32_t src_data_size);
SPI_ERR_T spinor_instr_read(unsigned char flash_ins, unsigned int reg_data_len, unsigned char *reg_data);

extern int crc_check(unsigned char *buffer);
extern void update_upgrade_flag(unsigned int value);
extern void create_flash_conf_task(void);
extern void WriteBufferInit(unsigned char *flash_addr);
extern int WriteBuffer(unsigned char *buffer, int size);
extern int WriteLastBuffer(void);
extern void read_mac_addr(unsigned char *ptr_mac);
extern void update_mac_addr(unsigned char *ptr_mac);
extern int flash_partition_copy(unsigned char src_part, unsigned char dest_part, unsigned int size);
extern int flash_partition_read(unsigned char *buf, int size, unsigned int offset, char part);
extern int flash_partition_write_finish(void);
extern int flash_partition_write(unsigned char *buf, int size);
extern int flash_partition_write_init(char part);


#endif /* __NDS32_DEFS_H__ */
