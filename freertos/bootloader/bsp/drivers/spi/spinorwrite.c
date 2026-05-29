#include <stdio.h>
#include <stdlib.h>
#include <platform.h>
#include <string.h>
#include "util.h"
#include <cache.h>
#include "FreeRTOS.h"
#include <nds32_intrinsic.h>
#include "task.h"
#include "spinorwrite.h"
#include "cmd_func.h"

static unsigned char spi_write_inited = 0;
static unsigned char random_buf[SPI_NOR_SECTOR_SIZE];

/* holding the 4k-aligned flash address to program. */
static unsigned char *g_flash_addr;
/* holding the next address-to-program of g_flash_buf. */
static unsigned char *g_buf_walker;
/* a buffer to gather data from application, in order to
 * accelerate program efficiency.
 */
unsigned char g_flash_buf[SPI_NOR_SECTOR_SIZE];
/* indicating whether there are data remaining in the g_flash_buf. */
static unsigned char g_data_received = 0;

static flash_config_t flash_conf;
static unsigned int image_lengh;

static const unsigned int crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/* FUNCTION NAME:   spinor_write_sector
 * PURPOSE:
 *      save parameter which driver will use,
 *      and jump to execute SPI driver
 * INPUT:
 *      src_addr                -- the source data which would be write into flash
 *      flash_dst_addr          -- destination address of flash
 *      src_data_size           -- how many bytes need to be writen
 * OUTPUT:
 *      None
 * RETURN:
 *      SPI_SUCCESS             -- success
 *      SPI_ERR_ILLEGAL_ADDR    -- write data into flash illegal address
 * NOTES:
 *
 */
SPI_ERR_T spinor_write_sector(uint32_t src_addr, uint32_t flash_dst_addr, uint32_t src_data_size)
{
    SPI_ERR_T rtn_code;

    if(((flash_dst_addr & (SPI_NOR_SECTOR_SIZE - 1)) != 0) || (src_data_size != SPI_NOR_SECTOR_SIZE))
    {
        return -SPI_ERR_INVAL;
    }

    taskDISABLE_INTERRUPTS();
    rtn_code = _spi_nor_entry(src_addr, flash_dst_addr & 0x1ffff000, src_data_size, 0);
    taskENABLE_INTERRUPTS();

    if(rtn_code == SPI_SUCCESS)
    {
        nds32_dcache_invalidate_range((unsigned long)flash_dst_addr, (unsigned long)(flash_dst_addr + src_data_size));
    }

    return rtn_code;
}

/* FUNCTION NAME:   spinor_read_sector
 * PURPOSE:
 *      Copy data from flash to a specific memory address.
 * INPUT:
 *      src_addr                -- address of flash to be copied.
 *      size                    -- size of data to copy.
 * OUTPUT:
 *      dst                     -- destination address of memory to hold the flash data.
 * RETURN:
 *      N/A
 * NOTES:
 *
 */
void spinor_read_sector(void *src, void *dst, uint32_t size)
{
    uint32_t i;
    uint8_t *p_src = (uint8_t *)src;
    uint8_t *p_dst = (uint8_t *)dst;

    for (i = 0; i < size; i++)
    {
        p_dst[i] = p_src[i];
    }
}

/* FUNCTION NAME:   crc32buf
 * PURPOSE:
 *      calculate CRC32.
 * INPUT:
 *      buf                 -- data buffer start adderss
 * OUTPUT:
 *      None
 * RETURN:
 *      crc32               -- crc value
 * NOTES:
 *
 */
unsigned int crc32buf(unsigned char *buf, unsigned int len)
{
    unsigned int i = 0;
    unsigned int crc32 = 0xFFFFFFFFUL;

    for (i = 0; i < len; i++)
    {
        crc32 = (crc_32_tab[((crc32) ^ ((unsigned int) buf[i])) & 0xff] ^ ((crc32) >> 8));
    }

    return (crc32 ^ 0xFFFFFFFFUL);
}

/* FUNCTION NAME:   crc_check
 * PURPOSE:
 *      check FW CRC.
 * INPUT:
 *      buffer              -- fw image start adderss
 * OUTPUT:
 *      None
 * RETURN:
 *      FW_CRC_MATCH        -- fw image crc identical
 *      FW_MAGIC_MISMATCH   -- fw image magic number mismatch
 *      FW_CRC_MISMATCH     -- fw image crc mismatch
 * NOTES:
 *
 */
