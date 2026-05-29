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

/* FILE NAME:   air_cfg.h
 * PURPOSE:
 *      Customer configuration on AIR SDK.
 * NOTES:
 */

#ifndef AIR_CFG_H
#define AIR_CFG_H

/* INCLUDE FILE DECLARATIONS
 */

#include <air_error.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM     (4)
#define AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP (4)

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AIR_CFG_TYPE_IFMON_ENABLE,                   /* used to enable or disable IFMON thread
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - disable, 1 - enable
                                                  */
    AIR_CFG_TYPE_IFMON_THREAD_PRI,               /* used to customize IFMON thread priority
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : priority
                                                  */
    AIR_CFG_TYPE_IFMON_THREAD_STACK,             /* used to customize IFMON thread stack size
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : stack size, unit is byte
                                                  */
    AIR_CFG_TYPE_PHY_ACCESS_TYPE,                /* used to configure PHY access type
                                                  * param0: port
                                                  * param1: ignore
                                                  * value : 0 - direct mdio, 1: i2c indirect mdio
                                                  */
    AIR_CFG_TYPE_PHY_I2C_BUS_ID,                 /* used to configure I2C bus id
                                                  * param0: port
                                                  * param1: ignore
                                                  * value: i2c bus id, valid setting is 0 or 1,
                                                  *        default is 1 if no specified
                                                  */
    AIR_CFG_TYPE_PHY_I2C_ADDRESS,                /* used to configure I2C slave address of I2C type PHY
                                                  * param0: port
                                                  * param1: ignore
                                                  * value : i2c address for indirect access mdio
                                                  *         Only valid if AIR_CFG_TYPE_PHY_ACCESS_TYPE is
                                                  *         i2c indirect mdio
                                                  */
    AIR_CFG_TYPE_PHY_ADDRESS,                    /* used to configure PHY address
                                                  * param0: port
                                                  * param1: ignore
                                                  * value : phy address 0-31
                                                  */
    AIR_CFG_TYPE_PHY_LED_BEHAVIOR,               /* used to configure PHY LED behavior
                                                  * param0: port
                                                  * param1: led id
                                                  * value : bit(0, 1, 2) wrt LED on for (1000M, 100M, 10M) link up
                                                  *         bit(3, 4) wrt LED on for (full, half) duplex
                                                  *         bit(5, 6) wrt LED blink for 1000M (TX, RX) activity
                                                  *         bit(7, 8) wrt LED blink for 100M (TX, RX) activity
                                                  *         bit(9, 10) wrt LED blink for 10M (TX, RX) activity
                                                  *         bit11 for LED high active
                                                  *         bit12 wrt LED on for 2500M link up
                                                  *         bit(13, 14) wrt LED on for 2500M (TX, RX) activity
                                                  */
    AIR_CFG_TYPE_PHY_LED_TYPE,                   /* used to configure PHY LED type
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - serial LED, 1 - parallel LED
                                                  */
    AIR_CFG_TYPE_PHY_LED_COUNT,                  /* used to configure per-port PHY LED count
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : LED count, valid: 1-2
                                                  */
    AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,        /* used to configure serdes polarity reverse
                                                  * param0: port
                                                  * param1: ignore
                                                  * value : 0 - no reverse, 1 - tx reverse,
                                                  *         2 - rx reverse, 3 - tx_rx reverse
                                                  */
    AIR_CFG_TYPE_MAX_ENTRY_CNT_MAC_BASED_VLAN,   /* used to configure shared table entry count for MAC based VLAN
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : max entry number
                                                  */
    AIR_CFG_TYPE_MAX_ENTRY_CNT_IPV4_BASED_VLAN,  /* used to configure shared table entry count for
                                                  * IPv4 subnet based VLAN
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : max entry number
                                                  */
    AIR_CFG_TYPE_MAX_ENTRY_CNT_SERVICE_VLAN,     /* used to configure shared table entry count for service VLAN
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : max entry number
                                                  */
    AIR_CFG_TYPE_FORCE_DEVICE_ID,                /* used to force device id
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : device id
                                                  */
    AIR_CFG_TYPE_MDIO_ENABLE,                    /* used to enable or disable MDIO(SMI) interface for external bus
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - disable(default), 1 - enable
                                                  */
    AIR_CFG_TYPE_MDIO_CLOCK,                     /* used to configure MDIO(SMI) interface clock
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - 700KHz, 1 - 2.8MHz, 2 - 5.6MHz, 3 - 11.2MHz
                                                  */
    AIR_CFG_TYPE_SIF_LOCAL_CLOCK,                /* used to configure local SIF(I2C) interface clock
                                                  * param0: local sif channel
                                                  * param1: ignore
                                                  * value : 0 - 100kHz, 1 - 400kHz, 2 - 1MHz
                                                  */
    AIR_CFG_TYPE_SIF_REMOTE_SLAVE_ID,            /* used to configure remote SIF(I2C) slave address
                                                  * prarm0: local sif channel
                                                  * param1: device index
                                                  * value : slave id
                                                  */
    AIR_CFG_TYPE_SIF_REMOTE_CLOCK,               /* used to configure remote SIF(I2C) interface clock
                                                  * param0: local sif channel
                                                  * param1: slave id
                                                  * value : 0 - 100kHz, 1 - 400kHz, 2 - 1MHz
                                                  */
    AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,           /* used to configure peripheral pin as GPIO
                                                  * param0: pin number
                                                  * param1: ignore
                                                  * value : 0 - disable, 1 - enable
                                                  */
    AIR_CFG_TYPE_COMBO_PORT_LED_TYPE,            /* used to configure combo port LED type
                                                  * param0: port
                                                  * param1: ignore
                                                  * value : 0 - serial and parallel LED
                                                  *         1 - parallel LED
                                                  *         2 - parallel LED without GPHY LED
                                                  */
    AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE,     /* used to configure serdes system side operation mode
                                                  * param0: port
                                                  * param1: ignore
                                                  * value : 0 - Speed changed by line side link speed,
                                                  *         1 - Speed fixed at serdes maximum speed
                                                  */
    AIR_CFG_TYPE_PORT_ADMIN_STATE,               /* used to configure default port admin state
                                                  * param0: port
                                                  * param1: ignore
                                                  * value : 0 - disable, 1 - enable
                                                  */
    AIR_CFG_TYPE_SERDES_PORT_OPTION,             /* used to configure serdes port operation options
                                                  * param0: port
                                                  * param1: ignore
                                                  * value : 0 - default, 1 - force rate-adaption,
                                                  *         2 - skip serdes driver
                                                  *         default -
                                                  *              all air port API's behavior will depend on
                                                  *              serdes mode setting and configure on both
                                                  *              serdes and MAC
                                                  *         force rate-adaption -
                                                  *              used for the peer serdes not support
                                                  *              automatical rate adaption
                                                  *         skip serdes driver -
                                                  *              all air port API's behavior will bypass serdes
                                                  *              and only configure on MAC
                                                  *
                                                  */
    AIR_CFG_TYPE_L2_POLL_THREAD_PRI,             /* used to customize L2_POLL thread priority
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : priority
                                                  */
    AIR_CFG_TYPE_L2_POLL_THREAD_STACK,           /* used to customize L2_POLL thread stack size
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : stack size, unit is byte
                                                  */
    AIR_CFG_TYPE_L2_POLL_INTERVAL,               /* used to customize L2_POLL polling interval
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - 1000 ms
                                                  *         1 -  500 ms
                                                  *         2 -  250 ms
                                                  *         3 -  125 ms
                                                  */
    AIR_CFG_TYPE_L2_POLL_ENABLE,                 /* used to enable or disable L2_POLL polling thread
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - disable, 1 - enable
                                                  */
    AIR_CFG_TYPE_SURGE_PROTECTION_MODE,          /* used to configure surge protection mode
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - 0R mode, 1 - 5R mode
                                                  */
    AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR, /* used to configure combo port SerDes LED behavior
                                                  * param0: port
                                                  * param1: led id
                                                  * value : bit(0, 1, 2) wrt LED on for (1000M, 100M, 10M) link up
                                                  *         bit(3, 4) wrt LED on for (full, half) duplex
                                                  *         bit(5, 6) wrt LED blink for 1000M (TX, RX) activity
                                                  *         bit(7, 8) wrt LED blink for 100M (TX, RX) activity
                                                  *         bit(9, 10) wrt LED blink for 10M (TX, RX) activity
                                                  *         bit11 for LED high active
                                                  *         bit12 wrt LED on for 2500M link up
                                                  *         bit(13, 14) wrt LED on for 2500M (TX, RX) activity
                                                  * notes :
                                                  *         only be used when AIR_CFG_TYPE_COMBO_PORT_LED_TYPE = 0,
                                                  *         and if GPHY and SerDes's behavior is not the same
                                                  */
    AIR_CFG_TYPE_CHIP_CO_CLOCK_ENABLE,           /* used to enable or disable chip co-clock pin output
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - disable, 1 - enable(default)
                                                  */
    AIR_CFG_TYPE_LPDET_ETH_TYPE,                 /* used to configure loop detect frame ether type value
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : loop detect frame ether type, default is 0x88b6
                                                  */
    AIR_CFG_TYPE_BACKPRES_CTRL_MODE,             /* used to control the backpressure behavior
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - control by auto-negotiation result(default),
                                                  *         1 - control by manual port API setting
                                                  */
    AIR_CFG_TYPE_PERIF_GPIO_AUTO_MODE,           /* used to configure gpio pin as auto output mode
                                                  * param0: pin number
                                                  * param1: ignore
                                                  * value : 0 - disable(default), 1 - enable
                                                  */
    AIR_CFG_TYPE_PERIF_GPIO_AUTO_PATT,           /* used to configure the output pattern of GPIO pin
                                                  * param0: pin number
                                                  * param1: ignore
                                                  * value : 0 - 0.5Hz(default), 1 - 1Hz, 2 - 2Hz, 3 - 8Hz,
                                                  */
    AIR_CFG_TYPE_POE_LED_BEHAVIOR,               /* used to configure PoE LED behavior
                                                  * param0: device index
                                                  * param1: ignore
                                                  * value : bit0 for PoE LED active level
                                                  *         0 - low active(default), 1 - high active
                                                  */
    AIR_CFG_TYPE_POE_RESET_SCRATCH,              /* used to configure PoE scratch value for detecting unexpected
                                                  * device resets.
                                                  * param0: poe device index
                                                  * param1: ignore
                                                  * value : scratch value 1-255 (default is 165 if no specified)
                                                  */
    AIR_CFG_TYPE_PERIF_FORCE_GPIO_LEVEL_INVERSE, /* used to configure peripheral GPIO output level inverse
                                                  * param0: pin number
                                                  * param1: ignore
                                                  * value : 0 - no inverse(default), 1 - inverse
                                                  * notes :
                                                  *         In inverse mode, when the GPIO API sets the output to
                                                  *         high, the actual output will be low. When the output is
                                                  *         set to low, the actual output will be high.
                                                  */
    AIR_CFG_TYPE_L2_FDB_FULL_BEHAVIOR,           /* used to configure L2 learning behavior when hash bucket full
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : 0 - don't replace the oldest dynamic MAC entry(default)
                                                  *         1 - replace the oldest dynamic MAC entry when HW learning
                                                  */
    AIR_CFG_TYPE_MAX_RETRANSMISSION_COUNT,       /* used to configure the maximum retransmission count after a collision
                                                  * is detected during packet transmission in half-duplex mode
                                                  * param0: ignore
                                                  * param1: ignore
                                                  * value : valid: 0-15
                                                  *         0     - unlimited retransmission(default)
                                                  *         1..15 - retransmission up to the specified count
                                                  */
    AIR_CFG_TYPE_LAST
} AIR_CFG_TYPE_T;
/* DON'T forget to fill the table _sdk_ref_cfg_info[] in sdk_ref_config.c */

