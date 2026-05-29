/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

/* FILE NAME:   vlan_utils.h
 * PURPOSE:
 *      Define the VLAN utils API in AIR Magic Wand system.
 * NOTES:
 */

#ifndef VLAN_UTILS_H
#define VLAN_UTILS_H

/* INCLUDE FILE DECLARATIONS
 */
#include <mw_utils.h>
#include <mw_types.h>
#include <db_api.h>
#include <mw_platform.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define VLAN_MIN_VID            (0)
#define VLAN_MAX_VID            (4095)
#define VLAN_DEFAULT_VID        (1)
#define VLAN_CONFIG_MIN_VID     (1)
#define VLAN_CONFIG_MAX_VID     (4094)

#define VLAN_STATE_DISABLE      (0)
#define VLAN_STATE_ENABLE       (1)

/* MACRO FUNCTION DECLARATIONS
 */
#define VLAN_IS_VID_VALID(__vlan__)                                                 \
            (((__vlan__) > VLAN_MIN_VID) && ((__vlan__) < VLAN_MAX_VID))

#define BITMAP_VLAN_SUBSTRACT(res, bmp, bmp2) do                                    \
    {                                                                               \
        int v, tmp; res = 0;                                                        \
        for(v = 0; v < MAX_VLAN_ENTRY_NUM; v++) {                                   \
            tmp = (((bmp) >> v) & 1) ^ ((((bmp) >> v) & 1) & (((bmp2) >> v) & 1));  \
            if(tmp) res |= 1 << v;                                                  \
        }                                                                           \
    } while(0)

#define BITMAP_VLAN_ADD(res, bmp, bmp2) do                                          \
    {                                                                               \
        int v, tmp; res = 0;                                                        \
        for(v = 0; v < MAX_VLAN_ENTRY_NUM; v++) {                                   \
            tmp = ((((bmp) >> v) & 1) | (((bmp2) >> v) & 1));                       \
            if(tmp) res |= 1 << v;                                                  \
        }                                                                           \
    } while(0)

#define BITMAP_PORT_SUBSTRACT(res, bmp, bmp2) do                                    \
    {                                                                               \
        int p, tmp; res = 0;                                                        \
        for(p = 0; p < MW_DEFAULT_MAX_PORT_NUM; p++) {                              \
            tmp = (((bmp) >> p) & 1) ^ ((((bmp) >> p) & 1) & (((bmp2) >> p) & 1));  \
            if(tmp) res |= 1 << p;                                                  \
        }                                                                           \
    } while(0)

#define BITMAP_PORT_ADD(res, bmp, bmp2) do                                          \
    {                                                                               \
        int p, tmp; res = 0;                                                        \
        for(p = 0; p < MW_DEFAULT_MAX_PORT_NUM; p++) {                              \
            tmp = ((((bmp) >> p) & 1) | (((bmp2) >> p) & 1));                       \
            if(tmp) res |= 1 << p;                                                  \
        }                                                                           \
    } while(0)

#define BITMAP_PORT_FOREACH(bitmap, port)                           \
            for(port = 0; port < MW_DEFAULT_MAX_PORT_NUM; port++)   \
                if(BIT_CHK(bitmap, port))

#define BITMAP_VLAN_FOREACH(bitmap, vlan)                           \
            for(vlan = 0; vlan < MAX_VLAN_ENTRY_NUM; vlan++)        \
                if(BIT_CHK(bitmap, vlan))

#define OFFSET_1_BYTES(__res__, __ptr_data__, __idx__) do                   \
    {                                                                       \
        (__res__) = ((__ptr_data__)[(__idx__)]);                            \
    } while(0)

#define OFFSET_2_BYTES(__res__, __ptr_data__, __idx__) do                   \
    {                                                                       \
        (__res__) = ((__ptr_data__)[(__idx__)] |                            \
                    ((UI16_T)(__ptr_data__)[(__idx__) + 1] << 8));          \
    } while(0)

#define OFFSET_4_BYTES(__res__, __ptr_data__, __idx__) do                   \
    {                                                                       \
        (__res__) = ((__ptr_data__)[(__idx__)] |                            \
                    ((UI32_T)(__ptr_data__)[(__idx__) + 1] << 8) |          \
                    ((UI32_T)(__ptr_data__)[(__idx__) + 2] << 16) |         \
                    ((UI32_T)(__ptr_data__)[(__idx__) + 3] << 24));         \
    } while(0)

/* DATA TYPE DECLARATIONS
 */
typedef struct VLAN_ENTRY_INFO_S
{
    UI16_T          vlan_id;            /* The ID of the VLAN */
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    UI8_T          vlan_fid;           /* The fid of the VLAN */
#endif
    VLAN_DESCR_T    descr;              /* The description of the VLAN */
    UI32_T          port_member;        /* The member ports of the port-based VLAN */
    UI32_T          tagged_member;      /* The member ports of the tagged VLAN */
    UI32_T          untagged_member;    /* The member ports of the untagged VLAN */
} ATTRIBUTE_PACK VLAN_ENTRY_INFO_T;

#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
#define VLAN_ENTRY_SIZE   (sizeof(UI16_T) + sizeof(UI8_T) + sizeof(VLAN_DESCR_T) + sizeof(UI32_T) + sizeof(UI32_T) + sizeof(UI32_T))
#else
#define VLAN_ENTRY_SIZE   (sizeof(UI16_T) + sizeof(VLAN_DESCR_T) + sizeof(UI32_T) + sizeof(UI32_T) + sizeof(UI32_T))
#endif
typedef enum
{
    VLAN_NONE = 0,
    VLAN_PORT_ENABLE = 1,       /* The port-based VLAN enabled */
    VLAN_1Q_ENABLE,             /* The 802.1Q VLAN enabled */
    VLAN_MTU_ENABLE,            /* The MTU VLAN enabled */
    VLAN_LAST
} VLAN_T;

typedef enum
{
    VLAN_MAC_NONE = 0,
    VLAN_MAC_VOICE_ENABLE,      /* Voice VLAN */
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    VLAN_MAC_SURVEIL_ENABLE,    /* Surveillance VLAN */
#endif
    VLAN_MAC_LAST
} VLAN_MAC_T;

typedef enum
{
    PORT_VLAN_ACTION_NOCHANGE = 0,
    PORT_VLAN_ACTION_ADD,
    PORT_VLAN_ACTION_DELETE,
    PORT_VLAN_ACTION_RERUN,
    PORT_VLAN_ACTION_ADD_DELETE,
    PORT_VLAN_ACTION_LAST
} PORT_VLAN_ACTION_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
MW_ERROR_NO_T vlan_get_plat_max_portBmp(UI32_T *out_portbmp);

#endif  /* End of VLAN_UTILS_H */
