/* FILE NAME:   air_port.h
 * PURPOSE:
 *      Define port function in AIR SDK.
 *
 * NOTES:
 *      None
 */

#ifndef AIR_PORT_H
#define AIR_PORT_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_MAX_NUM_OF_UNIT  (1)
#define AIR_DST_DEFAULT_PORT (31)
#define AIR_ALL_PORT_BITMAP  (0x7F)

#define AIR_PORT_NUM              (32)
#define AIR_PORT_BITMAP_SIZE      AIR_BITMAP_SIZE(AIR_PORT_NUM)
#define AIR_MAX_LEN_OF_PORT_PRINT (15)
/* The max length of port string is '1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31' */
#define AIR_MAX_LEN_OF_PORT_STR (42)
#define AIR_PORT_CABLE_MAX_PAIR (4)

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_PORT_ADD(bitmap, port) (((bitmap)[(port) / 32]) |= (1U << ((port) % 32)))
#define AIR_PORT_DEL(bitmap, port) (((bitmap)[(port) / 32]) &= ~(1U << ((port) % 32)))
#define AIR_PORT_CHK(bitmap, port) ((((bitmap)[(port) / 32] & (1U << ((port) % 32)))) != 0)

#define AIR_PORT_FOREACH(bitmap, port)          \
    for (port = 0; port < AIR_PORT_NUM; port++) \
        if (AIR_PORT_CHK(bitmap, port))

#define AIR_PORT_NOT(bitmap, bitmapValid, port)  \
    for (port = 0; port < AIR_PORT_NUM; port++)  \
    {                                            \
        if (AIR_PORT_CHK(bitmap, port))          \
        {                                        \
            AIR_PORT_DEL(bitmap, port);          \
        }                                        \
        else                                     \
        {                                        \
            if (AIR_PORT_CHK(bitmapValid, port)) \
            {                                    \
                AIR_PORT_ADD(bitmap, port);      \
            }                                    \
        }                                        \
    }

#define AIR_PORT_BITMAP_CLEAR(bitmap) ((bitmap)[0] = 0)

#define AIR_PORT_BITMAP_EMPTY(bitmap) ((bitmap)[0] == 0)

#define AIR_PORT_BITMAP_EQUAL(bitmap_a, bitmap_b) ((bitmap_a)[0] == (bitmap_b)[0])

#define AIR_PORT_BITMAP_COPY(bitmap_dst, bitmap_src) \
    do                                               \
    {                                                \
        UI32_T i;                                    \
        for (i = 0; i < AIR_PORT_BITMAP_SIZE; i++)   \
        {                                            \
            ((bitmap_dst)[i] = (bitmap_src)[i]);     \
        }                                            \
    } while (0)

/* DATA TYPE DECLARATIONS
 */
typedef struct AIR_PORT_INFO_S
{
    UI32_T total_port;
    UI32_T giga_port;
    UI32_T sgmii_port_begin;
    UI32_T sgmii_port_end;
} AIR_PORT_INFO_T;

/* AIR_PORT_BITMAP_T is the data type for physical port bitmap. */
typedef UI32_T AIR_PORT_BITMAP_T[AIR_PORT_BITMAP_SIZE];
/* Definition of SGMII mode */
typedef enum
{
    AIR_PORT_SERDES_MODE_SGMII,
    AIR_PORT_SERDES_MODE_1000BASE_X,
    AIR_PORT_SERDES_MODE_HSGMII,
    AIR_PORT_SERDES_MODE_100BASE_FX,
    AIR_PORT_SERDES_MODE_LAST
} AIR_PORT_SERDES_MODE_T;

/* Definition of port speed */
typedef enum
{
    AIR_PORT_SPEED_10M,
    AIR_PORT_SPEED_100M,
    AIR_PORT_SPEED_1000M,
    AIR_PORT_SPEED_2500M,
    AIR_PORT_SPEED_5000M,
    AIR_PORT_SPEED_10000M,
    AIR_PORT_SPEED_LAST
} AIR_PORT_SPEED_T;

typedef enum
{
    AIR_PORT_DUPLEX_HALF,
    AIR_PORT_DUPLEX_FULL,
    AIR_PORT_DUPLEX_LAST
} AIR_PORT_DUPLEX_T;

typedef enum
{
    AIR_PORT_LINK_DOWN,
    AIR_PORT_LINK_UP,
    AIR_PORT_LINK_LAST
} AIR_PORT_LINK_T;

