#ifndef __NDS32_SPINORWRITE_H__
#define __NDS32_SPINORWRITE_H__

typedef union
{
    struct image_version
    {
        unsigned char major;
        unsigned char minor;
        unsigned short build;
    } img_ver;
    unsigned int raw_ver;
} version_t;

/*****************************************************************************
 * Flash Config
 ****************************************************************************/
typedef struct flash_config
{
    unsigned int    upgrade_flag;   /* set to 1 to notice bootloader need to upgrade */
    unsigned int    backup_flag;    /* set to 1 to notice bootloader backuped */
    unsigned int    rollback_flag;  /* set to 1 to notice bootloader need to rollback */
    unsigned int    status_flag;    /* notice fw upgrade failed */
    version_t   main_img_ver;   /* main image version */
    unsigned int    main_img_len;   /* main image length */
    unsigned int    main_img_crc;   /* main image crc checksum */
    version_t   backup_img_ver; /* backup image version */
    unsigned int    backup_img_len; /* backup image length */
    unsigned int    backup_img_crc; /* backup image crc checksum */
    unsigned int    reserved[54];   /* Reserved to 256B  */
} flash_config_t;

/*****************************************************************************
 * Image Header
 ****************************************************************************/
typedef struct image_header
{
    unsigned int    magic_num;      /* FW_MAGIC_NUM */
    unsigned int    img_model;      /* FW image model */
    version_t   img_version;    /* FW image version */
    unsigned int    img_offset;     /* FW image offset */
    unsigned int    img_len;        /* FW image length */
    unsigned int    img_crc;        /* FW image crc32 */
    unsigned int    sig_offset;     /* FW signature offset */
    unsigned int    sig_len;        /* FW signature length */
    unsigned int    reserved[56];   /* Reserved to 256B */
} image_header_t;

/*****************************************************************************
 * SPI driver API's error code
 ****************************************************************************/
typedef enum{
    SPI_SUCCESS = 0,
    SPI_ERR_INIT = 1,
    SPI_ERR_WR_SIZE = 2,
    SPI_ERR_ILLEGAL_ADDR = 3,
    SPI_ERR_INVAL = 5
}SPI_ERR_T;

/*****************************************************************************
 * SPI Flash Available test
 ****************************************************************************/
typedef enum{
    SPI_AVL_TEST_SUCCESS = 0,
    SPI_AVL_TEST_FAIL = 1
}SPI_AVL_TEST_T;
#define SPI_AVL_TIMES_BASE (3000)

/*****************************************************************************
 * SPI Driver memory layout
 ****************************************************************************/
#define SPI_NOR_SECTOR_SIZE         (4096)

#define UPIMAGE_MAGIC           0x746e6365
#define MAX_CODE_SIZE           4096
#define FLASH_WRITE_CODE_OFFSET 8

#define WRITE_FLASH_BUFFER_SIZE S_4K

#define FW_MAGIC_NUM        "ARHT"
#define FW_VERISON_MAJOR    (0)
#define FW_VERISON_MINOR    (0)
#define FW_VERISON_BUILD    (1)

#define FW_CRC_MATCH         (0)
#define FW_MAGIC_MISMATCH   (-1)
#define FW_CRC_MISMATCH     (-2)

#define FW_UPGRADE_NONPROCESS   (0)
#define FW_UPGRADE_PROCESS      (1)
#define FW_UPGRADE_SUCCESS      (2)
#define FW_UPGRADE_FAILED       (-1)
#define FW_BACKUP_FAILED        (-2)
#define FW_BOOT_FAILED          (-3)

/* a buffer to gather data from application, in order to
 * accelerate program efficiency.
 */
extern unsigned char g_flash_buf[SPI_NOR_SECTOR_SIZE];

SPI_ERR_T spinor_write_init(void);
SPI_ERR_T spinor_write(unsigned int src_addr, unsigned int flash_dst_addr, unsigned int src_data_size);
void spinor_write_bootloader(int image_size);
SPI_AVL_TEST_T spinor_write_sector_test(unsigned int flash_dst_addr, unsigned int run_time);
SPI_ERR_T _spi_nor_entry(uint32_t src, uint32_t dst, uint32_t size, uint32_t op_mode);
SPI_ERR_T spinor_write_sector(uint32_t src_addr, uint32_t flash_dst_addr, uint32_t src_data_size);

extern int crc_check(unsigned char *buffer);
extern int fw_upgrade(void);
extern void update_upgrade_flag(unsigned int value);
extern void create_flash_conf_task(void);
extern int WriteBuffer(unsigned char *buffer, int size);
extern int WriteLastBuffer(void);
extern void WriteBufferInit(unsigned char *flash_addr);

#endif /* __NDS32_DEFS_H__ */