/* Check Image CRC: return 0, if value is identical */
int crc_check(unsigned char *buffer)
{
    image_header_t *img_hdr = (image_header_t *) buffer;
    unsigned int crc32 = 0xFFFFFFFFUL;

    if (strncmp((const char *)(&(img_hdr->magic_num)), FW_MAGIC_NUM, 4) != 0)
    {
            printf("FW_MAGIC_NUM is mismatch\r\n");
            return FW_MAGIC_MISMATCH;
    }

    crc32 = crc32buf((buffer + img_hdr->img_offset), img_hdr->img_len);

    if(img_hdr->img_crc ^ crc32)
        return FW_CRC_MISMATCH;

    return FW_CRC_MATCH;
}

void upgrade_flash_conf(void)
{
    if(SPI_SUCCESS == spinor_write((unsigned int)&flash_conf, TempBootConfigBase, sizeof(flash_conf)))
    {
        spinor_write((unsigned int)&flash_conf, BootConfigBase, sizeof(flash_conf));
    }
}

void read_flash_conf(void)
{
    int i = 0;

    memset(&flash_conf, 0 , sizeof(flash_config_t));
    for (i = 0; i < sizeof(flash_config_t); i++)
    {
        *(((unsigned char *) &flash_conf) + i) = VPchar(((unsigned char *) BootConfigBase) + i);
    }
    /* Check config is valid or not */
    if((0xFFFFFFFF == flash_conf.upgrade_flag) ||
       (0xFFFFFFFF == flash_conf.backup_flag) ||
       (0xFFFFFFFF == flash_conf.rollback_flag ||
       (0xFFFFFFFF == flash_conf.status_flag)))
    {
        memset(&flash_conf, 0 , sizeof(flash_config_t));
        for (i = 0; i < sizeof(flash_config_t); i++)
        {
            *(((unsigned char *) &flash_conf) + i) = VPchar(((unsigned char *) TempBootConfigBase) + i);
        }
        /* Update config to BootConfig area */
        spinor_write((unsigned int)&flash_conf, BootConfigBase, sizeof(flash_conf));
    }
}

/* FUNCTION NAME:   move_image
 * PURPOSE:
 *      Move image from flash addr:A to flash addr:B and calculate CRC value
 * INPUT:
 *      src                 -- image src addrss on flash
 *      dst                 -- image dst addrss on flash
 *      size                -- image size
 * OUTPUT:
 *      None
 * RETURN:
 *      crc32               -- crc value
 * NOTES:
 *
 */
/* Check Image CRC: return 0, if value is identical */
unsigned int move_image(unsigned int src, unsigned int dst, unsigned int size)
{
    unsigned int i = 0;
    unsigned int temp_src = src;
    unsigned int temp_dst =dst;
    unsigned int temp_size = size;

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

    return crc32buf((unsigned char *) dst, size);
}


/* FUNCTION NAME:   fw_upgrade
 * PURPOSE:
 *      Process FW upgrade flow
 *          - backup FW from MainSystem to BackupSystem
 *          - upgrade FW form TempSystem to MainSystem
 *          - rollback FW from BackupSystem to MainSystem
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      status_flag         --  the status of firmware upgrade.
 * NOTES:
 *
 */