typedef enum
{
    AIR_PORT_DIR_TX = 0,
    AIR_PORT_DIR_RX,
    AIR_PORT_DIR_BOTH,
    AIR_PORT_DIR_LAST
} AIR_PORT_DIR_T;

typedef enum
{
    AIR_PORT_LPBK_DIR_NEAR_END = 0,
    AIR_PORT_LPBK_DIR_FAR_END,
    AIR_PORT_LPBK_DIR_LAST
} AIR_PORT_LPBK_DIR_T;

/* Definition of packet length */

typedef enum
{
    AIR_PORT_PS_MODE_DISABLE = 0,
    AIR_PORT_PS_MODE_EEE,
    AIR_PORT_PS_MODE_LAST
} AIR_PORT_PS_MODE_T;

/* Definition of Auto-Negotiation action */
typedef enum
{
    AIR_PORT_PHY_AN_DISABLE = 0,
    AIR_PORT_PHY_AN_ENABLE,
    AIR_PORT_PHY_AN_RESTART,
    AIR_PORT_PHY_AN_LAST
} AIR_PORT_PHY_AN_T;

/* Definition of AN Advertisement Register */
typedef struct AIR_PORT_PHY_AN_ADV_S
{
#define AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX     (1U << 0)
#define AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX     (1U << 1)
#define AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX    (1U << 2)
#define AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX    (1U << 3)
#define AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX   (1U << 4)
#define AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE  (1U << 5)
#define AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE (1U << 6)
#define AIR_PORT_PHY_AN_ADV_FLAGS_EEE        (1U << 7)
#define AIR_PORT_PHY_AN_ADV_FLAGS_2500M      (1U << 8)
#define AIR_PORT_PHY_AN_ADV_FLAGS_5000M      (1U << 9)
#define AIR_PORT_PHY_AN_ADV_FLAGS_10000M     (1U << 10)
    UI32_T flags;
} AIR_PORT_PHY_AN_ADV_T;

/* Definition of Link Status of a specific port */
typedef struct AIR_PORT_STATUS_S
{
#define AIR_PORT_STATUS_FLAGS_LINK_UP      (1U << 0)
#define AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX (1U << 1)
#define AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX (1U << 2)
#define AIR_PORT_STATUS_FLAGS_EEE          (1U << 3)
    UI32_T            flags;
    AIR_PORT_SPEED_T  speed;
    AIR_PORT_DUPLEX_T duplex;
} AIR_PORT_STATUS_T;

/* Definition of Smart speed down will occur after AN failed how many times */
typedef enum
{
    AIR_PORT_SSD_MODE_DISABLE,
    AIR_PORT_SSD_MODE_2T = 2,
    AIR_PORT_SSD_MODE_3T,
    AIR_PORT_SSD_MODE_4T,
    AIR_PORT_SSD_MODE_5T,
    AIR_PORT_SSD_MODE_LAST
} AIR_PORT_SSD_MODE_T;

typedef enum
{
    /* Port matrix mode  */
    AIR_PORT_VLAN_MODE_PORT_MATRIX = 0,

    /* Fallback mode  */
    AIR_PORT_VLAN_MODE_FALLBACK,

    /* Check mode  */
    AIR_PORT_VLAN_MODE_CHECK,

    /* Security mode  */
    AIR_PORT_VLAN_MODE_SECURITY,
    AIR_PORT_VLAN_MODE_LAST
} AIR_PORT_VLAN_MODE_T;

typedef enum
{
    AIR_PORT_COMBO_MODE_PHY,
    AIR_PORT_COMBO_MODE_SERDES,
    AIR_PORT_COMBO_MODE_LAST
} AIR_PORT_COMBO_MODE_T;

/* Definition of port LED control */
typedef enum
{
    AIR_PORT_PHY_LED_CTRL_MODE_PHY = 0,
    AIR_PORT_PHY_LED_CTRL_MODE_FORCE,
    AIR_PORT_PHY_LED_CTRL_MODE_LAST
} AIR_PORT_PHY_LED_CTRL_MODE_T;

typedef enum
{
    AIR_PORT_PHY_LED_STATE_FORCE_PATT = 0,
    AIR_PORT_PHY_LED_STATE_OFF,
    AIR_PORT_PHY_LED_STATE_ON,
    AIR_PORT_PHY_LED_STATE_LAST
} AIR_PORT_PHY_LED_STATE_T;

