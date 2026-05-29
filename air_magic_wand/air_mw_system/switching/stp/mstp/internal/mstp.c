// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

#include <string.h>
#include "mstp_base_types.h"
#include "mstp_port.h"
#include "mstp_bridge.h"
#include "mstp.h"
#include "mstp_log.h"
#include "mstp_sm.h"

#ifdef AIR_SUPPORT_MSTP
#include "osapi_memory.h"
#include "osapi_string.h"
#include "osapi_thread.h"
#include "mw_utils.h"
#include "mw_types.h"
#include "db_api.h"

#include "mstp_sys.h"
#include <air_swc.h>
#include "mbedtls/md5.h"
#include "mw_log.h"
#include "mbedtls/build_info.h"
#include "mw_cmd_util.h"
#endif
#include "mstp_procedures.h"

#ifdef AIR_SUPPORT_MSTP
#ifndef MBEDTLS_MD5_C
#error "MSTP requires MD5 support"
#endif
static STP_BRIDGE* _bridge = NULL;
#endif

#ifdef AIR_SUPPORT_MSTP
#define STP_LOG_ERR(fmt, ...)   MW_LOG_ERROR(STP, "%s[%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)
#define STP_LOG_DBG(fmt, ...)   MW_LOG_DEBUG(STP, fmt"\n", ##__VA_ARGS__)
#else
#define STP_LOG_ERR(fmt, ...)
#define STP_LOG_DBG(fmt, ...)
#endif

/* 802.1Q 2018 table13-1 */
static const unsigned char _configurationDigestSignatureKey[16] = {0x13, 0xAC, 0x06, 0xA6, 0x2E, 0x47, 0xFD, 0x51, 0xF9, 0x5D, 0x2B, 0xA2, 0x43, 0xCD, 0x03, 0x46 };

static void RunStateMachines(STP_BRIDGE* bridge, unsigned int timestamp);
static void RestartStateMachines(STP_BRIDGE* bridge, unsigned int timestamp);
static void RecomputePrioritiesAndPortRoles(STP_BRIDGE* bridge, unsigned int treeIndex, unsigned int timestamp);
#ifndef AIR_SUPPORT_MSTP
static void ComputeMstConfigDigest(STP_BRIDGE* bridge);
#endif

#ifdef AIR_SUPPORT_MSTP
static inline void
_initComputeMstConfigDigest(
    STP_BRIDGE* bridge);
#endif
// ============================================================================

STP_BRIDGE* STP_CreateBridge(
    unsigned int portCount,
    unsigned int trunkCount,
    unsigned int mstiCount,
    unsigned int maxVlanNumber,
    const unsigned char bridgeAddress[6])
{
#ifndef AIR_SUPPORT_MSTP
    unsigned int treeIndex;
#endif
    unsigned int portIndex;
    STP_BRIDGE* bridge = NULL;
    unsigned int len;

#ifndef AIR_SUPPORT_MSTP
    // Let's make a few checks on the data types, because we might be compiled with strange
    // compiler options which will turn upside down all our assumptions about structure layouts.
    // These really should use static_assert, but I'm not sure all compilers support static_assert.
    // If you get one of these asserts, you should reset your compiler options to their defaults,
    // at least those options related to structure layouts, at least for the files belonging to the STP library.
    assert(sizeof(unsigned short) == 2);
    assert(sizeof(unsigned int) == 4);
    //assert(sizeof(uint16_nbo) == 2);
    //assert(sizeof(uint32_nbo) == 4);
    assert(sizeof(STP_BRIDGE_ADDRESS) == 6);
    assert(sizeof(BRIDGE_ID) == 8);
    assert(sizeof(Port_ID) == 2);
    assert(sizeof(PRIORITY_VECTOR) == 34);
    assert(sizeof(MSTP_BPDU) == 102);
    assert(sizeof(MSTI_CONFIG_MESSAGE) == 16);

    // Upper limit for number of MSTIs is defined in 13.29.28 in 802.1Q-2018:
    // NOTE-No more than 64 MSTIs may be supported. The parameter sets for all of these can be encoded in a
    // standard-sized Ethernet frame. The number of MSTIs supported can be zero: an SPT Bridge, for example,
    // is not obliged to have MSTIs configured in order to support SPB.
    assert(mstiCount <= 64);

    // As specified in 12.3.i) in 802.1Q-2018, valid port numbers are 1..4095, so our valid port indexes will be 0..4094.
    // This means a maximum of 4095 ports.
    assert((portCount >= 1) && (portCount < 4096));
    assert(maxVlanNumber <= 4094);
#endif

    osapi_calloc(sizeof(STP_BRIDGE), MSTP_NAME, (void **)&bridge);
    if (NULL == bridge)
    {
        STP_LOG_ERR("Alloc memory for mstp data failed!");
        return NULL;
    }

    // See "13.6.2 Force Protocol Version" on page 332
    bridge->ForceProtocolVersion = STP_VERSION_MULTIPLE_STP;
    bridge->TxHoldCount = 6;
    bridge->MigrateTime = 3;
    bridge->portCount = portCount;
    bridge->treeCount = mstiCount + 1;
    bridge->maxVlanNumber = maxVlanNumber;  /* max supported vid num */

    // ------------------------------------------------------------------------
    len = bridge->treeCount * sizeof(BRIDGE_TREE*);
    osapi_calloc(len, MSTP_NAME, (void **)&bridge->trees);
    if (NULL == bridge->trees)
    {
        STP_LOG_ERR("Alloc memory for mstp instance data failed!");
        STP_DestroyBridge(bridge);
        return NULL;
    }

    len = (portCount+trunkCount) * sizeof(PORT*);
    osapi_calloc(len, MSTP_NAME, (void **)&bridge->ports);
    if (NULL == bridge->ports)
    {
        STP_LOG_ERR("Alloc memory for mstp port data failed!");
        STP_DestroyBridge(bridge);
        return NULL;
    }

    // per-bridge CIST vars
    len = sizeof(BRIDGE_TREE);
    osapi_calloc(len, MSTP_NAME, (void **)&(bridge->trees[CIST_INDEX]));
    if (NULL == bridge->trees[CIST_INDEX])
    {
        STP_LOG_ERR("Alloc memory for mstp cist tree failed!");
        STP_DestroyBridge(bridge);
        return NULL;
    }

    *(unsigned short *)(bridge->trees[CIST_INDEX]->BridgeIdentifier) = htons(0x8000);
    memcpy((bridge->trees[CIST_INDEX]->BridgeIdentifier)+2, bridgeAddress, 6);
    memcpy(bridge->trees[CIST_INDEX]->BridgePriority.RootId, bridge->trees[CIST_INDEX]->BridgeIdentifier, sizeof(BRIDGE_ID));
    memcpy(bridge->trees[CIST_INDEX]->BridgePriority.RegionalRootId, bridge->trees[CIST_INDEX]->BridgeIdentifier, sizeof(BRIDGE_ID));
    memcpy(bridge->trees[CIST_INDEX]->BridgePriority.DesignatedBridgeId, bridge->trees[CIST_INDEX]->BridgeIdentifier, sizeof(BRIDGE_ID));

    // 13.26.4 in 802.1Q-2018
    // Defaults from Table 13-5 on page 510 in 802.1Q-2018
    bridge->trees[CIST_INDEX]->BridgeTimes.HelloTime     = 2;
    bridge->trees[CIST_INDEX]->BridgeTimes.remainingHops = 20;
    bridge->trees[CIST_INDEX]->BridgeTimes.ForwardDelay  = 15;
    bridge->trees[CIST_INDEX]->BridgeTimes.MaxAge        = 20;

#ifndef AIR_SUPPORT_MSTP
    // per-bridge MSTI vars
    len = sizeof(BRIDGE_TREE);
    for (treeIndex = 1; treeIndex < bridge->treeCount; treeIndex++)
    {
        osapi_calloc(len, MSTP_NAME, (void **)&bridge->trees[treeIndex]);
        if (NULL == bridge->trees[treeIndex])
        {
            STP_DestroyBridge(bridge);
            return NULL;
        }

        *(unsigned short *)(bridge->trees[treeIndex]->BridgeIdentifier) = htons((0x8000 | treeIndex));
        memcpy((bridge->trees[treeIndex]->BridgeIdentifier)+2, bridgeAddress, 6);
        memcpy(bridge->trees[treeIndex]->BridgePriority.RegionalRootId, bridge->trees[treeIndex]->BridgeIdentifier, sizeof(BRIDGE_ID));
        memcpy(bridge->trees[treeIndex]->BridgePriority.DesignatedBridgeId, bridge->trees[treeIndex]->BridgeIdentifier, sizeof(BRIDGE_ID));
        bridge->trees[treeIndex]->BridgeTimes.remainingHops = 20;
    }
#endif

    // per-port vars
    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        len = sizeof(PORT);
        osapi_calloc(len, MSTP_NAME, (void **)&bridge->ports[portIndex]);
        if (NULL == bridge->ports[portIndex])
        {
            STP_LOG_ERR("Alloc memory for mstp port %d data failed!", portIndex);
            STP_DestroyBridge(bridge);
            return NULL;
        }

        PORT* port = bridge->ports[portIndex];

        len = bridge->treeCount * sizeof(PORT_TREE*);
        osapi_calloc(len, MSTP_NAME, (void **)&port->trees);
        if (NULL == port->trees)
        {
            STP_LOG_ERR("Alloc memory for mstp port %d instance data failed!", portIndex);
            STP_DestroyBridge(bridge);
            return NULL;
        }

#ifdef AIR_SUPPORT_MSTP
        len = sizeof(PORT_TREE);
        osapi_calloc(len, MSTP_NAME, (void **)&port->trees[CIST_INDEX]);
        if (NULL == port->trees[CIST_INDEX])
        {
            STP_LOG_ERR("Alloc memory for mstp port %d instance data failed!", portIndex);
            STP_DestroyBridge(bridge);
            return NULL;
        }
        port->trees[CIST_INDEX]->portId = ((0x80 << 8) | (portIndex + 1));
        port->trees[CIST_INDEX]->portTimes = bridge->trees[CIST_INDEX]->BridgeTimes;

#else
        // per-port CIST and MSTI vars
        for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
        {
            len = sizeof(PORT_TREE);
            osapi_calloc(len, MSTP_NAME, (void **)&port->trees[treeIndex]);
            if (NULL == port->trees[treeIndex])
            {
                STP_DestroyBridge(bridge);
                return NULL;
            }

            //port->trees[treeIndex]->portId.Set(0x80, (unsigned short) portIndex + 1);
            port->trees[treeIndex]->portId = ((0x80 << 8) | (portIndex + 1));
            //SET_PRIORITY_IN_IDENTIFIER(0x80, port->trees[treeIndex]->portId);

            port->trees[treeIndex]->portTimes = bridge->trees[treeIndex]->BridgeTimes;
            //memcpy(&port->trees[treeIndex]->pseudoRootId, &bridge->trees[treeIndex]->BridgeIdentifier, sizeof(BRIDGE_ID));
            //port->trees[treeIndex]->pseudoRootId = bridge->trees[treeIndex]->GetBridgeIdentifier();
        }
#endif
        port->adminPointToPointMAC = STP_ADMIN_P2P_AUTO;
        port->AutoEdge = true;
        port->enableBPDUrx = true;
        port->enableBPDUtx = true;
    }
    bridge->portCount = portCount + trunkCount;

    bridge->receivedBpduContent = NULL; // see comment at declaration of receivedBpduContent

    // These were already zeroed by the allocation routine.
    bridge->MstConfigId.ConfigurationIdentifierFormatSelector = 0;
    bridge->MstConfigId.RevisionLevel = 0;

    // Let's set a default name for the MST Config.
    STP_GetDefaultMstConfigName(bridgeAddress, bridge->MstConfigId.ConfigurationName);

#ifndef AIR_SUPPORT_MSTP
    len =  maxVlanNumber * sizeof(unsigned short);
    osapi_calloc(len, MSTP_NAME, (void **)&bridge->mstConfigTable);
    if (NULL == bridge->mstConfigTable)
    {
        STP_DestroyBridge(bridge);
        return NULL;
    }
#endif

    // The config table is all zeroes now, so all VIDs map to the CIST, no VID mapped to any MSTI.
    //ComputeMstConfigDigest(bridge);
#ifdef MBEDTLS_MD5_C
    _initComputeMstConfigDigest(bridge);
#endif

    return bridge;
}

// ============================================================================

void STP_DestroyBridge(STP_BRIDGE* bridge)
{
    unsigned int portIndex;
    unsigned int treeIndex;

    //osapi_free(bridge->mstConfigTable);

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        if (NULL == bridge->ports[portIndex])
        {
            continue;
        }
        for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
        {
            if (NULL != bridge->ports[portIndex]->trees[treeIndex])
            {
                osapi_free(bridge->ports[portIndex]->trees[treeIndex]);
            }
        }

        osapi_free(bridge->ports[portIndex]->trees);
        osapi_free(bridge->ports[portIndex]);
    }

    for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
    {
        osapi_free(bridge->trees[treeIndex]);
    }

    osapi_free(bridge->ports);
    osapi_free(bridge->trees);
#if STP_USE_LOG
    osapi_free(bridge->logBuffer);
#endif
    osapi_free(bridge);
}

// ============================================================================