/* Process FW upgrade */
int fw_upgrade(void)
{
    unsigned int crc32 = 0;
    image_header_t *img_hdr = (image_header_t *) TempSystemBase;

    read_flash_conf();

    /* FW upgrade: start fw upgrade */
    if (flash_conf.upgrade_flag && (flash_conf.upgrade_flag < 3))
    {
        flash_conf.upgrade_flag += 1;

#if DUAL_IMAGE
        /* FW upgrade: move main image to backup area, set backup_flag to 1 */
        if ((0 == flash_conf.backup_flag))
        {
            if ((0xFFFFFFFF == flash_conf.main_img_len) || (0x0 == flash_conf.main_img_len))
            {
                flash_conf.main_img_len = SystemSize;
                flash_conf.main_img_crc = crc32buf((unsigned char *) MainSystemBase, SystemSize);
            }

            crc32 = move_image(MainSystemBase, BackupSystemBase, flash_conf.main_img_len);
            if (crc32 == flash_conf.main_img_crc)
            {
                flash_conf.backup_flag = 1;
                flash_conf.backup_img_ver.raw_ver= flash_conf.main_img_ver.raw_ver;
                flash_conf.backup_img_len = flash_conf.main_img_len;
                flash_conf.backup_img_crc = flash_conf.main_img_crc;
                upgrade_flash_conf();
            }
            else
            {
                flash_conf.upgrade_flag = 0;
                flash_conf.backup_flag = 0;
                flash_conf.rollback_flag = 0;
                flash_conf.status_flag = FW_BACKUP_FAILED;
                upgrade_flash_conf();
                return flash_conf.status_flag;
            }
        }
#endif
        /* Temp image CRC error, clear upgrade_flag */
        if (crc_check((unsigned char *) TempSystemBase))
        {
            flash_conf.upgrade_flag = 0;
            flash_conf.backup_flag = 0;
            flash_conf.rollback_flag = 0;
            flash_conf.status_flag = FW_UPGRADE_FAILED;
        }
        else
        {
            /* FW upgrade: move new image to main area, set rollback_flag to 1 and boot to system */
            crc32 = move_image((TempSystemBase + img_hdr->img_offset), MainSystemBase, img_hdr->img_len);
            if (crc32 == img_hdr->img_crc)
            {
                flash_conf.upgrade_flag = 0;
#if DUAL_IMAGE
                flash_conf.rollback_flag = 1;
#endif
                flash_conf.main_img_ver.raw_ver = img_hdr->img_version.raw_ver;
                flash_conf.main_img_len = img_hdr->img_len;
                flash_conf.main_img_crc = img_hdr->img_crc;
            }
            else
            {
#if DUAL_IMAGE
                if ((1 == flash_conf.backup_flag))
                {
                    crc32 = move_image(BackupSystemBase, MainSystemBase, flash_conf.backup_img_len);
                    if (crc32 == flash_conf.backup_img_crc)
                    {
                        flash_conf.main_img_ver.raw_ver = flash_conf.backup_img_ver.raw_ver;
                        flash_conf.main_img_len = flash_conf.backup_img_len;
                        flash_conf.main_img_crc = flash_conf.backup_img_crc;
                    }
                }
#endif
                flash_conf.status_flag = FW_UPGRADE_FAILED;
            }
        }

        upgrade_flash_conf();
    }
    else
    {
        if (flash_conf.upgrade_flag == 3)
        {
            flash_conf.upgrade_flag = 0;
#if DUAL_IMAGE
            if ((1 == flash_conf.backup_flag))
            {
                crc32 = move_image(BackupSystemBase, MainSystemBase, flash_conf.backup_img_len);
                if (crc32 == flash_conf.backup_img_crc)
                {
                    flash_conf.main_img_ver.raw_ver = flash_conf.backup_img_ver.raw_ver;
                    flash_conf.main_img_len = flash_conf.backup_img_len;
                    flash_conf.main_img_crc = flash_conf.backup_img_crc;
                }
            }
#endif
            flash_conf.status_flag = FW_UPGRADE_FAILED;
            upgrade_flash_conf();
        }

#if DUAL_IMAGE
        /* Boot failed, rollback image */
        if ((1 == flash_conf.rollback_flag))
        {
            flash_conf.status_flag = FW_BOOT_FAILED;
            crc32 = move_image(BackupSystemBase, MainSystemBase, flash_conf.backup_img_len);
            if (crc32 == flash_conf.backup_img_crc)
            {
                flash_conf.main_img_ver.raw_ver = flash_conf.backup_img_ver.raw_ver;
                flash_conf.main_img_len = flash_conf.backup_img_len;
                flash_conf.main_img_crc = flash_conf.backup_img_crc;
            }
            upgrade_flash_conf();
        }
#endif
    }

    return flash_conf.status_flag;
}

/* FUNCTION NAME:   flash_conf_task
 * PURPOSE:
 *      Read flash_conf form flash
 *      If flash is empty, set flash_conf to 0 and write back to flash.
 * INPUT:
 *      pvParameters        -- NULL
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *
 */