typedef enum
{
    AIR_PORT_PHY_LED_PATT_HZ_HALF = 0,
    AIR_PORT_PHY_LED_PATT_HZ_ONE,
    AIR_PORT_PHY_LED_PATT_HZ_TWO,
    AIR_PORT_PHY_LED_PATT_LAST
} AIR_PORT_PHY_LED_PATT_T;

typedef enum
{
    AIR_PORT_CABLE_STATUS_OPEN,
    AIR_PORT_CABLE_STATUS_SHORT,
    AIR_PORT_CABLE_STATUS_NORMAL,
    AIR_PORT_CABLE_STATUS_LAST
} AIR_PORT_CABLE_STATUS_T;

typedef enum
{
    AIR_PORT_CABLE_TEST_PAIR_A,
    AIR_PORT_CABLE_TEST_PAIR_B,
    AIR_PORT_CABLE_TEST_PAIR_C,
    AIR_PORT_CABLE_TEST_PAIR_D,
    AIR_PORT_CABLE_TEST_PAIR_ALL,
    AIR_PORT_CABLE_TEST_PAIR_LAST
} AIR_PORT_CABLE_TEST_PAIR_T;

typedef struct AIR_PORT_CABLE_TEST_RSLT_S
{
    AIR_PORT_CABLE_STATUS_T status[AIR_PORT_CABLE_MAX_PAIR];

    /* Cable length = length * 0.1 m */
    UI32_T                  length[AIR_PORT_CABLE_MAX_PAIR];
} AIR_PORT_CABLE_TEST_RSLT_T;

typedef enum
{
    AIR_PORT_OP_MODE_NORMAL,
    AIR_PORT_OP_MODE_LONG_REACH,
    AIR_PORT_OP_MODE_LAST
} AIR_PORT_OP_MODE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_port_setSerdesMode
 * PURPOSE:
 *      Set the SGMII mode for SGMII or 1000BASE_X
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      mode                     -- Struct of the serdes mode
 *                                  AIR_PORT_SERDES_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Speed will change to 1000M, when serdes mode set to 1000BASE_X.
 */
AIR_ERROR_NO_T
air_port_setSerdesMode(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_SERDES_MODE_T mode);

