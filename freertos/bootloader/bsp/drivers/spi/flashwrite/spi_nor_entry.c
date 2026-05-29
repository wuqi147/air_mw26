#include "flash_util.h"
#include "spi_controller.h"
#include "spinorwrite.h"

/*****************************************************************************
 * SPI Controller APIs
 ****************************************************************************/
#define _SPI_NOR_GET_ADDRESSING_MODE            SPI_CONTROLLER_Get_Addressing_Mode
#define _SPI_NOR_ENABLE_MANUAL_MODE             SPI_CONTROLLER_Enable_Manual_Mode
#define _SPI_NOR_ENABLE_AUTO_MODE               SPI_CONTROLLER_Enable_Auto_Mode
#define _SPI_NOR_CHIP_SELECT_LOW                SPI_CONTROLLER_Chip_Select_Low
#define _SPI_NOR_CHIP_SELECT_HIGH               SPI_CONTROLLER_Chip_Select_High
#define _SPI_NOR_WRITE_ONE_BYTE                 SPI_CONTROLLER_Write_One_Byte
#define _SPI_NOR_WRITE_NBYTE                    SPI_CONTROLLER_Write_NByte
#define _SPI_NOR_READ_NBYTE                     SPI_CONTROLLER_Read_NByte

/*****************************************************************************
 * Flash instructions
 ****************************************************************************/
#define _SPI_NOR_OP_RD_ID                       (0x9F)  /* Read ID */
#define _SPI_NOR_OP_RD_STATUS1                  (0x05)  /* Read Status 1 */
#define _SPI_NOR_OP_RD_STATUS3                  (0x15)  /* Read Status 3 */
#define _SPI_NOR_OP_RD_BANK                     (0x16)  /* Read bank register */
#define _SPI_NOR_OP_RD_NONVOL_CONF              (0xB5)  /* Read nonvolatile configuration register */
#define _SPI_NOR_OP_WR_STATUS1                  (0x01)  /* Write Status 1 */
#define _SPI_NOR_OP_WR_STATUS3                  (0x11)  /* Read Status 3 */
#define _SPI_NOR_OP_WR_NONVOL_CONF              (0xB1)  /* Write nonvolatile configuration register */
#define _SPI_NOR_OP_WR_BANK                     (0x17)  /* Write bank register */
#define _SPI_NOR_OP_WRITE_EN                    (0x06)  /* Write Enable */
#define _SPI_NOR_OP_4BYTE_MODE_EN               (0xB7)  /* 4-Byte Mode Enable */
#define _SPI_NOR_OP_4BYTE_MODE_DS               (0xE9)  /* 4-Byte Mode Disable */
#define _SPI_NOR_OP_WRITE_DS                    (0x04)  /* Write Disable */
#define _SPI_NOR_OP_WRITE_SINGLE                (0x02)  /* Write data of SPI NOR chip, single speed */
#define _SPI_NOR_OP_4BYTE_WRITE_SINGLE          (0x12)  /* 4 byte mode Write data of SPI NOR chip, single speed */
#define _SPI_NOR_OP_READ_SINGLE                 (0x03)  /* Read data of SPI NOR chip, single speed */
#define _SPI_NOR_OP_4BYTE_READ_SINGLE           (0x13)  /* 4 byte mode Read data of SPI NOR chip, single speed */
#define _SPI_NOR_OP_ERASE_BLK                   (0xD8)  /* Eease SPI NOR block */
#define _SPI_NOR_OP_ERASE_SEC                   (0x20)  /* Eease SPI NOR SECTOR */
#define _SPI_NOR_OP_4BYTE_ERASE_BLK             (0xDC)  /* 4 byte mode Eease SPI NOR block */
#define _SPI_NOR_OP_4BYTE_ERASE_SEC             (0x21)  /* 4 byte mode Eease SPI NOR sector */
#define _SPI_NOR_OP_READ_SFDP                   (0x5A)  /* read SFDP(Serial Flash Discoverable Parameter)*/

/*****************************************************************************
 * Flash Status Register-1
 ****************************************************************************/
#define _SPI_NOR_STATUS_WIP                     (0x01)  /* Write-In-Progress */
#define _SPI_NOR_STATUS_WEL                     (0x02)  /* Write-Enable-Latch */

/*****************************************************************************
 * Flash SFDP information
 ****************************************************************************/
#define _SFDP_SIGNATURE                         (0x50444653)
#define _SFDP_HEADER                            (0x00)
#define _SFDP_1ST_PARAM_HEADER_PTP              (0x0C)
#define _SFDP_DWORD_SIZE                        (0x04)

/*****************************************************************************
 * useful defines
 ****************************************************************************/
