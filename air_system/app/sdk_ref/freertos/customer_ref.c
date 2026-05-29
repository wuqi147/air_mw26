#include <air_cfg.h>
#include <air_init.h>
#include <osal/osal.h>

#ifdef AIR_EN_SFP_LED
#include <sfp_led.h>
#endif

#ifdef AIR_EN_I2C_BITBANG
#include <i2c_bitbang.h>
#endif

typedef struct SDK_CFG_S
{
    UI32_T                      unit;
    AIR_CFG_TYPE_T              cfg_type;
    UI32_T                      param0;
    UI32_T                      param1;
    UI32_T                      value;
} SDK_CFG_T;


const static SDK_CFG_T _customer_ref_cfg[] =
{
/*
 *  unit    AIR_CFG_TYPE_XXX                    param0  param1  value
 *  ----    ----------------                    ------  ------  -----
 */
    {0, AIR_CFG_TYPE_IFMON_ENABLE,              0,      0,     TRUE},
    {0, AIR_CFG_TYPE_IFMON_THREAD_PRI,          0,      0,        5},
    {0, AIR_CFG_TYPE_IFMON_THREAD_STACK,        0,      0,      606},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               1,      0,        8},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               2,      0,        7},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               3,      0,        6},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               4,      0,        5},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               5,      0,        1},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               6,      0,        2},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               7,      0,        3},
    {0, AIR_CFG_TYPE_PHY_ADDRESS,               8,      0,        4},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      0,     0x61},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          1,      1,    0x182},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          2,      1,    0x182},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          3,      1,    0x182},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          4,      1,    0x182},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          5,      1,    0x182},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          6,      1,    0x182},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          7,      1,    0x182},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          8,      1,    0x182},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,          9,      0,    0x861},
    {0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR,         10,      0,    0x861},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,   9,      0,        1},
    {0, AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,  10,      0,        1},
    {0, AIR_CFG_TYPE_PHY_LED_COUNT,             0,      0,        2},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      6,      0,        1},
    {0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,      8,      0,        1},
};

const static AIR_INIT_PORT_MAP_T _ref_board_port_map[] =
{
/*
 * port    AIR_INIT_PORT_SPEED_T    AIR_INIT_PORT_TYPE_T    ...
 * ----    ---------------------    --------------------
 */
    { 0,    AIR_INIT_PORT_TYPE_CPU, AIR_INIT_PORT_SPEED_1000M},
    { 1,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port={0, 7}},
    { 2,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port={0, 6}},
    { 3,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port={0, 5}},
    { 4,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port={0, 4}},
    { 5,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port={0, 0}},
    { 6,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port={0, 1}},
    { 7,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port={0, 2}},
    { 8,  AIR_INIT_PORT_TYPE_ENHANCED_BASET, AIR_INIT_PORT_SPEED_1000M,  .enhanced_baset_port={0, 3}},
    { 9,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 0, 0}},
    {10,  AIR_INIT_PORT_TYPE_XSGMII, AIR_INIT_PORT_SPEED_1000M,  .xsgmii_port={0, 1, 0}}
};

#ifdef AIR_EN_POE
const static AIR_INIT_POE_DEVICE_MAP_T _poe_device_map[] =
{
  /* unit,  device index,  i2c bus id, i2c address */
    {0,     0,             0,          0x74},
};

const static AIR_INIT_POE_PORT_MAP_T _poe_port_map[] =
{
  /* unit,  air port,   type,   primary port,   secondary port */
    {0, 1,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 0} },
    {0, 2,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 1} },
    {0, 3,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 2} },
    {0, 4,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 3} },
    {0, 5,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 4} },
    {0, 6,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 5} },
    {0, 7,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 6} },
    {0, 8,  AIR_INIT_POE_PORT_TYPE_AF_AT,   .primary_port = {0, 7} },
};
#endif

#ifdef AIR_EN_SFP_LED
/* SFP port id and LED pin id mapping table
 * Should be defined in compile time
 */
const static SFP_LED_PORT_MAP_T _sfp_led_port_map[] =
{
  /* unit,  port id,  led pin id */
    {0,     9,        6},
    {0,    10,        8}
};
#endif