static void flash_conf_task(void *pvParameters)
{
    read_flash_conf();
    if ((flash_conf.upgrade_flag == 0xFFFFFFFF))
    {
        memset(&flash_conf, 0, sizeof(flash_conf));
        upgrade_flash_conf();
    }
    vTaskDelete(NULL);
}

/* Create highest priority task for check config */
void create_flash_conf_task(void)
{
        xTaskCreate(flash_conf_task,        /* The function that implements the task. */
                "flash_conf_check",         /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                configMINIMAL_STACK_SIZE,   /* The size of the stack to allocate to the task. */
                NULL,                       /* The parameter passed to the task - not used in this simple case. */
                (configMAX_PRIORITIES -1),  /* The priority assigned to the task. */
                NULL );                     /* The task handle is not required, so NULL is passed. */

    return;
}

/* Update upgrade_flag and reset to bootloader */
void update_upgrade_flag(unsigned int value)
{
    flash_conf.status_flag = FW_UPGRADE_PROCESS;
    flash_conf.upgrade_flag = value;
    upgrade_flash_conf();

    io_write32(0x10005010, 0);
    io_write32(0x100050c0, 0x80000000);
}

/* FUNCTION NAME:   WriteBuffer
 * PURPOSE:
 *      Gather the input data until we have received 4k data (g_flash_buf is full
 *      , to be more specific).
 * INPUT:
 *      input               -- the input buffer provided from application.
 *      size                -- size of the input.
 * OUTPUT:
 *      None
 * RETURN:
 *      ret                 -- indicate program pass or not.
 * NOTES:
 *      WriteBufferInit must be invoked first to indicate where to program.
 *      After each program, we need to calculate the next section to program. To
 *      ensure the correctness of flash program, we need to read back the target
 *      section (next section). In short, this function performs the followings:
 *          1. copy input data into g_flash_buf.
 *          2. mark g_data_received if any data recevied.
 *          3. program to flash if g_flash_buf is full.
 *          4. calculate next section to program.
 *          5. read back the target section after program.
 *          6. reset g_buf_walker to the begining of g_flash_buf after program.
 *          7. makr g_data_received as FALSE after program.
 */
int WriteBuffer(unsigned char *input, int size)
{
    int i = 0;
    int ret = SPI_SUCCESS;

    for (i = 0; i < size; i++)
    {
        /* copy input data into g_flash_buf */
        *g_buf_walker++ = *input++;
        /* mark data_recieved so that WriteLastBuffer() will be capable of programming
         * the remaining data in g_flash_buf. */
        g_data_received = 1;

        /* when walker pointed at the end of g_flash_buf, a program is needed.
         * Otherwise, just continue to gather the data. */
        if (g_buf_walker != (g_flash_buf + SPI_NOR_SECTOR_SIZE))
            continue;
        /* program to flash. */
        ret = spinor_write_sector((uint32_t)g_flash_buf, (uint32_t)g_flash_addr, SPI_NOR_SECTOR_SIZE);
#if 0
        printf("WB: flash_addr: 0x%x g_buf_walker: 0x%x\n", g_flash_addr, (g_buf_walker - g_flash_buf));
#endif
        if (ret != SPI_SUCCESS)
        {
            return ret;
        }

        /* increment global data.
         * 1. add flash addr by 4k to write the next 4k.
         * 2. read back the next 4k section. (to avoid the last write with data < 4k)
         * 3. Reset walker to the begining of flash_buf
         * 4. 4k has been written, signify to no data received.
         */
        g_flash_addr += SPI_NOR_SECTOR_SIZE;
        spinor_read_sector(g_flash_addr, g_flash_buf, SPI_NOR_SECTOR_SIZE);
        g_buf_walker = g_flash_buf;
        g_data_received = 0;

    }

    /* cummulate the image size. */
    image_lengh += size;

    return ret;
}

/* FUNCTION NAME:   WriteLastBuffer
 * PURPOSE:
 *      The last packet from application may not fill the entire 4k-buffer. This
 *      function is designed to program the remaining data in the g_flash_buf.
 * INPUT:
 *      N/A
 * OUTPUT:
 *      None
 * RETURN:
 *      ret                 -- indicate program pass or not.
 * NOTES:
 *      N/A
 */