#define _FLASH_SIZE_128M                        (0xFFFFFF)
#define _FLASH_SIZE_256M                        (0x1FFFFFF)
#define _FLASH_SIZE_512M                        (0x3FFFFFF)
#define _SPI_NOR_PAGE_SIZE                      (0x100)
#define _SPI_NOR_SECTOR_SIZE                    (0x1000)
#define _3BYTE_ADDRESS_MODE                     (0)
#define _4BYTE_ADDRESS_MODE                     (1)

#define RST_CTRL1                               (0x100050c0)

struct SPI_NOR_PARAM_S
{
    uint32_t src_addr;
    uint32_t flash_dst_addr;
    uint32_t src_data_size;
    uint32_t flash_size;        /*flash capacity or density*/
    uint8_t addressing_mode;    /*0: 3-byte mode, 1: 4-byte mode*/
    uint8_t error_code;         /*this error code return back to RTOS*/
};

/*Initialization of parameter*/
struct SPI_NOR_PARAM_S spi_nor_param =
{
    .flash_size = 0xFFFFFFFF,
};

static uint32_t _getFlashSize(void);

static void _spiflash_write_status_register1(uint8_t data);
static void _spiflash_read_status_register1(uint8_t *status);
static void _spiflash_disable_write(void);
static void _spiflash_read_id(uint8_t *id, uint32_t len);
static void _spiflash_read_sfdp(uint32_t offset, uint8_t *p_sfdp_data, uint8_t len);
static void _spiflash_enable_write(void);
static void _spiflash_erase_sector(uint32_t addr);
static void _spiflash_write_page(uint8_t *p_data, uint32_t addr, uint32_t len);
static void _spiflash_write_sector(uint8_t *p_data, uint32_t addr, uint32_t len);
static void _spiflash_read_sector(uint8_t *p_data, uint32_t addr, uint32_t len);

static void _spiflash_write_status_register1(uint8_t data)
{
    _spiflash_enable_write();
    _SPI_NOR_CHIP_SELECT_LOW();
    _SPI_NOR_WRITE_ONE_BYTE(_SPI_NOR_OP_WR_STATUS1);
    _SPI_NOR_WRITE_ONE_BYTE(data);
    _SPI_NOR_CHIP_SELECT_HIGH();
    _spiflash_disable_write();
}

static void _spiflash_read_status_register1(uint8_t *status)
{
    _SPI_NOR_CHIP_SELECT_LOW();
    _SPI_NOR_WRITE_ONE_BYTE(_SPI_NOR_OP_RD_STATUS1);
    _SPI_NOR_READ_NBYTE(status, 0x1, SPI_CONTROLLER_SPEED_SINGLE);
    _SPI_NOR_CHIP_SELECT_HIGH();
}

static void _spiflash_disable_write(void)
{
    uint8_t status = 0;
    do
    {
        _spiflash_read_status_register1(&status);
    }
    while (status & _SPI_NOR_STATUS_WIP);

    _SPI_NOR_CHIP_SELECT_LOW();
    _SPI_NOR_WRITE_ONE_BYTE(_SPI_NOR_OP_WRITE_DS);
    _SPI_NOR_CHIP_SELECT_HIGH();
}

static void _spiflash_read_id(uint8_t *id, uint32_t len)
{
    _SPI_NOR_CHIP_SELECT_LOW();
    _SPI_NOR_WRITE_ONE_BYTE(_SPI_NOR_OP_RD_ID);
    _SPI_NOR_READ_NBYTE(id, len, SPI_CONTROLLER_SPEED_SINGLE);
    _SPI_NOR_CHIP_SELECT_HIGH();
}

static void _spiflash_read_sfdp(uint32_t offset, uint8_t *p_sfdp_data, uint8_t len)
{
    _SPI_NOR_CHIP_SELECT_LOW();
    _SPI_NOR_WRITE_ONE_BYTE(_SPI_NOR_OP_READ_SFDP);
    _SPI_NOR_WRITE_ONE_BYTE((offset >> 16)&(0xff));
    _SPI_NOR_WRITE_ONE_BYTE((offset >> 8)&(0xff));
    _SPI_NOR_WRITE_ONE_BYTE(offset&(0xff));
    _SPI_NOR_WRITE_ONE_BYTE(0x00);  /*dummy byte : 0x00, this value is don't care*/
    _SPI_NOR_READ_NBYTE(p_sfdp_data, len, SPI_CONTROLLER_SPEED_SINGLE);
    _SPI_NOR_CHIP_SELECT_HIGH();
}

static void _spiflash_enable_write(void)
{
    uint8_t status = 0;
    do
    {
        _SPI_NOR_CHIP_SELECT_LOW();
        _SPI_NOR_WRITE_ONE_BYTE(_SPI_NOR_OP_WRITE_EN);
        _SPI_NOR_CHIP_SELECT_HIGH();
        _spiflash_read_status_register1(&status);
    }
    while(!(status & _SPI_NOR_STATUS_WEL));
}