void STP_StartBridge(STP_BRIDGE* bridge, unsigned int timestamp)
{
    LOG(bridge, 0, -1, "Starting the bridge...\r\n");

    if (bridge->started != false)
    {
        return;
    }

    bridge->started = true;

    RestartStateMachines(bridge, timestamp);

    LOG(bridge, 0, -1, "Bridge started.\r\n");
    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

// ============================================================================

void STP_StopBridge(STP_BRIDGE* bridge, unsigned int timestamp)
{
    unsigned int pi;
    unsigned int ti;
    PORT*        port;
    PORT_TREE*   tree;

    if (!bridge->started)
    {
        return;
    }

    for (pi = 0; pi < bridge->portCount; pi++)
    {
        port = bridge->ports[pi];
        if (NULL == port)
        {
            continue;
        }

        for (ti = 0; ti < bridge->treeCount; ti++)
        {
            tree= port->trees[ti];
            if (NULL == tree)
            {
                continue;
            }

            if (!tree->learning)
            {
                //bridge->callbacks.enableLearning(bridge, pi, ti, true, timestamp);
                LOG(bridge, pi+1, -1, "    Port %u tree %u to learn\n", pi, ti);
                tree->learning = true;
            }

            if (!tree->forwarding)
            {
                //bridge->callbacks.enableForwarding(bridge, pi, ti, true, timestamp);
                LOG(bridge, pi+1, -1, "    Port %u tree %u to forward\n", pi, ti);
                tree->forwarding = true;
            }
        }
    }

    // This one last, to allow the callbacks to still call "const" library functions.
    bridge->started = false;

    LOG(bridge, 0, -1, "Bridge stopped.\r\n");
    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

void STP_SetBridgeAddress(STP_BRIDGE* bridge, const unsigned char* address, unsigned int timestamp)
{
    unsigned int treeIndex;

    LOG(bridge, 0, -1, "Setting bridge MAC address to {%02x%02x-%02x%02x-%02x%02x}...", address[0],
        address[1],address[2],address[3],address[4],address[5]);

    const unsigned char* currentAddress = (const unsigned char*)&bridge->trees[CIST_INDEX]->BridgeIdentifier;
    if (memcmp(currentAddress+2, address, 6) == 0)
    {
        LOG(bridge, 0, -1, " nothing changed.\r\n");
    }
    else
    {
        LOG(bridge, 0, -1, "\r\n");

        for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
        {
            // change the MAC address without changing the priority
            //BRIDGE_ID bid = bridge->trees[CIST_INDEX]->BridgeIdentifier;
            //bid.SetAddress(address);
            //bridge->trees[treeIndex]->SetBridgeIdentifier(bid);
            unsigned char* addr = (unsigned char*)&bridge->trees[treeIndex]->BridgeIdentifier;
            memcpy(addr+2, address, 6);
        }

        if (bridge->started)
        {
            RecomputePrioritiesAndPortRoles(bridge, CIST_INDEX, timestamp);
        }
    }

    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

const struct STP_BRIDGE_ADDRESS* STP_GetBridgeAddress(const STP_BRIDGE* bridge)
{
    unsigned char* addr = (unsigned char*)&bridge->trees[CIST_INDEX]->BridgeIdentifier;

    return (struct STP_BRIDGE_ADDRESS *)(addr+2);
}

// ============================================================================

// Table 13-4 on page 502 of 802.1Q-2018
static unsigned int GetDefaultPortPathCost(unsigned int speedMegabitsPerSecond)
{
    if (speedMegabitsPerSecond == 0)
        return 200000000;
    else if (speedMegabitsPerSecond <= 1)
        return 20000000;
    else if (speedMegabitsPerSecond <= 10)
        return 2000000;
    else if (speedMegabitsPerSecond <= 100)
        return 200000;
    else if (speedMegabitsPerSecond <= 1000)
        return 20000;
    else if (speedMegabitsPerSecond <= 10000)
        return 2000;
    else if (speedMegabitsPerSecond <= 100000)
        return 200;
    else if (speedMegabitsPerSecond <= 1000000)
        return 20;
    else
        return 2;
}

void STP_OnPortEnabled(STP_BRIDGE* bridge,
                        unsigned int portIndex,
                        unsigned int speedMegabitsPerSecond,
                        bool detectedPointToPointMAC,
                        unsigned int timestamp)
{
    unsigned int treeIndex;
    PORT_TREE*   portTree;
    PORT*        port;

    LOG(bridge, portIndex+1, -1, "Port {%u} good\r\n", 1 + portIndex);

    port = bridge->ports[portIndex];
    if (NULL == port)
    {
        return;
    }

    if (true == port->portEnabled)
    {
        return;
    }

    port->portEnabled = true;

    port->detectedPointToPointMAC = detectedPointToPointMAC;
    if (port->adminPointToPointMAC != STP_ADMIN_P2P_AUTO)
    {
        port->operPointToPointMAC = (port->adminPointToPointMAC == STP_ADMIN_P2P_FORCE_TRUE);
    }
    else
    {
        port->operPointToPointMAC = detectedPointToPointMAC;
    }

    port->detectedPortPathCost = GetDefaultPortPathCost(speedMegabitsPerSecond);

    if (port->adminExternalPortPathCost != 0)
    {
        port->ExternalPortPathCost = port->adminExternalPortPathCost;
    }
    else
    {
        port->ExternalPortPathCost = port->detectedPortPathCost;
    }

    for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
    {
        portTree = port->trees[treeIndex];
        if (NULL == portTree)
        {
            continue;
        }
        if (portTree->adminInternalPortPathCost != 0)
        {
            portTree->InternalPortPathCost = portTree->adminInternalPortPathCost;
        }
        else
        {
            portTree->InternalPortPathCost = port->detectedPortPathCost;
        }
    }

    if (bridge->started)
    {
        RunStateMachines(bridge, timestamp);
    }

    LOG(bridge, portIndex+1, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

// ============================================================================

void STP_OnPortDisabled(STP_BRIDGE* bridge, unsigned int portIndex, unsigned int timestamp)
{
    PORT* port;

    LOG(bridge, portIndex+1, -1, "Port {%u} down\r\n", 1 + portIndex);

    port = bridge->ports[portIndex];
    if (NULL == port)
    {
        return;
    }

    // We allow calling this function on an already disabled port.
    if (port->portEnabled)
    {
        port->detectedPointToPointMAC = false;
        port->operPointToPointMAC = false;
        port->detectedPortPathCost = 0;
        port->ExternalPortPathCost = 0;
        // TODO: clear also InternalPortPathCost

        port->portEnabled = false;

        if (bridge->started)
        {
            RunStateMachines(bridge, timestamp);
        }
    }

    LOG(bridge, portIndex+1, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

// ============================================================================

void STP_OnOneSecondTick(STP_BRIDGE* bridge, unsigned int timestamp)
{
    unsigned int givenPort;

    if (bridge->started)
    {
        LOG(bridge, 0, -1, "One second:\r\n");

        for (givenPort = 0; givenPort < bridge->portCount; givenPort++)
        {
            if (NULL == bridge->ports[givenPort])
            {
                continue;
            }
            bridge->ports[givenPort]->tick = true;
        }

        RunStateMachines(bridge, timestamp);

        LOG(bridge, 0, -1, "------------------------------------\r\n");
        FLUSH_LOG(bridge);
    }
}

// ============================================================================

void STP_OnBpduReceived(STP_BRIDGE* bridge,
                            unsigned int portIndex,
                            const unsigned char* bpdu,
                            unsigned int bpduSize,
                            unsigned int timestamp)
{
#ifdef AIR_SUPPORT_MSTP
    unsigned int i = 0;
#endif

    if (NULL == bridge->ports[portIndex])
    {
        return;
    }

    if (bridge->started)
    {
        if (bridge->ports[portIndex]->portEnabled == false)
        {
            LOG(bridge, portIndex+1, -1, "{T}: WARNING: BPDU received on disabled port {%d}. The STP library is discarding it.\r\n", portIndex+1);
        }
        else
        {
            LOG(bridge, portIndex+1, -1, "{T}: BPDU received on Port {%d}:\r\n", portIndex+1);

            VALIDATED_BPDU_TYPE type = STP_GetValidatedBpduType(bridge->ForceProtocolVersion, bpdu, bpduSize);
            switch (type)
            {
                case VALIDATED_BPDU_TYPE_STP_CONFIG:
                    #if STP_USE_LOG
                        LOG(bridge, portIndex+1, -1, "Config BPDU:\r\n");
                        LOG_INDENT(bridge);
                        DumpConfigBpdu(bridge, portIndex, -1, (const MSTP_BPDU*) bpdu);
                        LOG_UNINDENT(bridge);
                    #endif
                    break;

                case VALIDATED_BPDU_TYPE_RST:
                    #if STP_USE_LOG
                        LOG(bridge, portIndex+1, -1, "RSTP BPDU:\r\n");
                        LOG_INDENT(bridge);
                        DumpRstpBpdu(bridge, portIndex, -1, (const MSTP_BPDU*) bpdu);
                        LOG_UNINDENT(bridge);
                    #endif
                    break;

                case VALIDATED_BPDU_TYPE_MST:
                case VALIDATED_BPDU_TYPE_SPT:
                    #if STP_USE_LOG
                        if (type == VALIDATED_BPDU_TYPE_MST)
                        {
                            LOG(bridge, portIndex+1, -1, "MSTP BPDU:\r\n");
                        }
                        else
                        {
                            LOG(bridge, portIndex+1, -1, "SPT BPDU (processed as MSTP):\r\n");
                        }
                        LOG_INDENT(bridge);
                        DumpMstpBpdu(bridge, portIndex, -1, (const MSTP_BPDU*) bpdu);
                        LOG_UNINDENT(bridge);
                    #endif
                    break;

                case VALIDATED_BPDU_TYPE_STP_TCN:
                    LOG(bridge, portIndex+1, -1, "TCN BPDU.\r\n");
                    break;

                case VALIDATED_BPDU_TYPE_UNKNOWN:
                    LOG(bridge, portIndex+1, -1, "Invalid BPDU received. Discarding it.\r\n");
                    break;

                default:
                    break;
            }

            if (type != VALIDATED_BPDU_TYPE_UNKNOWN)
            {
                if ((bridge->receivedBpduContent != NULL) ||
                    (bridge->receivedBpduType != VALIDATED_BPDU_TYPE_UNKNOWN) ||
                    (bridge->ports[portIndex]->rcvdBpdu != false))
                {
                    return;
                }

                bridge->receivedBpduContent = (MSTP_BPDU*)bpdu;
                bridge->receivedBpduType = type;
                bridge->receivedBpduPort = bridge->ports[portIndex];
                bridge->ports[portIndex]->rcvdBpdu = true;

                RunStateMachines(bridge, timestamp);

                bridge->receivedBpduContent = NULL; // to cause an exception on access
                bridge->receivedBpduType = VALIDATED_BPDU_TYPE_UNKNOWN; // to cause asserts on access
                bridge->receivedBpduPort = NULL;

#ifdef AIR_SUPPORT_MSTP
                if (NULL != bridge->ports[portIndex])
                {
                    for (i = 0; i < bridge->treeCount; i++)
                    {
                        if (NULL == bridge->ports[portIndex]->trees[i])
                        {
                            continue;
                        }
                        bridge->ports[portIndex]->trees[i]->msgFlagsPortRole = 0;
                        bridge->ports[portIndex]->trees[i]->msgFlagsProposal = false;
                        bridge->ports[portIndex]->trees[i]->msgFlagsLearning = false;
                        bridge->ports[portIndex]->trees[i]->msgFlagsForwarding = false;
                        bridge->ports[portIndex]->trees[i]->msgFlagsAgreement = false;
                    }
                }
#endif
                // Check that the state machines did process the BPDU.
                if (bridge->ports[portIndex]->rcvdBpdu != false)
                {
                    LOG(bridge, portIndex+1, -1, "The stm did not process bpdu\r\n");
                    return;
                }
            }
        }

        LOG(bridge, portIndex+1, -1, "------------------------------------\r\n");
        FLUSH_LOG (bridge);
    }
}

// ============================================================================

bool STP_IsBridgeStarted(const STP_BRIDGE* bridge)
{
    return bridge->started;
}

// ============================================================================

void STP_EnableLogging(STP_BRIDGE* bridge, bool enable)
{
#if STP_USE_LOG
    bridge->loggingEnabled = enable;
#endif
}

bool STP_IsLoggingEnabled(const STP_BRIDGE* bridge)
{
#if STP_USE_LOG
    return bridge->loggingEnabled;
#else
    return false;
#endif
}

// ============================================================================
#if STP_USE_LOG
void LogTransitionMsti(STP_BRIDGE* bridge, const char* smName, const char* newStateName, PortTreeArgs* pt)
{
    LOG(bridge, pt->portIndex, pt->treeIndex, "Bridge: ");
    if (bridge->ForceProtocolVersion >= STP_VERSION_MULTIPLE_STP)
    {
        if (pt->treeIndex == CIST_INDEX)
        {
            LOG(bridge, pt->portIndex, pt->treeIndex, "CIST: ");
        }
        else
        {
            LOG(bridge, pt->portIndex, pt->treeIndex, "MST %d: ", pt->treeIndex);
        }
    }

    LOG(bridge, pt->portIndex, pt->treeIndex, "{%s}: -> {%s}\r\n", smName, newStateName);
}

void LogTransitionPort(STP_BRIDGE* bridge, const char* smName, const char* newStateName, PortTreeArgs* pt)
{
    LOG(bridge, pt->portIndex, -1, "Port {%d}: ", 1 + pt->portIndex);
    LOG(bridge, pt->portIndex, -1, "{%s}: -> {%s}\r\n", smName, newStateName);
}

void LogTransitionMstiPort(STP_BRIDGE* bridge, const char* smName, const char* newStateName, PortTreeArgs* pt)
{
    LOG(bridge, pt->portIndex, pt->treeIndex, "Port {%d}: ", 1 + pt->portIndex);
    if (bridge->ForceProtocolVersion >= STP_VERSION_MULTIPLE_STP)
    {
        if (pt->treeIndex == CIST_INDEX)
        {
            LOG(bridge, pt->portIndex, pt->treeIndex, "CIST: ");
        }
        else
        {
            LOG(bridge, pt->portIndex, pt->treeIndex, "MST{%d}: ", pt->treeIndex);
        }
    }
    LOG(bridge, pt->portIndex, pt->treeIndex, "{%s}: -> {%s}\r\n", smName, newStateName);
}
#endif

// ============================================================================

static bool RunStateMachineInstance(STP_BRIDGE* bridge, const struct StateMachine* smInfo, unsigned char *ptr_state, unsigned int timestamp, PortTreeArgs* portTreeArgs)
{
    bool changed = false;
    unsigned char newState;
    unsigned char state = *ptr_state;
#ifdef STP_DEBUG
    const char* stateName;
#endif
    int num_iterations = 0;

rep:
#ifdef STP_DEBUG
    stateName = smInfo->getStateName(state);
#endif
    LOG(bridge,  portTreeArgs->portIndex+1, portTreeArgs->treeIndex, "STM-Init: %s, Port %d, instance %d, stateName %s\n",
            smInfo->smName, portTreeArgs->portIndex+1, portTreeArgs->treeIndex, stateName);
    newState = smInfo->checkConditions(bridge, portTreeArgs, state);
    if (newState != 0)
    {
#ifdef STP_DEBUG
        stateName = smInfo->getStateName(newState);
#endif
        LOG(bridge,  portTreeArgs->portIndex+1, portTreeArgs->treeIndex, "STM: %s, Port %d, instance %d, stateName %s\n",
                smInfo->smName, portTreeArgs->portIndex+1, portTreeArgs->treeIndex, stateName);

        smInfo->initState(bridge, portTreeArgs, newState, timestamp);

        state = newState;
        *ptr_state = newState;
        changed = true;
        num_iterations++;
        if (MAX_STP_ITERATIONS_INSTANCE > num_iterations)
        {
            goto rep;
        }
    }

    return changed;
}

// ============================================================================
static void RunStateMachines(STP_BRIDGE* bridge, unsigned int timestamp)
{
    bool changed;
    bool roleChanged;
    unsigned int  portIndex;
    unsigned int  treeIndex;
    PortTreeArgs  pt;
    PORT*         ptr_port;
    PORT_TREE*    ptr_tree;
    BRIDGE_TREE*  ptr_bridgeTree;
    unsigned char old_role;
#ifdef AIR_SUPPORT_MSTP
    UI32_T        start_tick, end_tick, dur_tick;
    start_tick = osapi_sysTickGet();
#endif

    do
    {
        changed = false;

        for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
        {
            ptr_port = bridge->ports[portIndex];
            if (NULL == ptr_port)
            {
                continue;
            }
#ifdef AIR_SUPPORT_MSTP
            if (!ptr_port->run_flag)
            {
                continue;
            }
#endif
            pt.portIndex = portIndex;
            pt.treeIndex = 0;
            changed |= RunStateMachineInstance(bridge, &ptimer_stm, &ptr_port->portTimersState, timestamp, &pt);
#ifdef AIR_SUPPORT_MSTP
            changed |= RunStateMachineInstance(bridge, &pr_stm, &ptr_port->portReceiveState, timestamp, &pt);
            changed |= RunStateMachineInstance(bridge, &ppm_stm, &ptr_port->portProtocolMigrationState, timestamp, &pt);
#else
            changed |= RunStateMachineInstance(bridge, &ppm_stm, &ptr_port->portProtocolMigrationState, timestamp, &pt);
            changed |= RunStateMachineInstance(bridge, &pr_stm, &ptr_port->portReceiveState, timestamp, &pt);
#endif
            changed |= RunStateMachineInstance(bridge, &bd_stm, &ptr_port->bridgeDetectionState, timestamp, &pt);
            //changed |= RunStateMachineInstance(bridge, &l2g_stm, &port->l2gpState, timestamp, &pt);

            roleChanged = false;
            for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
            {
                ptr_tree = ptr_port->trees[treeIndex];
                if (NULL == ptr_tree)
                {
                    continue;
                }
                pt.portIndex = portIndex;
                pt.treeIndex = treeIndex;
                changed |= RunStateMachineInstance(bridge, &pi_stm, &ptr_tree->portInformationState, timestamp, &pt);
#ifdef AIR_SUPPORT_MSTP
                old_role = ptr_tree->portRoleTransitionsState;
                changed |= RunStateMachineInstance(bridge, &prt_stm, &ptr_tree->portRoleTransitionsState, timestamp, &pt);
                roleChanged |= (old_role != ptr_tree->portRoleTransitionsState) ? true : false;
#else
                changed |= RunStateMachineInstance(bridge, &prt_stm, &ptr_tree->portRoleTransitionsState, timestamp, &pt);
#endif
                changed |= RunStateMachineInstance(bridge, &pst_stm, &ptr_tree->portStateTransitionState, timestamp, &pt);
                changed |= RunStateMachineInstance(bridge, &tc_stm, &ptr_tree->topologyChangeState, timestamp, &pt);
            }
#ifdef AIR_SUPPORT_MSTP
            if (roleChanged)
            {
                updtPortInstanceRole(bridge, portIndex);
            }
#endif
        }

        for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
        {
            ptr_bridgeTree = bridge->trees[treeIndex];
            if (NULL == ptr_bridgeTree)
            {
                continue;
            }
            pt.portIndex = 0;
            pt.treeIndex = treeIndex;
            changed |= RunStateMachineInstance(bridge, &prs_stm, &ptr_bridgeTree->portRoleSelectionState, timestamp, &pt);
        }

        // We execute the PortTransmit state machine only after all other state machines have finished executing,
        // so as to avoid transmitting BPDUs containing results from intermediary calculations.
        // See Note 1 on page 541 of 802.1Q-2018.
        if (!changed)
        {
            for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
            {
                ptr_port = bridge->ports[portIndex];
                if (NULL == ptr_port)
                {
                    continue;
                }
                pt.portIndex = portIndex;
                pt.treeIndex = 0;
                changed |= RunStateMachineInstance(bridge, &pt_stm, &ptr_port->portTransmitState, timestamp, &pt);
            }
        }
#ifdef AIR_SUPPORT_MSTP
        end_tick = osapi_sysTickGet();
        if (end_tick >= start_tick)
        {
            dur_tick = end_tick - start_tick;
        }
        else
        {
            dur_tick = 0xFFFFFFFF - start_tick + end_tick + 1;
        }

        if (dur_tick > MAX_STP_RUN_TIME)
        {
            return;
        }
#endif
    } while (changed);
}

static void RestartStateMachines(STP_BRIDGE* bridge, unsigned int timestamp)
{
    unsigned int portIndex;
    unsigned int treeIndex;
    PORT* ptr_port;
    PORT_TREE* ptr_tree;

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        ptr_port = bridge->ports[portIndex];
        if (NULL == ptr_port)
        {
            continue;
        }
        ptr_port->portTimersState            = 0;
        ptr_port->portProtocolMigrationState = 0;
        ptr_port->portReceiveState           = 0;
        ptr_port->bridgeDetectionState       = 0;
        ptr_port->l2gpState                  = 0;
        ptr_port->portTransmitState          = 0;

        for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
        {
            ptr_tree = ptr_port->trees[treeIndex];
            if (NULL == ptr_tree)
            {
                continue;
            }
            ptr_tree->portInformationState     = 0;
            ptr_tree->portRoleTransitionsState = 0;
            ptr_tree->portStateTransitionState = 0;
            ptr_tree->topologyChangeState      = 0;
        }
    }

    for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
    {
        if (NULL == bridge->trees[treeIndex])
        {
            continue;
        }
        bridge->trees[treeIndex]->portRoleSelectionState = 0;
    }

    bridge->BEGIN = true;
    RunStateMachines(bridge, timestamp);
    bridge->BEGIN = false;
    RunStateMachines(bridge, timestamp);
}

// ============================================================================

void STP_SetPortAdminEdge(STP_BRIDGE* bridge, unsigned int portIndex, bool adminEdge, unsigned int timestamp)
{
    if (NULL != bridge->ports[portIndex])
    {
        bridge->ports[portIndex]->AdminEdge = adminEdge;
    }
}

bool STP_GetPortAdminEdge(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return false;
    }
    return bridge->ports[portIndex]->AdminEdge;
}

// ============================================================================

void STP_SetPortAutoEdge(STP_BRIDGE* bridge, unsigned int portIndex, bool autoEdge, unsigned int timestamp)
{
    if (NULL != bridge->ports[portIndex])
    {
        bridge->ports[portIndex]->AutoEdge = autoEdge;
    }
}

bool STP_GetPortAutoEdge(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL != bridge->ports[portIndex])
    {
        return false;
    }
    return bridge->ports[portIndex]->AutoEdge;
}

// ============================================================================

void STP_SetAdminPointToPointMAC(STP_BRIDGE* bridge,
                                        unsigned int portIndex,
                                        STP_ADMIN_P2P adminPointToPointMAC,
                                        unsigned int timestamp)
{
#ifdef STP_DEBUG
    const char* p2pString = STP_GetAdminP2PString(adminPointToPointMAC);
#endif
    PORT*       port;

    LOG(bridge, portIndex+1, -1, "Setting adminPointToPointMAC = {%s} on port {%d}...\r\n", p2pString, 1 + portIndex);

    port = bridge->ports[portIndex];
    if (NULL == port)
    {
        return;
    }

    port->adminPointToPointMAC = adminPointToPointMAC;

    if (port->portEnabled)
    {
        bool newOperPointToPointMAC = (adminPointToPointMAC == STP_ADMIN_P2P_FORCE_TRUE)
            || ((adminPointToPointMAC == STP_ADMIN_P2P_AUTO) && port->detectedPointToPointMAC);

        if (port->operPointToPointMAC != newOperPointToPointMAC)
        {
            port->operPointToPointMAC = newOperPointToPointMAC;
            if (bridge->started)
            {
                RunStateMachines(bridge, timestamp);
            }
        }
    }

    LOG(bridge, portIndex+1, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

STP_ADMIN_P2P STP_GetAdminPointToPointMAC(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return STP_ADMIN_P2P_AUTO;
    }
    return bridge->ports[portIndex]->adminPointToPointMAC;
}

bool STP_GetDetectedPointToPointMAC(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return false;
    }
    return bridge->ports[portIndex]->detectedPointToPointMAC;
}

bool STP_GetOperPointToPointMAC(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return false;
    }
    return bridge->ports[portIndex]->operPointToPointMAC;
}

// ============================================================================

static void RecomputePrioritiesAndPortRoles(STP_BRIDGE* bridge, unsigned int treeIndex, unsigned int timestamp)
{
    unsigned int portIndex;
    PORT_TREE*   portTree;

    // From page 511 of 802.1Q-2018:
    // BridgeIdentifier, BridgePriority, and BridgeTimes are not modified by the operation of the spanning tree
    // protocols but are treated as constants by the state machines. If they are modified by management, spanning
    // tree priority vectors and Port Role assignments for all trees shall be recomputed, as specified by the
    // operation of the Port Role Selection state machine (13.36) by clearing selected (13.27.67) and setting
    // reselect (13.27.62) for all Bridge Ports for the relevant MSTI and for all trees if the CIST parameter is
    // changed.
    if (treeIndex == CIST_INDEX)
    {
        // Recompute all trees.
        // Note that callers of this function expect recomputation for all trees when CIST_INDEX is passed, so don't change this functionality.
        for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
        {
            for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
            {
                if (NULL == bridge->ports[portIndex])
                {
                    continue;
                }
                portTree = bridge->ports[portIndex]->trees[treeIndex];
                if (NULL != portTree)
                {
                    portTree->selected = false;
                    portTree->reselect = true;
                }
            }
        }
    }
    else
    {
        // recompute specified MSTI
        for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
        {
            if (NULL == bridge->ports[portIndex])
            {
                continue;
            }
            portTree = bridge->ports[portIndex]->trees[treeIndex];
            if (NULL != portTree)
            {
                portTree->selected = false;
                portTree->reselect = true;
            }
        }
    }
#ifndef AIR_SUPPORT_MSTP
    RunStateMachines(bridge, timestamp);
#endif
}

// Note AG: Problem when setting a worse bridge priority (numerically higher)
// on the root bridge, and that bridge remains root even with the new priority:
//
// BPDUs with the old priority could still be propagating throughout the network, until they are discarded
// due to MaxAge / remainingHops. These BPDUs will mess up all priority calculations while propagating,
// because they have the same bridge address, so they will be Superior to the BPDUs newly generated by the same root.
//
// This increases the convergence time by up to HelloTime seconds, and it seems to be a problem of the protocol itself.
// If compounded with delays introduced either by other problems of the protocol, or by problems in the rest of the firmware,
// it might leads to the formation of loops. I don't think this could be resolved given the current BPDU format.
void STP_SetBridgePriority(STP_BRIDGE* bridge, unsigned int treeIndex, unsigned short instanceId, unsigned short bridgePriority, unsigned int timestamp)
{
    unsigned short priority;

    // See table 13-3 on page 501 of 802.1Q-2018.
    if ((bridgePriority & 0x0FFF) != 0)
    {
        return;
    }
    if (treeIndex >= bridge->treeCount)
    {
        return;
    }

    if (NULL == bridge->trees[treeIndex])
    {
        return;
    }

    LOG(bridge, 0, -1, "Setting bridge priority: tree {%u} prio = {%u}...\r\n", treeIndex, bridgePriority);
    bridgePriority = htons(bridgePriority | instanceId);

    //BRIDGE_ID bid = bridge->trees[treeIndex]->GetBridgeIdentifier();
    memcpy(&priority, bridge->trees[treeIndex]->BridgeIdentifier, sizeof(unsigned short));
    if (priority != bridgePriority)
    {
        LOG(bridge, 0, -1, "\r\n");

        memcpy(bridge->trees[treeIndex]->BridgeIdentifier, &bridgePriority, sizeof(unsigned short));
        //bid.SetPriorityAndMstid(bridgePriority, treeIndex);
        //bridge->trees[treeIndex]->SetBridgeIdentifier(bid);

        if (treeIndex == CIST_INDEX)
        {
            memcpy(bridge->trees[treeIndex]->BridgePriority.RootId, bridge->trees[treeIndex]->BridgeIdentifier, sizeof(BRIDGE_ID));
            //BridgePriority.ExternalRootPathCost = 0;
        }

        memcpy(bridge->trees[treeIndex]->BridgePriority.RegionalRootId, bridge->trees[treeIndex]->BridgeIdentifier, sizeof(BRIDGE_ID));
        //BridgePriority.InternalRootPathCost = 0;
        memcpy(bridge->trees[treeIndex]->BridgePriority.DesignatedBridgeId, bridge->trees[treeIndex]->BridgeIdentifier, sizeof(BRIDGE_ID));
        //BridgePriority.DesignatedPortId = 0;

        if ((bridge->started) && (treeIndex < bridge->treeCount))
        {
            RecomputePrioritiesAndPortRoles(bridge, treeIndex, timestamp);
        }
    }
    else
    {
        LOG(bridge, 0, -1, " nothing changed.\r\n");
    }

    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

unsigned short STP_GetBridgePriority(const STP_BRIDGE* bridge, unsigned int treeIndex)
{
    unsigned short priority;

    if (treeIndex >= bridge->treeCount)
    {
        return 0;
    }
    if(NULL == bridge->trees[treeIndex])
    {
        return 0;
    }
    memcpy(&priority, bridge->trees[treeIndex]->BridgeIdentifier, sizeof(unsigned short));
    priority = ntohs(priority);

    //return bridge->trees[treeIndex]->GetBridgeIdentifier().GetPriorityWithoutMstid();
    return (priority & 0xF000);
}

// ============================================================================

void STP_SetPortPriority(STP_BRIDGE* bridge,
                            unsigned int portIndex,
                            unsigned int treeIndex,
                            unsigned char portPriority,
                            unsigned int timestamp)
{
    // See table 13-3 on page 501 of 802.1Q-2018.
    // See 13.27.46 in 802.1Q-2018.

    if ((portPriority % 16) != 0)
    {
        return;
    }
    if (portIndex >= bridge->portCount)
    {
        return;
    }
    if (treeIndex >= bridge->treeCount)
    {
        return;
    }

    if (NULL == bridge->ports[portIndex])
    {
        return;
    }

    if (NULL == bridge->ports[portIndex]->trees[treeIndex])
    {
        return;
    }

    LOG(bridge, portIndex+1, -1, "Setting port priority: port {%d} tree {%d} prio = {%d}...\r\n",
         1 + portIndex,
         treeIndex,
         portPriority);

    //bridge->ports[portIndex]->trees[treeIndex]->portId.SetPriority(portPriority);
    //SET_PRIORITY_IN_IDENTIFIER(portPriority, bridge->ports[portIndex]->trees[treeIndex]->portId);
    bridge->ports[portIndex]->trees[treeIndex]->portId = ((portPriority << 8) | (portIndex + 1));

    // It would make sense that stuff is recomputed also when the port priority in the portId variable
    // is changed (as it is recomputed for the bridge priority), but either the spec does not mention this, or I'm not seeing it.
    // Anyway, information about the new port priority can only be propagated by such a recomputation, so let's do that.
    if ((bridge->started) && (treeIndex < bridge->treeCount))
    {
        RecomputePrioritiesAndPortRoles(bridge, treeIndex, timestamp);
    }

    LOG(bridge, portIndex+1, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

unsigned char STP_GetPortPriority(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex)
{
    unsigned char priority;

    // See 13.27.46 in 802.1Q-2018.
    if (portIndex >= bridge->portCount)
    {
        return 128;
    }
    if (treeIndex >= bridge->treeCount)
    {
        return 128;
    }

    if (NULL == bridge->ports[portIndex])
    {
        return 128;
    }

    if (NULL == bridge->ports[portIndex]->trees[treeIndex])
    {
        return 128;
    }

    priority = GET_PRIORITY_FROM_IDENTIFIER(bridge->ports[portIndex]->trees[treeIndex]->portId);
    return priority;
}

unsigned short STP_GetPortIdentifier(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex)
{
    unsigned short id;

    if (portIndex >= bridge->portCount)
    {
        return 0;
    }

    if (treeIndex >= bridge->treeCount)
    {
        return 0;
    }

    if (NULL == bridge->ports[portIndex])
    {
        return 0;
    }

    if (NULL == bridge->ports[portIndex]->trees[treeIndex])
    {
        return 0;
    }

    id = (bridge->ports[portIndex]->trees[treeIndex]->portId);
    return id;
}

// ============================================================================

void STP_GetDefaultMstConfigName(const unsigned char bridgeAddress[6], char nameOut[18])
{
    // Cisco uses lowercase letters here; let's do the same.
    char* ptr = nameOut;
    unsigned int i;

    for (i = 0; i < 6; i++)
    {
        unsigned int val = bridgeAddress[i] >> 4;
        *ptr++ = (val < 10) ? (val + '0') : (val - 10 + 'a');
        val = bridgeAddress[i] & 0x0F;
        *ptr++ = (val < 10) ? (val + '0') : (val - 10 + 'a');
        *ptr++ = (i < 5) ? ':' : 0;
    }
}

void STP_SetMstConfigName(STP_BRIDGE* bridge, const char* name, unsigned int timestamp)
{
    if (strlen(name) > 32)
    {
        return;
    }

    LOG(bridge, 0, -1, "Setting MST Config Name to \"{%s}\"...\r\n", name);

    memset(bridge->MstConfigId.ConfigurationName, 0, 32);
    memcpy(bridge->MstConfigId.ConfigurationName, name, strlen(name));

    if (bridge->started)
    {
        RestartStateMachines(bridge, timestamp);
    }

    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

// ============================================================================

void STP_SetMstConfigRevisionLevel(STP_BRIDGE* bridge, unsigned short revisionLevel, unsigned int timestamp)
{
    LOG(bridge, 0, -1, "Setting MST Config Revision Level to {%d}...\r\n", revisionLevel);

    bridge->MstConfigId.RevisionLevel = revisionLevel;

    if (bridge->started)
    {
        RestartStateMachines(bridge, timestamp);
    }

    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG (bridge);
}

#ifndef AIR_SUPPORT_MSTP
static void ComputeMstConfigDigest(STP_BRIDGE* bridge)
{
    unsigned short zero = 0;
    MD5_CTX context;
    int i;

    HMAC_MD5_Init(&context);
    HMAC_MD5_Update(&context, bridge->mstConfigTable, 2 * (1 + bridge->maxVlanNumber));

    for (i = (1 + bridge->maxVlanNumber); i < 4096; i++)
    {
        HMAC_MD5_Update(&context, &zero, 2);
    }

    HMAC_MD5_End(&context);

    memcpy(bridge->MstConfigId.ConfigurationDigest, context.digest, 16);
}

void STP_SetMstConfigTable(STP_BRIDGE* bridge,
                                const STP_CONFIG_TABLE_ENTRY* entries,
                                unsigned int entryCount,
                                unsigned int timestamp)
{
    unsigned int vlan;

    assert(entryCount == 1 + bridge->maxVlanNumber);

    LOG(bridge, 0, -1, "Setting MST Config Table... ");

    if (memcmp(bridge->mstConfigTable, entries, entryCount * 2) == 0)
    {
        LOG(bridge, 0, -1, "... nothing changed.\r\n");
    }
    else
    {
        // Check that the caller is not trying to map a VLAN to a too-large tree number.
        assert(entries[0].unused == 0);
        assert(entries[0].treeIndex == 0);
        for (vlan = 1; vlan < entryCount; vlan++)
        {
            assert(entries[vlan].unused == 0);
            assert(entries[vlan].treeIndex < bridge->treeCount);
        }

        if (entryCount == 4096)
        {
            assert(entries[4095].treeIndex == 0);
        }

        memcpy(bridge->mstConfigTable, entries, entryCount * 2);

        ComputeMstConfigDigest(bridge);

        LOG(bridge, 0, -1, "New digest: 0x{X2}{X2}...{X2}{X2}.\r\n",
            bridge->MstConfigId.ConfigurationDigest[0], bridge->MstConfigId.ConfigurationDigest[1],
            bridge->MstConfigId.ConfigurationDigest[14], bridge->MstConfigId.ConfigurationDigest[15]);

        if (bridge->started)
        {
            RestartStateMachines(bridge, timestamp);
        }
    }

    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

void STP_SetMstConfigTableEntry(STP_BRIDGE* bridge, unsigned int vlanNumber, unsigned int treeIndex, unsigned int timestamp)
{
    assert(vlanNumber <= bridge->maxVlanNumber);

    LOG(bridge, 0, -1, "Setting MST Config Table... ");

    if (bridge->mstConfigTable[vlanNumber] == treeIndex)
    {
        LOG (bridge, 0, -1, "... nothing changed.\r\n");
    }
    else
    {
        // Check that the caller is not trying to map a VLAN to a too-large tree number.
        if ((vlanNumber == 0) || (vlanNumber == 4095))
        {
            assert(treeIndex == 0);
        }
        else
        {
            assert(treeIndex < bridge->treeCount);
        }

        bridge->mstConfigTable[vlanNumber] = (unsigned short)treeIndex;

        ComputeMstConfigDigest(bridge);

        LOG(bridge, 0, -1, "New digest: 0x{%-02x}{%-02x}...{%-02x}{%-02x}.\r\n",
            bridge->MstConfigId.ConfigurationDigest[0], bridge->MstConfigId.ConfigurationDigest[1],
            bridge->MstConfigId.ConfigurationDigest[14], bridge->MstConfigId.ConfigurationDigest[15]);

        if (bridge->started)
        {
            RestartStateMachines(bridge, timestamp);
        }
    }

    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}
#endif
const STP_CONFIG_TABLE_ENTRY* STP_GetMstConfigTable(STP_BRIDGE* bridge, unsigned int* entryCountOut)
{
    *entryCountOut = 1 + bridge->maxVlanNumber;
    return (const STP_CONFIG_TABLE_ENTRY*)bridge->mstConfigTable;
}

// ============================================================================

unsigned int STP_GetPortCount(const STP_BRIDGE* bridge)
{
    return bridge->portCount;
}

unsigned int STP_GetMstiCount(const STP_BRIDGE* bridge)
{
    return bridge->treeCount-1;
}

STP_VERSION STP_GetStpVersion(const STP_BRIDGE* bridge)
{
    return bridge->ForceProtocolVersion;
}

void STP_SetStpVersion(STP_BRIDGE* bridge, STP_VERSION version, unsigned int timestamp)
{
    LOG(bridge, 0, -1, "{Switching to {%s}... ", STP_GetVersionString(version));

    if (bridge->ForceProtocolVersion == version)
    {
        LOG(bridge, 0, -1, "... bridge was already running {%s}.\r\n", STP_GetVersionString(version));
    }
    else
    {
        LOG(bridge, 0, -1, "\r\n");

        bridge->ForceProtocolVersion = version;

        if (bridge->started)
        {
            RestartStateMachines(bridge, timestamp);
        }
    }

    LOG(bridge, 0, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

bool STP_GetPortEnabled(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return false;
    }
    return bridge->ports[portIndex]->portEnabled;
}

STP_PORT_ROLE STP_GetPortRole(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return STP_PORT_ROLE_UNDEFINED;
    }

    if (NULL == bridge->ports[portIndex]->trees[treeIndex])
    {
        return STP_PORT_ROLE_UNDEFINED;
    }

    // This value has meaning only while STP is enabled. Let's check that it is.
    if (!bridge->started)
    {
        return STP_PORT_ROLE_UNDEFINED;
    }
    return bridge->ports[portIndex]->trees[treeIndex]->role;
}

bool STP_GetPortLearning(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return false;
    }

    if (NULL == bridge->ports[portIndex]->trees[treeIndex])
    {
        return false;
    }

    // This value has meaning only while STP is enabled. Let's check that it is.
    if (!bridge->started)
    {
        return false;
    }
    return bridge->ports[portIndex]->trees[treeIndex]->learning;
}

bool STP_GetPortForwarding(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return false;
    }

    if (NULL == bridge->ports[portIndex]->trees[treeIndex])
    {
        return false;
    }

    // This value has meaning only while STP is enabled. Let's check that it is.
    if (!bridge->started)
    {
        return false;
    }
    return bridge->ports[portIndex]->trees[treeIndex]->forwarding;
}

bool STP_GetPortOperEdge(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return false;
    }

    // This value has meaning only while STP is enabled. Let's check that it is.
    if (!bridge->started)
    {
        return false;
    }
    return bridge->ports[portIndex]->operEdge;
}

unsigned char STP_GetMaxVlanNumber(const STP_BRIDGE* bridge)
{
    return bridge->maxVlanNumber;
}

unsigned int STP_GetTreeIndexFromVlanNumber(const STP_BRIDGE* bridge, unsigned int vlanNumber)
{
    assert(vlanNumber <= bridge->maxVlanNumber);

    switch (bridge->ForceProtocolVersion)
    {
        case STP_VERSION_LEGACY_STP:
        case STP_VERSION_RSTP:
            return 0;

        case STP_VERSION_MULTIPLE_STP:
#ifdef AIR_SUPPORT_MSTP
            return 0;
#else
            return bridge->mstConfigTable[vlanNumber];
#endif

        default:
            assert(false); return 0;
    }
}

const STP_MST_CONFIG_ID* STP_GetMstConfigId(const STP_BRIDGE* bridge)
{
    return &bridge->MstConfigId;
}

// ============================================================================

const char* STP_GetPortRoleString(STP_PORT_ROLE portRole)
{
    switch (portRole)
    {
        case STP_PORT_ROLE_UNDEFINED:
            return "(undefined)";
        case STP_PORT_ROLE_DISABLED:
            return "Disabled";
        case STP_PORT_ROLE_ROOT:
            return "Root";
        case STP_PORT_ROLE_DESIGNATED:
            return "Designated";
        case STP_PORT_ROLE_ALTERNATE:
            return "Alternate";
        case STP_PORT_ROLE_BACKUP:
            return "Backup";
        case STP_PORT_ROLE_MASTER:
            return "Master";
        default:
            return "(undefined)";
    }
}

const char* STP_GetVersionString(STP_VERSION version)
{
    switch (version)
    {
        case STP_VERSION_LEGACY_STP:
            return "LegacySTP";
        case STP_VERSION_RSTP:
            return "RSTP";
        case STP_VERSION_MULTIPLE_STP:
            return "MSTP";
        default:
            return "(undefined)";
    }
}

const char* STP_GetAdminP2PString(STP_ADMIN_P2P adminP2P)
{
    switch (adminP2P)
    {
        case STP_ADMIN_P2P_AUTO:
            return "Auto";
        case STP_ADMIN_P2P_FORCE_TRUE:
            return "ForceTrue";
        case STP_ADMIN_P2P_FORCE_FALSE:
            return "ForceFalse";
        default:
            return "(undefined)";
    }
}

// ============================================================================

void STP_GetRootPriorityVector(const STP_BRIDGE* bridge, unsigned int treeIndex, unsigned char priorityVectorOut[36])
{
    const unsigned char* rootPriority;
    const unsigned char* rootPortId;

    if (!bridge->started)
    {
        return;
    }

    if (NULL == bridge->trees[treeIndex])
    {
        return;
    }

    rootPriority = (const unsigned char*)&bridge->trees[treeIndex]->rootPriority;
    rootPortId   = (const unsigned char*)&bridge->trees[treeIndex]->rootPortId;
    memcpy(priorityVectorOut, rootPriority, 34);
    priorityVectorOut [34] = rootPortId [0];
    priorityVectorOut [35] = rootPortId [1];
}

void STP_GetRootTimes(const STP_BRIDGE* bridge,
                        unsigned int treeIndex,
                        unsigned short* forwardDelayOutOrNull,
                        unsigned short* helloTimeOutOrNull,
                        unsigned short* maxAgeOutOrNull,
                        unsigned short* messageAgeOutOrNull,
                        unsigned char* remainingHopsOutOrNull)
{
    // A MSTI can be specified (as opposed to the CIST) only while running MSTP.
    if (treeIndex >= bridge->treeCount)
    {
        return;
    }

    BRIDGE_TREE* tree = bridge->trees[treeIndex];

    if (NULL == tree)
    {
        return;
    }

    if (forwardDelayOutOrNull != NULL)
    {
        *forwardDelayOutOrNull = tree->rootTimes.ForwardDelay;
    }

    if (helloTimeOutOrNull != NULL)
    {
        *helloTimeOutOrNull = tree->rootTimes.HelloTime;
    }

    if (maxAgeOutOrNull != NULL)
    {
        *maxAgeOutOrNull = tree->rootTimes.MaxAge;
    }

    if (messageAgeOutOrNull != NULL)
    {
        *messageAgeOutOrNull = tree->rootTimes.MessageAge;
    }

    if (remainingHopsOutOrNull != NULL)
    {
        *remainingHopsOutOrNull = tree->rootTimes.remainingHops;
    }
}

// ============================================================================

bool STP_IsCistRoot(const STP_BRIDGE* bridge)
{
    if (!bridge->started)
    {
        return false;
    }
    BRIDGE_TREE* cist = bridge->trees[CIST_INDEX];
    //return cist->rootPriority.RootId == cist->GetBridgeIdentifier();
    return (0 == memcmp(cist->rootPriority.RootId, cist->BridgeIdentifier, sizeof(BRIDGE_ID)));
}

bool STP_IsRegionalRoot(const STP_BRIDGE* bridge, unsigned int treeIndex)
{
    if (!bridge->started)
    {
        return false;
    }

    if (treeIndex >= bridge->treeCount)
    {

    }

    BRIDGE_TREE* tree = bridge->trees[treeIndex];
    if (NULL == tree)
    {
        return false;
    }
    //return tree->rootPriority.RegionalRootId == tree->GetBridgeIdentifier();
    return (0 == memcmp(tree->rootPriority.RegionalRootId, tree->BridgeIdentifier, sizeof(BRIDGE_ID)));
}

// ============================================================================

void  STP_SetApplicationContext(STP_BRIDGE* bridge, void* applicationContext)
{
    //bridge->applicationContext = applicationContext;
}

void* STP_GetApplicationContext(const STP_BRIDGE* bridge)
{
    //return bridge->applicationContext;
    return NULL;
}

// ============================================================================

void STP_SetAdminExternalPortPathCost(STP_BRIDGE* bridge,
                                            unsigned int portIndex,
                                            unsigned int adminExternalPortPathCost,
                                            unsigned int timestamp)
{
    unsigned int newCost;
    PORT*        port;

    LOG(bridge, portIndex+1, -1, "Setting Port {%d} AdminExternalPortPathCost to {%d}...\r\n", portIndex+1, adminExternalPortPathCost);

    port = bridge->ports[portIndex];
    if (NULL == port)
    {
        return;
    }

    if (port->adminExternalPortPathCost != adminExternalPortPathCost)
    {
        port->adminExternalPortPathCost = adminExternalPortPathCost;

        if (port->portEnabled)
        {
            newCost = (port->adminExternalPortPathCost != 0) ? port->adminExternalPortPathCost : port->detectedPortPathCost;
            if (port->ExternalPortPathCost != newCost)
            {
                port->ExternalPortPathCost = newCost;
                if (bridge->started)
                {
                    RecomputePrioritiesAndPortRoles(bridge, CIST_INDEX, timestamp);
                }
            }
        }
    }

    LOG(bridge, portIndex+1, -1, "------------------------------------\r\n");
    FLUSH_LOG (bridge);
}

void STP_SetAdminInternalPortPathCost(STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex, unsigned int adminInternalPortPathCost, unsigned int timestamp)
{
    LOG(bridge, portIndex+1, -1, "{Setting Port {%d} Tree{%d} AdminInternalPortPathCost to {%d}...\r\n", portIndex+1, treeIndex, adminInternalPortPathCost);

    PORT* port;
    PORT_TREE* portTree;
    unsigned int newCost;

    port = bridge->ports[portIndex];
    if (NULL == port)
    {
        return;
    }

    portTree = port->trees[treeIndex];
    if (NULL == portTree)
    {
        return;
    }

    if (portTree->adminInternalPortPathCost != adminInternalPortPathCost)
    {
        portTree->adminInternalPortPathCost = adminInternalPortPathCost;

        if (port->portEnabled)
        {
            newCost = (portTree->adminInternalPortPathCost != 0) ? portTree->adminInternalPortPathCost : port->detectedPortPathCost;
            if (portTree->InternalPortPathCost != newCost)
            {
                portTree->InternalPortPathCost = newCost;
                if (bridge->started)
                {
                    RecomputePrioritiesAndPortRoles(bridge, treeIndex, timestamp);
                }
            }
        }
    }

    LOG(bridge, portIndex+1, -1, "------------------------------------\r\n");
    FLUSH_LOG(bridge);
}

unsigned int STP_GetAdminExternalPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return 0;
    }

    return bridge->ports[portIndex]->adminExternalPortPathCost;
}

unsigned int STP_GetDetectedPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return 0;
    }

    return bridge->ports[portIndex]->detectedPortPathCost;
}

unsigned int STP_GetExternalPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    const PORT* port = bridge->ports[portIndex];

    if (NULL == port)
    {
        return 0;
    }

    if (!port->portEnabled)
    {
        return 0;
    }

    return port->ExternalPortPathCost;
}

