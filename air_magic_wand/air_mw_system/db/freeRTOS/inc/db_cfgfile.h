/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2024 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:   db_cfgfile.h
 * PURPOSE:
 *      Database configuration file handling functions
 * NOTES:
 */

#ifndef DB_CFG_FILE_H
#define DB_CFG_FILE_H

/* INCLUDE FILE DECLARATIONS
*/
#include "mw_error.h"
#include "mw_types.h"
#include "mw_utils.h"
#include "osapi_string.h"
#include "core_json.h"
#include "db_api.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DB_MIN_ENTRY_LEN    (7)     /* {"A":0} */
/* config file EOF */
#define DB_ETX              (0x03)  /* End of text */

#define DB_TEST_TYPE        (0)

typedef enum
{
    DB_CFG_MINOR_RANGE_MIN0 = 0,
    DB_CFG_MINOR_RANGE_MIN1,
    DB_CFG_MINOR_RANGE_MIN2,
    DB_CFG_MINOR_RANGE_MIN3,
    DB_CFG_MINOR_RANGE_MIN4 = 4,
    DB_CFG_MINOR_RANGE_MIN6 = 6,
}DB_CFG_MINOR_RANGE_MINI_T;

typedef enum
{
    DB_CFG_MINOR_RANGE1_MAX = 1,
    DB_CFG_MINOR_RANGE2_MAX = 2,
    DB_CFG_MINOR_RANGE3_MAX = 3,
    DB_CFG_MINOR_RANGE4_MAX = 4,
    DB_CFG_MINOR_RANGE7_MAX = 7,
    DB_CFG_MINOR_RANGE10_MAX = 10,
    DB_CFG_MINOR_RANGE11_MAX = 11,
    DB_CFG_MINOR_RANGE30_MAX = 30,
    DB_CFG_MINOR_RANGE40_MAX = 40,
    DB_CFG_MINOR_RANGE100_MAX = 100,
    DB_CFG_MINOR_RANGE240_MAX = 240,
    DB_CFG_MINOR_RANGE255_MAX = 255,
    DB_CFG_MINOR_RANGE4K_MAX  = 4095,
    DB_CFG_MINOR_RANGE61440_MAX = 61440,
    DB_CFG_MINOR_RANGE80K_MAX = 80000,
    DB_CFG_MINOR_RANGE1M_MAX = 1000000,
    DB_CFG_MINOR_RANGE200M_MAX = 200000000,
    DB_CFG_MINOR_RANGE_MAX = 0xFFFFFFFF,
    DB_CFG_MINOR_RANGESPD_MAX = 31,
    DB_CFG_MINOR_RANGEVID_MAX = 4094,
    DB_CFG_MINOR_RANGEWIEGHT_MAX = 128,
    DB_CFG_MINOR_RANGEDHCP_MAX = 255,
    DB_CFG_MINOR_RANGE65535_MAX = 65535,
}DB_CFG_MINOR_RANGE_MAX_T;

typedef enum
{
    DB_CFG_MINOR_RANGE_STEP16 = 16,
    DB_CFG_MINOR_RANGE_STEP4096 = 4096,
}DB_CFG_MINOR_RANGE_STEP_T;

