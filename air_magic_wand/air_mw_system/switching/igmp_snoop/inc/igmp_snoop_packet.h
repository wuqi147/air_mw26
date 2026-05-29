/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  igmp_snoop_packet.h
 * PURPOSE:
 *      This file defines the data structure for IGMP snooping packets.
 *
 * NOTES:
 */
#ifndef IGMP_SNOOP_PACKET_H
#define IGMP_SNOOP_PACKET_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "lwip/pbuf.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define IGMP_V3_MEMB_REPORT                     (0x22)     /* Ver. 3 membership report */
#if AIR_SUPPORT_IGMPV3_AWARE
#define IGMP_SNP_REPORTV3_CHECK_GROUP_REC_NUM   (4)
#endif

/* MACRO FUNCTION DECLARATIONS
 */
/* Macros to get struct igmp_msg fields: */
#define IGMP_TYPE(igmp_msg)           ((igmp_msg)->igmp_msgtype)
#define IGMP_MAX_RESP_T(igmp_msg)     ((igmp_msg)->igmp_maxresp)
#define IGMP_CHKSUM(igmp_msg)         ((igmp_msg)->igmp_checksum)
#define IGMP_GRP(igmp_msg)            ((igmp_msg)->igmp_group_address)

/* DATA TYPE DECLARATIONS
 */
#if AIR_SUPPORT_IGMPV3_AWARE
typedef enum
{
    IGMP_REPORTV3_RECORD_TYPE_INVALID = 0,
    IGMP_REPORTV3_IS_INCLUDE = 1,
    IGMP_REPORTV3_IS_EXCLUDE,
    IGMP_REPORTV3_CHANGE_TO_INCLUDE,
    IGMP_REPORTV3_CHANGE_TO_EXCLUDE,
    IGMP_REPORTV3_ALLOW_NEW_SOURCE,
    IGMP_REPORTV3_BLOCK_OLD_SOURCE,
    IGMP_REPORTV3_RECORD_TYPE_LAST
} IGMP_SNP_V3REPORT_RECORD_TYPE_T;

typedef struct IGMP_SNP_V3REPORT_MSG_S {
    UI8_T      msg_type;
    UI8_T      reserved_1;
    UI16_T     igmp_checksum;
    UI16_T     reserved_2;
    UI16_T     num_of_group_records;
} IGMP_SNP_V3REPORT_MSG_T;

typedef struct IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_S {
    UI8_T      record_type;
    UI8_T      aux_data_len;
    UI16_T     num_of_sources;
    UI32_T     multicast_address;
} IGMP_SNP_V3REPORT_GROUP_RECORD_MSG_T;

#endif /* AIR_SUPPORT_IGMPV3_AWARE */

#if AIR_SUPPORT_IGMP_SNP_QUERIER_IGMPV3 | AIR_SUPPORT_IGMPV3_AWARE
typedef struct IGMP_SNP_V3QUERY_MSG_S {
    UI8_T      igmp_msgtype;
    UI8_T      igmp_maxresp;
    UI16_T     igmp_checksum;
    UI32_T     igmp_group_address;
    UI8_T      igmp_resv: 4;
    UI8_T      igmp_s: 1;
    UI8_T      igmp_qrv: 3;
    UI8_T      igmp_qqic;
    UI16_T     igmp_n_srcs;
} IGMP_SNP_V3QUERY_MSG_T;
#endif

typedef struct IGMP_SNP_PACKET_INFO_S
{
    struct ip_hdr      *ptr_iphdr;
    struct pbuf        *ptr_pbuf;
    struct netif       *ptr_netif;
    UI32_T             forward_bmp;
#if AIR_SUPPORT_IGMPV3_AWARE
    UI16_T             payload_len;
#endif
} IGMP_SNP_PACKET_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   igmp_snp_packet_input
 * PURPOSE:
 *      This API is for extern compnent to input a IGMP message.
 *
 * INPUT:
 *      ptr_pbuf          -- pointer to the pbuf containing the IGMP message
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_packet_input(
    struct pbuf *ptr_pbuf);

/* FUNCTION NAME:   igmp_snp_packet_initVariable
 * PURPOSE:
 *      This API is used to initialize IGMP snooping module variables.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_packet_initVariable(
    void);

#endif  /* IGMP_SNOOP_PACKET_H */
