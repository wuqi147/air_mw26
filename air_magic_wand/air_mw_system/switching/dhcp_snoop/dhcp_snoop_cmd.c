/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:   dhcp_snoop_cmd.c
 * PURPOSE:
 *      Define DHCP snooping command function.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <dhcp_snoop.h>
#include <dhcp_snoop_log.h>
#include <dhcp_snoop_db.h>

#include <mw_cmd_parser.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define dhcp_snp_cmd_sendMsg(...) dhcp_snp_db_update(__VA_ARGS__)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: dhcp_snp_cmd_setEnable
 * PURPOSE:
 *      Set DHCP snooping enable command API
 *
 * INPUT:
 *      enable   -  1: enable, 0: disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setEnable(
    const UI8_T enable)
{
    dhcp_snp_cmd_sendMsg(DHCP_SNP_INFO, DHCP_SNP_ENABLE, DB_ALL_ENTRIES, (void * const)&enable, sizeof(UI8_T));
}
/* FUNCTION NAME: dhcp_snp_cmd_setTrustPort
 * PURPOSE:
 *      Set DHCP snooping trust port command API
 *
 * INPUT:
 *      port     -  port index
 *      enable   -  1: enable, 0: disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setTrustPort(
    const UI16_T port,
    const UI8_T  enable)
{
    if ((port >= 1) && (port <= PLAT_MAX_PORT_NUM))
    {
        dhcp_snp_cmd_sendMsg(DHCP_SNP_PORT_INFO, DHCP_SNP_TRUST_PORT, port, (void * const)&enable, sizeof(UI8_T));
    }
}
/* FUNCTION NAME: dhcp_snp_cmd_setOption82Enable
 * PURPOSE:
 *      Set DHCP snooping option 82 enable command API
 *
 * INPUT:
 *      port     -  port index
 *      enable   -  1: enable, 0: disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setOption82Enable(
    const UI16_T port,
    const UI8_T  enable)
{
    if ((port >= 1) && (port <= PLAT_MAX_PORT_NUM))
    {
        dhcp_snp_cmd_sendMsg(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82, port, (void * const)&enable, sizeof(UI8_T));
    }
}
/* FUNCTION NAME: dhcp_snp_cmd_setOption82Mode
 * PURPOSE:
 *      Set DHCP snooping option 82 operation mode command API
 *
 * INPUT:
 *      port     -  port index
 *      mode     -  0: no process, 1: replace, 2: drop
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setOption82Mode(
    const UI16_T port,
    const UI8_T  mode)
{
    if ((port >= 1) && (port <= PLAT_MAX_PORT_NUM))
    {
        dhcp_snp_cmd_sendMsg(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_MODE, port, (void * const)&mode, sizeof(UI8_T));
    }
}
/* FUNCTION NAME: dhcp_snp_cmd_setOption82CircuitId
 * PURPOSE:
 *      Set DHCP snooping option 82 circuit id command API
 *
 * INPUT:
 *      port     -  port index
 *      type     -  circuit ID type (0: default, 255: user config)
 *      ptr_id   -  circuit ID string
 *      len      -  circuit ID string length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setOption82CircuitId(
    const UI16_T  port,
    const UI16_T  type,
    const C8_T    *ptr_id,
    const UI8_T   len)
{
    UI8_T data[MW_CMD_CMD_MAX_LENGTH] = {0};
    if (len > 0)
    {
        osapi_memcpy(data, ptr_id, len);
    }
    if ((port >= 1) && (port <= PLAT_MAX_PORT_NUM))
    {
        dhcp_snp_cmd_sendMsg(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_CIRCUIT_ID_TYPE, port, (void * const)&type, 1);
        dhcp_snp_cmd_sendMsg(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_CIRCUIT_ID, port, (void * const)data,MAX_OPT82_ID_LEN);
    }
}
/* FUNCTION NAME: dhcp_snp_cmd_setOption82RemoteId
 * PURPOSE:
 *      Set DHCP snooping option 82 remote id command API
 *
 * INPUT:
 *      port     -  port index
 *      type     -  circuit ID type (0: MAC, 1: IP, 255: user config)
 *      ptr_id   -  remote ID string
 *      len      -  remote ID string length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
dhcp_snp_cmd_setOption82RemoteId(
    const UI16_T  port,
    const UI16_T  type,
    const C8_T    *ptr_id,
    const UI8_T   len)
{
    UI8_T data[MW_CMD_CMD_MAX_LENGTH] = {0};
    if (len > 0)
    {
        osapi_memcpy(data, ptr_id, len);
    }
    if ((port >= 1) && (port <= PLAT_MAX_PORT_NUM))
    {
        dhcp_snp_cmd_sendMsg(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_REMOTE_ID_TYPE, port, (void * const)&type, 1);
        dhcp_snp_cmd_sendMsg(DHCP_SNP_PORT_INFO, DHCP_SNP_OPT82_REMOTE_ID, port, (void * const)data,MAX_OPT82_ID_LEN);
    }
}