/* Config type */
typedef enum
{
    DB_CFG_MINOR_STRING = 0,            /* 0:Do not check */
    DB_CFG_MINOR_MAC,                   /* 1:MAC address */
    DB_CFG_MINOR_VER,                   /* 2:Version string */
    DB_CFG_MINOR_NAME,                  /* 3:Login name */
    DB_CFG_MINOR_PASSWD,                /* 4:Password */
    DB_CFG_MINOR_ADDR,                  /* 5:Ethernet address */
    DB_CFG_MINOR_RANGE1,                /* 6:0:1 */
    DB_CFG_MINOR_RANGE2,                /* 7:0:2 */
    DB_CFG_MINOR_RANGE3,                /* 8:0:3 */
    DB_CFG_MINOR_RANGE4,                /* 9:0:4 */
    DB_CFG_MINOR_RANGE7,                /* 10:0:7 */
    DB_CFG_MINOR_RANGE11,               /* 11:0:11 */
    DB_CFG_MINOR_RANGE100,              /* 12:0:100 */
    DB_CFG_MINOR_RANGE255,              /* 13:0:255 */
    DB_CFG_MINOR_RANGE80K,              /* 14:0:80000 */
    DB_CFG_MINOR_RANGE1M,               /* 15:0:1000000 */
    DB_CFG_MINOR_RANGEMAX,              /* 16:0:0xFFFFFFFF */
    DB_CFG_MINOR_SPD_ABILITY,           /* 17:1:31 */
    DB_CFG_MINOR_PORTMAP,               /* 18:Based on MAX_PORT_NUM */
    DB_CFG_MINOR_PORTNUM,               /* 19:Based on MAX_PORT_NUM */
    DB_CFG_MINOR_PVID,                  /* 20:1:4094 */
    DB_CFG_MINOR_VID,                   /* 21:0:4094 */
    DB_CFG_MINOR_WEIGHT,                /* 22:1:128 */
    DB_CFG_MINOR_DHCPSNPC,              /* 23:ctype - 0,255 */
    DB_CFG_MINOR_DHCPSNPR,              /* 24:rtype - 0,1,255 */
    DB_CFG_MINOR_RSTP_FORCE_VER,        /* 25: force protocol version - 0,2 */
    DB_CFG_MINOR_RSTP_FORWARD_DELAY,    /* 26: forward delay - 4:30 */
    DB_CFG_MINOR_RSTP_MAX_AGE,          /* 27: max age - 6:40 */
    DB_CFG_MINOR_RSTP_TX_HOLD_CNT,      /* 28: transmit hold count - 1:10 */
    DB_CFG_MINOR_RSTP_PRIORITY,         /* 29: priority - 0:61440,
                                               in steps of 4096 */
    DB_CFG_MINOR_RSTP_PORT_PRIORITY,    /* 30: port priority - 0:240,
                                               in steps of 16 */
    DB_CFG_MINOR_RSTP_PORT_COST,        /* 31: port cost - 1:200000000 */
    DB_CFG_MINOR_RSTP_PORT_TC_INTERVAL, /* 32: tc interval - 1:10 */
    DB_CFG_MINOR_RESERVE,               /* 33:reserve */
    DB_CFG_MINOR_RANGE65535,            /* 34:0:65535*/
    DB_CFG_MINOR_TIMEZONE,              /* 35:timezone : -12:15*/
    DB_CFG_MINOR_PORT_SETTINGS,         /* 36: port settings: port settings: bit31 port mode(0: AN; 1 Force);
                                               other bits PORT_SETTINGS_FLAGS_X in port_uilts.h */
    DB_CFG_MINOR_TRUNK_MEMBERS,         /* 37: trunk member:PORTMAP & trunk mode:0-2 */
    DB_CFG_MINOR_OUI,                   /* 38: OUI */
    DB_CFG_MINOR_MSTP_INSTANCE,         /* 38: mstp instance:1-4095 */
#ifdef AIR_SUPPORT_IPV6
    DB_CFG_MINOR_IPV6_ADDR,             /* 39: ipv6 address */
#endif
    DB_CFG_MINOR_MSTP_REGION_NAME,      /* 40: region name */
    DB_CFG_MINOR_LAST
}DB_CFG_MINOR_T;

/* MACRO FUNCTION DECLARATIONS
*/
#define DB_APPEND_CHAR(buf, bufsize, data)    \
    do                                        \
    {                                         \
        osapi_strcat((buf), (data));          \
        bufsize++;                            \
    } while (0)

#define CFGTYPE(major, minor)   (major | (minor << 3))
#define CFGMAJOR(type)          (type & 0x07)
#define CFGMINOR(type)          (type >> 3)

/* DATA TYPE DECLARATIONS
*/
typedef struct DB_CFG_S
{
    C8_T ptr_name[DB_MAX_KEY_SIZE + 1];
    UI16_T type;
}DB_CFG_T;

typedef MW_ERROR_NO_T (*printHandle)(UI8_T *f_idx, UI8_T *fields_size, UI8_T *ptr_raw, UI8_T *ptr_cfgtext, UI16_T *data_size);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* The Function handle to print config entry */
MW_ERROR_NO_T
db_cfgfile_saveField(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI8_T e_idx,
    UI8_T       *ptr_raw,
    UI8_T       *ptr_cfgtext,
    UI16_T      *ptr_data_size);

