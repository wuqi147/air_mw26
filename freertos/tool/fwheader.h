#ifndef __FWHEADER_H__
#define __FWHEADER_H__

#define FW_MAGIC_NUM        "ARHT"
#define FW_VERISON_MAJOR    (0)
#define FW_VERISON_MINOR    (0)
#define FW_VERISON_BUILD    (1)

typedef union
{
    struct image_version
    {
        unsigned char major;
        unsigned char minor;
        unsigned short build;
    } img_ver;
    uint32_t raw_ver;
} version_t;

/*****************************************************************************
 * Image Header
 ****************************************************************************/
typedef struct image_header
{
    uint32_t    magic_num;      /* FW_MAGIC_NUM */
    uint32_t    img_model;      /* FW image model */
    version_t   img_version;    /* FW image version */
    uint32_t    img_offset;     /* FW image offset */
    uint32_t    img_len;        /* FW image length */
    uint32_t    img_crc;        /* FW image crc32 */
    uint32_t    sig_offset;     /* FW signature offset */
    uint32_t    sig_len;        /* FW signature length */
    uint32_t    reserved[56];   /* Reserved to 256B */
} image_header_t;

#endif /* __FWHEADER_H__ */