unsigned int STP_GetInternalPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned treeIndex)
{
    const PORT* port = bridge->ports[portIndex];

    if (NULL == port)
    {
        return 0;
    }

    if (!port->portEnabled)
    {
        return 0;
    }

    return port->trees[treeIndex]->InternalPortPathCost;
}

unsigned int STP_GetAdminInternalPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return 0;
    }

    if (NULL == bridge->ports[portIndex]->trees[treeIndex])
    {
        return 0;
    }

    return bridge->ports[portIndex]->trees[treeIndex]->adminInternalPortPathCost;
}

// ============================================================================

void STP_SetBridgeHelloTime(STP_BRIDGE* bridge, unsigned int helloTime, unsigned int timestamp)
{
    // Note AG: In recent versions of the standard this is fixed to two seconds (Table 13-5 on page 510 in 802.1Q-2018),
    // and it's even required to ignore any HelloTime value received and to use two seconds instead (13.29.20 in 802.1Q-2018).
    // I wrote this function only as a placeholder for this comment, so people won't wonder about "missing" functionality.
    assert(helloTime == 2);
}

unsigned int STP_GetBridgeHelloTime(const STP_BRIDGE* bridge)
{
    return bridge->trees[CIST_INDEX]->BridgeTimes.HelloTime;
}

