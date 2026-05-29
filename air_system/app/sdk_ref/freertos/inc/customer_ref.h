#ifndef CUSTOMER_REF_H
#define CUSTOMER_REF_H

AIR_ERROR_NO_T customer_ref_registerConfig(UI32_T unit);
AIR_ERROR_NO_T customer_ref_initPortMap(UI32_T unit);

#ifdef AIR_EN_POE
AIR_ERROR_NO_T customer_ref_initPoePortMap(UI32_T unit);
AIR_ERROR_NO_T customer_ref_initPoeDeviceMap(UI32_T unit);
#endif

#ifdef AIR_EN_SFP_LED
AIR_ERROR_NO_T customer_ref_initSfpLed(UI32_T unit);
#endif

#ifdef AIR_EN_I2C_BITBANG
AIR_ERROR_NO_T customer_ref_initAppI2cBitbang(UI32_T unit);
#endif
#endif