static void _spiflash_erase_sector(uint32_t addr)
{
    _spiflash_enable_write();
    _SPI_NOR_CHIP_SELECT_LOW();
    if (spi_nor_param.addressing_mode == _3BYTE_ADDRESS_MODE)
    {
        _SPI_NOR_WRITE_ONE_BYTE( _SPI_NOR_OP_ERASE_SEC );
    }
    else
    {
        _SPI_NOR_WRITE_ONE_BYTE( _SPI_NOR_OP_ERASE_SEC );
        _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 24 ) &(0xff)) );
    }
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 16 ) &(0xff)) );
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 8 ) &(0xff)) );
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 0 ) &(0xff)) );
    _SPI_NOR_CHIP_SELECT_HIGH();
    _spiflash_disable_write();
}

static void _spiflash_write_page(uint8_t *p_data, uint32_t addr, uint32_t len)
{
    _spiflash_enable_write();
    _SPI_NOR_CHIP_SELECT_LOW();
    if (spi_nor_param.addressing_mode == _3BYTE_ADDRESS_MODE)
    {
        _SPI_NOR_WRITE_ONE_BYTE( _SPI_NOR_OP_WRITE_SINGLE );
    }
    else
    {
        _SPI_NOR_WRITE_ONE_BYTE( _SPI_NOR_OP_WRITE_SINGLE );
        _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 24 ) &(0xff)) );
    }
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 16 ) &(0xff)) );
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 8 ) &(0xff)) );
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 0 ) &(0xff)) );
    _SPI_NOR_WRITE_NBYTE(p_data, len, SPI_CONTROLLER_SPEED_SINGLE);
    _SPI_NOR_CHIP_SELECT_HIGH();
    _spiflash_disable_write();
}

static void _spiflash_write_sector(uint8_t *p_data, uint32_t addr, uint32_t len)
{
    uint32_t tmp_len = 0;
    SPI_DEBUG("write flash addr = %x\n", addr);

    while (len)
    {
        if (len > _SPI_NOR_PAGE_SIZE)
        {
            tmp_len = _SPI_NOR_PAGE_SIZE;
        }
        else
        {
            tmp_len = len;
        }
        _spiflash_write_page(p_data, addr, tmp_len);
        p_data += tmp_len;
        addr += tmp_len;
        len -= tmp_len;
    }
}

static void _spiflash_read_sector(uint8_t *p_data, uint32_t addr, uint32_t len)
{
    _SPI_NOR_CHIP_SELECT_LOW();
    if (spi_nor_param.addressing_mode == _3BYTE_ADDRESS_MODE)
    {
        _SPI_NOR_WRITE_ONE_BYTE( _SPI_NOR_OP_READ_SINGLE );
    }
    else
    {
        _SPI_NOR_WRITE_ONE_BYTE( _SPI_NOR_OP_READ_SINGLE );
        _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 24 ) &(0xff)) );
    }
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 16 ) &(0xff)) );
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 8 ) &(0xff)) );
    _SPI_NOR_WRITE_ONE_BYTE( ((addr >> 0 ) &(0xff)) );
    _SPI_NOR_READ_NBYTE(p_data, len, SPI_CONTROLLER_SPEED_SINGLE);
    _SPI_NOR_CHIP_SELECT_HIGH();
}

/* FUNCTION NAME:   _getFlashSize
 * PURPOSE:
 *      three ways to get flash size, the order is:
 *      1.read SFDP table
 *      2.read RDID
 *      3.current addressing mode
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      flash capacity
 * NOTES:
 *
 */