// ============================================================================

void STP_SetBridgeMaxAge(STP_BRIDGE* bridge, unsigned short maxAge, unsigned int timestamp)
{
    // Table 13-5 in 802.1Q-2018
    if ((maxAge < 6) || (maxAge > 40))
    {
        return;
    }

    if (bridge->trees[CIST_INDEX]->BridgeTimes.MaxAge != maxAge)
    {
        bridge->trees[CIST_INDEX]->BridgeTimes.MaxAge = maxAge;
        if (bridge->started)
        {
            RecomputePrioritiesAndPortRoles(bridge, CIST_INDEX, timestamp);
        }
    }
}

unsigned short STP_GetBridgeMaxAge(const STP_BRIDGE* bridge)
{
    return bridge->trees[CIST_INDEX]->BridgeTimes.MaxAge;
}

unsigned int STP_GetMaxAge(const STP_BRIDGE* bridge)
{
    return bridge->trees[CIST_INDEX]->rootTimes.MaxAge;
}

// ============================================================================
void STP_SetBridgeForwardDelay(STP_BRIDGE* bridge, unsigned short forwardDelay, unsigned int timestamp)
{
    // Table 13-5 in 802.1Q-2018
    if ((forwardDelay < 4) || (forwardDelay > 30))
    {
        return;
    }

    if (bridge->trees[CIST_INDEX]->BridgeTimes.ForwardDelay != forwardDelay)
    {
        bridge->trees[CIST_INDEX]->BridgeTimes.ForwardDelay = forwardDelay;
        if (bridge->started)
        {
            RecomputePrioritiesAndPortRoles(bridge, CIST_INDEX, timestamp);
        }
    }
}

