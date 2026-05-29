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

/* FILE NAME:  sfp_led.h
 * PURPOSE:
 *      It provide SFP LED application API.
 * NOTES:
 */

#ifndef SFP_LED_H
#define SFP_LED_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_LED_THREAD_PRI      (2)          /* Priority of SFP LED working thread */
#define SFP_LED_THREAD_INTERVAL (500)        /* Thread loop interval, time in ms */
#define SFP_LED_INVALID_ID      (0xFFFFFFFF) /* Invalid port */
#define SFP_LED_APP_NAME        ("sfp_led")  /* SFP LED application name */
#define SFP_LED_STACK_SIZE      (512)        /* SFP LED working thread stack size */

/* DATA TYPE DECLARATIONS
 */

/* Keep current GPIO output data and auto pattern to prevent non-necessary perif API invoke */
typedef enum
{
    SFP_LED_GPIO_OUTPUT_AUTO_PATT_HZ_HALF = 0,
    SFP_LED_GPIO_OUTPUT_AUTO_PATT_HZ_ONE,
    SFP_LED_GPIO_OUTPUT_AUTO_PATT_HZ_TWO,
    SFP_LED_GPIO_OUTPUT_AUTO_PATT_HZ_EIGHT,
    SFP_LED_GPIO_OUTPUT_DATA_HIGH,
    SFP_LED_GPIO_OUTPUT_DATA_LOW,
    SFP_LED_GPIO_OUTPUT_LAST
} SFP_LED_GPIO_OUTPUT_STATE_T;

/* Definition of port speed */
typedef enum
{
    SFP_LED_PORT_SPEED_10M,
    SFP_LED_PORT_SPEED_100M,
    SFP_LED_PORT_SPEED_1000M,
    SFP_LED_PORT_SPEED_2500M,
    SFP_LED_PORT_SPEED_5000M,
    SFP_LED_PORT_SPEED_LAST
} SFP_LED_PORT_SPEED_T;

/* Definition of link status of a specific port */
typedef struct SFP_LED_PORT_STATUS_S
{
#define SFP_LED_PORT_STATUS_FLAGS_LINK_UP     (1U << 0)
#define SFP_LED_PORT_STATUS_FLAGS_DUPLEX_FULL (1U << 1)
    UI32_T               flags;
    SFP_LED_PORT_SPEED_T speed;
} SFP_LED_PORT_STATUS_T;

typedef AIR_ERROR_NO_T (*SFP_LED_PORT_STATUS_FUNC_T)(const UI32_T unit, const UI32_T port, SFP_LED_PORT_STATUS_T *ptr_status);

/* SFP port, LED behavior, tx/rx oct, GPIO output flags */
typedef struct SFP_LED_PORT_INFO_S
{
#define SFP_LED_FLAGS_LED_ON_LINK_UP_1000M (1U << 0)
#define SFP_LED_FLAGS_LED_ON_LINK_UP_100M  (1U << 1)
#define SFP_LED_FLAGS_LED_ON_LINK_UP_10M   (1U << 2)
#define SFP_LED_FLAGS_LED_ON_DUPLEX_FULL   (1U << 3)
#define SFP_LED_FLAGS_LED_ON_DUPLEX_HALF   (1U << 4)
#define SFP_LED_FLAGS_LED_BLINK_TX_1000M   (1U << 5)
#define SFP_LED_FLAGS_LED_BLINK_RX_1000M   (1U << 6)
#define SFP_LED_FLAGS_LED_BLINK_TX_100M    (1U << 7)
#define SFP_LED_FLAGS_LED_BLINK_RX_100M    (1U << 8)
#define SFP_LED_FLAGS_LED_BLINK_TX_10M     (1U << 9)
#define SFP_LED_FLAGS_LED_BLINK_RX_10M     (1U << 10)
#define SFP_LED_FLAGS_LED_ACTIVE_HIGH      (1U << 11)
#define SFP_LED_FLAGS_LED_ON_LINK_UP_2500M (1U << 12)
#define SFP_LED_FLAGS_LED_BLINK_TX_2500M   (1U << 13)
#define SFP_LED_FLAGS_LED_BLINK_RX_2500M   (1U << 14)
    /* configuration */
    UI32_T                      flags;       /* LED behavior config, init from _customer_ref_cfg[] */
    UI32_T                      port;        /* Port id, inited from sfp_led_init() */
    UI32_T                      led_id;      /* Led id */
    UI32_T                      gpio_pin_id; /* GPIO pin id, inited from sfp_led_init() */
    /* status */
    UI16_T                      rx_oct;            /* RX octets, get from air_mib_getPortCnt() */
    UI16_T                      tx_oct;            /* TX octets, get from air_mib_getPortCnt() */

    SFP_LED_GPIO_OUTPUT_STATE_T gpio_output_state; /* Keep GPIO output data and auto pattern state */
} SFP_LED_PORT_INFO_T;

typedef struct SFP_LED_PORT_MAP_S /* SPF port id and LED pin id mapping */
{
    UI32_T unit;                  /* Device unit */
    UI32_T port;                  /* Port id */
    UI32_T gpio_pin_id;           /* GPIO pin id*/
} SFP_LED_PORT_MAP_T;

/* SFP control block structure */
typedef struct SFP_LED_CB_S
{
    /* SFP LED application working thread id */
    AIR_THREAD_ID_T            thread_id;
    /* To keep if SFP LED module is init */
    BOOL_T                     sfp_led_inited;
    /* To keep total led config count */
    UI8_T                      total_led_cfg_cnt;
    /* SFP port ID, LED behavior config, tx/rx oct, GPIO output flags */
    SFP_LED_PORT_INFO_T       *ptr_sfp_port_info;
    /* Port status function pointer, inited from sfp_led_init() */
    SFP_LED_PORT_STATUS_FUNC_T port_status_func;
    /* Thread running count */
    UI32_T                     thread_runnning_cnt;
} SFP_LED_CB_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: sfp_led_init
 *
 * PURPOSE:
 *      Initialize SFP LED application.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
sfp_led_init(
    UI32_T                     unit,
    const UI32_T               port_map_cnt,
    SFP_LED_PORT_MAP_T        *ptr_sfp_port_map,
    SFP_LED_PORT_STATUS_FUNC_T port_status_func);

/* FUNCTION NAME: sfp_led_deinit
 *
 * PURPOSE:
 *      Deinitialize SFP LED application.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
sfp_led_deinit(
    UI32_T unit);

/* FUNCTION NAME: sfp_led_updateLedStatus
 *
 * PURPOSE:
 *      Update LED status.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_NOT_SUPPORT   --  Not support.
 * NOTES:
 *      If SFP LED thread is create by SFP LED app, then return not support.
 *
 */
AIR_ERROR_NO_T
sfp_led_updateLedStatus(
    UI32_T unit);

#endif /*SFP_LED_H*/