#ifdef AIR_EN_I2C_BITBANG
const static I2C_BITBANG_CFG_T i2c_bitbang_cfg[] = {
/**
 * unit  SDA  SCL
 * ----  ---  ---
 */
    {0,   19,  20}
};
#endif

static AIR_ERROR_NO_T
_ref_getConfigValue(
    const UI32_T unit,
    const AIR_CFG_TYPE_T type,
    AIR_CFG_VALUE_T *ptr_value)
{
    AIR_ERROR_NO_T rc = AIR_E_ENTRY_NOT_FOUND;
    const SDK_CFG_T *ptr_cfg;
    UI32_T i;
    UI32_T cfg_num;

    cfg_num = sizeof(_customer_ref_cfg)/sizeof(SDK_CFG_T);
    ptr_cfg = _customer_ref_cfg;

    for (i = 0; i < cfg_num; i++)
    {
        if ((ptr_cfg[i].unit == unit)
            && (ptr_cfg[i].cfg_type == type)
            && (ptr_cfg[i].param0 == ptr_value->param0)
            && (ptr_cfg[i].param1 == ptr_value->param1))
        {
            rc = AIR_E_OK;
            ptr_value->value = ptr_cfg[i].value;
            break;
        }
    }

    return  rc;
}

AIR_ERROR_NO_T customer_ref_registerConfig(UI32_T unit)
{
    AIR_ERROR_NO_T rc;
    rc = air_cfg_register(unit, _ref_getConfigValue);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to register customized sdk setting!\n");
    }
    else
    {
        osal_printf("Registering customized sdk setting...\n");
    }
    return rc;
}

AIR_ERROR_NO_T customer_ref_initPortMap(UI32_T unit)
{
    AIR_ERROR_NO_T rc;
    rc = air_init_initSdkPortMap(unit, sizeof(_ref_board_port_map) / sizeof(AIR_INIT_PORT_MAP_T), _ref_board_port_map);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize port map setting!\n");
    }
    else
    {
        osal_printf("Initializing port map setting...\n");
    }
    return rc;
}

#ifdef AIR_EN_POE
AIR_ERROR_NO_T customer_ref_initPoePortMap(UI32_T unit)
{
    AIR_ERROR_NO_T rc;
    rc = air_init_initPoePortMap(unit, sizeof(_poe_port_map) / sizeof(AIR_INIT_POE_PORT_MAP_T), _poe_port_map);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize poe port map setting!\n");
    }
    else
    {
        osal_printf("Initializing poe port map setting...\n");
    }
    return rc;
}

AIR_ERROR_NO_T customer_ref_initPoeDeviceMap(UI32_T unit)
{
    AIR_ERROR_NO_T rc;
    rc = air_init_initPoeDeviceMap(unit, sizeof(_poe_device_map) / sizeof(AIR_INIT_POE_DEVICE_MAP_T), _poe_device_map);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize poe device map setting!\n");
    }
    else
    {
        osal_printf("Initializing poe device map setting...\n");
    }
    return rc;
}
#endif

#ifdef AIR_EN_SFP_LED
AIR_ERROR_NO_T customer_ref_initSfpLed(UI32_T unit)
{
    AIR_ERROR_NO_T rc;
    rc = sfp_led_init(unit, sizeof(_sfp_led_port_map) / sizeof(SFP_LED_PORT_MAP_T), (SFP_LED_PORT_MAP_T *)_sfp_led_port_map, NULL);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize sfp led setting!\n");
    }
    else
    {
        osal_printf("Initializing sfp led setting...\n");
    }
    return rc;
}
#endif

#ifdef AIR_EN_I2C_BITBANG
AIR_ERROR_NO_T customer_ref_initAppI2cBitbang(UI32_T unit)
{
    UI32_T rc = AIR_E_OK;

    rc = i2c_bitbang_init(unit, 1, (I2C_BITBANG_CFG_T *)i2c_bitbang_cfg);
    if(rc != AIR_E_OK)
    {
        osal_printf("Error: Failed to initialize i2c bitbang setting!\n", rc);
    }
    else
    {
        osal_printf("Initializing i2c bitbang setting...\n");
    }
    return rc;
}
#endif