/* FUNCTION NAME:   air_port_getSerdesMode
 * PURPOSE:
 *      Get the SGMII mode
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_mode                 -- Struct of the serdes mode
 *                                  AIR_PORT_SERDES_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getSerdesMode(
    const UI32_T            unit,
    const UI32_T            port,
    AIR_PORT_SERDES_MODE_T *ptr_mode);

/* FUNCTION NAME:   air_port_setPhyAutoNego
 * PURPOSE:
 *      Set the auto-negotiation mode for a specific port
 *      (Auto or Forced)
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      auto_nego                -- Struct of the auto negotiation
 *                                  AIR_PORT_PHY_AN_T

 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setPhyAutoNego(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_PHY_AN_T auto_nego);

/* FUNCTION NAME:   air_port_getPhyAutoNego
 * PURPOSE:
 *      Get the auto-negotiation mode for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_auto_nego            -- Struct of the auto negotiation
 *                                  AIR_PORT_PHY_AN_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhyAutoNego(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_PHY_AN_T *ptr_auto_nego);

/* FUNCTION NAME:   air_port_setPhyLocalAdvAbility
 * PURPOSE:
 *      Set the auto-negotiation advertisement for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      ptr_adv                  -- Struct of the AN advertisement
 *                                  setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setPhyLocalAdvAbility(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   air_port_getPhyLocalAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_adv                  -- Struct of the AN advertisement
 *                                  setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhyLocalAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   air_port_getPhyRemoteAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a partner
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_adv                  -- Struct of the AN advertisement
 *                                  setting
 *                                  AIR_PORT_PHY_AN_ADV_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhyRemoteAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME:   air_port_setSpeed
 * PURPOSE:
 *      Set the speed for a specific port. This setting is used on
 *      force mode only
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      speed                    -- Struct of the port speed
 *                                  AIR_PORT_SPEED_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setSpeed(
    const UI32_T           unit,
    const UI32_T           port,
    const AIR_PORT_SPEED_T speed);

/* FUNCTION NAME:   air_port_getSpeed
 * PURPOSE:
 *      Get the speed for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_speed                -- Struct of the port speed
 *                                  AIR_PORT_SPEED_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getSpeed(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_SPEED_T *ptr_speed);

/* FUNCTION NAME:   air_port_setDuplex
 * PURPOSE:
 *      Get the duplex for a specific port. This setting is used on
 *      force mode only
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      duplex                   -- Struct of the port duplex
 *                                  AIR_PORT_DUPLEX_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setDuplex(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DUPLEX_T duplex);

/* FUNCTION NAME:   air_port_getDuplex
 * PURPOSE:
 *      Get the duplex for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_duplex               -- Struct of the port duplex
 *                                  AIR_PORT_DUPLEX_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getDuplex(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_DUPLEX_T *ptr_duplex);

/* FUNCTION NAME:   air_port_setBackPressure
 * PURPOSE:
 *      Set the back pressure configuration for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- FALSE: Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setBackPressure(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   air_port_getBackPressure
 * PURPOSE:
 *      Get the back pressure configuration for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- FALSE: Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getBackPressure(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_port_setFlowCtrl
 * PURPOSE:
 *      Set the flow control configuration for specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port
 *                                  direction
 *                                  AIR_PORT_DIR_T
 *      fc_en                    -- TRUE: Enable select port flow
 *                                        control
 *                                  FALSE:Disable select port flow
 *                                        control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    const BOOL_T         fc_en);

/* FUNCTION NAME:   air_port_getFlowCtrl
 * PURPOSE:
 *      Get the flow control configuration for specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port direction
 *                                  AIR_PORT_DIR_T
 * OUTPUT:
 *      ptr_fc_en                -- FALSE: Port flow control disable
 *                                  TRUE: Port flow control enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    BOOL_T              *ptr_fc_en);

/* FUNCTION NAME:   air_port_setPhyLoopBack
 * PURPOSE:
 *      Set the loop back configuration for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port loopback
 *                                  direction
 *                                  AIR_PORT_LPBK_DIR_T
 *      enable                   -- FALSE:Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    const BOOL_T              enable);

/* FUNCTION NAME:   air_port_getPhyLoopBack
 * PURPOSE:
 *      Get the loop back configuration for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      dir                      -- Struct of the port loopback
 *                                  direction
 *                                  AIR_PORT_LPBK_DIR_T
 * OUTPUT:
 *      ptr_enable               -- FALSE:Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    BOOL_T                   *ptr_enable);

/* FUNCTION NAME:   air_port_getPortStatus
 * PURPOSE:
 *      Get the physical link status for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_status               -- Strcut of the port status
 *                                  AIR_PORT_STATUS_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPortStatus(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_status);

/* FUNCTION NAME:   air_port_setAdminState
 * PURPOSE:
 *      Set the port status for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      enable                   -- TRUE: port enable
 *                                  FALSE: port disable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setAdminState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   air_port_getAdminState
 * PURPOSE:
 *      Get the port status for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_enable               -- TRUE: port enable
 *                                  FALSE: port disable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getAdminState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_port_setPhySmartSpeedDown
 * PURPOSE:
 *      Set Smart speed down feature for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      ssd_mode                 -- Struct of the port SSD mode
 *                                  AIR_PORT_SSD_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setPhySmartSpeedDown(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_SSD_MODE_T ssd_mode);

/* FUNCTION NAME:   air_port_getPhySmartSpeedDown
 * PURPOSE:
 *      Get Smart speed down feature for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_ssd_mode             -- Struct of the port SSD mode
 *                                  AIR_PORT_SSD_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getPhySmartSpeedDown(
    const UI32_T         unit,
    const UI32_T         port,
    AIR_PORT_SSD_MODE_T *ptr_ssd_mode);

/* FUNCTION NAME:   air_port_setPortMatrix
 * PURPOSE:
 *      Set port matrix from the specified device
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      port_bitmap              -- Matrix port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPortMatrix(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_BITMAP_T port_bitmap);

/* FUNCTION NAME:   air_port_getPortMatrix
 * PURPOSE:
 *      Get port matrix from the specified device
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      port_bitmap              -- Matrix port bitmap
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPortMatrix(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_BITMAP_T port_bitmap);

/* FUNCTION NAME:   air_port_setVlanMode
 * PURPOSE:
 *      Set port-based vlan mechanism from the specified device
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      mode                     -- Struct of the port vlan mode
 *                                  AIR_PORT_VLAN_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setVlanMode(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_PORT_VLAN_MODE_T mode);

/* FUNCTION NAME:   air_port_getVlanMode
 * PURPOSE:
 *      Get port-based vlan mechanism from the specified device
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_mode                 -- Struct of the port vlan mode
 *                                  AIR_PORT_VLAN_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getVlanMode(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_PORT_VLAN_MODE_T *ptr_mode);

/* FUNCTION NAME:   air_port_setPhyLedOnCtrl
 * PURPOSE:
 *      Set LED state for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 *      enable                   -- FALSE:Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable);

/* FUNCTION NAME:   air_port_getPhyLedOnCtrl
 * PURPOSE:
 *      Get LED state for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_enable               -- FALSE:Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_port_setComboMode
 * PURPOSE:
 *      Set the combo mode to PHY or SERDES for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      combo_mode               -- Struct of the port combo mode
 *                                  AIR_PORT_COMBO_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_setComboMode(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_PORT_COMBO_MODE_T combo_mode);

/* FUNCTION NAME:   air_port_getComboMode
 * PURPOSE:
 *      Get the combo mode for a specific port
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_combo_mode           -- Struct of the port combo
 *                                  mode
 *                                  AIR_PORT_COMBO_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
air_port_getComboMode(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_COMBO_MODE_T *ptr_combo_mode);

/* FUNCTION NAME:   air_port_setPhyLedCtrlMode
 * PURPOSE:
 *      Set led control mode for a specific led of the port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 *      ctrl_mode                -- LED control mode enumeration type
 *                                  AIR_PORT_PHY_LED_CTRL_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyLedCtrlMode(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       led_id,
    const AIR_PORT_PHY_LED_CTRL_MODE_T ctrl_mode);

/* FUNCTION NAME:   air_port_getPhyLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_ctrl_mode            -- LED control enumeration type
 *                                  AIR_PORT_PHY_LED_CTRL_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    AIR_PORT_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   air_port_setPhyLedForceState
 * PURPOSE:
 *      Set led force state of the port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 *      state                    -- LED force state
 *                                  AIR_PORT_PHY_LED_STATE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyLedForceState(
    const UI32_T                   unit,
    const UI32_T                   port,
    const UI32_T                   led_id,
    const AIR_PORT_PHY_LED_STATE_T state);

/* FUNCTION NAME:   air_port_getPhyLedForceState
 * PURPOSE:
 *      Get led force state of the port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_state                -- LED force state enumeration type
 *                                  AIR_PORT_PHY_LED_STATE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    AIR_PORT_PHY_LED_STATE_T *ptr_state);

/* FUNCTION NAME:   air_port_setPhyLedForcePattCfg
 * PURPOSE:
 *      Set led force pattern.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 *      pattern                  -- LED force pattern
 *                                  AIR_PORT_PHY_LED_PATT_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyLedForcePattCfg(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const AIR_PORT_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   air_port_getPhyLedForcePattCfg
 * PURPOSE:
 *      Get led force pattern.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_pattern              -- LED force pattern enumeration type
 *                                  AIR_PORT_PHY_LED_PATT_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    AIR_PORT_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME:   air_port_triggerCableTest
 * PURPOSE:
 *      Trigger port cable status.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      test_pair                -- Tested cable pair
 *                                  AIR_PORT_CABLE_TEST_PAIR_T
 * OUTPUT:
 *      ptr_test_rslt            -- Cable diagnostic information
 *                                  AIR_PORT_CABLE_TEST_RSLT_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Support cable diagnostic in speed 1G only.
 */
AIR_ERROR_NO_T
air_port_triggerCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_PAIR_T  test_pair,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME:   air_port_setPhyOpMode
 * PURPOSE:
 *      Set Phy operation mode.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Phy operation mode
 *                                  AIR_PORT_OP_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_setPhyOpMode(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_PORT_OP_MODE_T mode);

/* FUNCTION NAME:   air_port_getPhyOpMode
 * PURPOSE:
 *      Get Phy operation mode.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_mode                 -- Phy operation mode enumeration type
 *                                  AIR_PORT_OP_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_port_getPhyOpMode(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_OP_MODE_T *ptr_mode);

/* FUNCTION NAME:   air_port_triggerLinkDownCableTest
 * PURPOSE:
 *      Trigger to perform link down cable diagnosis.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_test_rslt            -- Cable diagnostic information
 *                                  AIR_PORT_CABLE_TEST_RSLT_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      Support cable diagnostic in link down mode only.
 */
AIR_ERROR_NO_T
air_port_triggerLinkDownCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* GLOBAL VARIABLE EXTERN DECLARATIONS
 */

#endif /* AIR_PORT_H */