int WriteLastBuffer(void)
{
    int ret = SPI_SUCCESS;
    /* if there exist any remaining data in g_flash_buf, program it. */
    if (g_data_received)
    {
        ret = spinor_write_sector((uint32_t)g_flash_buf, (uint32_t)g_flash_addr, SPI_NOR_SECTOR_SIZE);
        g_data_received = 0;
#if 0
        printf("LB: flash_addr: 0x%x g_buf_walker: 0x%x\n", g_flash_addr, (g_buf_walker - g_flash_buf));
#endif
    }

    return ret;
}


/* FUNCTION NAME:   WriteBufferInit
 * PURPOSE:
 *      Initialize global data for programming flash.
 * INPUT:
 *      flash_addr          -- flash address to be programmed.
 * OUTPUT:
 *      None
 * RETURN:
 *      N/A
 * NOTES:
 *      This function needs to fulfill the read-back part of the old spinor_write.
 *      Futhermore, to indicate the next address to program is essential for
 *      WriteBuffer() to function correctly. In short, this function must perform
 *      the following:
 *          1. g_flash_addr will point at the section to be programmed.
 *          2. Read back the target section into g_flash_buf.
 *          3. Set g_buf_walker, which indicates the next address to program.
 */
void WriteBufferInit(unsigned char *flash_addr)
{
    /* 1. Initialize where to program by 4k-aligned address (mathematically floor). */
    g_flash_addr = (unsigned char *)((unsigned int)flash_addr & (~(SPI_NOR_SECTOR_SIZE - 1)));
    g_data_received = 0;
    image_lengh = 0;

    /* 2. Readback first sector. */
    spinor_read_sector(g_flash_addr, g_flash_buf, SPI_NOR_SECTOR_SIZE);

    /* 3. Calculate the first address to program. */
    g_buf_walker = g_flash_buf + ((unsigned int)flash_addr % SPI_NOR_SECTOR_SIZE);

#if 0
    printf("WI: flash_addr: 0x%x g_buf_walker: 0x%x\n", g_flash_addr, (g_buf_walker - g_flash_buf));
#endif
}

/* FUNCTION NAME:   spinor_write_sector_test
 * PURPOSE:
 *      write flash available test to make sure the good quality of the flash device
 * INPUT:
 *      flash_dst_addr          -- destination address of flash
 *      run_time                -- how many times(SPI_AVL_TIMES_BASE*times) want to test
 * OUTPUT:
 *      None
 * RETURN:
 *      SPI_AVL_TEST_SUCCESS    -- FLASH AVAILABLE TEST PASS
 *      SPI_AVL_TEST_FAIL       -- FLASH AVAILABLE TEST FAIL
 * NOTES:
 *
 */
SPI_AVL_TEST_T spinor_write_sector_test(unsigned int flash_dst_addr, unsigned int run_time)
{
    unsigned int time_cnt = 0, flash_addr, i, j;
    unsigned char write_buf_data, read_data;
    int random_num;
    while(time_cnt < run_time*SPI_AVL_TIMES_BASE)
    {
        printf("time_cnt = %d\n", time_cnt);
        /*
         *0 : write sector 0 [0~4095],
         *1 : write sector 0 [2048~4095] & sector 1 [0~2048]
         *2 : write sector 1 [0~4095],
         *3 : write sector 1 [2048~4095] & sector 2 [0~2048]
         *4 : write sector 2 [0~4095],
         *5 : write sector 2 [2048~4095] & sector 3 [0~2048]
         *6 : write sector 3 [0~4095]
         */
        for(i = 0; i < 7; i++)
        {
            /*set random data to random buffer*/
            for(j = 0; j < SPI_NOR_SECTOR_SIZE; j=j+4)
            {
                random_num = rand();
                random_buf[j] = random_num & 0xff;
                random_buf[j+1] = (random_num>>8) & 0xff;
                random_buf[j+2] = (random_num>>16) & 0xff;
                random_buf[j+3] = (random_num>>24) & 0xff;
            }

            flash_addr = flash_dst_addr + ((i*SPI_NOR_SECTOR_SIZE)>>1);
            printf("flash_addr = %x\n", flash_addr);

            /*write random_buf to flash device*/
            spinor_write((unsigned int)&random_buf, flash_addr, SPI_NOR_SECTOR_SIZE);

            /*
             *before cpu read flash address map, it should set the cache be invalid
             *to ensure the read data is read from flash
             */
            nds32_dcache_invalidate_range(flash_addr, flash_addr + SPI_NOR_SECTOR_SIZE);

            /*compare random_buf and flash data*/
            for (j = 0; j < SPI_NOR_SECTOR_SIZE; j++)
            {
                write_buf_data = random_buf[j];
                read_data = VPchar(((unsigned char *)flash_addr + j));

                if(write_buf_data != read_data)
                {
                    printf("\n0x%08x = %02x\n", (unsigned int)&random_buf[j], (unsigned int)write_buf_data);
                    printf("0x%08x = %02x\n", flash_addr + j, read_data);
                    printf("SPI NOR FLASH AVAILABLE TEST FAIL\n");
                    return SPI_AVL_TEST_FAIL;
                }
            }
        }
        time_cnt++;
    }
    printf("SPI NOR FLASH AVAILABLE TEST SUCCESS\n");
    return SPI_AVL_TEST_SUCCESS;
}

