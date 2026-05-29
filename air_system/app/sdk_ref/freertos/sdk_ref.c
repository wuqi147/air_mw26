#include <sdk_ref.h>

#include <air_init.h>
#include <customer_ref.h>
#include <osal/osal.h>
#include <osal/osali.h>
#include <stdio.h>
#include <string.h>
#ifdef AIR_EN_SFP_LED
#include <sfp_led.h>
#endif

#ifdef AIR_EN_I2C_BITBANG
#include <i2c_bitbang.h>
#endif

#ifdef AIR_EN_POE
AIR_ERROR_NO_T
air_init_initPoeMap(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = customer_ref_initPoeDeviceMap(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    rc = customer_ref_initPoePortMap(unit);
    return rc;
}
#endif

AIR_ERROR_NO_T
sdk_ref_initApp(
    const UI32_T unit)
{
    UI32_T rc = AIR_E_OK;

#ifdef AIR_EN_SFP_LED
    rc = customer_ref_initSfpLed(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }
#endif

#ifdef AIR_EN_I2C_BITBANG
    rc = customer_ref_initAppI2cBitbang(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }
#endif

    return rc;
}

void
sdk_ref_write(
    const void *ptr_buf,
    UI32_T      len)
{
    C8_T buf[OSAL_PRN_BUF_SZ];
    /* length not include '\0', it need add 1*/
    snprintf(buf, len + 1, "%s", (const char *)ptr_buf);
    printf("%s", buf);
}

AIR_ERROR_NO_T
sdk_ref_init(
    void)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    AIR_INIT_PARAM_T init_cmn;

    printf("Initializing common modules...\n");
    memset(&init_cmn, 0, sizeof(init_cmn));
    init_cmn.dsh_write_func = sdk_ref_write;
    init_cmn.debug_write_func = sdk_ref_write;
    rc = air_init_initCmnModule(&init_cmn);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize common modules!\n");
        return rc;
    }

    /* register customer configuration callback for customized sdk setting */
    rc = customer_ref_registerConfig(0);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    /* air init */
    osal_printf("Initializing low level modules...\n");
    rc = air_init_initLowLevel(0);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize low level modules!\n");
        return rc;
    }

    /* initialize port mapping */
    rc = customer_ref_initPortMap(0);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

#ifdef AIR_EN_POE
    /* initialize poe mapping */
    rc = air_init_initPoeMap(0);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize poe port & device mapping!\n");
        return rc;
    }
#endif

    osal_printf("Initializing sdk task resources...\n");
    rc = air_init_initTaskRsrc(0);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize sdk task resources!\n");
        return rc;
    }

    /* sdk module initialization */
    osal_printf("Initializing sdk modules...\n");
    rc = air_init_initModule(0);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize sdk modules!\n");
        return rc;
    }

    osal_printf("Initializing sdk tasks...\n");
    rc = air_init_initTask(0);
    if (AIR_E_OK != rc)
    {
        osal_printf("Error: Failed to initialize sdk tasks!\n");
        return rc;
    }

    osal_printf("Initializing sdk apps...\n");
    rc = sdk_ref_initApp(0);

    return rc;
}
