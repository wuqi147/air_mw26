#include <gpio.h>
#include <sif.h>
#include <timer.h>

#define GPIO_PIN_HIGH           (1)
#define GPIO_PIN_LOW            (0)
#define GPIO_PIN_INPUT          (0)
#define GPIO_PIN_OUTPUT         (1)
#define SLED_CLOCK_NUM          (25)    /* for per-port 3 LED */

#define EN8808_1_RST_PIN        (12)
#define EN8808_2_RST_PIN        (1)
#define EN8804_RST_PIN          (7)
#define SLED_RST_PIN            (4)     /* EN8851C is gpio 12 */
#define SLED_CLK_PIN            (2)
#define SLED_DATA_PIN           (3)

#define SFP_LED_NUM             (8)
#define SFP_1_LED0_PIN          (4)
#define SFP_1_LED1_PIN          (5)
#define SFP_2_LED0_PIN          (6)
#define SFP_2_LED1_PIN          (9)
#define SFP_3_LED0_PIN          (11)
#define SFP_3_LED1_PIN          (13)
#define SFP_4_LED0_PIN          (15)
#define SFP_4_LED1_PIN          (16)

/* i2c definitation */
#define I2C_CHANNEL             (1)
#define EN8804_SID              (0x52)
#define EN8808_1_SID            (0x50)
#define EN8808_2_SID            (0x51)
#define PRODUCT_ID_REG          (0x10005000)
#define IOMUX_REG               (0x10000090)

#define EN8808_4_IOMUX_DATA     (0x3bfff)
#define EN8804_IOMUX_DATA       (0x3bfff)
#define EN8851_ID               (0x8851)

/* MACRO FUNCTION DECLARATIONS*/
#define SET_DIR_OUTPUT_LOW(x)    do                 \
    {                                               \
        gpio_set_direction(x, GPIO_PIN_OUTPUT);     \
        gpio_set_data(x, GPIO_PIN_LOW);             \
    }while(0)

#define I2C_RG_WRITE(__SID__, __REG__, __VAL__) do  \
    {                                               \
        sif_parm.bus = I2C_CHANNEL;                 \
        sif_parm.slave_id = __SID__;                \
        sif_parm.addr = __REG__;                    \
        sif_parm.ptr_data = __VAL__;                \
        sif_write(&sif_parm);                       \
    }while(0)

#define I2C_RG_READ(__SID__, __REG__, __VAL__) do   \
    {                                               \
        sif_parm.bus = I2C_CHANNEL;                 \
        sif_parm.slave_id = __SID__;                \
        sif_parm.addr = __REG__;                    \
        sif_parm.ptr_data = __VAL__;                \
        sif_read(&sif_parm);                        \
    }while(0)

#ifdef BOOT_LED_ALL_ON
static unsigned int sfp_led_pin[] = { 4, 6, 11, 15, 5, 9, 13, 16 };
#endif