unsigned short STP_GetBridgeForwardDelay(const STP_BRIDGE* bridge)
{
    return bridge->trees[CIST_INDEX]->BridgeTimes.ForwardDelay;
}

unsigned short STP_GetForwardDelay(const STP_BRIDGE* bridge)
{
    return bridge->trees[CIST_INDEX]->rootTimes.ForwardDelay;
}

// ============================================================================

void STP_SetTxHoldCount(STP_BRIDGE* bridge, unsigned short txHoldCount, unsigned int timestamp)
{
    unsigned int pi;

    // Table 13-5 in 802.1Q-2018.
    if ((txHoldCount < 1) || (txHoldCount > 10))
    {
        return;
    }
    if (bridge->TxHoldCount != txHoldCount)
    {
        bridge->TxHoldCount = txHoldCount;
        for (pi = 0; pi < bridge->portCount; pi++)
        {
            if (NULL != bridge->ports[pi])
            {
                bridge->ports[pi]->txCount = 0;
            }
        }
    }
}

unsigned short STP_GetTxHoldCount(const STP_BRIDGE* bridge)
{
    return bridge->TxHoldCount;
}

unsigned int STP_GetTxCount(const STP_BRIDGE* bridge, unsigned int portIndex)
{
    if (NULL == bridge->ports[portIndex])
    {
        return 0;
    }

    return bridge->ports[portIndex]->txCount;
}

#ifdef AIR_SUPPORT_MSTP
inline static MST_INSTANCE_VLAN_ENTRY*
_getMstTableByVid(
    MST_INSTANCE_VLAN_ENTRY *ptr_entry,
    UI16_T                  vid)
{
    UI16_T i;

    for (i = 0; i < _bridge->maxVlanNumber; i++)
    {
        if (vid == ptr_entry[i].vid)
        {
            return &ptr_entry[i];
        }
    }

    return NULL;
}

#ifdef MBEDTLS_MD5_C
static inline void
_initComputeMstConfigDigest(
    STP_BRIDGE* bridge)
{
    int                     ret;
    UI8_T                   buf[MSTP_MCID_DIGEST_LEN];
    mbedtls_md5_context     *ptr_ctx = NULL;
    UI8_T                   *ptr_pad = NULL;
    UI16_T                  value = 0, i;
    UI32_T                  len;

    len = sizeof(mbedtls_md5_context);
    osapi_calloc(len, MSTP_NAME, (void **)&ptr_ctx);
    if (NULL == ptr_ctx)
    {
        return;
    }

    len = sizeof(UI8_T) * (MSTP_MCID_PADDING_LEN+1);
    osapi_calloc(len, MSTP_NAME, (void **)&ptr_pad);
    if (NULL == ptr_pad)
    {
        MW_FREE(ptr_ctx);
        return;
    }

    mbedtls_md5_init(ptr_ctx);
    ret = mbedtls_md5_starts(ptr_ctx);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    memset(ptr_pad, 0, len);
    memcpy(ptr_pad, _configurationDigestSignatureKey, sizeof(_configurationDigestSignatureKey));
    for (i = 0; i < MSTP_MCID_PADDING_LEN; i++)
    {
        ptr_pad[i] ^= 0x36;
    }

    ret = mbedtls_md5_update(ptr_ctx, ptr_pad, MSTP_MCID_PADDING_LEN);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    /* Update instance-vlan-mapping */
    for (i = 0; i < MSTP_MAX_VID_NUM; i++)
    {
        ret = mbedtls_md5_update(ptr_ctx, (UI8_T *)&value, sizeof(value));
        if (0 != ret)
        {
            mbedtls_md5_free(ptr_ctx);
            MW_FREE(ptr_ctx);
            MW_FREE(ptr_pad);
            return;
        }
    }

    ret = mbedtls_md5_finish(ptr_ctx, buf);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    ret = mbedtls_md5_starts(ptr_ctx);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    memset(ptr_pad, 0, len);
    memcpy(ptr_pad, _configurationDigestSignatureKey, sizeof(_configurationDigestSignatureKey));
    for (i = 0; i < MSTP_MCID_PADDING_LEN; i++)
    {
        ptr_pad[i] ^= 0x5c;
    }

    ret = mbedtls_md5_update(ptr_ctx, ptr_pad, MSTP_MCID_PADDING_LEN);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    ret = mbedtls_md5_update(ptr_ctx, buf, MSTP_MCID_DIGEST_LEN);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    ret = mbedtls_md5_finish(ptr_ctx, buf);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    memcpy(bridge->MstConfigId.ConfigurationDigest, buf, MSTP_MCID_DIGEST_LEN);
    mbedtls_md5_free(ptr_ctx);
    MW_FREE(ptr_ctx);
    MW_FREE(ptr_pad);
}

static void _ComputeMstConfigDigest(
    MST_INSTANCE_VLAN_ENTRY *ptr_entry)
{
    int         ret;
    UI16_T      value = 0, instance;
    UI8_T       buf[MSTP_MCID_DIGEST_LEN];
    mbedtls_md5_context         *ptr_ctx = NULL;
    UI8_T                       *ptr_pad = NULL;
    MST_INSTANCE_VLAN_ENTRY     *ptr_vlaninfo;
    UI16_T                      i;
    UI32_T                      len;

    len = sizeof(mbedtls_md5_context);
    osapi_calloc(len, MSTP_NAME, (void **)&ptr_ctx);
    if (NULL == ptr_ctx)
    {
        STP_LOG_ERR("Alloc memory for mst config digest failed!");
        return;
    }

    len = sizeof(UI8_T) * (MSTP_MCID_PADDING_LEN+1);
    osapi_calloc(len, MSTP_NAME, (void **)&ptr_pad);
    if (NULL == ptr_pad)
    {
        MW_FREE(ptr_ctx);
        STP_LOG_ERR("Alloc memory for mst config digest failed!");
        return;
    }

    mbedtls_md5_init(ptr_ctx);
    ret = mbedtls_md5_starts(ptr_ctx);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    memset(ptr_pad, 0, len);
    memcpy(ptr_pad, _configurationDigestSignatureKey, sizeof(_configurationDigestSignatureKey));
    for (i = 0; i < MSTP_MCID_PADDING_LEN; i++)
    {
        ptr_pad[i] ^= 0x36;
    }

    ret = mbedtls_md5_update(ptr_ctx, ptr_pad, MSTP_MCID_PADDING_LEN);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    /* Update instance-vlan-mapping */
    ret = mbedtls_md5_update(ptr_ctx, (UI8_T *)&value, sizeof(value));
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }
    for (i = 1; i < MSTP_MAX_VID_NUM; i++)
    {
        ptr_vlaninfo = _getMstTableByVid(ptr_entry, i);
        if (NULL != ptr_vlaninfo)
        {
            instance = (ptr_vlaninfo->instance) & 0xfff;
            instance = htons(instance);
            ret = mbedtls_md5_update(ptr_ctx, (UI8_T *)&instance, sizeof(instance));
            if (0 != ret)
            {
                mbedtls_md5_free(ptr_ctx);
                MW_FREE(ptr_ctx);
                MW_FREE(ptr_pad);
                return;
            }
        }
        else
        {
            ret = mbedtls_md5_update(ptr_ctx, (UI8_T *)&value, sizeof(value));
            if (0 != ret)
            {
                mbedtls_md5_free(ptr_ctx);
                MW_FREE(ptr_ctx);
                MW_FREE(ptr_pad);
                return;
            }
        }
    }

    ret = mbedtls_md5_finish(ptr_ctx, buf);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    ret = mbedtls_md5_starts(ptr_ctx);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    memset(ptr_pad, 0, len);
    memcpy(ptr_pad, _configurationDigestSignatureKey, sizeof(_configurationDigestSignatureKey));
    for (i = 0; i < MSTP_MCID_PADDING_LEN; i++)
    {
        ptr_pad [i] ^= 0x5c;
    }

    ret = mbedtls_md5_update(ptr_ctx, ptr_pad, MSTP_MCID_PADDING_LEN);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    ret = mbedtls_md5_update(ptr_ctx, buf, MSTP_MCID_DIGEST_LEN);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    ret = mbedtls_md5_finish(ptr_ctx, buf);
    if (0 != ret)
    {
        mbedtls_md5_free(ptr_ctx);
        MW_FREE(ptr_ctx);
        MW_FREE(ptr_pad);
        return;
    }

    memcpy(_bridge->MstConfigId.ConfigurationDigest, buf, MSTP_MCID_DIGEST_LEN);
    mbedtls_md5_free(ptr_ctx);
    MW_FREE(ptr_ctx);
    MW_FREE(ptr_pad);
}
#endif

