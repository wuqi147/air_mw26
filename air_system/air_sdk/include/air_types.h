/* FILE NAME:   air_types.h
 * PURPOSE:
 *      Define the commom data type in AIR SDK.
 * NOTES:
 */

#ifndef AIR_TYPES_H
#define AIR_TYPES_H

/* INCLUDE FILE DECLARATIONS
 */
#include <osal/osal_type.h>

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef LOW
#define LOW (0)
#endif

#ifndef HIGH
#define HIGH (1)
#endif

#define AIR_INVALID_ID   (0xFFFFFFFF)
#define AIR_PORT_INVALID (AIR_INVALID_ID)

#define AIR_IPV4_ZERO (0)

/* MACRO FUNCTION DECLARATIONS
 */
/* porting from nps */
#if defined(AIR_EN_HOST_32_BIT_BIG_ENDIAN) || defined(AIR_EN_HOST_32_BIT_LITTLE_ENDIAN)
typedef unsigned int AIR_HUGE_T;
#elif defined(AIR_EN_HOST_64_BIT_BIG_ENDIAN) || defined(AIR_EN_HOST_64_BIT_LITTLE_ENDIAN)
typedef unsigned long long int AIR_HUGE_T;
#else
typedef unsigned int AIR_HUGE_T;
#endif
#define AIR_BITMAP_SIZE(bit_num) ((((bit_num) - 1) / 32) + 1)
/*                  */

#define AIR_SEMAPHORE_BINARY       (1)
#define AIR_SEMAPHORE_SYNC         (0)
#define AIR_SEMAPHORE_WAIT_FOREVER (0xFFFFFFFF)

#define AIR_IPV4_IS_MULTICAST(addr) (0xE0000000 == ((addr) & 0xF0000000))
#define AIR_IPV6_IS_MULTICAST(addr) (0xFF == (((UI8_T *)(addr))[0]))
#define AIR_L3_IP_IS_MULTICAST(ptr_ip)                        \
    ((TRUE == (ptr_ip)->ipv4) ?                               \
         AIR_IPV4_IS_MULTICAST((ptr_ip)->ip_addr.ipv4_addr) : \
         AIR_IPV6_IS_MULTICAST((ptr_ip)->ip_addr.ipv6_addr))

#define AIR_L2_MAC_IS_MULTICAST(mac) ((((UI8_T *)mac)[0]) & 0x01)

/* DATA TYPE DECLARATIONS
 */

/* porting from nps_types.h*/

typedef UI8_T  AIR_BIT_MASK_8_T;
typedef UI16_T AIR_BIT_MASK_16_T;
typedef UI32_T AIR_BIT_MASK_32_T;
typedef UI64_T AIR_BIT_MASK_64_T;

typedef UI8_T  AIR_MAC_T[6];
typedef UI32_T AIR_IPV4_T;
typedef UI8_T  AIR_IPV6_T[16];

typedef UI32_T AIR_TIME_T;

/* Bridge Domain id data type. */
typedef UI32_T AIR_BRIDGE_DOMAIN_T;

typedef union AIR_IP_U
{
    AIR_IPV4_T ipv4_addr;
    AIR_IPV6_T ipv6_addr;

} AIR_IP_T;

typedef struct AIR_IP_ADDR_S
{
    AIR_IP_T ip_addr;
    BOOL_T   ipv4;
} AIR_IP_ADDR_T;

typedef UI16_T AIR_VLAN_T;
typedef UI32_T AIR_PORT_T;

typedef enum
{
    AIR_PORT_TYPE_NORMAL = 0,
    AIR_PORT_TYPE_UNIT_PORT,
    AIR_PORT_TYPE_LAG,
    AIR_PORT_TYPE_CPU_PORT,
    AIR_PORT_TYPE_LAST
} AIR_PORT_TYPE_T;

/*support Green/Yellow/Red color*/
typedef enum
{
    AIR_COLOR_GREEN = 0,
    AIR_COLOR_YELLOW,
    AIR_COLOR_RED,
    AIR_COLOR_LAST
} AIR_COLOR_T;

typedef enum
{
    AIR_FWD_ACTION_FLOOD = 0,
    AIR_FWD_ACTION_NORMAL,
    AIR_FWD_ACTION_DROP,
    AIR_FWD_ACTION_COPY_TO_CPU,
    AIR_FWD_ACTION_REDIRECT_TO_CPU,
    AIR_FWD_ACTION_FLOOD_COPY_TO_CPU,
    AIR_FWD_ACTION_DROP_COPY_TO_CPU,
    AIR_FWD_ACTION_LAST
} AIR_FWD_ACTION_T;

typedef AIR_HUGE_T AIR_THREAD_ID_T;
typedef AIR_HUGE_T AIR_SEMAPHORE_ID_T;
typedef AIR_HUGE_T AIR_ISRLOCK_ID_T;
typedef AIR_HUGE_T AIR_IRQ_FLAGS_T;