static void _probe_i2c_phy(const unsigned int sid, const unsigned int reset_pin, const unsigned int led_active)
{
    SIF_PARAM_T     sif_parm;
    unsigned int    val = 0, i = 0, clk_high = 0, clk_low = 0;
#ifdef BOOT_LED_ALL_ON
    unsigned int    val_oe = 0, val_data = 0, val_ctl = 0, val_ctl1 = 0, pin = 0;
#endif

    I2C_RG_READ(sid, PRODUCT_ID_REG, &val);
    if (EN8851_ID == val)
    {
        /* force GPIO 2 & 3 to output */
        val = EN8808_4_IOMUX_DATA;
        I2C_RG_WRITE(sid, IOMUX_REG, &val);
        I2C_RG_READ(sid, RG_GPIOOE, &val);
        val |= ((1 << SLED_CLK_PIN) | (1 << SLED_DATA_PIN));
        I2C_RG_WRITE(sid, RG_GPIOOE, &val);
#ifdef BOOT_LED_ALL_ON
        val_oe = val;
#endif
        I2C_RG_READ(sid, RG_GPIODATA, &clk_low);
        if (GPIO_PIN_LOW == led_active)
        {
            clk_low &= ~(1 << SLED_DATA_PIN);    /* set SLED data to low */
        }
        else
        {
            clk_low |= (1 << SLED_DATA_PIN);    /* set SLED data to high */
        }
        clk_low &= ~(1 << SLED_CLK_PIN);
        I2C_RG_WRITE(sid, RG_GPIODATA, &clk_low);
#ifdef BOOT_LED_ALL_ON
        val_data = clk_low;
#endif
        I2C_RG_READ(sid, RG_GPIOCTRL, &val);
        val &= ~((TWO_BIT_MSK << (SLED_CLK_PIN * 2)) | (TWO_BIT_MSK << (SLED_DATA_PIN * 2)));
        val |= ((GPIO_PIN_OUTPUT << (SLED_CLK_PIN * 2)) | (GPIO_PIN_OUTPUT << (SLED_DATA_PIN * 2)));
        I2C_RG_WRITE(sid, RG_GPIOCTRL, &val);
#ifdef BOOT_LED_ALL_ON
        val_ctl = val;
#endif
        clk_high = clk_low | (1 << SLED_CLK_PIN);
        for (i = 0; i < SLED_CLOCK_NUM; i++)
        {
            I2C_RG_WRITE(sid, RG_GPIODATA, &clk_high);
            I2C_RG_WRITE(sid, RG_GPIODATA, &clk_low);
        }
#ifdef BOOT_LED_ALL_ON
        /* always active low */
        if (EN8804_SID == sid)
        {
            I2C_RG_READ(sid, RG_GPIOCTRL1, &val_ctl1);
            for (i = 0; i < SFP_LED_NUM; i++)
            {
                pin = sfp_led_pin[i];
                val_oe |= (1 << pin);
                val_data &= ~(1 << pin);
                if (pin < 16)
                {
                    val_ctl |= (1 << (pin * 2));
                }
                else
                {
                    val_ctl1 |= (1 << ((pin - 16) * 2));
                }
            }
            I2C_RG_WRITE(sid, RG_GPIOCTRL, &val_ctl);
            I2C_RG_WRITE(sid, RG_GPIOCTRL1, &val_ctl1);
            I2C_RG_WRITE(sid, RG_GPIODATA, &val_data);
            I2C_RG_WRITE(sid, RG_GPIOOE, &val_oe);
        }
#endif
    }
    else
    {
        if (SLED_RST_PIN != reset_pin)
        {
            /* if the PHY can not be probed then keep gpio in input */
            gpio_set_direction(reset_pin, GPIO_PIN_INPUT);
        }
    }
}

int sled_init(void)
{
    unsigned int i = 0, led_active = GPIO_PIN_LOW;

    gpio_request(EN8808_1_RST_PIN);
    gpio_request(EN8808_2_RST_PIN);
    gpio_request(EN8804_RST_PIN);
    gpio_request(SLED_RST_PIN);
    gpio_request(SLED_CLK_PIN);
    gpio_request(SLED_DATA_PIN);

    SET_DIR_OUTPUT_LOW(EN8808_1_RST_PIN);
    SET_DIR_OUTPUT_LOW(EN8808_2_RST_PIN);
    SET_DIR_OUTPUT_LOW(EN8804_RST_PIN);
    SET_DIR_OUTPUT_LOW(SLED_RST_PIN);
    SET_DIR_OUTPUT_LOW(SLED_CLK_PIN);
    SET_DIR_OUTPUT_LOW(SLED_DATA_PIN);
    delay1ms(10);
    gpio_set_data(EN8808_1_RST_PIN, GPIO_PIN_HIGH);
    gpio_set_data(EN8808_2_RST_PIN, GPIO_PIN_HIGH);
    gpio_set_data(EN8804_RST_PIN, GPIO_PIN_HIGH);
    gpio_set_data(SLED_RST_PIN, GPIO_PIN_HIGH);

#ifdef BOOT_LED_ACTIVE_HIGH
#ifdef BOOT_LED_ALL_ON
    led_active = GPIO_PIN_HIGH;
#else
    led_active = GPIO_PIN_LOW;
#endif
#else
#ifdef BOOT_LED_ALL_ON
    led_active = GPIO_PIN_LOW;
#else
    led_active = GPIO_PIN_HIGH;
#endif
#endif
    gpio_set_data(SLED_DATA_PIN, led_active);
    for (i = 0; i < SLED_CLOCK_NUM; i++)
    {
        gpio_set_data(SLED_CLK_PIN, GPIO_PIN_HIGH);
        gpio_set_data(SLED_CLK_PIN, GPIO_PIN_LOW);
    }
    gpio_free(SLED_CLK_PIN);
    gpio_free(SLED_DATA_PIN);
    /* prboe i2c PHY */
    _probe_i2c_phy(EN8804_SID, EN8804_RST_PIN, led_active);
    _probe_i2c_phy(EN8808_1_SID, EN8808_1_RST_PIN, led_active);
    _probe_i2c_phy(EN8808_2_SID, EN8808_2_RST_PIN, led_active);
    return 0;
}