/* Updates the port_enabled parameter. */
static void
_update_port_enabled__(STP_BRIDGE* _bridge, UI32_T port)
{
    PORT        *ptr_port = _bridge->ports[port];
    UI32_T      treeIndex;
    PORT_TREE   *portTree;
    Port_ID     portID;
    UI32_T      adminInternalPortPathCost;

    if (NULL == ptr_port)
    {
        return;
    }

    if (ptr_port->mac_operational && ptr_port->is_administrative_bridge_port)
    {
        ptr_port->portEnabled = TRUE;
        if (ptr_port->adminExternalPortPathCost != 0)
        {
            ptr_port->ExternalPortPathCost = ptr_port->adminExternalPortPathCost;
        }
        else
        {
            ptr_port->ExternalPortPathCost = ptr_port->detectedPortPathCost;
        }

        for (treeIndex = 0; treeIndex < _bridge->treeCount; treeIndex++)
        {
            portTree = ptr_port->trees[treeIndex];
            if (NULL == portTree)
            {
                continue;
            }
            if (portTree->adminInternalPortPathCost != 0)
            {
                portTree->InternalPortPathCost = portTree->adminInternalPortPathCost;
            }
            else
            {
                portTree->InternalPortPathCost = ptr_port->detectedPortPathCost;
            }
        }
    }
    else
    {
        ptr_port->portEnabled = FALSE;
        ptr_port->ExternalPortPathCost = 0;

        ptr_port->portTimersState            = 0;
        ptr_port->portProtocolMigrationState = 0;
        ptr_port->portReceiveState           = 0;
        ptr_port->bridgeDetectionState       = 0;
        ptr_port->l2gpState                  = 0;
        ptr_port->portTransmitState          = 0;

        for (treeIndex = 0; treeIndex < _bridge->treeCount; treeIndex++)
        {
            portTree = ptr_port->trees[treeIndex];
            if (NULL == portTree)
            {
                continue;
            }
            adminInternalPortPathCost = portTree->adminInternalPortPathCost;
            portID = portTree->portId;
            memset(portTree, 0, sizeof(PORT_TREE));
            portTree->adminInternalPortPathCost = adminInternalPortPathCost;
            portTree->portId = portID;
            memcpy(&portTree->portTimes, &_bridge->trees[treeIndex]->BridgeTimes, sizeof(TIMES));
        }
        memcpy(&ptr_port->trees[CIST_INDEX]->designatedTimes, &_bridge->trees[CIST_INDEX]->BridgeTimes, sizeof(TIMES));
    }
}

static void
_mstp_update_init_port_data(
    UI32_T portIndex)
{
    UI32_T   i;

    for (i = 0; i < _bridge->treeCount; i++)
    {
        enableForwarding(_bridge, portIndex, i, 0);
    }
    updtPortDisableRole(_bridge, portIndex);
}

MW_ERROR_NO_T
mstp_create(
    const UI32_T  portCount,
    const UI32_T  trunkCount,
    const UI32_T  mstiCount,
    const UI32_T  maxVlanNumber)
{
    AIR_MAC_T  mac;
    UI32_T     unit = 0;
    UI8_T      bridgeAddress[6];

    osapi_memset(mac, 0, sizeof(AIR_MAC_T));
    if (AIR_E_OK != air_swc_getSystemMac(unit, mac))
    {
        STP_LOG_ERR("Mstp enable, get system mac failed!");
        return MW_E_OP_INVALID;
    }

    osapi_memcpy(bridgeAddress, mac, sizeof(AIR_MAC_T));

    STP_LOG_DBG("Create mstp instance data");
    _bridge = STP_CreateBridge(portCount, trunkCount, mstiCount, maxVlanNumber, bridgeAddress);
    if (NULL == _bridge)
    {
        STP_LOG_ERR("Create mstp instance data failed!");
        return MW_E_NO_MEMORY;
    }

    STP_StartBridge(_bridge, 0);

    return MW_E_OK;
}

void
mstp_delete(
    void)
{
    _bridge->started = false;

    STP_LOG_DBG("Delete mstp instance data");
    STP_DestroyBridge(_bridge);
    _bridge = NULL;
}

void
mstp_setRegionConfig(
    unsigned short revision,
    unsigned char *ptr_name)
{
    if (NULL == _bridge)
    {
        return;
    }

    if ((revision != _bridge->MstConfigId.RevisionLevel)
        || (0 != osapi_strncmp(_bridge->MstConfigId.ConfigurationName, (C8_T *)ptr_name, STP_REGION_NAME_LEN)))
    {
        STP_LOG_DBG("Set region config, revision %u, region name %s", revision, ptr_name);

        _bridge->MstConfigId.RevisionLevel = htons(revision);
        osapi_strncpy(_bridge->MstConfigId.ConfigurationName, (C8_T *)ptr_name, STP_REGION_NAME_LEN);

        if (_bridge->started)
        {
            RestartStateMachines(_bridge, 0);
        }
    }
}

void
mstp_setInstanceMappingVlan(
    MST_INSTANCE_VLAN_ENTRY *ptr_entry)
{
    if (NULL == _bridge)
    {
        return;
    }
    STP_LOG_DBG("Compute mst config digest");
#ifdef MBEDTLS_MD5_C
    _ComputeMstConfigDigest(ptr_entry);
#endif
}

void
mstp_setBridgePriority(
    UI16_T instanceIdx,
    UI16_T priority)
{
    UI32_T   idx;
    UI16_T   instanceId;

    if (NULL == _bridge)
    {
        return;
    }

    idx = (instanceIdx & 0xf000) >> 12;
    instanceId = instanceIdx & 0x0fff;

    if (idx >= _bridge->treeCount)
    {
        return;
    }

    STP_LOG_DBG("Set instace %d fid %d priority %u", instanceId, idx, priority);
    return STP_SetBridgePriority(_bridge, idx, instanceId, priority, 0);
}

void
mstp_setPortPriority(
    UI32_T port_id,
    UI32_T instanceIdx,
    UI8_T  portPriority)
{
    if (NULL == _bridge)
    {
        return;
    }

    if (instanceIdx >= _bridge->treeCount)
    {
        return;
    }

    STP_LOG_DBG("Set port %d fid %d priority %u", port_id, instanceIdx, portPriority);
    return STP_SetPortPriority(_bridge, (port_id-1), instanceIdx, portPriority, 0);
}

void
mstp_setPortPathCost(
    UI32_T port_id,
    UI32_T instanceIdx,
    UI32_T portCost)
{
    if (NULL == _bridge)
    {
        return;
    }
    if (CIST_INDEX == instanceIdx)
    {
        STP_LOG_DBG("Setting port %d adminExternalPortPathCost to %d", port_id, portCost);
        STP_SetAdminExternalPortPathCost(_bridge, (port_id-1), portCost, 0);
    }

    STP_LOG_DBG("Setting port %d fid %d adminInternalPortPathCost to %d", port_id, instanceIdx, portCost);
    STP_SetAdminInternalPortPathCost(_bridge, (port_id-1), instanceIdx, portCost, 0);

    return;
}

void
mstp_tick_timers(
    void)
{
    if (NULL == _bridge)
    {
        return;
    }

    STP_OnOneSecondTick(_bridge, 0);
}

void
mstp_bpdu_recv(
    UI32_T      portNum,
    const UI8_T *bpdu,
    UI32_T      bpduSize)
{
    if (NULL == _bridge)
    {
        return;
    }

    if ((portNum-1) >= _bridge->portCount)
    {
        return;
    }

    STP_OnBpduReceived(_bridge, (portNum-1), bpdu, bpduSize, 0);
}

void
mstp_updt_instance_portMap(
    UI32_T   insIdx,
    UI32_T   bmp,
    UI8_T    clearFlag)
{
    if (NULL == _bridge)
    {
        return;
    }

    if (insIdx >= _bridge->treeCount)
    {
        return;
    }

    if (NULL != _bridge->trees[insIdx])
    {
        if (clearFlag)
        {
            _bridge->trees[insIdx]->memBmp = 0;
        }
        else
        {
            _bridge->trees[insIdx]->memBmp |= bmp;
        }
    }
}