/* FUNCTION NAME:   spinor_write_init
 * PURPOSE:
 *      copy the SPI driver image from flash to DMEM
 * INPUT:
 *      none
 * OUTPUT:
 *      None
 * RETURN:
 *      SPI_SUCCESS             -- success
 *      SPI_ERR_INIT            -- SPI driver image size is greater than
 *                                 the memory layout of driver image
 * NOTES:
 *
 */
SPI_ERR_T spinor_write_init(void)
{
    extern unsigned char __dmemimg_lmastart, __dmemimg_vmastart, __dmemimg_vmaend;
    memcpy((unsigned char*)&(__dmemimg_vmastart),
        (unsigned char*)&(__dmemimg_lmastart),
        (unsigned int)(&(__dmemimg_vmaend)-&(__dmemimg_vmastart)));
    nds32_dcache_flush();
    nds32_icache_flush();

    spi_write_inited = 1;

    return SPI_SUCCESS;
}

/* FUNCTION NAME:   spinor_write
 * PURPOSE:
 *      write data to spi nor flash, and the write
 *      maximun size is 4096 bytes
 * INPUT:
 *      src_addr                -- the source data which would be write into flash
 *      flash_dst_addr          -- destination address of flash
 *      src_data_size           -- how many bytes need to be writen
 * OUTPUT:
 *      None
 * RETURN:
 *      SPI_SUCCESS             -- success
 *      SPI_ERR_INIT            -- SPI driver is not initial
 *      SPI_ERR_WR_SIZE         -- write size is greater than 4096kB
 *      SPI_ERR_ILLEGAL_ADDR    -- write data into flash illegal address
 * NOTES:
 *
 */
SPI_ERR_T spinor_write(unsigned int src_addr, unsigned int flash_dst_addr, unsigned int src_data_size)
{
    SPI_ERR_T ret = SPI_SUCCESS;
    if (!spi_write_inited)
    {
        return -SPI_ERR_INIT;
    }

    if (src_data_size > SPI_NOR_SECTOR_SIZE)
    {
        return -SPI_ERR_WR_SIZE;
    }

    WriteBufferInit((uint8_t *)flash_dst_addr);
    ret = WriteBuffer((uint8_t *)src_addr, src_data_size);
    if(ret != SPI_SUCCESS)
    {
        return ret;
    }
    ret = WriteLastBuffer();

    return ret;
}

void spinor_write_bootloader(int image_size)
{
    if(image_size < 0)
    {
        printf("\r\nxmodem received error[%d]\n", image_size);
    }
    else if(image_size == 0)
    {
        printf("\r\nbootloader image is empty\n");
    }
    else if(image_size > BootLoaderSize)
    {
        printf("\r\nbootloader image is oversize(maximum is 0x%x bytes)\n", BootLoaderSize);
    }
    else
    {
        taskDISABLE_INTERRUPTS();
        _spi_nor_entry((TempSystemBase & 0x1fffffff), BootLoaderBase & 0x1ffff000, image_size, 1);
        taskENABLE_INTERRUPTS();
    }
}
