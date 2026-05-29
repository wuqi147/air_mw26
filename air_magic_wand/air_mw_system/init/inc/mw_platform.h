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

/* FILE NAME:  mw_platform.h
 * PURPOSE:
 *      Middleware initialization function call.
 *
 * NOTES:
 */

#ifndef MW_PLATFORM_H
#define MW_PLATFORM_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdint.h>
#include <spinorwrite.h>
#include <platform.h>
#include <air_port.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_DEFAULT_MAX_PORT_NUM     (AIR_PORT_NUM)
#define MW_DEFAULT_CPU_PORT         (0)
#define MW_SYS_DESCRI_LEN_MAX       (80)

#define PLAT_MAX_PORT_NUM           (_mw_eth_port)          /* BASET + XSGMII */
#define PLAT_TOTAL_NUM              (_mw_total_port)        /* CPU + BASET + XSGMII */
#define PLAT_CPU_PORT               (_mw_cpu_port)          /* CPU */
#define PLAT_PORT_BMP_TOTAL         (_mw_port_bmp_total)    /* CPU + BASET + XSGMII */
#define PLAT_PORT_CAPACITY          (_mw_port_capacity)     /* XSGMII(2.5G) */
#define PLAT_MAC_ADDR               (_mw_mac_addr)
#define PLAT_SYSTEM_DESCRIPTION     (_mw_sys_desc)
#define PLAT_PORT_BMP_GPHY          (_mw_port_gphy)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* GLOBAL VARIABLE EXTERN DECLARATIONS
*/
extern unsigned int _mw_total_port;
extern unsigned int _mw_eth_port;
extern unsigned int _mw_cpu_port;
extern AIR_PORT_BITMAP_T _mw_port_gphy;
extern AIR_PORT_BITMAP_T _mw_port_capacity;
extern AIR_PORT_BITMAP_T _mw_port_bmp_total;
extern unsigned char _mw_mac_addr[MAC_ADDRESS_LEN];
extern char _mw_sys_desc[MW_SYS_DESCRI_LEN_MAX];

#endif  /* #ifndef MW_PLATFORM_H */