static uint32_t _getFlashSize(void)
{
    uint8_t capacity, ChipID[3];
    uint32_t readvalue, offset, tmp_size, flash_size;

    _spiflash_read_sfdp(_SFDP_HEADER, (uint8_t *)&readvalue, _SFDP_DWORD_SIZE);

    /*check SFDP signature, 0x53 = 'S', 0x46 = 'F', 0x44 = 'D', 0x50 = 'P'*/
    if(readvalue == _SFDP_SIGNATURE)
    {
        /*read parameter table pointer(PTP)*/
        _spiflash_read_sfdp(_SFDP_1ST_PARAM_HEADER_PTP, (uint8_t *)&readvalue, _SFDP_DWORD_SIZE-1);
        SPI_DEBUG("SFDP:signature match\n");
        SPI_DEBUG("PTP = %x\n", readvalue);
         /*2 nd DWORD*/
        offset = (readvalue & 0xFFFFFF) + _SFDP_DWORD_SIZE;

        _spiflash_read_sfdp(offset, (uint8_t *)&readvalue, _SFDP_DWORD_SIZE);
        flash_size = (readvalue>>3);
    }

    else
    {
        SPI_DEBUG("read RDID\n");
        /*if SFDP signature is not correct, then read id to get flash capactity*/
        _spiflash_read_id(ChipID, sizeof(ChipID));
        /*
         * ChipID[7:0] = Manufacturer ID
         * ChipID[15:8]= Memory type
         * ChipID[23:16] = Memory capacity
         */
        capacity = ChipID[2];
        /*
         *11:1Mb,   12:2Mb,   13:4Mb,   14:8Mb,   15:16Mb
         *16:32Mb,  17:64Mb,  18:128Mb, 19:256Mb, 1a:512Mb
         */
        if((capacity > 0x10) && (capacity < 0x1B))
        {
            SPI_DEBUG("RDID:flash density range : 1Mb~512Mb\n");
            tmp_size = 1;
            while(capacity)
            {
                tmp_size = tmp_size << 1;
                capacity--;
            }
            flash_size = (tmp_size-1);
        }
        else
        {
            SPI_DEBUG("RDID:out of flash density range\n");
            /*set flash_size = 512Mb*/
            flash_size = _FLASH_SIZE_512M;
        }
    }

    SPI_DEBUG("flash_size = %x\n", flash_size);

    /*flash capacity = 256Mb or 512Mb, address mode = 3-byte, write address > 0xFFFFFF*/
    if((flash_size > _FLASH_SIZE_128M) && (spi_nor_param.addressing_mode == _3BYTE_ADDRESS_MODE))
    {
        SPI_DEBUG("run in 3-byte mode, set the flash density as 128Mb\n");
        flash_size = _FLASH_SIZE_128M;
    }

    return flash_size;

}

SPI_ERR_T _spi_nor_entry(uint32_t src, uint32_t dst, uint32_t size, uint32_t op_mode)
{
    uint32_t raddr;   /*src address*/
    uint32_t waddr;   /*write address*/
    uint32_t wsize;   /*write size*/
    uint32_t eaddr;   /*erase address*/

    spi_nor_param.src_addr = src;
    spi_nor_param.flash_dst_addr = dst;
    spi_nor_param.src_data_size = size;

    raddr = spi_nor_param.src_addr;
    waddr = spi_nor_param.flash_dst_addr;
    wsize = spi_nor_param.src_data_size;

    /*set spi controller into manual mode*/
    _SPI_NOR_ENABLE_MANUAL_MODE();

    /*set flash all block to be unprotected before write and erase operation*/
    _spiflash_write_status_register1(0);

    /*get current address mode*/
    _SPI_NOR_GET_ADDRESSING_MODE(&spi_nor_param.addressing_mode);

    /*get capacity(unit:byte) of flash device, 0xFFFFFFFF is a default value : means the driver has not get the flash capacity yet*/
    if(spi_nor_param.flash_size == 0xFFFFFFFF)
    {
        SPI_DEBUG("first get flash size\n");
        spi_nor_param.flash_size = _getFlashSize();
    }
    /*check if the write address is legal*/
    if(waddr > spi_nor_param.flash_size)
    {
        /*set spi controller back to auto mode and return fail*/
        SPI_DEBUG("SPI driver error : SPI_ERR_ILLEGAL_ADDR\n");
        _SPI_NOR_ENABLE_AUTO_MODE();
        return -SPI_ERR_ILLEGAL_ADDR;
    }

    /*sector erase*/
    for(eaddr = waddr; eaddr < (waddr + wsize) ; eaddr = (eaddr + _SPI_NOR_SECTOR_SIZE))
    {
        _spiflash_erase_sector(eaddr);
    }

    /*sector write*/
    if((op_mode == 1) && (wsize > _SPI_NOR_SECTOR_SIZE))
    {
        /*update bootloader*/
        uint32_t temp_dst;
        uint32_t temp_src;
        for(temp_src = raddr, temp_dst = waddr; temp_src < (raddr + wsize) ; temp_src+=_SPI_NOR_SECTOR_SIZE, temp_dst+=_SPI_NOR_SECTOR_SIZE)
        {
            _spiflash_read_sector(g_flash_buf, temp_src, _SPI_NOR_SECTOR_SIZE);
            _spiflash_write_sector(g_flash_buf, temp_dst, _SPI_NOR_SECTOR_SIZE);
        }
        onram_io_write32(RST_CTRL1, 0x80000000);
    }
    else
    {
        _spiflash_write_sector((uint8_t *)raddr, waddr, wsize);
    }
    /*set spi controller back to auto mode and return success*/
    _SPI_NOR_ENABLE_AUTO_MODE();

    return SPI_SUCCESS;
}
