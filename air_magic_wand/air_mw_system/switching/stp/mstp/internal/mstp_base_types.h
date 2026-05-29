
// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

#ifndef MSTP_LIB_BASE_TYPES_H
#define MSTP_LIB_BASE_TYPES_H

#include <assert.h>
#include <string.h>
#include <stdint.h>

typedef unsigned char       bool;
typedef unsigned short      Port_ID;
typedef unsigned int        PortIndex;
typedef unsigned int        TreeIndex;

/* RSTP Bridge identifier [9.2.5].  Top four most significant bits are a
 * priority value. The next most significant twelve bits are a locally
 * assigned system ID extension. Bottom 48 bits are MAC address of bridge.
 */
typedef unsigned char  BRIDGE_ID[8];

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef false
#define false               0
#endif

#ifndef true
#define true                1
#endif

#ifndef NULL
#define NULL                (void *)0
#endif
#define STP_REGION_NAME_LEN (32)

#define BRIDGE_STR          "%02x-%02x: %02x-%02x-%02x-%02x-%02x-%02x"
#define BRIDGE_VALUE(BID)   BID[0], BID[1], BID[2], BID[3], BID[4], BID[5],BID[6], BID[7]

#define CIST_INDEX      (0)
/* These macros work well for both PortID and BridgeID */
#define GET_PRIORITY_FROM_IDENTIFIER(id)    (((unsigned char *)(&(id)))[0] & 0xF0)
#define SET_PRIORITY_IN_IDENTIFIER(pri, id) do{ \
    unsigned char *first_octet = (unsigned char *)(&(id));        \
    *first_octet &= 0x0F;                       \
    *first_octet |= ((pri) & 0xF0);               \
    }while(0)


// ============================================================================
// 13.10 and 13.11 in 802.1Q-2018
typedef struct PRIORITY_VECTOR_S
{
    BRIDGE_ID       RootId;                 // a) - used for CIST, zero for MSTIs
    unsigned int    ExternalRootPathCost;   // b) - used for CIST, zero for MSTIs
    BRIDGE_ID       RegionalRootId;         // c)
    unsigned int    InternalRootPathCost;   // d)
    BRIDGE_ID       DesignatedBridgeId;     // e)
    unsigned short  DesignatedPortId;       // f)
}__attribute__((packed))PRIORITY_VECTOR;

// ============================================================================

typedef struct TIMES_S
{
    unsigned short ForwardDelay;
    unsigned short HelloTime;
    unsigned short MaxAge;
    unsigned short MessageAge;
    unsigned char remainingHops;
}__attribute__((packed))TIMES;

// ============================================================================
typedef enum STP_PORT_ROLE_S
{
    STP_PORT_ROLE_UNDEFINED = 0,
    STP_PORT_ROLE_DISABLED,
    STP_PORT_ROLE_ROOT,
    STP_PORT_ROLE_DESIGNATED,
    STP_PORT_ROLE_ALTERNATE,
    STP_PORT_ROLE_BACKUP,
    STP_PORT_ROLE_MASTER,
}STP_PORT_ROLE;

// 11.3 Point-to-point parameters in 802.1AC-2016 (values correspond to ieee8021BridgeBasePortAdminPointToPoint)
typedef enum STP_ADMIN_P2P_S
{
    STP_ADMIN_P2P_FORCE_TRUE = 1,
    STP_ADMIN_P2P_FORCE_FALSE = 2,
    STP_ADMIN_P2P_AUTO = 3,
}STP_ADMIN_P2P;

typedef enum STP_VERSION_S
{
    STP_VERSION_LEGACY_STP = 0,
    STP_VERSION_RSTP = 2,
    STP_VERSION_MULTIPLE_STP = 3,
}STP_VERSION;

// 13.8 in 802.1Q-2018
typedef struct STP_MST_CONFIG_ID_S
{
    unsigned char ConfigurationIdentifierFormatSelector;// 1)
    char ConfigurationName[STP_REGION_NAME_LEN];       // 2)
    unsigned short RevisionLevel;            // 3)
    unsigned char ConfigurationDigest [16]; // 4)
}__attribute__((packed))STP_MST_CONFIG_ID;

// Six-byte MAC address, not aligned in memory.
struct STP_BRIDGE_ADDRESS
{
    unsigned char bytes[6];
};

const char* GetPortRoleName(STP_PORT_ROLE role);

typedef enum INFO_IS_S
{
    INFO_IS_DISABLED,
    INFO_IS_MINE,
    INFO_IS_AGED,
    INFO_IS_RECEIVED,
}INFO_IS;

typedef enum RCVD_INFO_S
{
    RCVD_INFO_UNKNOWN,
    RCVD_INFO_SUPERIOR_DESIGNATED,
    RCVD_INFO_REPEATED_DESIGNATED,
    RCVD_INFO_INFERIOR_DESIGNATED,
    RCVD_INFO_INFERIOR_ROOT_ALTERNATE,
    RCVD_INFO_OTHER,
}RCVD_INFO;

// ============================================================================

#endif
