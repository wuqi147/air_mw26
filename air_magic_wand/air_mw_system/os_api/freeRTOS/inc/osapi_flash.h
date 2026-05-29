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

/* FILE NAME:  osapi_flash.h
 * PURPOSE:
 *      Wrapper APIs for freeRTOS flash function call.
 *
 * NOTES:
 */

#ifndef OSAPI_FLASH_H
#define OSAPI_FLASH_H

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"

/* NAMING CONSTANT DECLARATIONS
 */
 #define INIT   0x0
 #define WRITE  0x1
 #define END    0x2

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: osapi_flashCopy
 * PURPOSE:
 *      Coopy data from specific partition to other partition
 *
 * INPUT:
 *      src_part        --  Source partition number
 *      dst_part        --  Destination partition number
 *      size            --  Number of data to read
 *
 * OUTPUT:
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_flashCopy(
    const UI8_T  src_part,
    const UI8_T  dst_part,
    const UI32_T size);

/* FUNCTION NAME: osapi_flashRead
 * PURPOSE:
 *      Read number of data from flash on specific address
 *
 * INPUT:
 *      offset          --  offset of partition to read
 *      size            --  Number of data to read
 *      ptr_data        --  A pointer to data buffer
 *      part            --  Partition number
 *
 * OUTPUT:
 *      ptr_data        --  A pointer to copy flash data
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
osapi_flashRead(
    const UI32_T offset,
    const UI32_T size,
    UI8_T *const ptr_data,
    const UI8_T  part);

/* FUNCTION NAME: osapi_flashWrite
 * PURPOSE:
 *      Write number of data to flash on specific partition
 *
 * INPUT:
 *      opt             --  Init/Write/End
 *      size            --  Number of data to write
 *      ptr_data        --  A pointer to data buffer
 *      part            --  Partition number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Step 1. opt=Init and specific part which partitin to wirte
 *      Step 2. opt=Write and specific size / ptr_data to write buffer to flash
 *      Step 3. opt=End and specific all data is transfered
 */
MW_ERROR_NO_T
osapi_flashWrite(
    const UI8_T  opt,
    const UI32_T size,
    const UI8_T *ptr_data,
    const UI8_T  part);

/* GLOBAL VARIABLE EXTERN DECLARATIONS
*/

#endif  /* #ifndef OSAPI_FLASH_H */