/* FUNCTION NAME: db_cfgfile_parsing
 * PURPOSE:
 *      parse the configuration file to DB raw data
 *
 * INPUT:
 *      req           -- Current handled request type
 *      ptr_fdata     -- A pointer to the config tmp buffer
 *      field_size    -- The field size
 *      ptr_cfgtext   -- A pointer to the config text stream
 *      data_size     -- The config text size
 *      flag          -- The type of parsing method
 *
 * OUTPUT:
 *      ptr_raw       -- A pointer to temp db raw data
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER_
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_parsing(
    DB_REQUEST_TYPE_T req,
    DB_PAYLOAD_T *ptr_fdata,
    UI8_T field_size,
    void *ptr_raw,
    void *ptr_cfgtext,
    UI16_T data_size,
    UI8_T flag);

/* FUNCTION NAME: db_cfgfile_parsing_ext
 * PURPOSE:
 *      parse the configuration file to DB raw data
 *
 * INPUT:
 *      req           -- Current handled request type
 *      ptr_data      -- A pointer to the config tmp buffer
 *      field_size    -- The field size
 *      ptr_cfgtext   -- A pointer to the config text stream
 *      data_size     -- The config text size
 *      flag          -- The type of parsing method
 *
 * OUTPUT:
 *      ptr_raw       -- A pointer to temp db raw data
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER_
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_parsing_ext(
    DB_REQUEST_TYPE_T req,
    DB_PAYLOAD_T *ptr_data,
    UI8_T field_size,
    void *ptr_raw,
    void *ptr_cfgtext,
    UI16_T data_size,
    UI8_T flag);

/* FUNCTION NAME: db_cfgfile_printTable
 * PURPOSE:
 *      print table name in JSON format
 *
 * INPUT:
 *      t_idx           -- The table index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_cfgtext     -- The output json string
 *      data_size       -- The output json string length
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      Append the key string to the end of the buffer
 *
 */
MW_ERROR_NO_T
db_cfgfile_printTable(
    TABLES_T t_idx,
    UI8_T *ptr_cfgtext,
    UI16_T *data_size);

/* FUNCTION NAME: db_cfgfile_searchTable
 * PURPOSE:
 *      find the table index in JSON file
 *
 * INPUT:
 *      pptr_cfgtext    -- A dpointer to the config text
 *      t_idx           -- Current table index
 *
 * OUTPUT:
 *      pptr_cfgtext    -- A dpointer to the config text
 *      t_idx           -- Founded table index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Will move the pointer to the data start if table founded
 *
 */
MW_ERROR_NO_T
db_cfgfile_searchTable(
    UI8_T **pptr_cfgtext,
    UI8_T *t_idx);

/* FUNCTION NAME: db_cfgfile_bufAppend
 * PURPOSE:
 *      Append the value to config text sting safely.
 *
 * INPUT:
 *      type            -- The type of the data
 *      value           -- a pointer to the data value
 *
 * OUTPUT:
 *      ptr_cfgtext     -- The output json string
 *      data_size       -- The output json string length
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Append data to the end of the buffer
 *
 */
MW_ERROR_NO_T
db_cfgfile_bufAppend(
    C8_T *ptr_cfgtext,
    UI16_T *data_size,
    JSONTypes_t type,
    void *value);

/* FUNCTION NAME: db_cfgfile_getTableName
 * PURPOSE:
 *      Return the table name of the specific key
 *
 * INPUT:
 *      t_idx           -- The table index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_tbltext     -- The output name string
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_getTableName(
    UI8_T t_idx,
    UI8_T data_size,
    C8_T *ptr_tbltext);

/* FUNCTION NAME: db_cfgfile_getFieldName
 * PURPOSE:
 *      Return the Field name of the specific key
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_fldtext     -- The output name string
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_getFieldName(
    UI8_T t_idx,
    UI8_T f_idx,
    UI8_T data_size,
    C8_T *ptr_fldtext);

/* FUNCTION NAME: db_cfgfile_getTableIdx
 * PURPOSE:
 *      Return the table index of the specific table name
 *
 * INPUT:
 *      table_name      -- The table name string
 *
 * OUTPUT:
 *      ptr_tblidx      -- The table index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_getTableIdx(
    C8_T *table_name,
    UI8_T *ptr_tblidx);

/* FUNCTION NAME: db_cfgfile_getFieldIdx
 * PURPOSE:
 *      Return the Field index of the specific field name
 *
 * INPUT:
 *      t_idx           -- The table index
 *      field_name      -- The field name
 *
 * OUTPUT:
 *      ptr_fldidx      -- The field index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_cfgfile_getFieldIdx(
    UI8_T t_idx,
    C8_T *field_name,
    UI8_T *ptr_fldidx);

#endif /* End of DB_CFG_FILE_H */