typedef enum
{
    AIR_DIR_INGRESS = 0,
    AIR_DIR_EGRESS,
    AIR_DIR_BOTH,
    AIR_DIR_LAST
} AIR_DIR_T;

typedef enum
{
    AIR_VLAN_ACTION_SET,
    AIR_VLAN_ACTION_KEEP,
    AIR_VLAN_ACTION_REMOVE,
    AIR_VLAN_ACTION_LAST
} AIR_VLAN_ACTION_T;

/* VLAN Precedence */
/* 000 = SUBNET_PROTOCOL_MAC_PORT
 * 001 = SUBNET_MAC_PROTOCOL_PORT
 * 010 = PROTOCOL_SUBNET_MAC_PORT
 * 011 = PROTOCOL_MAC_SUBNET_PORT
 * 100 = MAC_SUBNET_PROTOCOL_PORT
 * 101 = MAC_PROTOCOL_SUBNET_PORT
 */
typedef enum
{
    AIR_VLAN_PRECEDENCE_SUBNET_MAC_PROTOCOL_PORT = 1,
    AIR_VLAN_PRECEDENCE_MAC_SUBNET_PROTOCOL_PORT = 4,
    AIR_VLAN_PRECEDENCE_PORT_ONLY = 7,
    AIR_VLAN_PRECEDENCE_FAVOR_TYPE = 8,
    AIR_VLAN_PRECEDENCE_FAVOR_ADDR = 9,
    AIR_VLAN_PRECEDENCE_LAST
} AIR_VLAN_PRECEDENCE_T;

/* VLAN Tag Type */
typedef enum
{
    AIR_VLAN_TAG_NONE = 0,   /* UnTag                                */
    AIR_VLAN_TAG_SINGLE_PRI, /* Single Customer/Service Priority Tag */
    AIR_VLAN_TAG_SINGLE,     /* Single Customer/Service Tag          */
    AIR_VLAN_TAG_DOUBLE_PRI, /* Double Tag with any VID=0            */
    AIR_VLAN_TAG_DOUBLE,     /* Double Tag                           */
    AIR_VLAN_TAG_LAST
} AIR_VLAN_TAG_T;

typedef struct AIR_BUM_INFO_S
{
    UI32_T mcast_id;
    UI32_T group_label; /* l2 da group label */
    UI32_T vid;         /* used when FLAGS_ADD_VID is set */

#define AIR_BUM_INFO_FLAGS_MCAST_VALID    (1U << 0)
#define AIR_BUM_INFO_FLAGS_TO_CPU         (1U << 1)
#define AIR_BUM_INFO_FLAGS_ADD_VID        (1U << 2) /* single tag to double tag (i.e) QinQ */
#define AIR_BUM_INFO_FLAGS_TRILL_ALL_TREE (1U << 3)
    UI32_T flags;
} AIR_BUM_INFO_T;

typedef enum
{
    AIR_PHY_TYPE_INTERNAL = 0x0,
    AIR_PHY_TYPE_EXTERNAL,
    AIR_PHY_TYPE_LAST
} AIR_PHY_TYPE_T;

typedef enum
{
    AIR_PHY_DEVICE_ADDR_PMA_PMD = 1,
    AIR_PHY_DEVICE_ADDR_WIS = 2,
    AIR_PHY_DEVICE_ADDR_PCS = 3,
    AIR_PHY_DEVICE_ADDR_PHY_XS = 4,
    AIR_PHY_DEVICE_ADDR_DTE_XS = 5,
    AIR_PHY_DEVICE_ADDR_TC = 6,
    AIR_PHY_DEVICE_ADDR_AN = 7,
    AIR_PHY_DEVICE_ADDR_VENDOR_1 = 30,
    AIR_PHY_DEVICE_ADDR_VENDOR_2 = 31,
    AIR_PHY_DEVICE_ADDR_LAST
} AIR_PHY_DEVICE_ADDR_T;

typedef enum
{
    AIR_BULK_OP_MODE_ERR_STOP = 0,
    AIR_BULK_OP_MODE_ERR_CONTINUE,
    AIR_BULK_OP_MODE_LAST
} AIR_BULK_OP_MODE_T;

typedef struct AIR_RANGE_INFO_S
{
    UI32_T min_id;
    UI32_T max_id;
    UI32_T max_member_cnt;

#define AIR_RANGE_INFO_FLAGS_MAX_MEMBER_CNT (1U << 0)
    UI32_T flags;
} AIR_RANGE_INFO_T;

typedef struct AIR_FDL_INFO_S
{
    UI32_T probability /* percentage from 0~100 */;
    UI32_T threshold; /* range 0 ~ (2^20)-1 */
} AIR_FDL_INFO_T;

typedef enum
{
    AIR_IP_TYPE_IPV4 = 0,
    AIR_IP_TYPE_IPV6,
    AIR_IP_TYPE_LAST
} AIR_IP_TYPE_T;

/***************************/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif /* AIR_TYPES_H */
