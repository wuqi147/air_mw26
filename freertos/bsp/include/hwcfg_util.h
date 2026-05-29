#ifndef HWCFG_UTIL_H
#define HWCFG_UTIL_H

/* INCLUDE FILE DECLARTIONS
*/

/* NAMING CONSTANT DECLARATIONS
*/

#define PHY_IAC                             (0x10218020)
#define CSR_PHY_ACS_ST_OFFT                 (31)
#define CSR_PHY_ACS_ST_LENG                 (1)
#define CSR_PHY_ACS_ST_RELMASK              (0x00000001)
#define CSR_PHY_ACS_ST_MASK                 (CSR_PHY_ACS_ST_RELMASK << CSR_PHY_ACS_ST_OFFT)
#define CSR_MDIO_REG_ADDR_OFFT              (25)
#define CSR_MDIO_REG_ADDR_LENG              (5)
#define CSR_MDIO_REG_ADDR_RELMASK           (0x0000001f)
#define CSR_MDIO_REG_ADDR_MASK              (CSR_MDIO_REG_ADDR_RELMASK << CSR_MDIO_REG_ADDR_OFFT)
#define CSR_MDIO_PHY_ADDR_OFFT              (20)
#define CSR_MDIO_PHY_ADDR_LENG              (5)
#define CSR_MDIO_PHY_ADDR_RELMASK           (0x0000001f)
#define CSR_MDIO_PHY_ADDR_MASK              (CSR_MDIO_PHY_ADDR_RELMASK << CSR_MDIO_PHY_ADDR_OFFT)
#define CSR_MDIO_CMD_OFFT                   (18)
#define CSR_MDIO_CMD_LENG                   (2)
#define CSR_MDIO_CMD_RELMASK                (0x00000003)
#define CSR_MDIO_CMD_MASK                   (CSR_MDIO_CMD_RELMASK << CSR_MDIO_CMD_OFFT)
#define CSR_MDIO_ST_OFFT                    (16)
#define CSR_MDIO_ST_LENG                    (2)
#define CSR_MDIO_ST_RELMASK                 (0x00000003)
#define CSR_MDIO_ST_MASK                    (CSR_MDIO_ST_RELMASK << CSR_MDIO_ST_OFFT)
#define CSR_MDIO_WR_DATA_OFFT               (0)
#define CSR_MDIO_WR_DATA_LENG               (16)
#define CSR_MDIO_WR_DATA_RELMASK            (0x0000FFFF)
#define CSR_MDIO_WR_DATA_MASK               (CSR_MDIO_WR_DATA_RELMASK << CSR_MDIO_WR_DATA_OFFT)

#define PHY_IAD                             (0x10218024)
#define CSR_MDIO_RD_DATA_OFFT               (0)
#define CSR_MDIO_RD_DATA_LENG               (16)
#define CSR_MDIO_RD_DATA_RELMASK            (0x0000FFFF)
#define CSR_MDIO_RD_DATA_MASK               (CSR_MDIO_RD_DATA_RELMASK << CSR_MDIO_RD_DATA_OFFT)
#define PHY_ACS_TIMEOUT                     (0x80000000)

/* EN8801S PBUS */
#define EN8801S_PBUS_RG_OUI                 (0x19a4)
#define EN8801S_RG_SMI_ADDR                 (0x19a8)
#define EN8801S_RG_BUCK_CTL                 (0x1a20)
#define EN8801S_RG_LTR_CTL                  (0x0cf8)

#define EN8801S_PBUS_OUI                    (0x17a5)
#define EN8801S_1_PHY_ADDR                  (0x1b)
#define EN8801S_1_PBUS_ADDR                 (EN8801S_1_PHY_ADDR + 1)
#define EN8801S_2_PHY_ADDR                  (0x1d)
#define EN8801S_2_PBUS_ADDR                 (EN8801S_2_PHY_ADDR + 1)
#define EN8801S_DEFAULT_PBUS_ADDR           (0x1e)
#define MII_PHYSID1                         (0x02)
#define MII_PHYSID2                         (0x03)

/* MACRO FUNCTION DECLARATIONS
*/

#define SET_MDIO_ACS_IDLE                   (0 << CSR_PHY_ACS_ST_OFFT)
#define SET_MDIO_ACS_START                  (1 << CSR_PHY_ACS_ST_OFFT)
#define SET_MDIO_CMD_CL22W                  (1 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL22R                  (2 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL45ADDR               (0 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL45W                  (1 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL45INC                (2 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_CMD_CL45R                  (3 << CSR_MDIO_CMD_OFFT)
#define SET_MDIO_ST_CL45                    (0 << CSR_MDIO_ST_OFFT)
#define SET_MDIO_ST_CL22                    (1 << CSR_MDIO_ST_OFFT)
#define SET_MDIO_PHY(phy)                   ((phy&CSR_MDIO_PHY_ADDR_RELMASK) << CSR_MDIO_PHY_ADDR_OFFT)
#define SET_MDIO_REG(reg)                   ((reg&CSR_MDIO_REG_ADDR_RELMASK) << CSR_MDIO_REG_ADDR_OFFT)
#define SET_MDIO_DATA(data)                 ((data&CSR_MDIO_WR_DATA_RELMASK) << CSR_MDIO_WR_DATA_OFFT)
#define GET_MDIO_DATA(data)                 ((data&CSR_MDIO_RD_DATA_RELMASK) << CSR_MDIO_RD_DATA_OFFT)

/* GPIO */
#define RG_FORCE_GPIO_EN    (0x10000090)
#define RG_GPIO_IO_CTRL     (0x1000a300)
#define RG_GPIO_IO_CTRL1    (0x1000a320)
#define RG_GPIO_DATA        (0x1000a304)
#define RG_GPIO_OUTPUT_EN   (0x1000a314)
#define ENABLE_PIN_OE       (1)
#define DISABLE_PIN_OE      (0)
#define GPIO_PIN_HIGH       (1)
#define GPIO_PIN_LOW        (0)

#define GPIO_PIN0           (0)
#define GPIO_PIN1           (1)
#define GPIO_PIN2           (2)
#define GPIO_PIN3           (3)
#define GPIO_PIN4           (4)
#define GPIO_PIN5           (5)
#define GPIO_PIN6           (6)
#define GPIO_PIN7           (7)
#define GPIO_PIN8           (8)
#define GPIO_PIN9           (9)
#define GPIO_PIN10          (10)
#define GPIO_PIN11          (11)
#define GPIO_PIN12          (12)
#define GPIO_PIN13          (13)
#define GPIO_PIN14          (14)
#define GPIO_PIN15          (15)
#define GPIO_PIN16          (16)
#define GPIO_PIN17          (17)
#define GPIO_PIN18          (18)

void mdio_writeCL22(
    const unsigned char  phy_addr,
    const unsigned char  reg,
    const unsigned short data);

void mdio_readCL22(
    const unsigned char  phy_addr,
    const unsigned char  reg,
          unsigned short *ptr_data);

unsigned int en8801s_pbus_rd(unsigned char pid, unsigned int pbus_address);
void en8801s_pbus_wr(unsigned char pid, unsigned int pbus_address, unsigned int pbus_data);

#endif /* End of HWCFG_UTIL_H */