typedef struct AIR_CFG_VALUE_S
{
    UI32_T param0; /* (Optional) The optional parameter which is available
                    * when the AIR_CFG_TYPE_T needs the first arguments
                    */
    UI32_T param1; /* (Optional) The optional parameter which is available
                    * when the AIR_CFG_TYPE_T needs the second arguments
                    */
    I32_T  value;
} AIR_CFG_VALUE_T;

typedef AIR_ERROR_NO_T (*AIR_CFG_GET_FUNC_T)(
    const UI32_T         unit,
    const AIR_CFG_TYPE_T cfg_type,
    AIR_CFG_VALUE_T     *ptr_cfg_value);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_cfg_getValue
 * PURPOSE:
 *      This API is used to get the value of customized configuration.
 * INPUT:
 *      unit                 -- Device ID
 *      cfg_type             -- The type of config
 *                              AIR_CFG_TYPE_T
 * OUTPUT:
 *      ptr_value            -- The value of config
 *                              AIR_CFG_VALUE_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_cfg_getValue(
    const UI32_T         unit,
    const AIR_CFG_TYPE_T cfg_type,
    AIR_CFG_VALUE_T     *ptr_value);

/* FUNCTION NAME:   air_cfg_register
 * PURPOSE:
 *      The function is to register user configuration callback to SDK.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      ptr_cfg_callback     -- Pointer to user configuration callback
 *                              AIR_CFG_GET_FUNC_T
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *
 * NOTES:
 *      1. During SDK initializtion, it will call registered user
 *         configuration callback to get configuration and apply them.
 *         If there is no registered user configuration callback or
 *         can not get specified user configuration callback, SDK will
 *         apply default setting.
 *      2. This function should be called before calling air_init
 */
AIR_ERROR_NO_T
air_cfg_register(
    const UI32_T       unit,
    AIR_CFG_GET_FUNC_T ptr_cfg_callback);

#endif /* AIR_CFG_H */