MW_ERROR_NO_T
mstp_add_port(
    UI32_T   port)
{
    PORT            *ptr_port = NULL;
    PORT_TREE       *ptr_portTree = NULL;
    UI32_T          i;
    BRIDGE_TREE     *ptr_bridge;

    if (NULL == _bridge)
    {
        return MW_E_OK;
    }
    if (((port-1) >= _bridge->portCount) || (0 == port))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (NULL == _bridge->ports[port-1])
    {
        osapi_calloc(sizeof(PORT), MSTP_NAME, (void **)&ptr_port);
        if (NULL == ptr_port)
        {
            STP_LOG_ERR("Alloc memory for port %d failed!", port);
            return MW_E_NO_MEMORY;
        }
        osapi_calloc(_bridge->treeCount * sizeof(PORT_TREE*), MSTP_NAME, (void **)&ptr_port->trees);
        if (NULL == ptr_port->trees)
        {
            MW_FREE(ptr_port);
            STP_LOG_ERR("Alloc memory for port %d failed!", port);
            return MW_E_NO_MEMORY;
        }

        ptr_port->adminPointToPointMAC = STP_ADMIN_P2P_AUTO;
        ptr_port->AutoEdge = true;
        ptr_port->enableBPDUrx = true;
        ptr_port->enableBPDUtx = true;
        _bridge->ports[port-1] = ptr_port;
    }

    for (i = 0; i < _bridge->treeCount; i++)
    {
        ptr_bridge = _bridge->trees[i];
        if (NULL == ptr_bridge)
        {
            continue;
        }

        if (NULL !=  _bridge->ports[port-1]->trees[i])
        {
            _bridge->ports[port-1]->trees[i]->portId = ((0x80 << 8) | port);
            _bridge->ports[port-1]->trees[i]->portTimes = _bridge->trees[i]->BridgeTimes;
            continue;
        }

        osapi_calloc(sizeof(PORT_TREE), MSTP_NAME, (void **)&ptr_portTree);
        if (NULL == ptr_portTree)
        {
            STP_LOG_ERR("Alloc memory for port %d fid %d instance failed!", port, i);
            return MW_E_NO_MEMORY;
        }

        ptr_portTree->portId = ((0x80 << 8) | port);
        ptr_portTree->portTimes = _bridge->trees[i]->BridgeTimes;
         _bridge->ports[port-1]->trees[i] = ptr_portTree;
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
mstp_add_instance_port(
    MSTP_INS_PARAM_T *info)
{
    MW_ERROR_NO_T   ret = MW_E_ENTRY_EXISTS;
    PORT            *ptr_port = NULL;
    PORT_TREE       *ptr_portTree = NULL;
    UI32_T          portIdx = info->portNum -1;
    UI32_T          mstiIndex = (info->insIdx & 0xf000) >> 12;
    UI32_T          instance = (info->insIdx & 0xfff);
    BRIDGE_TREE     *ptr_bridge;
    UI32_T          len;
    UI16_T          priority;

    if (NULL == _bridge)
    {
        return MW_E_OK;
    }

    if ((portIdx >= _bridge->portCount)
        || (0 == info->portNum)
        || (mstiIndex >= _bridge->treeCount))
    {
        return MW_E_BAD_PARAMETER;
    }

    if (NULL == _bridge->trees[mstiIndex])
    {
        ret = MW_E_OK;
        len = sizeof(BRIDGE_TREE);
        osapi_calloc(len, MSTP_NAME, (void **)&ptr_bridge);
        if (NULL == ptr_bridge)
        {
            STP_LOG_ERR("Alloc memory for instance %d fid %d failed!", instance, mstiIndex);
            return MW_E_NO_MEMORY;
        }

        priority = (32768 | instance);
        priority = htons(priority);
        memcpy(ptr_bridge->BridgeIdentifier, &priority, sizeof(priority));
        memcpy((ptr_bridge->BridgeIdentifier)+2, (_bridge->trees[CIST_INDEX]->BridgeIdentifier)+2, 6);
        memcpy(ptr_bridge->BridgePriority.RegionalRootId, ptr_bridge->BridgeIdentifier, sizeof(BRIDGE_ID));
        memcpy(ptr_bridge->BridgePriority.DesignatedBridgeId, ptr_bridge->BridgeIdentifier, sizeof(BRIDGE_ID));
        ptr_bridge->BridgeTimes.remainingHops = 20;

        ptr_bridge->instance = instance;
        _bridge->trees[mstiIndex] = ptr_bridge;
    }

    if (NULL == _bridge->ports[portIdx])
    {
        ret = MW_E_OK;
        len = sizeof(PORT);
        osapi_calloc(len, MSTP_NAME, (void **)&ptr_port);
        if (NULL == ptr_port)
        {
            STP_LOG_ERR("Alloc memory for port %d failed!", portIdx+1);
            return MW_E_NO_MEMORY;
        }

        ptr_port->adminPointToPointMAC = STP_ADMIN_P2P_AUTO;
        ptr_port->AutoEdge = true;
        ptr_port->enableBPDUrx = true;
        ptr_port->enableBPDUtx = true;
        ptr_port->detectedPortPathCost = 20000;
        _bridge->ports[portIdx] = ptr_port;

        len = _bridge->treeCount * sizeof(PORT_TREE *);
        osapi_calloc(len, MSTP_NAME, (void **)&(ptr_port->trees));
        if (NULL == ptr_port->trees)
        {
            STP_LOG_ERR("Alloc memory for port %d failed!", portIdx+1);
            return MW_E_NO_MEMORY;
        }
    }

    if (NULL == _bridge->ports[portIdx]->trees[mstiIndex])
    {
        ret = MW_E_OK;
        len = sizeof(PORT_TREE);
        osapi_calloc(len, MSTP_NAME, (void **)&ptr_portTree);
        if (NULL == ptr_portTree)
        {
            STP_LOG_ERR("Alloc memory for port %d instance %d fid %d failed!", portIdx+1, instance, mstiIndex);
            return MW_E_NO_MEMORY;
        }

        ptr_portTree->portId = ((0x80 << 8) | info->portNum);
        ptr_portTree->portTimes = _bridge->trees[mstiIndex]->BridgeTimes;
        _bridge->ports[portIdx]->trees[mstiIndex] = ptr_portTree;
    }

    return ret;
}

void
mstp_del_instance_port(
    MSTP_INS_PARAM_T *info)
{
    UI32_T      portIdx = info->portNum -1;

    if (NULL == _bridge)
    {
        return;
    }

    if (0 == info->insIdx)
    {
        return;
    }

    if ((portIdx >= _bridge->portCount)
        || (0 == info->portNum)
        || (info->insIdx >= _bridge->treeCount))
    {
        return;
    }

    if (NULL == _bridge->ports[portIdx])
    {
        return;
    }

    if (NULL == _bridge->ports[portIdx]->trees)
    {
        return;
    }

    if (NULL == _bridge->ports[portIdx]->trees[info->insIdx])
    {
        return;
    }

    MW_FREE(_bridge->ports[portIdx]->trees[info->insIdx]);

    if ((_bridge->started) && (info->insIdx < _bridge->treeCount))
    {
        RecomputePrioritiesAndPortRoles(_bridge, info->insIdx, 0);
    }

}

void
mstp_del_port(
    UI32_T   port)
{
    UI32_T      portIdx = port -1;
    UI32_T      i;

    if (NULL == _bridge)
    {
        return;
    }
    if ((portIdx >= _bridge->portCount) || (0 == port))
    {
        return;
    }

    if (NULL == _bridge->ports[portIdx])
    {
        return;
    }

    if (NULL == _bridge->ports[portIdx]->trees)
    {
        return;
    }

    for (i = 0; i < _bridge->treeCount; i++)
    {
        if (NULL == _bridge->ports[portIdx]->trees[i])
        {
            continue;
        }
        MW_FREE(_bridge->ports[portIdx]->trees[i]);
        if ((_bridge->started) && (i < _bridge->treeCount))
        {
            RecomputePrioritiesAndPortRoles(_bridge, i, 0);
        }
    }
    MW_FREE(_bridge->ports[portIdx]->trees);
    MW_FREE(_bridge->ports[portIdx]);
    STP_LOG_DBG("Delete mstp port %d data", portIdx+1);
}

void
mstp_del_instance(
    MSTP_INS_PARAM_T *info)
{
    UI32_T      portIdx;
    UI32_T      mstiIndex = (info->insIdx & 0xf000) >> 12;
    UI8_T       changeFlag = FALSE;

    if (NULL == _bridge)
    {
        return;
    }

    if (CIST_INDEX == mstiIndex)
    {
        return;
    }

    if (mstiIndex >= _bridge->treeCount)
    {
        return;
    }

    for (portIdx = 0; portIdx < _bridge->portCount; portIdx++)
    {
        if (NULL == _bridge->ports[portIdx])
        {
            continue;
        }

        if (NULL == _bridge->ports[portIdx]->trees[mstiIndex])
        {
            continue;
        }

        MW_FREE(_bridge->ports[portIdx]->trees[mstiIndex]);
        changeFlag = TRUE;
    }
    if (NULL != _bridge->trees[mstiIndex])
    {
        MW_FREE(_bridge->trees[mstiIndex]);
        changeFlag = TRUE;
    }

    if (changeFlag && (_bridge->started))
    {
        RecomputePrioritiesAndPortRoles(_bridge, mstiIndex, 0);
    }
}

void
mstp_set_bridge_forward_delay(
    UI16_T  forward_delay)
{
    UI16_T  fwdDelay;

    if (NULL == _bridge)
    {
        return;
    }

    STP_LOG_DBG("Set forward delay to %u", forward_delay);
    fwdDelay = STP_GetBridgeForwardDelay(_bridge);
    if (fwdDelay != forward_delay)
    {
        STP_SetBridgeForwardDelay(_bridge, forward_delay, 0);
    }
}

void
mstp_set_bridge_max_age(
    UI16_T  max_age)
{
    UI16_T  maxAge;

    if (NULL == _bridge)
    {
        return;
    }

    STP_LOG_DBG("Set max age to %u", max_age);
    maxAge = STP_GetBridgeMaxAge(_bridge);
    if (maxAge != max_age)
    {
        STP_SetBridgeMaxAge(_bridge, max_age, 0);
    }
}

void
mstp_set_bridge_transmit_hold_count(
    UI16_T  tx_cnt)
{
    UI16_T  tx_count;

    if (NULL == _bridge)
    {
        return;
    }

    STP_LOG_DBG("Set tx hold count to %u", tx_cnt);
    tx_count = STP_GetTxHoldCount(_bridge);
    if (tx_count != tx_cnt)
    {
        STP_SetTxHoldCount(_bridge, tx_cnt, 0);
    }
}

void
mstp_set_port_macOperational(
    UI32_T  port,
    UI8_T   status)
{
    PORT *ptr_port;
    bool state = FALSE;
    UI32_T  i;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }
    STP_LOG_DBG("Set port %d mac-operational to %u", status);
    if (ptr_port->mac_operational != status)
    {
        ptr_port->mac_operational = status;
        state = ptr_port->portEnabled;
        _update_port_enabled__(_bridge, (port-1));
        if ((_bridge->started) && (state != ptr_port->portEnabled))
        {
            RunStateMachines(_bridge, 0);
        }
        ptr_port->run_flag = ptr_port->portEnabled;
        if (!ptr_port->portEnabled)
        {
            _mstp_update_init_port_data(port-1);
        }

        /* port link down, set port state is discard */
        if (FALSE == status)
        {
            for (i = 0; i < _bridge->treeCount; i++)
            {
                disableForwarding(_bridge, (port-1), i, 0);
            }
        }
    }
}

void
mstp_set_port_administrative_bridge_port(
    UI32_T  port,
    BOOL_T  status)
{
    PORT      *ptr_port;
    bool      state = FALSE;
    UI32_T    i;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d admin-bridge-port  to %u", status);
    if (ptr_port->is_administrative_bridge_port != status)
    {
        ptr_port->is_administrative_bridge_port = status;
        state = ptr_port->portEnabled;
        _update_port_enabled__(_bridge, (port-1));

        if (TRUE == status)
        {
            for (i = 0; i < _bridge->treeCount; i++)
            {
                disableForwarding(_bridge, (port-1), i, 0);
            }
        }

        if ((_bridge->started) && (state != ptr_port->portEnabled))
        {
            RunStateMachines(_bridge, 0);
        }

        ptr_port->run_flag = ptr_port->portEnabled;
        if (!ptr_port->portEnabled)
        {
            _mstp_update_init_port_data(port-1);
        }
        if (FALSE == status)
        {
            for (i = 0; i < _bridge->treeCount; i++)
            {
                enableForwarding(_bridge, (port-1), i, 0);
            }
        }
    }
}

void
mstp_set_port_oper_point_to_point_mac(
    UI32_T  port,
    UI8_T   oper_mac_p2p_mac)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d p2p mac to %u", oper_mac_p2p_mac);
    if (ptr_port->operPointToPointMAC == oper_mac_p2p_mac)
    {
        return;
    }

    if (ptr_port->adminPointToPointMAC == STP_ADMIN_P2P_AUTO)
    {
        ptr_port->operPointToPointMAC = oper_mac_p2p_mac;
        STP_LOG_DBG("Set port %d mstp p2p-mac to %d", port, oper_mac_p2p_mac);
        _update_port_enabled__(_bridge, (port-1));
    }
}

#ifdef AIR_SUPPORT_RSTP_SECURITY
UI8_T
mstp_get_port_bpdu_status(
    UI32_T  port)
{
    PORT    *ptr_port;
    UI8_T   status = FALSE;

    if (NULL == _bridge)
    {
        return status;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL != ptr_port)
    {
        status = ptr_port->bpdu_status;
    }

    return status;
}

void
mstp_set_port_bpdu_status(
    UI32_T  port,
    UI8_T   status)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d mstp bpdu status to %d", port, status);
    ptr_port->bpdu_status = status;
}

void
mstp_set_port_tc_status(
    UI32_T  port,
    UI8_T   status)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d mstp tc-protect status to %d", port, status);
    ptr_port->tc_status = status;
}

void
mstp_set_port_tc_interval(
    UI32_T  port,
    UI8_T   interval)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d mstp tc-protect interval to %d", port, interval);
    ptr_port->tc_interval = interval;
}

void
mstp_set_port_tc_threshold(
    UI32_T  port,
    UI8_T   threshold)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d mstp tc-protect threshold to %d", port, threshold);
    ptr_port->tc_threshold = threshold;
}

void
mstp_set_port_root_guard_status(
    UI32_T  port,
    UI8_T   status)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d mstp root-protect status to %d", port, status);
    ptr_port->root_status = status;
}

void
mstp_set_port_loop_guard_status(
    UI32_T  port,
    UI8_T   status)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d mstp loop-protect status to %d", port, status);
    ptr_port->loop_status = status;
}
#endif

void
mstp_set_port_admin_edge(
    UI32_T  port,
    UI8_T   status)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d mstp admin edge status to %d", port, status);
    _bridge->ports[port-1]->AdminEdge = status;
}

void
mstp_set_port_mcheck(
    UI32_T  port,
    UI8_T   status)
{
    PORT *ptr_port;

    if (NULL == _bridge)
    {
        return;
    }

    ptr_port = _bridge->ports[port-1];
    if (NULL == ptr_port)
    {
        return;
    }

    STP_LOG_DBG("Set port %d mstp mcheck status to %d", port, status);
    _bridge->ports[port-1]->mcheck = status;
}

void
mstp_get_mcid_digest(
    UI8_T  *ptr_digest)
{
    if (NULL == _bridge)
    {
        return;
    }

    memcpy(ptr_digest, _bridge->MstConfigId.ConfigurationDigest, 16);
}

void
mstp_show_port_param(
    UI32_T   portId)

{
    PORT           *ptr_port;
    PORT_TREE      *ptr_tree;
    UI32_T         i;

    MW_CMD_OUTPUT("[Port:%u]\n", portId);
    MW_CMD_OUTPUT("------------------------------------------------------------\n");

    MW_CMD_OUTPUT("[Instance param:]\n");
    MW_CMD_OUTPUT("    %-5s  %-5s\n", "Instance", "Exist");
    for (i = 0; i < _bridge->treeCount; i++)
    {
        MW_CMD_OUTPUT("    %-8d  %-8s\n", i, (_bridge->trees[i])?"YES":"NO");
    }

    MW_CMD_OUTPUT("[Port param:]\n");
    ptr_port = _bridge->ports[portId-1];
    if (NULL == ptr_port)
    {
        MW_CMD_OUTPUT("No create\n");
        return;
    }

    MW_CMD_OUTPUT("    %-5s, %-5s\n", "Instance", "Exist");
    for (i = 0; i < _bridge->treeCount; i++)
    {
        ptr_tree = ptr_port->trees[i];
        MW_CMD_OUTPUT("    %-8d, %-8s\n", i, (ptr_tree)?"YES":"NO");
    }
}

void
mstp_show_port_instance_param(
    UI32_T   treeId,
    UI32_T   portId)
{
    PORT           *ptr_port;
    PORT_TREE      *ptr_tree;
    BRIDGE_TREE    *ptr_bridge_tree;

    if (NULL == _bridge)
    {
        return;
    }

    if (MSTP_INVALID_FID == treeId)
    {
        mstp_show_port_param(portId);
    }

    if (treeId >= _bridge->treeCount)
    {
        return;
    }

    ptr_port = _bridge->ports[portId-1];
    if (NULL == ptr_port)
    {
        return;
    }

    ptr_tree = ptr_port->trees[treeId];
    ptr_bridge_tree = _bridge->trees[treeId];

    MW_CMD_OUTPUT("[Port:%u, treeIndex %u]\n", portId, treeId);
    MW_CMD_OUTPUT("------------------------------------------------------------\n");

    MW_CMD_OUTPUT( "Instance param:\n");
    if (NULL != ptr_bridge_tree)
    {
        MW_CMD_OUTPUT( "\n");
        MW_CMD_OUTPUT("BridgeIdentifier: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_bridge_tree->BridgeIdentifier));
        MW_CMD_OUTPUT("BridgePriority.root_bridge_id: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_bridge_tree->BridgePriority.RootId));
        MW_CMD_OUTPUT("BridgePriority.ExternalRootPathCost: %u\n", ptr_bridge_tree->BridgePriority.ExternalRootPathCost);
        MW_CMD_OUTPUT("BridgePriority.RegionalRootId: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_bridge_tree->BridgePriority.RegionalRootId));
        MW_CMD_OUTPUT("BridgePriority.designated_port_id: 0x%04x\n", ptr_bridge_tree->BridgePriority.DesignatedPortId);

        MW_CMD_OUTPUT( "\n");
        MW_CMD_OUTPUT( "BridgeTimes.forward_delay: %u\n", ptr_bridge_tree->BridgeTimes.ForwardDelay);
        MW_CMD_OUTPUT( "BridgeTimes.hello_time: %u\n", ptr_bridge_tree->BridgeTimes.HelloTime);
        MW_CMD_OUTPUT( "BridgeTimes.max_age: %u\n", ptr_bridge_tree->BridgeTimes.MaxAge);
        MW_CMD_OUTPUT( "BridgeTimes.message_age: %u\n", ptr_bridge_tree->BridgeTimes.MessageAge);
        MW_CMD_OUTPUT( "BridgeTimes.remainingHops: %u\n", ptr_bridge_tree->BridgeTimes.remainingHops);

        MW_CMD_OUTPUT("rootPortId: 0x%04x\n", ptr_bridge_tree->rootPortId);

        MW_CMD_OUTPUT("rootPriority.root_bridge_id: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_bridge_tree->rootPriority.RootId));
        MW_CMD_OUTPUT("rootPriority.ExternalRootPathCost: %u\n", ptr_bridge_tree->rootPriority.ExternalRootPathCost);
        MW_CMD_OUTPUT("rootPriority.RegionalRootId: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_bridge_tree->rootPriority.RegionalRootId));
        MW_CMD_OUTPUT("rootPriority.designated_port_id: 0x%04x\n", ptr_bridge_tree->rootPriority.DesignatedPortId);

        MW_CMD_OUTPUT( "rootTimes.forward_delay: %u\n", ptr_bridge_tree->rootTimes.ForwardDelay);
        MW_CMD_OUTPUT( "rootTimes.hello_time: %u\n", ptr_bridge_tree->rootTimes.HelloTime);
        MW_CMD_OUTPUT( "rootTimes.max_age: %u\n", ptr_bridge_tree->rootTimes.MaxAge);
        MW_CMD_OUTPUT( "rootTimes.message_age: %u\n", ptr_bridge_tree->rootTimes.MessageAge);
        MW_CMD_OUTPUT( "rootTimes.remainingHops: %u\n", ptr_bridge_tree->rootTimes.remainingHops);

        MW_CMD_OUTPUT("portRoleSelectionState: %d\n", ptr_bridge_tree->portRoleSelectionState);
    }

    MW_CMD_OUTPUT( "\n");
    MW_CMD_OUTPUT( "Port param:\n");

    if (NULL != ptr_port)
    {
        MW_CMD_OUTPUT( "AdminEdge: %u\n", ptr_port->AdminEdge);
        MW_CMD_OUTPUT( "AutoEdge: %u\n", ptr_port->AutoEdge);
        MW_CMD_OUTPUT( "AutoIsolate: %u\n", ptr_port->AutoIsolate);
        MW_CMD_OUTPUT( "enableBPDUrx: %u\n", ptr_port->enableBPDUrx);
        MW_CMD_OUTPUT( "enableBPDUtx: %u\n", ptr_port->enableBPDUtx);
        MW_CMD_OUTPUT( "isL2gp: %u\n", ptr_port->isL2gp);
        MW_CMD_OUTPUT( "isolate: %u\n", ptr_port->isolate);
        MW_CMD_OUTPUT( "mcheck: %u\n", ptr_port->mcheck);
        MW_CMD_OUTPUT( "newInfo: %u\n", ptr_port->newInfo);
        MW_CMD_OUTPUT( "operEdge: %u\n", ptr_port->operEdge);
        MW_CMD_OUTPUT( "portEnabled: %u\n", ptr_port->portEnabled);
        MW_CMD_OUTPUT( "rcvdBpdu: %u\n", ptr_port->rcvdBpdu);
        MW_CMD_OUTPUT( "rcvdRSTP: %u\n", ptr_port->rcvdRSTP);
        MW_CMD_OUTPUT( "rcvdSTP: %u\n", ptr_port->rcvdSTP);
        MW_CMD_OUTPUT( "rcvdTcAck: %u\n", ptr_port->rcvdTcAck);
        MW_CMD_OUTPUT( "rcvdTcn: %u\n", ptr_port->rcvdTcn);
        MW_CMD_OUTPUT( "restrictedRole: %u\n", ptr_port->restrictedRole);
        MW_CMD_OUTPUT( "restrictedRole: %u\n", ptr_port->restrictedTcn);
        MW_CMD_OUTPUT( "sendRSTP: %u\n", ptr_port->sendRSTP);
        MW_CMD_OUTPUT( "tcAck: %u\n", ptr_port->tcAck);
        MW_CMD_OUTPUT( "tcAck: %u\n", ptr_port->tick);
        MW_CMD_OUTPUT( "detectedPointToPointMAC: %u\n", ptr_port->detectedPointToPointMAC);
        MW_CMD_OUTPUT( "operPointToPointMAC: %u\n", ptr_port->operPointToPointMAC);
        MW_CMD_OUTPUT( "rcvdInternal: %u\n", ptr_port->rcvdInternal);
        MW_CMD_OUTPUT( "restrictedDomainRole: %u\n", ptr_port->restrictedDomainRole);
        MW_CMD_OUTPUT( "newInfoMsti: %u\n", ptr_port->newInfoMsti);
        MW_CMD_OUTPUT( "infoInternal: %u\n", ptr_port->infoInternal);
        MW_CMD_OUTPUT( "master: %u\n", ptr_port->master);
        MW_CMD_OUTPUT( "mastered: %u\n", ptr_port->mastered);
        MW_CMD_OUTPUT( "txCount: %u\n", ptr_port->txCount);
        MW_CMD_OUTPUT( "ageingTime: %u\n", ptr_port->ageingTime);
        MW_CMD_OUTPUT( "ExternalPortPathCost: %u\n", ptr_port->ExternalPortPathCost);
        MW_CMD_OUTPUT( "edgeDelayWhile: %u\n", ptr_port->edgeDelayWhile);
        MW_CMD_OUTPUT( "helloWhen: %u\n", ptr_port->helloWhen);
        MW_CMD_OUTPUT( "mDelayWhile: %u\n", ptr_port->mDelayWhile);
        MW_CMD_OUTPUT( "adminPointToPointMAC: %u\n", ptr_port->adminPointToPointMAC);
        MW_CMD_OUTPUT( "detectedPortPathCost: %u\n", ptr_port->detectedPortPathCost);
        MW_CMD_OUTPUT( "adminExternalPortPathCost: %u\n", ptr_port->adminExternalPortPathCost);
        MW_CMD_OUTPUT( "portTimersState: %u\n", ptr_port->portTimersState);
        MW_CMD_OUTPUT( "portProtocolMigrationState: %u\n", ptr_port->portProtocolMigrationState);
        MW_CMD_OUTPUT( "portReceiveState: %u\n", ptr_port->portReceiveState);
        MW_CMD_OUTPUT( "bridgeDetectionState: %u\n", ptr_port->bridgeDetectionState);
        MW_CMD_OUTPUT( "l2gpState: %u\n", ptr_port->l2gpState);
        MW_CMD_OUTPUT( "portTransmitState: %u\n", ptr_port->portTransmitState);
    }

    MW_CMD_OUTPUT( "Per-port-tree param:\n");

    if ((NULL == ptr_port) || (NULL == ptr_tree))
    {
        return;
    }

    MW_CMD_OUTPUT("== designated_priority_vector ==\n");
    MW_CMD_OUTPUT("root_bridge_id: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->designatedPriority.RootId));
    MW_CMD_OUTPUT("ExternalRootPathCost: %u\n", ptr_tree->designatedPriority.ExternalRootPathCost);
    MW_CMD_OUTPUT("RegionalRootId: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->designatedPriority.RegionalRootId));
    MW_CMD_OUTPUT("InternalRootPathCost: %u\n", ptr_tree->designatedPriority.InternalRootPathCost);
    MW_CMD_OUTPUT("designated_bridge_id: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->designatedPriority.DesignatedBridgeId));
    MW_CMD_OUTPUT("designated_port_id: 0x%04x\n", ptr_tree->designatedPriority.DesignatedPortId);
    MW_CMD_OUTPUT("bridge_port_id: 0x%04x\n", ptr_tree->portId);

    MW_CMD_OUTPUT( "\n== msg_priority ==\n");
    MW_CMD_OUTPUT("root_bridge_id: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->msgPriority.RootId));
    MW_CMD_OUTPUT("ExternalRootPathCost: %u\n", ptr_tree->msgPriority.ExternalRootPathCost);
    MW_CMD_OUTPUT("RegionalRootId: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->msgPriority.RegionalRootId));
    MW_CMD_OUTPUT("InternalRootPathCost: %u\n", ptr_tree->msgPriority.InternalRootPathCost);
    MW_CMD_OUTPUT("designated_bridge_id: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->msgPriority.DesignatedBridgeId));
    MW_CMD_OUTPUT("designated_port_id: 0x%04x\n", ptr_tree->msgPriority.DesignatedPortId);
    MW_CMD_OUTPUT("bridge_port_id: 0x%04x\n", ptr_tree->portId);

    MW_CMD_OUTPUT( "\n== port_priority ==\n");
    MW_CMD_OUTPUT("root_bridge_id: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->portPriority.RootId));
    MW_CMD_OUTPUT("ExternalRootPathCost: %u\n", ptr_tree->portPriority.ExternalRootPathCost);
    MW_CMD_OUTPUT("RegionalRootId: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->portPriority.RegionalRootId));
    MW_CMD_OUTPUT("InternalRootPathCost: %u\n", ptr_tree->portPriority.InternalRootPathCost);
    MW_CMD_OUTPUT("designated_bridge_id: "BRIDGE_STR"\n", BRIDGE_VALUE(ptr_tree->portPriority.DesignatedBridgeId));
    MW_CMD_OUTPUT("designated_port_id: 0x%04x\n", ptr_tree->portPriority.DesignatedPortId);
    MW_CMD_OUTPUT("bridge_port_id: 0x%04x\n", ptr_tree->portId);

    MW_CMD_OUTPUT( "\n");
    MW_CMD_OUTPUT( "designated_times.forward_delay: %u\n", ptr_tree->designatedTimes.ForwardDelay);
    MW_CMD_OUTPUT( "designated_times.hello_time: %u\n", ptr_tree->designatedTimes.HelloTime);
    MW_CMD_OUTPUT( "designated_times.max_age: %u\n", ptr_tree->designatedTimes.MaxAge);
    MW_CMD_OUTPUT( "designated_times.message_age: %u\n", ptr_tree->designatedTimes.MessageAge);
    MW_CMD_OUTPUT( "designated_times.remainingHops: %u\n", ptr_tree->designatedTimes.remainingHops);

    MW_CMD_OUTPUT( "\n");
    MW_CMD_OUTPUT( "msg_times.forward_delay: %u\n", ptr_tree->msgTimes.ForwardDelay);
    MW_CMD_OUTPUT( "msg_times.hello_time: %u\n", ptr_tree->msgTimes.HelloTime);
    MW_CMD_OUTPUT( "msg_times.max_age: %u\n", ptr_tree->msgTimes.MaxAge);
    MW_CMD_OUTPUT( "msg_times.message_age: %u\n", ptr_tree->msgTimes.MessageAge);
    MW_CMD_OUTPUT( "msg_times.remainingHops: %u\n", ptr_tree->msgTimes.remainingHops);

    MW_CMD_OUTPUT( "\n");
    MW_CMD_OUTPUT( "port_times.forward_delay: %u\n", ptr_tree->portTimes.ForwardDelay);
    MW_CMD_OUTPUT( "port_times.hello_time: %u\n", ptr_tree->portTimes.HelloTime);
    MW_CMD_OUTPUT( "port_times.max_age: %u\n", ptr_tree->portTimes.MaxAge);
    MW_CMD_OUTPUT( "port_times.message_age: %u\n", ptr_tree->portTimes.MessageAge);
    MW_CMD_OUTPUT( "port_times.remainingHops: %u\n", ptr_tree->portTimes.remainingHops);

    MW_CMD_OUTPUT( "agree: %u\n", ptr_tree->agree);
    MW_CMD_OUTPUT( "agreed: %u\n", ptr_tree->agreed);
    MW_CMD_OUTPUT( "disputed: %u\n", ptr_tree->disputed);
    MW_CMD_OUTPUT( "fdbFlush: %u\n", ptr_tree->fdbFlush);
    MW_CMD_OUTPUT( "forward: %u\n", ptr_tree->forward);
    MW_CMD_OUTPUT( "forwarding: %u\n", ptr_tree->forwarding);
    MW_CMD_OUTPUT( "learn: %u\n", ptr_tree->learn);
    MW_CMD_OUTPUT( "learning: %u\n", ptr_tree->learning);
    MW_CMD_OUTPUT( "proposed: %u\n", ptr_tree->proposed);
    MW_CMD_OUTPUT( "proposing: %u\n", ptr_tree->proposing);
    MW_CMD_OUTPUT( "rcvdMsg: %u\n", ptr_tree->rcvdMsg);
    MW_CMD_OUTPUT( "rcvdTc: %u\n", ptr_tree->rcvdTc);
    MW_CMD_OUTPUT( "reRoot: %u\n", ptr_tree->reRoot);
    MW_CMD_OUTPUT( "reselect: %u\n", ptr_tree->reselect);
    MW_CMD_OUTPUT( "selected: %u\n", ptr_tree->selected);
    MW_CMD_OUTPUT( "sync: %u\n", ptr_tree->sync);
    MW_CMD_OUTPUT( "synced: %u\n", ptr_tree->synced);
    MW_CMD_OUTPUT( "tcProp: %u\n", ptr_tree->tcProp);
    MW_CMD_OUTPUT( "updtInfo: %u\n", ptr_tree->updtInfo);
    MW_CMD_OUTPUT( "msgFlagsTc: %u\n", ptr_tree->msgFlagsTc);
    MW_CMD_OUTPUT( "msgFlagsProposal: %u\n", ptr_tree->msgFlagsProposal);
    MW_CMD_OUTPUT( "msgFlagsPortRole: %u\n", ptr_tree->msgFlagsPortRole);
    MW_CMD_OUTPUT( "msgFlagsLearning: %u\n", ptr_tree->msgFlagsLearning);
    MW_CMD_OUTPUT( "msgFlagsForwarding: %u\n", ptr_tree->msgFlagsForwarding);
    MW_CMD_OUTPUT( "msgFlagsAgreement: %u\n", ptr_tree->msgFlagsAgreement);
    MW_CMD_OUTPUT( "msgFlagsTcAckOrMaster: %u\n", ptr_tree->msgFlagsTcAckOrMaster);
    MW_CMD_OUTPUT( "infoIs: %u\n", ptr_tree->infoIs);
    MW_CMD_OUTPUT( "rcvdInfo: %u\n", ptr_tree->rcvdInfo);
    MW_CMD_OUTPUT( "role: %u\n", ptr_tree->role);
    MW_CMD_OUTPUT( "selectedRole: %u\n", ptr_tree->selectedRole);
    MW_CMD_OUTPUT( "InternalPortPathCost: %u\n", ptr_tree->InternalPortPathCost);

    MW_CMD_OUTPUT( "fdWhile: %u\n", ptr_tree->fdWhile);
    MW_CMD_OUTPUT( "rrWhile: %u\n", ptr_tree->rrWhile);
    MW_CMD_OUTPUT( "rbWhile: %u\n", ptr_tree->rbWhile);
    MW_CMD_OUTPUT( "tcWhile: %u\n", ptr_tree->tcWhile);
    MW_CMD_OUTPUT( "rcvdInfoWhile: %u\n", ptr_tree->rcvdInfoWhile);
    MW_CMD_OUTPUT( "tcDetected: %u\n", ptr_tree->tcDetected);
    MW_CMD_OUTPUT( "adminInternalPortPathCost: %u\n", ptr_tree->adminInternalPortPathCost);
    MW_CMD_OUTPUT( "portInformationState: %u\n", ptr_tree->portInformationState);
    MW_CMD_OUTPUT( "portRoleTransitionsState: %u\n", ptr_tree->portRoleTransitionsState);
    MW_CMD_OUTPUT( "portStateTransitionState: %u\n", ptr_tree->portStateTransitionState);
    MW_CMD_OUTPUT( "topologyChangeState: %u\n", ptr_tree->topologyChangeState);
}

#endif
