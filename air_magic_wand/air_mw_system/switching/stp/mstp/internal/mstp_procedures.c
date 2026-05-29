// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.29 from 802.1Q-2018.
#include "mstp_base_types.h"
#include "mstp_port.h"
#include "mstp_bridge.h"
#include "mstp_conditions_and_params.h"
#include "mstp_log.h"
#include <assert.h>
#include <stddef.h>
#ifdef AIR_SUPPORT_MSTP
#include "ethernet.h"
#include "mw_utils.h"
#include "air_swc.h"
#include "switch.h"
#include "mw_error.h"
#include "stp.h"
#include "stp_db.h"
#endif
#include "mstp_procedures.h"

#ifdef AIR_SUPPORT_MSTP
static UI32_T      _port_mode = 0;
#endif

static inline size_t _getInstanceIdxByMsti(
    STP_BRIDGE* bridge,
    unsigned short msti)
{
    size_t i;

    for(i = 1; i < bridge->treeCount; i++)
    {
        if (NULL == bridge->trees[i])
        {
            continue;
        }

        if (bridge->trees[i]->instance == msti)
        {
            return i;
        }
    }

    return 0;
}

static bool IsBetterThan(
    PRIORITY_VECTOR *p1,
    PRIORITY_VECTOR *p2)
{
    PRIORITY_VECTOR vec1;
    PRIORITY_VECTOR vec2;

    memcpy(&vec1, p1, sizeof(PRIORITY_VECTOR));
    memcpy(&vec2, p2, sizeof(PRIORITY_VECTOR));
    vec1.DesignatedPortId = htons(vec1.DesignatedPortId);
    vec2.DesignatedPortId = htons(vec2.DesignatedPortId);
    vec1.ExternalRootPathCost = htonl(vec1.ExternalRootPathCost);
    vec2.ExternalRootPathCost = htonl(vec2.ExternalRootPathCost);
    vec1.InternalRootPathCost = htonl(vec1.InternalRootPathCost);
    vec2.InternalRootPathCost = htonl(vec2.InternalRootPathCost);

    return (memcmp(&vec1, &vec2, sizeof(PRIORITY_VECTOR)) < 0);
}

static bool _isBetterThanOrSameAs(
    PRIORITY_VECTOR *p1,
    PRIORITY_VECTOR *p2)
{
    PRIORITY_VECTOR vec1;
    PRIORITY_VECTOR vec2;

    memcpy(&vec1, p1, sizeof(PRIORITY_VECTOR));
    memcpy(&vec2, p2, sizeof(PRIORITY_VECTOR));
    vec1.DesignatedPortId = htons(vec1.DesignatedPortId);
    vec2.DesignatedPortId = htons(vec2.DesignatedPortId);
    vec1.ExternalRootPathCost = htonl(vec1.ExternalRootPathCost);
    vec2.ExternalRootPathCost = htonl(vec2.ExternalRootPathCost);
    vec1.InternalRootPathCost = htonl(vec1.InternalRootPathCost);
    vec2.InternalRootPathCost = htonl(vec2.InternalRootPathCost);
    return (memcmp(&vec1, &vec2, sizeof(PRIORITY_VECTOR)) <= 0);
}

static bool _isSuperiorTo(
    PRIORITY_VECTOR *p1,
    PRIORITY_VECTOR *p2)
{
    PRIORITY_VECTOR vec1;
    PRIORITY_VECTOR vec2;

    memcpy(&vec1, p1, sizeof(PRIORITY_VECTOR));
    memcpy(&vec2, p2, sizeof(PRIORITY_VECTOR));
    vec1.DesignatedPortId = htons(vec1.DesignatedPortId);
    vec2.DesignatedPortId = htons(vec2.DesignatedPortId);
    vec1.ExternalRootPathCost = htonl(vec1.ExternalRootPathCost);
    vec2.ExternalRootPathCost = htonl(vec2.ExternalRootPathCost);
    vec1.InternalRootPathCost = htonl(vec1.InternalRootPathCost);
    vec2.InternalRootPathCost = htonl(vec2.InternalRootPathCost);

    if (memcmp(&vec1, &vec2, sizeof(PRIORITY_VECTOR)) < 0)
    {
        return true;
    }

    if ((memcmp((unsigned char *)(p1->DesignatedBridgeId)+2, (unsigned char *)(p2->DesignatedBridgeId)+2, 6) == 0)
        && ((p1->DesignatedPortId & 0xfff) == (p2->DesignatedPortId & 0xfff)))
    {
        return true;
    }

    return false;
}

static bool _isWorseThanOrSameAs(
    PRIORITY_VECTOR *p1,
    PRIORITY_VECTOR *p2)
{
    PRIORITY_VECTOR vec1;
    PRIORITY_VECTOR vec2;

    memcpy(&vec1, p1, sizeof(PRIORITY_VECTOR));
    memcpy(&vec2, p2, sizeof(PRIORITY_VECTOR));
    vec1.DesignatedPortId = htons(vec1.DesignatedPortId);
    vec2.DesignatedPortId = htons(vec2.DesignatedPortId);
    vec1.ExternalRootPathCost = htonl(vec1.ExternalRootPathCost);
    vec2.ExternalRootPathCost = htonl(vec2.ExternalRootPathCost);
    vec1.InternalRootPathCost = htonl(vec1.InternalRootPathCost);
    vec2.InternalRootPathCost = htonl(vec2.InternalRootPathCost);

    return (memcmp(&vec1, &vec2, sizeof(PRIORITY_VECTOR)) >= 0);
}

bool IsNotBetterThan(
    PRIORITY_VECTOR *p1,
    PRIORITY_VECTOR *p2)
{
    return _isWorseThanOrSameAs(p1, p2);
}

static bool _isEqual(void *p1, void *p2, unsigned int len)
{
    return (memcmp(p1, p2, len) == 0);
}

#ifdef AIR_SUPPORT_MSTP
inline static void
_bpdu_head_fill(
    UI32_T      portId,
    struct pbuf *ptr_pbuf)
{
    AIR_MAC_T   mac;
    UI32_T      unit = 0;
    UI32_T      egress_port;
    UI32_T      l_byte = 0;
#ifdef AIR_SUPPORT_STP_UNIFIED_SA
    UI8_T       is_support_unified_sa = FALSE;
#endif /* AIR_SUPPORT_STP_UNIFIED_SA */
    const struct eth_addr pkt_da = IEEE_802_RESERVE_00;

    stp_db_get_bpdu_egress_port(portId, &egress_port);
    if (AIR_E_OK == air_swc_getSystemMac(unit, mac))
    {
        mw_pbuf_init(ptr_pbuf);
        memset(ptr_pbuf->payload, 0, ptr_pbuf->len);
        memcpy(ptr_pbuf->ether_hdr.dest.addr, pkt_da.addr, sizeof(struct eth_addr));

#ifdef AIR_SUPPORT_STP_UNIFIED_SA
        stp_get_unified_sa_support(&is_support_unified_sa);
        if (FALSE == is_support_unified_sa)
        {
            l_byte = mac[5] + egress_port;
            if (256 <= l_byte)
            {
                mac[4] += l_byte / 256;
            }
            mac[5] = (l_byte % 256);
        }
#else
        l_byte = mac[5] + egress_port;
        if (256 <= l_byte)
        {
            mac[4] += l_byte / 256;
        }
        mac[5] = (l_byte % 256);
#endif /* AIR_SUPPORT_STP_UNIFIED_SA */

        memcpy(ptr_pbuf->ether_hdr.src.addr, mac, 6);

        ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp = (1 << egress_port);
        ptr_pbuf->flags |= PBUF_FLAG_802_3;
        ptr_pbuf->llc_hdr.llc_dsap = STP_LLC_DSAP;
        ptr_pbuf->llc_hdr.llc_ssap = STP_LLC_SSAP;
        ptr_pbuf->llc_hdr.llc_ctrl = STP_LLC_CNTL;
        ptr_pbuf->len += SIZEOF_LLC_HDR;
    }
}
#endif

// ============================================================================
// 13.29.a - 13.29.1 in 802.1Q-2018
// Returns TRUE if, for a given port and tree (CIST, or MSTI), either
// a) The procedure's parameter newInfoIs is Received, and infoIs is Received and the msgPriority vector
//    is better than or the same as (13.10) the portPriority vector; or,
// b) The procedure's parameter newInfoIs is Mine, and infoIs is Mine and the designatedPriority vector is
//    better than or the same as (13.10) the portPriority vector.
// Returns False otherwise.
bool betterorsameInfo(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree, INFO_IS newInfoIs)
{
    PORT* port = bridge->ports[givenPort];
    PORT_TREE* tree = port->trees[givenTree];

    if (NULL == port)
    {
        return false;
    }
    tree = port->trees[givenTree];
    if (NULL == tree)
    {
        return false;
    }

    if ((newInfoIs == INFO_IS_RECEIVED) && (tree->infoIs == INFO_IS_RECEIVED)
            //&& (tree->msgPriority.IsBetterThanOrSameAs(tree->portPriority)))
            && (_isBetterThanOrSameAs(&tree->msgPriority, &tree->portPriority)))
    {
        return true;
    }

    if ((newInfoIs == INFO_IS_MINE) && (tree->infoIs == INFO_IS_MINE)
        //&& (tree->designatedPriority.IsBetterThanOrSameAs(tree->portPriority)))
        && (_isBetterThanOrSameAs(&tree->designatedPriority, &tree->portPriority)))
    {
        return true;
    }

    return false;
}

// ============================================================================
// 13.29.b) - 13.29.2 in 802.1Q-2018
// Clears rcvdMsg for the CIST and all MSTIs, for this port.
void clearAllRcvdMsgs(STP_BRIDGE* bridge, PortIndex givenPort)
{
    unsigned int treeIndex;

    if (NULL == bridge->ports[givenPort])
    {
        return;
    }

    for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
    {
        if (NULL == bridge->ports[givenPort]->trees[treeIndex])
        {
            continue;
        }
        bridge->ports[givenPort]->trees[treeIndex]->rcvdMsg = false;
    }
}

// ============================================================================
// 13.29.c) - 13.29.3 in 802.1Q-2018
// Clears reselect for the tree (the CIST or a given MSTI) for all ports of the bridge.
void clearReselectTree(STP_BRIDGE* bridge, TreeIndex givenTree)
{
    unsigned int portIndex;

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        if (NULL == bridge->ports[portIndex])
        {
            continue;
        }

        if (NULL == bridge->ports[portIndex]->trees[givenTree])
        {
            continue;
        }
        bridge->ports[portIndex]->trees[givenTree]->reselect = false;
    }
}

// ============================================================================
// 13.29.d) - 13.29.4 in 802.1Q-2018
// An implementation-dependent procedure that causes the Forwarding Process (8.6) to stop forwarding frames
// through the port. The procedure does not complete until forwarding has stopped.
void disableForwarding(const STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree, unsigned int timestamp)
{
    FLUSH_LOG(bridge);
#ifdef AIR_SUPPORT_MSTP
    LOG(bridge, givenPort+1, -1, "Port %d instance %d disableForwarding\n", givenPort, givenTree);
    mstp_port_instance_state_update(givenTree, givenPort+1, MSTP_DISCARDING);
#endif
}

// ============================================================================
// 13.29.e) - 13.29.5 in 802.1Q-2018
// An implementation-dependent procedure that causes the Learning Process (8.7) to stop learning from the
// source address of frames received on the port. The procedure does not complete until learning has stopped.
void disableLearning(const STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree, unsigned int timestamp)
{
    FLUSH_LOG(bridge);
#ifdef AIR_SUPPORT_MSTP
    LOG(bridge, givenPort+1, -1, "Port %d instance %d disableLearning\n", givenPort, givenTree);
    mstp_port_instance_state_update(givenTree, givenPort+1, MSTP_DISCARDING);
#endif
}

// ============================================================================
// 13.29.f) - 13.29.6 in 802.1Q-2018
// An implementation-dependent procedure that causes the Forwarding Process (8.6) to start forwarding
// frames through the port. The procedure does not complete until forwarding has been enabled.
void enableForwarding(const STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree, unsigned int timestamp)
{
    FLUSH_LOG(bridge);
#ifdef AIR_SUPPORT_MSTP
    LOG(bridge, givenPort+1, -1, "Port %d instance %d enableForwarding\n", givenPort, givenTree);
    mstp_port_instance_state_update(givenTree, givenPort+1, MSTP_FORWARDING);
#endif
}

// ============================================================================
// 13.29.g) - 13.29.7 in 802.1Q-2018
// An implementation-dependent procedure that causes the Learning Process (8.7) to start learning from frames
// received on the port. The procedure does not complete until learning has been enabled.
void enableLearning(const STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree, unsigned int timestamp)
{
    FLUSH_LOG(bridge);
#ifdef AIR_SUPPORT_MSTP
    LOG(bridge, givenPort+1, -1, "Port %d instance %d enableLearning\n", givenPort, givenTree);
    mstp_port_instance_state_update(givenTree, givenPort+1, MSTP_LEARNING);
#endif
}

// ============================================================================
// 13.29.h) - 13.29.8 in 802.1Q-2018
// Returns TRUE if rcvdRSTP is TRUE, and the received BPDU conveys a MCID that matches that held for
// the Bridge. Returns FALSE otherwise.
bool fromSameRegion(STP_BRIDGE* bridge, PortIndex givenPort)
{
    // The SPT part of this function is not yet implemented.
    if (bridge->ForceProtocolVersion > STP_VERSION_MULTIPLE_STP)
    {
        return false;
    }

    PORT* port = bridge->ports[givenPort];

    if (NULL == port)
    {
        return false;
    }

    if (bridge->receivedBpduContent == NULL)
    {
        return false;
    }

    // Note AG: I added the condition "&& ForceProtocolVersion >= MSTP"
    // (if we're running STP or RSTP, we shouldn't be looking at our MST Config ID.)

    bool result = port->rcvdRSTP
        && (bridge->receivedBpduType == VALIDATED_BPDU_TYPE_MST)
        && (bridge->ForceProtocolVersion >= STP_VERSION_MULTIPLE_STP)
        //&& (bridge->receivedBpduContent->mstConfigId == bridge->MstConfigId);
        && (_isEqual((void *)&bridge->receivedBpduContent->mstConfigId, (void *)&bridge->MstConfigId, sizeof(STP_MST_CONFIG_ID)));

    return result;
}

// ============================================================================
// 13.29.i - 13.29.9 in 802.1Q-2018
// If the value of tcDetected is zero and sendRSTP is TRUE, this procedure sets the value of tcDetected to
// HelloTime plus one second. The value of HelloTime is taken from the CIST's portTimes parameter (13.27.48)
// for this port.
//
// If the value of tcDetected is zero and sendRSTP is FALSE, this procedure sets the value of tcDetected to the
// sum of the Max Age and Forward Delay components of rootTimes.
//
// Otherwise, the procedure takes no action.
void newTcDetected(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;

    if (NULL == port)
    {
        return;
    }
    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }

    if ((portTree->tcDetected == 0) && port->sendRSTP)
    {
        portTree->tcDetected = port->trees[CIST_INDEX]->portTimes.HelloTime + 1;
    }

    if ((portTree->tcDetected == 0) && !port->sendRSTP)
    {
        if (NULL != bridge->trees[givenTree])
        {
            portTree->tcDetected = bridge->trees[givenTree]->rootTimes.MaxAge + bridge->trees[givenTree]->rootTimes.ForwardDelay;
        }
    }
}

// ============================================================================
// 13.29.j) - 13.29.10 in 802.1Q-2018
// If the value of tcWhile is zero and sendRSTP is TRUE, this procedure sets the value of tcWhile to HelloTime
// plus one second and sets either newInfo TRUE for the CIST or newInfoMsti TRUE for a given MSTI. The
// value of HelloTime is taken from the CIST's portTimes parameter (13.27.48) for this port.
//
// If the value of tcWhile is zero and sendRSTP is FALSE, this procedure sets the value of tcWhile to the sum
// of the Max Age and Forward Delay components of rootTimes and does not change the value of either
// newInfo or newInfoMsti.
//
// Otherwise, the procedure takes no action.
void newTcWhile(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree, unsigned int timestamp)
{
    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;
    unsigned int portIndex;
    bool allZero = true;

    if (NULL == port)
    {
        return;
    }
    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }

    if ((portTree->tcWhile == 0) && port->sendRSTP)
    {
        // Note AG: See in 802.1Q-2018:
        //  - 12.8.1.1.3, b) and c);
        //  - 12.8.1.2.3, c) and d).
        //if (bridge->callbacks.onTopologyChange)
        {
            for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
            {
                if (NULL == bridge->ports[portIndex])
                {
                    continue;
                }
                allZero &= (bridge->ports[portIndex]->trees[givenTree]->tcWhile == 0);
            }
            if (allZero)
            {
                //bridge->callbacks.onTopologyChange(bridge, (unsigned int)givenTree, timestamp);
                LOG(bridge, portIndex+1, givenTree, "STP: TC\r\n");
            }
        }

        portTree->tcWhile = 1 + port->trees [CIST_INDEX]->portTimes.HelloTime;

        if (givenTree == CIST_INDEX)
        {
            port->newInfo = true;
        }
        else
        {
            port->newInfoMsti = true;
        }
    }

    if ((portTree->tcWhile == 0) && !port->sendRSTP)
    {
        if (NULL != bridge->trees[givenTree])
        {
            portTree->tcWhile = bridge->trees[givenTree]->rootTimes.MaxAge + bridge->trees[givenTree]->rootTimes.ForwardDelay;
        }
    }
}

// ============================================================================
// 13.29.k) - 13.29.11 in 802.1Q-2018
// Using local parameters, this procedure simulates the processing that would be applied by rcvInfo() and
// rcvMsgs() to a BPDU received on the port, from the same region and with the following parameters:
//   a) Message Age, Max Age, Hello Time and Forward Delay are derived from BridgeTimes (13.26.4).
//   b) The CIST information carries the message priority vector (13.10) with a value of {pseudoRootId, 0,
//      pseudoRootId, 0, 0, 0}.
//   c) A CIST Port Role of Designated Port, with the Learning and Forwarding flags set.
//   d) The Version 1 Length is 0 and Version 3 Length calculated appropriately.
//   e) For each MSTI configured on the Bridge, the corresponding MSTI Configuration Message carries
//      the following:
//      1) A message priority vector with a value of {pseudoRootId, 0, 0, 0}
//      2) A Port Role of Designated Port, with the Learning and Forwarding flags set
//      3) MSTI Remaining Hops set to the value of the MaxHops component of BridgeTimes (13.26.4)
//
// NOTE-If two L2GP ports are configured with the same CIST pseudoRootId then the IST may partition within the MST
// Region, but either of the L2GP ports can be selected to provide connectivity from the Region/customer network to a
// provider's network on an MSTI by MSTI basis.
void pseudoRcvMsgs(STP_BRIDGE* bridge, PortIndex givenPort)
{
    // The L2GP state machine is not yet implemented.
    //assert(false);
}

// ============================================================================
// 13.29.l) - 13.29.12 in 802.1Q-2018
RCVD_INFO rcvInfo(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;
    unsigned int i;
    PORT* ptr_port;
    PORT_TREE* ptr_portTree;

    if (NULL == port)
    {
        return RCVD_INFO_OTHER;
    }

    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return RCVD_INFO_OTHER;
    }

    // Returns SuperiorDesignatedInfo if, for a given port and tree (CIST or MSTI),
    //  a) The received CIST or MSTI message conveys a Designated Port Role and
    //     1) The message priority (msgPriority-13.27.39) is superior (13.10 or 13.11) to the port's port
    //        priority vector; or
    //     2) The message priority is the same as the port's port priority vector, and any of the received timer
    //        parameter values (msgTimes-13.27.40) differ from those already held for the port
    //        (portTimes-13.27.48).
    if (portTree->msgFlagsPortRole == BPDU_PORT_ROLE_DESIGNATED)
    {
        //if (portTree->msgPriority.IsSuperiorTo (portTree->portPriority)
            //|| ((portTree->msgPriority == portTree->portPriority) && (portTree->msgTimes != portTree->portTimes)))
        if (_isSuperiorTo(&portTree->msgPriority, &portTree->portPriority)
            || (_isEqual(&portTree->msgPriority, &portTree->portPriority, sizeof(PRIORITY_VECTOR))
                && (!_isEqual(&portTree->msgTimes, &portTree->portTimes, sizeof(TIMES)))))
        {
//LOG (bridge, givenPort, givenTree, "-------------------------\r\n");
//LOG (bridge, givenPort, givenTree, "{S}: portTree->msgPriority.IsSuperiorTo (portTree->portPriority)\r\n", port->debugName);
//LOG (bridge, givenPort, givenTree, "{S}: portTree->msgPriority  = {PVS}\r\n", port->debugName, &portTree->msgPriority);
//LOG (bridge, givenPort, givenTree, "{S}: portTree->portPriority = {PVS}\r\n", port->debugName, &portTree->portPriority);
//LOG (bridge, givenPort, givenTree, "{S}: portTree->msgTimes  = {TMS}\r\n", port->debugName, &portTree->msgTimes);
//LOG (bridge, givenPort, givenTree, "{S}: portTree->portTimes = {TMS}\r\n", port->debugName, &portTree->portTimes);
//LOG (bridge, givenPort, givenTree, "-------------------------\r\n");
#ifdef AIR_SUPPORT_MSTP
            if ((memcmp((unsigned char *)(portTree->msgPriority.DesignatedBridgeId), (unsigned char *)(portTree->portPriority.DesignatedBridgeId), 2) > 0)
                && (memcmp((unsigned char *)(portTree->msgPriority.DesignatedBridgeId)+2, (unsigned char *)(portTree->portPriority.DesignatedBridgeId)+2, 6) == 0)
                && ((portTree->msgPriority.DesignatedPortId & 0xfff) == (portTree->portPriority.DesignatedPortId & 0xfff)))
            {
                for (i = 0; i < bridge->portCount; i++)
                {
                    if (givenPort == i)
                    {
                        continue;
                    }

                    ptr_port = bridge->ports[i];
                    if (NULL == ptr_port)
                    {
                        continue;
                    }

                    ptr_portTree = ptr_port->trees[givenTree];
                    if (NULL == portTree)
                    {
                        continue;
                    }

                    if (INFO_IS_RECEIVED == ptr_portTree->infoIs)
                    {
                        ptr_portTree->infoIs = INFO_IS_AGED;
                    }
                }
            }
#endif
            return RCVD_INFO_SUPERIOR_DESIGNATED;
        }
    }

    // Otherwise, returns RepeatedDesignatedInfo if, for a given port and tree (CIST or MSTI),
    // b) The received CIST or MSTI message conveys a Designated Port Role and
    //    1) A message priority vector and timer parameters that are the same as the port's port priority
    //       vector and timer values and
    //    2) infoIs is Received.
    if ((portTree->msgFlagsPortRole == BPDU_PORT_ROLE_DESIGNATED)
        //&& ((portTree->msgPriority == portTree->portPriority) && (portTree->msgTimes == portTree->portTimes))
        && (_isEqual(&portTree->msgPriority, &portTree->portPriority, sizeof(PRIORITY_VECTOR))
            && _isEqual(&portTree->msgTimes, &portTree->portTimes, sizeof(TIMES)))
        && (portTree->infoIs == INFO_IS_RECEIVED))
    {
        return RCVD_INFO_REPEATED_DESIGNATED;
    }

    // Otherwise, returns InferiorDesignatedInfo if, for a given port and tree (CIST or MSTI),
    // c) The received CIST or MSTI message conveys a Designated Port Role.
    if (portTree->msgFlagsPortRole == BPDU_PORT_ROLE_DESIGNATED)
    {
        return RCVD_INFO_INFERIOR_DESIGNATED;
    }

    // Otherwise, returns InferiorRootAlternateInfo if, for a given port and tree (CIST or MSTI),
    // d) The received CIST or MSTI message conveys a Root Port, Alternate Port, or Backup Port Role and
    //    a CIST or MSTI message priority that is the same as or worse than the CIST or MSTI port priority
    //    vector.
    if (((portTree->msgFlagsPortRole == BPDU_PORT_ROLE_ROOT) || (portTree->msgFlagsPortRole == BPDU_PORT_ROLE_ALT_BACKUP))
        //&& (portTree->msgPriority.IsWorseThanOrSameAs (portTree->portPriority)))
        && (_isWorseThanOrSameAs(&portTree->msgPriority, &portTree->portPriority)))
    {
        return RCVD_INFO_INFERIOR_ROOT_ALTERNATE;
    }

    // Otherwise, returns OtherInfo.
    return RCVD_INFO_OTHER;
}

// ============================================================================
// 13.29.m) - 13.29.13 in 802.1Q-2018
void rcvMsgs(STP_BRIDGE* bridge, PortIndex givenPort)
{
    PORT* port = bridge->ports[givenPort];
    unsigned int treeIndex;

    if (NULL == port)
    {
        return;
    }

    // This procedure is invoked by the Port Receive state machine (13.31) to decode a received BPDU. Sets
    // rcvdTcn and rcvdTc for each and every MSTI if a TCN BPDU has been received, and extracts the message
    // priority and timer values from the received BPDU storing them in the msgPriority and msgTimes variables.
    if (bridge->receivedBpduType == VALIDATED_BPDU_TYPE_STP_TCN)
    {
        port->rcvdTcn = true;

        for (treeIndex = 1; treeIndex < bridge->treeCount; treeIndex++)
        {
            if (NULL == port->trees[treeIndex])
            {
                continue;
            }
            port->trees[treeIndex]->rcvdTc = true;
        }
    }
    else if ((bridge->receivedBpduType == VALIDATED_BPDU_TYPE_STP_CONFIG)
        ||   (bridge->receivedBpduType == VALIDATED_BPDU_TYPE_RST)
        ||   (bridge->receivedBpduType == VALIDATED_BPDU_TYPE_MST)
        ||   (bridge->receivedBpduType == VALIDATED_BPDU_TYPE_SPT))
    {
        PORT_TREE* portCistTree = port->trees[CIST_INDEX];

        if (NULL == portCistTree)
        {
            return;
        }
        // priority
        // See 13.27.39 in 802.1Q-2018
        // See the definition of "message priority vector" in "13.10 CIST Priority Vector calculations" in 802.1Q-2018
        //portCistTree->msgPriority.RootId                = bridge->receivedBpduContent->cistRootId;
        memcpy(portCistTree->msgPriority.RootId, bridge->receivedBpduContent->cistRootId, sizeof(BRIDGE_ID));
        portCistTree->msgPriority.ExternalRootPathCost  = ntohl(bridge->receivedBpduContent->cistExternalPathCost);
        //portCistTree->msgPriority.RegionalRootId        = bridge->receivedBpduContent->cistRegionalRootId;
        memcpy(portCistTree->msgPriority.RegionalRootId, bridge->receivedBpduContent->cistRegionalRootId, sizeof(BRIDGE_ID));
        if (port->rcvdInternal)
        {
            portCistTree->msgPriority.InternalRootPathCost = ntohl(bridge->receivedBpduContent->cistInternalRootPathCost);
            //portCistTree->msgPriority.DesignatedBridgeId   = bridge->receivedBpduContent->cistBridgeId;
            memcpy(portCistTree->msgPriority.DesignatedBridgeId, bridge->receivedBpduContent->cistBridgeId, sizeof(BRIDGE_ID));
        }
        else
        {
            // From page 486 of 802.1Q-2018:
            // NOTE 1-If a Configuration Message is received in an RST or STP BPDU, both the Regional Root Identifier and the
            // Designated Bridge Identifier are decoded from the single BPDU field used for the Designated Bridge Parameter (the
            // MST BPDU field in this position encodes the CIST Regional Root Identifier). An STP or RST Bridge is always treated
            // by MSTP as being in an region of its own, so the Internal Root Path Cost is decoded as zero.
            portCistTree->msgPriority.InternalRootPathCost = 0;
            //portCistTree->msgPriority.DesignatedBridgeId = bridge->receivedBpduContent->cistRegionalRootId;
            memcpy(portCistTree->msgPriority.DesignatedBridgeId, bridge->receivedBpduContent->cistRegionalRootId, sizeof(BRIDGE_ID));
        }
        portCistTree->msgPriority.DesignatedPortId       = ntohs(bridge->receivedBpduContent->cistPortId);

        // times
        // See 13.27.40 in 802.1Q-2018
        portCistTree->msgTimes.ForwardDelay = ntohs(bridge->receivedBpduContent->ForwardDelay) / 256;
        portCistTree->msgTimes.HelloTime    = ntohs(bridge->receivedBpduContent->HelloTime) / 256;
        portCistTree->msgTimes.MaxAge       = ntohs(bridge->receivedBpduContent->MaxAge) / 256;
        portCistTree->msgTimes.MessageAge   = ntohs(bridge->receivedBpduContent->MessageAge) / 256;
        // Note AG: Standard says: "If the BPDU is an STP or RST BPDU without MSTP parameters,
        // remainingHops is set to the value of the MaxHops component of BridgeTimes (13.26.4)"
        // I'm pretty sure that also BPDUs coming from a different MST region should be treated the same.
        // A false value in rcvdInternal covers all cases; it is also similar to the condition above for
        // setting the message priority, so likely correct.
        if (port->rcvdInternal)
        {
            portCistTree->msgTimes.remainingHops = bridge->receivedBpduContent->cistRemainingHops;
        }
        else
        {
            portCistTree->msgTimes.remainingHops = bridge->trees[CIST_INDEX]->BridgeTimes.remainingHops;
        }

        // flags
        if (bridge->receivedBpduType == VALIDATED_BPDU_TYPE_STP_CONFIG)
        {
            portCistTree->msgFlagsTc            = GetBpduFlagTc    (bridge->receivedBpduContent->cistFlags);
            portCistTree->msgFlagsTcAckOrMaster = GetBpduFlagTcAck (bridge->receivedBpduContent->cistFlags);

            // From the note at the end of 13.29.12 in 802.1Q-2018:
            // A Configuration BPDU implicitly conveys a Designated Port Role.
            portCistTree->msgFlagsPortRole      = BPDU_PORT_ROLE_DESIGNATED;

            // flags below are not present in a Config BPDU, but let's clear them nevertheless.
            portCistTree->msgFlagsProposal      = false;
            portCistTree->msgFlagsLearning      = false;
            portCistTree->msgFlagsForwarding    = false;
            portCistTree->msgFlagsAgreement     = false;
        }
        else
        {
            portCistTree->msgFlagsTc            = GetBpduFlagTc         (bridge->receivedBpduContent->cistFlags);
            portCistTree->msgFlagsProposal      = GetBpduFlagProposal   (bridge->receivedBpduContent->cistFlags);
            portCistTree->msgFlagsPortRole      = GetBpduFlagPortRole   (bridge->receivedBpduContent->cistFlags);
            portCistTree->msgFlagsLearning      = GetBpduFlagLearning   (bridge->receivedBpduContent->cistFlags);
            portCistTree->msgFlagsForwarding    = GetBpduFlagForwarding (bridge->receivedBpduContent->cistFlags);
            portCistTree->msgFlagsAgreement     = GetBpduFlagAgreement  (bridge->receivedBpduContent->cistFlags);
            portCistTree->msgFlagsTcAckOrMaster = false; // TcAck is found only in STP Config BPDUs, Master only in MSTIs; we are in neither case here.
        }
    }
    else
    {
        return;
    }

    // If ISIS-SPB is implemented, ForceProtocolVersion is 4 (or greater), the BPDU is an SPT BPDU, and has been
    // received on a Bridge Port that is internal to the SPT Region (i.e., is not a Boundary Port, see 13.12), then the
    // rcvAgreements() procedure processes the CIST and SPT information conveyed by the BPDU.
    if ((int)bridge->ForceProtocolVersion >= 4)
    {
        return; // not yet implemented for SPT
    }
    else
    {
        // Otherwise (i.e., if rcvAgreements() is not used), this procedure sets rcvdMsg for the CIST and makes the
        // received CST or CIST message available to the CIST Port Information state machines.
        if (NULL != bridge->ports[givenPort]->trees[CIST_INDEX])
        {
            bridge->ports[givenPort]->trees[CIST_INDEX]->rcvdMsg = true;
        }
    }

    // If and only if rcvdInternal is set, this procedure sets rcvdMsg for each and every MSTI for which a MSTI
    // message is conveyed in the BPDU, and makes available each MSTI message and the common parts of the
    // CIST message priority (the CIST Root Identifier, External Root Path Cost, and Regional Root Identifier) to
    // the Port Information state machine for that MSTI.
    if (port->rcvdInternal)
    {
        LOG(bridge, givenPort+1, -1, "rcvMsgs() -- rcvdInternal==1\r\n");

        // these assert conditions should have been checked while validating the received bpdu
        unsigned short version3Length = bridge->receivedBpduContent->Version3Length;
#ifdef AIR_SUPPORT_MSTP
        version3Length = ntohs(version3Length);
#endif
        size_t version3Offset = offsetof(MSTP_BPDU, mstConfigId);
        size_t version3CistLength = sizeof(MSTP_BPDU) - version3Offset;
        size_t mstiLength = version3Length - version3CistLength;
        // this should have been checked while validating the BPDU as MST
        if ((mstiLength % sizeof(MSTI_CONFIG_MESSAGE)) != 0)
        {
            return;
        }

        size_t mstiMessageCount = mstiLength / sizeof(MSTI_CONFIG_MESSAGE);
        size_t messageIndex;

        const MSTI_CONFIG_MESSAGE* mstiMessages = (MSTI_CONFIG_MESSAGE *)(bridge->receivedBpduContent + 1);

        if (mstiMessageCount > (bridge->treeCount-1))
        {
            // The sender sent us too many MSTI messages. Let's ignore the ones we can't handle.
            LOG(bridge, givenPort+1, -1, "rcvMsgs() -- Ignoring MSTI messages {%d}..{%d}\r\n", (int)bridge->treeCount, (int)mstiMessageCount - 1);
            mstiMessageCount = (bridge->treeCount-1);
        }

        for (messageIndex = 0; messageIndex < mstiMessageCount; messageIndex++)
        {
            const MSTI_CONFIG_MESSAGE* message = &mstiMessages[messageIndex];
            size_t mstid;
            unsigned short instance;
            unsigned short priority;
            memcpy(&instance, message->RegionalRootId, sizeof(unsigned short));
            instance = ntohs(instance);
            instance &= 0xfff;

            mstid =  _getInstanceIdxByMsti(bridge, instance);
            if (0 == mstid)
            {
                continue;
            }

            PORT_TREE* portTree = port->trees[mstid];
            if (NULL == portTree)
            {
                continue;
            }

            // See 13.11 in 802.1Q-2018, definition of "message priority vector".
            // First two components are always zero for MSTIs; the library never sets them.
            // portTree->msgPriority.RootId
            // portTree->msgPriority.ExternalRootPathCost
            
            //portTree->msgPriority.RegionalRootId        = message->RegionalRootId;
            memcpy(portTree->msgPriority.RegionalRootId, message->RegionalRootId, sizeof(BRIDGE_ID));
            portTree->msgPriority.InternalRootPathCost  = ntohl(message->InternalRootPathCost);
            //portTree->msgPriority.DesignatedBridgeId.SetPriorityAndMstid (message->BridgePriority << 8, (unsigned short)mstid); // 14.2.5 in 802.1Q-2018
            priority = (message->BridgePriority << 8) | (unsigned short)instance;
            priority = ntohs(priority);
            memcpy(portTree->msgPriority.DesignatedBridgeId, &priority, sizeof(unsigned short)); // 14.2.5 in 802.1Q-2018

            //portTree->msgPriority.DesignatedBridgeId.SetAddress (bridge->receivedBpduContent->cistBridgeId.GetAddress().bytes);
            memcpy(((unsigned char *)portTree->msgPriority.DesignatedBridgeId)+2, ((unsigned char *)bridge->receivedBpduContent->cistBridgeId)+2, 6);

            //portTree->msgPriority.DesignatedPortId.Set (message->PortPriority & 0xF0, bridge->receivedBpduContent->cistPortId.GetPortNumber());
            portTree->msgPriority.DesignatedPortId = (message->PortPriority << 8) | (ntohs(bridge->receivedBpduContent->cistPortId) & 0xff);
            portTree->msgTimes.remainingHops = message->RemainingHops;

            portTree->msgFlagsTc            = GetBpduFlagTc         (message->flags);
            portTree->msgFlagsProposal      = GetBpduFlagProposal   (message->flags);
            portTree->msgFlagsPortRole      = GetBpduFlagPortRole   (message->flags);
            portTree->msgFlagsLearning      = GetBpduFlagLearning   (message->flags);
            portTree->msgFlagsForwarding    = GetBpduFlagForwarding (message->flags);
            portTree->msgFlagsAgreement     = GetBpduFlagAgreement  (message->flags);
            portTree->msgFlagsTcAckOrMaster = GetBpduFlagMaster     (message->flags);

            portTree->rcvdMsg = true;
        }

        // Note AG: what are we supposed to do when the sender sends us _fewer_ MSTI messages than we have trees?
        // Various state machine procedures check the message flags of all trees, and in this case there are no such flags for some of our trees.
    }
    else
    {
        // From 13.11 in 802.1Q-2018: An MSTI message priority vector received from a Bridge not in the same MST Region is discarded.
        LOG(bridge, givenPort+1, -1, "rcvMsgs() -- rcvdInternal==0\r\n");
    }
}


// ============================================================================
// 13.29.14 in 802.1Q-2018
void rcvAgreements(STP_BRIDGE* bridge, PortIndex portIndex)
{
    //assert(false); // SPB not yet implemented
}

// ============================================================================
// 13.29.n) - 13.29.15 in 802.1Q-2018
void recordAgreement(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    // We're accessing msgFlags below, which is valid only when a received BPDU is being handled.
    if (bridge->receivedBpduContent == NULL)
    {
        return;
    }

    PORT* port = bridge->ports[givenPort];
    PORT_TREE* cistPortTree;
    PORT_TREE* portTree;
    unsigned int treeIndex;

    if (NULL == port)
    {
        return;
    }

    cistPortTree = port->trees[CIST_INDEX];
    portTree = port->trees[givenTree];
    if ((NULL == portTree) || (NULL == cistPortTree))
    {
        return;
    }

    if (givenTree == CIST_INDEX)
    {
        // For the CIST and a given port, if rstpVersion is TRUE, operPointToPointMAC (IEEE Std 802.1AC) is
        // TRUE, and the received CIST Message has the Agreement flag set, then the CIST agreed flag is set and the
        // CIST proposing flag is cleared. Otherwise, the CIST agreed flag is cleared. Additionally, if the CIST
        // message was received from a Bridge in a different MST Region, i.e., the rcvdInternal flag is clear, the agreed
        // and proposing flags for this port for all MSTIs are set or cleared to the same value as the CIST agreed and
        // proposing flags. If the CIST message was received from a Bridge in the same MST Region, the MSTI
        // agreed and proposing flags are not changed.

        if (rstpVersion(bridge) && port->operPointToPointMAC && portTree->msgFlagsAgreement)
        {
            portTree->agreed = true;
            portTree->proposing = false;
        }
        else
        {
            portTree->agreed = false;
        }

        if (port->rcvdInternal == false)
        {
            for (treeIndex = 1; treeIndex < bridge->treeCount; treeIndex++)
            {
                if (NULL == port->trees[treeIndex])
                {
                    continue;
                }
                port->trees[treeIndex]->agreed    = cistPortTree->agreed;
                port->trees[treeIndex]->proposing = cistPortTree->proposing;
            }
        }
    }
    else
    {
        // For a given MSTI and port, if operPointToPointMAC (IEEE Std 802.1AC) is TRUE, and
        //
        // a) The message priority vector of the CIST Message accompanying the received MSTI Message (i.e.,
        //    received in the same BPDU) has the same CIST Root Identifier, CIST External Root Path Cost, and
        //    Regional Root Identifier as the CIST port priority vector, and
        // b) The received MSTI Message has the Agreement flag set,
        //
        // the MSTI agreed flag is set and the MSTI proposing flag is cleared. Otherwise the MSTI agreed flag is
        // cleared.
        //
        // NOTE-MSTI Messages received from bridges external to the MST Region are discarded and not processed by
        // recordAgreeement() or recordProposal().

        if (!port->rcvdInternal)
        {
            return;
        }

        portTree->agreed = false;
        if (NULL == cistPortTree)
        {
            return;
        }

        if (port->operPointToPointMAC
            //&& (cistPortTree->msgPriority.RootId               == cistPortTree->portPriority.RootId)
            && (_isEqual(cistPortTree->msgPriority.RootId, cistPortTree->portPriority.RootId, sizeof(BRIDGE_ID)))
            && (cistPortTree->msgPriority.ExternalRootPathCost == cistPortTree->portPriority.ExternalRootPathCost)
            //&& (cistPortTree->msgPriority.RegionalRootId       == cistPortTree->portPriority.RegionalRootId)
            && (_isEqual(cistPortTree->msgPriority.RegionalRootId, cistPortTree->portPriority.RegionalRootId, sizeof(BRIDGE_ID)))
            && (portTree->msgFlagsAgreement))
        {
            portTree->agreed = true;
            portTree->proposing = false;
        }
        else
        {
            portTree->agreed = false;
        }
    }
}

// ============================================================================
// 13.29.o) - 13.29.16 in 802.1Q-2018
// For the CIST and a given port, if the CIST message has the learning flag set:
// a) The disputed variable is set; and
// b) The agreed variable is cleared.
//
// Additionally, if the CIST message was received from a bridge in a different MST region (i.e., if the
// rcvdInternal flag is clear), then for all the MSTIs:
// c) The disputed variable is set; and
// d) The agreed variable is cleared.
//
// For a given MSTI and port, if the received MSTI message has the learning flag set:
// e) The disputed variable is set; and
// f) The agreed variable is cleared.
void recordDispute(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    // we're accessing msgFlags below, which is valid only when a received BPDU is being handled
    if (bridge->receivedBpduContent == NULL)
    {
        return;
    }

    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;
    unsigned int treeIndex;

    if (NULL == port)
    {
        return;
    }
    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }

    // Note AG: Not clear: The condition for c/d is a sub-condition of the condition for a/b?
    // Or the two conditions are independent? Let's consider it a sub-condition in the code below;
    // the wording for independent conditions would probably have been simpler.

    if (givenTree == CIST_INDEX)
    {
        if (portTree->msgFlagsLearning)
        {
            portTree->disputed = true;
            portTree->agreed = false;

            if (!port->rcvdInternal)
            {
                for (treeIndex = 1; treeIndex < bridge->treeCount; treeIndex++)
                {
                    if (NULL == port->trees[treeIndex])
                    {
                        continue;
                    }
                    port->trees[treeIndex]->disputed = true;
                    port->trees[treeIndex]->agreed = false;
                }
            }
        }
    }
    else
    {
        if (portTree->msgFlagsLearning)
        {
            portTree->disputed = true;
            portTree->agreed = false;
        }
    }
}

// ============================================================================
// 13.29.p) - 13.29.17 in 802.1Q-2018
// For the CIST and a given port, if the CIST message was received from a bridge in a different MST Region,
// i.e. the rcvdInternal flag is clear, the mastered variable for this port is cleared for all MSTIs.
//
// For a given MSTI and port, if the MSTI message was received on a point-to-point link and the MSTI
// Message has the Master flag set, set the mastered variable for this MSTI. Otherwise reset the mastered
// variable.
void recordMastered(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    // we're accessing msgFlags below, which is valid only when a received BPDU is being handled
    if (bridge->receivedBpduContent == NULL)
    {
        return;
    }

    PORT* port = bridge->ports[givenPort];
    unsigned int treeIndex;

    if (NULL == port)
    {
        return;
    }

    if (givenTree == CIST_INDEX)
    {
        if (port->rcvdInternal == false)
        {
            for (treeIndex = 1; treeIndex < bridge->treeCount; treeIndex++)
            {
                port->mastered = false;
            }
        }
    }
    else
    {
        port->mastered = false;
        if (NULL != port->trees[givenTree])
        {
            if (bridge->receivedBpduPort->operPointToPointMAC && port->trees[givenTree]->msgFlagsTcAckOrMaster)
            {
                port->mastered = true;
            }
            else
            {
                port->mastered = false;
            }
        }
    }

    LOG(bridge, givenPort+1, givenTree, "Port {%d}: Tree {%d}: recordMastered(): {%d}\r\n", givenPort+1, givenTree, (int) port->mastered);
}

// ============================================================================
// 13.29.q) - 13.29.18 in 802.1Q-2018
// Sets the components of the portPriority variable to the values of the corresponding msgPriority components.
void recordPriority(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    // we're accessing msgPriority below, which is valid only when a received BPDU is being handled
    if (bridge->receivedBpduContent == NULL)
    {
        return;
    }

    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;

    if (NULL == port)
    {
        return;
    }
    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }
    memcpy(&portTree->portPriority, &portTree->msgPriority, sizeof(PRIORITY_VECTOR));

    //portTree->portPriority = portTree->msgPriority;

    LOG(bridge, givenPort+1, givenTree, "Port {%d}: Tree {%d}: recordPriority():"
        "{RootID:"BRIDGE_STR", Ecost %d, RegionId:"BRIDGE_STR", Icost %d, DesId"BRIDGE_STR", DesID: 0x%x}\r\n",
        givenPort+1, givenTree, BRIDGE_VALUE(portTree->portPriority.RootId), portTree->portPriority.ExternalRootPathCost,
        BRIDGE_VALUE(portTree->portPriority.RegionalRootId), portTree->portPriority.InternalRootPathCost,
        BRIDGE_VALUE(portTree->portPriority.DesignatedBridgeId), portTree->portPriority.DesignatedPortId);
}

// ============================================================================
// 13.29.r) - 13.27.19 in 802.1Q-2018
// For the CIST and a given port, if the received CIST Message conveys a Designated Port Role, and has the
// Proposal flag set, the CIST proposed flag is set. Otherwise the CIST proposed flag is not changed.
// Additionally, if the CIST Message was received from a bridge in a different MST Region, i.e., the
// rcvdInternal flag is clear, the proposed flags for this port for all MSTIs are set or cleared to the same value as
// the CIST proposed flag. If the CIST message was received from a bridge in the same MST Region, the
// MSTI proposed flags are not changed.
//
// For a given MSTI and port, if the received MSTI Message conveys a Designated Port Role, and has the
// Proposal flag set, the MSTI proposed flag is set. Otherwise the MSTI proposed flag is not changed.
void recordProposal(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    // we're accessing msgFlags below, which is valid only when a received BPDU is being handled
    if (bridge->receivedBpduContent == NULL)
    {
        return;
    }

    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;
    unsigned int mstiIndex;

    if (NULL == port)
    {
        return;
    }
    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }

    if (givenTree == CIST_INDEX)
    {
        if ((portTree->msgFlagsPortRole == BPDU_PORT_ROLE_DESIGNATED) && portTree->msgFlagsProposal)
        {
            portTree->proposed = true;

            if (!port->rcvdInternal)
            {
                for (mstiIndex = 1; mstiIndex < bridge->treeCount; mstiIndex++)
                {
                    if ((NULL != port->trees[mstiIndex] && (NULL != port->trees[CIST_INDEX])))
                    {
                        port->trees[mstiIndex]->proposed = port->trees[CIST_INDEX]->proposed;
                    }
                }
            }
        }
    }
    else
    {
        if ((portTree->msgFlagsPortRole == BPDU_PORT_ROLE_DESIGNATED) && portTree->msgFlagsProposal)
        {
            portTree->proposed = true;
        }
    }
}

// ============================================================================
// 13.29.s) - 13.29.20 in 802.1Q-2018
// For the CIST and a given port, sets portTimes' Message Age, Max Age, Forward Delay, and remainingHops
// to the received values held in msgTimes and portTimes' Hello Time to the default specified in Table 13-5.
//
// For a given MSTI and port, sets portTime's remainingHops to the received value held in msgTimes.
void recordTimes(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    // we're accessing msgTimes below, which is valid only when a received BPDU is being handled
    if (bridge->receivedBpduContent == NULL)
    {
        return;
    }

    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;

    if (NULL == port)
    {
        return;
    }
    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }

    if (givenTree == CIST_INDEX)
    {
        portTree->portTimes.MessageAge    = portTree->msgTimes.MessageAge;
        portTree->portTimes.MaxAge        = portTree->msgTimes.MaxAge;
        portTree->portTimes.ForwardDelay  = portTree->msgTimes.ForwardDelay;
        portTree->portTimes.remainingHops = portTree->msgTimes.remainingHops;
        portTree->portTimes.HelloTime     = 2;
    }
    else
    {
        portTree->portTimes.remainingHops = portTree->msgTimes.remainingHops;
    }
}

// ============================================================================
// 13.29.t) - 13.29.21 in 802.1Q-2018
// Sets reRoot TRUE for this tree (the CIST or a given MSTI) for all ports of the bridge.
void setReRootTree(STP_BRIDGE* bridge, TreeIndex givenTree)
{
    unsigned int portIndex;

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        if (NULL == bridge->ports[portIndex])
        {
            continue;
        }
        if (NULL == bridge->ports[portIndex]->trees[givenTree])
        {
            continue;
        }
#ifdef AIR_SUPPORT_MSTP
        if (!bridge->ports[portIndex]->portEnabled)
        {
            continue;
        }
#endif
        bridge->ports[portIndex]->trees[givenTree]->reRoot = true;
    }
}

// ============================================================================
// 13.29.u) - 13.29.22 in 802.1Q-2018
// Sets selected TRUE for this tree (the CIST or a given MSTI) for all ports of the bridge if reselect is FALSE
// for all ports in this tree. If reselect is TRUE for any port in this tree, this procedure takes no action.
void setSelectedTree(STP_BRIDGE* bridge, TreeIndex givenTree)
{
    unsigned int portIndex;

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        if (NULL == bridge->ports[portIndex])
        {
            continue;
        }
        if (NULL == bridge->ports[portIndex]->trees[givenTree])
        {
            continue;
        }
#ifdef AIR_SUPPORT_MSTP
        if (!bridge->ports[portIndex]->portEnabled)
        {
            continue;
        }
#endif
        if (bridge->ports[portIndex]->trees[givenTree]->reselect)
        {
            return;
        }
    }

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        if (NULL == bridge->ports[portIndex])
        {
            continue;
        }
        if (NULL == bridge->ports[portIndex]->trees[givenTree])
        {
            continue;
        }
#ifdef AIR_SUPPORT_MSTP
        if (!bridge->ports[portIndex]->portEnabled)
        {
            continue;
        }
#endif
        bridge->ports[portIndex]->trees[givenTree]->selected = true;
    }
}

// ============================================================================
// 13.29.v) - 13.29.23 in 802.1Q-2018
// Sets sync TRUE for this tree (the CIST or a given MSTI) for all ports of the bridge.
void setSyncTree(STP_BRIDGE* bridge, TreeIndex givenTree)
{
    unsigned int portIndex;

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        if (NULL == bridge->ports[portIndex])
        {
            continue;
        }
        if (NULL == bridge->ports[portIndex]->trees[givenTree])
        {
            continue;
        }
#ifdef AIR_SUPPORT_MSTP
        if (!bridge->ports[portIndex]->portEnabled)
        {
            continue;
        }
#endif
        bridge->ports[portIndex]->trees[givenTree]->sync = true;
    }
}

// ============================================================================
// 13.29.w) - 13.29.24 in 802.1Q-2018
// For the CIST and a given port:
// a) If the Topology Change Acknowledgment flag is set for the CIST in the received BPDU, sets
//    rcvdTcAck TRUE.
// b) If rcvdInternal is clear and the Topology Change flag is set for the CIST in the received BPDU, sets
//    rcvdTc TRUE for the CIST and for each and every MSTI.
// c) If rcvdInternal is set, sets rcvdTc for the CIST if the Topology Change flag is set for the CIST in the
//    received BPDU.
//
// For a given MSTI and port, sets rcvdTc for this MSTI if the Topology Change flag is set in the corresponding
// MSTI message.
void setTcFlags(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    // we're accessing msgFlags below, which is valid only when a received BPDU is being handled
    if (bridge->receivedBpduContent == NULL)
    {
        return;
    }

    PORT* port = bridge->ports[givenPort];
    unsigned int treeIndex;

    if (NULL == port)
    {
        return;
    }

    if (givenTree == CIST_INDEX)
    {
        PORT_TREE* cistTree = port->trees[CIST_INDEX];

        if ((NULL != cistTree) && (cistTree->msgFlagsTcAckOrMaster))
        {
            port->rcvdTcAck = true;
        }

        if ((port->rcvdInternal == false) && (NULL != cistTree) && (cistTree->msgFlagsTc))
        {
            for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
            {
                if (NULL == port->trees[treeIndex])
                {
                    continue;
                }
                port->trees[treeIndex]->rcvdTc = true;
            }
        }

        if (port->rcvdInternal)
        {
            if (cistTree->msgFlagsTc)
            {
                cistTree->rcvdTc = true;
            }
        }
    }
    else
    {
        PORT_TREE* portTree = port->trees[givenTree];

        if ((NULL != portTree) && (portTree->msgFlagsTc))
        {
            portTree->rcvdTc = true;
        }
    }
}

// ============================================================================
// 13.29.x) - 13.29.25 in 802.1Q-2018
// If and only if restrictedTcn is FALSE for the port that invoked the procedure, sets tcProp TRUE for the given
// tree (the CIST or a given MSTI) for all other ports.
void setTcPropTree(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    unsigned int portIndex;

    if (NULL == bridge->ports[givenPort])
    {
        return;
    }

    if (bridge->ports[givenPort]->restrictedTcn == false)
    {
        for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
        {
            if (NULL == bridge->ports[portIndex])
            {
                continue;
            }
            if (NULL == bridge->ports[portIndex]->trees[givenTree])
            {
                continue;
            }
#ifdef AIR_SUPPORT_MSTP
            if (!bridge->ports[portIndex]->portEnabled)
            {
                continue;
            }
#endif
            if (portIndex != (unsigned int)givenPort)
            {
                bridge->ports[portIndex]->trees[givenTree]->tcProp = true;
            }
        }
    }
}

// ============================================================================
// 13.29.y) - 13.29.26 in 802.1Q-2018
// For all MSTIs, for each port that has infoInternal set:
// a) Clears the agree, agreed, and synced variables; and
// b) Sets the sync variable.
void syncMaster(STP_BRIDGE* bridge)
{
    unsigned int portIndex;
    unsigned int treeIndex;

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        PORT* port = bridge->ports[portIndex];
        if (NULL == port)
        {
            continue;
        }

        if (port->infoInternal)
        {
            for (treeIndex = 1; treeIndex < bridge->treeCount; treeIndex++)
            {
                PORT_TREE* portTree = port->trees[treeIndex];
                if (NULL == portTree)
                {
                    continue;
                }
                portTree->agree = false;
                portTree->agreed = false;
                portTree->synced = false;
                portTree->sync = true;
            }
        }
    }
}

// ============================================================================
// 13.29.z) - 13.29.27 in 802.1Q-2018
// Transmits a Configuration BPDU. The first four components of the message priority vector (13.27.39)
// conveyed in the BPDU are set to the value of the CIST Root Identifier, External Root Path Cost, Bridge
// Identifier, and Port Identifier components of the CIST's designatedPriority parameter (13.27.20) for this
// port. The topology change flag is set if (tcWhile != 0) for the port. The topology change acknowledgment
// flag is set to the value of tcAck for the port. The remaining flags are set to zero. The value of the Message
// Age, Max Age, and Fwd Delay parameters conveyed in the BPDU are set to the values held in the CIST's
// designatedTimes parameter (13.27.21) for the port. The value of the Hello Time parameter conveyed in the
// BPDU is set to the value held in the CIST's portTimes parameter (13.27.48) for the port.
void txConfig(STP_BRIDGE* bridge, PortIndex givenPort, unsigned int timestamp)
{
    PORT* port = bridge->ports[givenPort-1];
    PORT_TREE* cistTree;
    MSTP_BPDU* bpdu = NULL;
    unsigned int bpduSize = (unsigned int)offsetof(MSTP_BPDU, Version1Length);

    if (NULL == port)
    {
        return;
    }
    cistTree = port->trees[CIST_INDEX];
    if (NULL == cistTree)
    {
        return;
    }

#ifdef AIR_SUPPORT_MSTP
    struct pbuf  *ptr_pbuf = NULL;
    ptr_pbuf = pbuf_alloc(PBUF_TRANSPORT, bpduSize, PBUF_RAM);
    if (NULL == ptr_pbuf)
    {
        return;
    }
    _bpdu_head_fill(givenPort, ptr_pbuf);
    bpdu = (MSTP_BPDU *)(ptr_pbuf->payload);
#endif

    //MSTP_BPDU* bpdu = (MSTP_BPDU*) bridge->callbacks.transmitGetBuffer (bridge, givenPort, bpduSize, timestamp);
    if (bpdu != NULL)
    {
        // 14.3.a) in 802.1Q-2018
        bpdu->head.protocolId = 0;
        bpdu->head.protocolVersionId = 0;
        bpdu->head.bpduType = 0;

        // 14.4 in 802.1Q-2018
        //bpdu->cistRootId           = cistTree->designatedPriority.RootId;               // h)
        memcpy(bpdu->cistRootId, cistTree->designatedPriority.RootId, sizeof(BRIDGE_ID));
        bpdu->cistExternalPathCost = htonl(cistTree->designatedPriority.ExternalRootPathCost);   // i)
        //bpdu->cistRegionalRootId   = cistTree->designatedPriority.DesignatedBridgeId;   // j)
        memcpy(bpdu->cistRegionalRootId, cistTree->designatedPriority.DesignatedBridgeId, sizeof(BRIDGE_ID));
        bpdu->cistPortId      = htons(cistTree->designatedPriority.DesignatedPortId);     // k)

        bpdu->cistFlags = 0;

        if (cistTree->tcWhile != 0)
        {
            bpdu->cistFlags |= (unsigned char) 1;
        }

        if (port->tcAck)
        {
            bpdu->cistFlags |= (unsigned char) 0x80;
        }

        bpdu->MessageAge   = htons(cistTree->designatedTimes.MessageAge * 256);
        bpdu->MaxAge       = htons(cistTree->designatedTimes.MaxAge * 256);
        bpdu->ForwardDelay = htons(cistTree->designatedTimes.ForwardDelay * 256);
        bpdu->HelloTime    = htons(cistTree->portTimes.HelloTime * 256);

        LOG (bridge, givenPort+1, -1, "TX Config BPDU to port {%d}:\r\n", givenPort+1);
#if STP_USE_LOG
        LOG_INDENT (bridge);
        DumpConfigBpdu (bridge, givenPort, -1, bpdu);
        LOG_UNINDENT (bridge);

        FLUSH_LOG (bridge);
#endif
#ifdef AIR_SUPPORT_MSTP
        ethernet_output_use_default_netif(ptr_pbuf, ptr_pbuf->len);
        pbuf_free(ptr_pbuf);
#endif
    }
}

// ============================================================================
// 13.29.aa) - 13.29.28
void txRstp(STP_BRIDGE* bridge, PortIndex givenPort, unsigned int timestamp)
{
    PORT* port = bridge->ports[givenPort-1];
    PORT_TREE* cistTree;
    unsigned int bpduSize;
    MSTP_BPDU* bpdu = NULL;
#ifdef AIR_SUPPORT_MSTP
    struct pbuf  *ptr_pbuf = NULL;
#endif
    unsigned int mstiIndex;
    PORT_TREE* tree;
    unsigned short mstiPortID;

    if (NULL == port)
    {
        return;
    }

    cistTree = port->trees[CIST_INDEX];
    if (NULL == cistTree)
    {
        return;
    }

    if (bridge->ForceProtocolVersion < 3)
    {
        bpduSize = (unsigned int)offsetof(MSTP_BPDU, Version3Length);
    }
    else
    {
        bpduSize = sizeof(MSTP_BPDU);
        for (mstiIndex = 0; mstiIndex < (bridge->treeCount-1); mstiIndex++)
        {
            tree = port->trees[1 + mstiIndex];
            if (NULL == tree)
            {
                continue;
            }
            bpduSize += sizeof(MSTI_CONFIG_MESSAGE);
        }
    }

    FLUSH_LOG(bridge);

#ifdef AIR_SUPPORT_MSTP
    ptr_pbuf = pbuf_alloc(PBUF_TRANSPORT, bpduSize, PBUF_RAM);
    if (NULL == ptr_pbuf)
    {
        return;
    }
    _bpdu_head_fill(givenPort, ptr_pbuf);
    bpdu = (MSTP_BPDU *)(ptr_pbuf->payload);
#endif

    if (bpdu == NULL)
    {
        return;
    }

    // octets 1 and 2 - 14.3 in 802.1Q-2018
    bpdu->head.protocolId = 0;

    // octets 3 and 4
    bpdu->head.bpduType = 2;
    if (bridge->ForceProtocolVersion < 3)
    {
        bpdu->head.protocolVersionId = 2; // 14.3.c)
    }
    else if (bridge->ForceProtocolVersion == 3)
    {
        bpdu->head.protocolVersionId = 3; // 14.3.d)
    }
    else
    {
#ifdef AIR_SUPPORT_MSTP
        pbuf_free(ptr_pbuf);
#endif
        return; // SPT not yet implemented by this function
    }

    // octet 5 - 14.4.a) to 14.4.g) in 802.1Q-2018
    bpdu->cistFlags = GetBpduPortRole(cistTree->role) << 2;
    if (cistTree->agree)
    {
        bpdu->cistFlags |= (unsigned char) 0x40;
    }

    if (cistTree->proposing)
    {
        bpdu->cistFlags |= (unsigned char) 2;
    }

    if (cistTree->tcWhile != 0)
    {
        bpdu->cistFlags |= (unsigned char) 1;
    }

    if (cistTree->learning)
    {
        bpdu->cistFlags |= (unsigned char) 0x10;
    }

    if (cistTree->forwarding)
    {
        bpdu->cistFlags |= (unsigned char) 0x20;
    }

    // octets 6 to 13 - 14.4.h) in 802.1Q-2018
    //bpdu->cistRootId = cistTree->designatedPriority.RootId;
    memcpy(bpdu->cistRootId, cistTree->designatedPriority.RootId, sizeof(BRIDGE_ID));

    // octets 14 to 17 - 14.4.i) in 802.1Q-2018
    //bpdu->cistExternalPathCost = cistTree->designatedPriority.ExternalRootPathCost;
    bpdu->cistExternalPathCost = htonl(cistTree->designatedPriority.ExternalRootPathCost);

    // octets 18 to 25 - 14.4.j) in 802.1Q-2018
    //bpdu->cistRegionalRootId = cistTree->designatedPriority.RegionalRootId;
    memcpy(bpdu->cistRegionalRootId, cistTree->designatedPriority.RegionalRootId, sizeof(BRIDGE_ID));

    // octets 26 to 27 - 14.4.k) in 802.1Q-2018
    //bpdu->cistPortId = cistTree->designatedPriority.DesignatedPortId;
    bpdu->cistPortId = htons(cistTree->designatedPriority.DesignatedPortId);

    // octets 28 to 29 - 14.4.l) in 802.1Q-2018
    //bpdu->MessageAge = cistTree->designatedTimes.MessageAge * 256;
    bpdu->MessageAge = htons(cistTree->designatedTimes.MessageAge * 256);

    // octets 30 to 31 - 14.4.m) in 802.1Q-2018
    //bpdu->MaxAge = cistTree->designatedTimes.MaxAge * 256;
    bpdu->MaxAge = htons(cistTree->designatedTimes.MaxAge * 256);

    // octets 32 to 33 - 14.4.n) in 802.1Q-2018
    //bpdu->HelloTime = cistTree->portTimes.HelloTime * 256;
    bpdu->HelloTime = htons(cistTree->portTimes.HelloTime * 256);

    // octets 34 to 35 - 14.4.o) in 802.1Q-2018
    //bpdu->ForwardDelay = cistTree->designatedTimes.ForwardDelay * 256;
    bpdu->ForwardDelay = htons(cistTree->designatedTimes.ForwardDelay * 256);

    // octet 36 - 14.4.p) in 802.1Q-2018
    bpdu->Version1Length = 0;

    if (bridge->ForceProtocolVersion >= 3)
    {
        // octet 37 to 38 - 14.4.q) in 802.1Q-2018
        //bpdu->Version3Length = (unsigned short)(bpduSize - 38);
        bpdu->Version3Length = htons((unsigned short)(bpduSize - 38));

        // octet 39 to 89 - 14.4.r) in 802.1Q-2018
        //bpdu->mstConfigId = bridge->MstConfigId;
        memcpy(&bpdu->mstConfigId, &bridge->MstConfigId, sizeof(STP_MST_CONFIG_ID));

        // octet 90 to 93 - 14.4.s) in 802.1Q-2018
        //bpdu->cistInternalRootPathCost = cistTree->designatedPriority.InternalRootPathCost;
        bpdu->cistInternalRootPathCost = htonl(cistTree->designatedPriority.InternalRootPathCost);

        // octet 94 to 101 - 14.4.t) in 802.1Q-2018
        //bpdu->cistBridgeId = cistTree->designatedPriority.DesignatedBridgeId;
        //bpdu->cistBridgeId.SetPriorityAndMstid(bpdu->cistBridgeId.GetPriorityWithoutMstid(), 0);
        memcpy(bpdu->cistBridgeId, cistTree->designatedPriority.DesignatedBridgeId, sizeof(BRIDGE_ID));

        // octet 102 - 14.4.u) in 802.1Q-2018
        bpdu->cistRemainingHops = cistTree->designatedTimes.remainingHops;

        MSTI_CONFIG_MESSAGE* mstiMessage = (MSTI_CONFIG_MESSAGE *)(bpdu + 1);

        // 14.4.1 in 802.1Q-2018
        for (mstiIndex = 0; mstiIndex < (bridge->treeCount-1); mstiIndex++)
        {
            tree = port->trees[1 + mstiIndex];
            if (NULL == tree)
            {
                continue;
            }
            if (NULL == bridge->trees[1 + mstiIndex])
            {
                continue;
            }

            // a)
            mstiMessage->flags = GetBpduPortRole(tree->role) << 2;

            if (tree->agree)
            {
                mstiMessage->flags |= (unsigned char) 0x40;
            }

            if (tree->proposing)
            {
                mstiMessage->flags |= (unsigned char) 2;
            }

            if (tree->tcWhile != 0)
            {
                mstiMessage->flags |= (unsigned char) 1;
            }

            if (port->master)
            {
                mstiMessage->flags |= (unsigned char) 0x80;
            }

            if (tree->learning)
            {
                mstiMessage->flags |= (unsigned char) 0x10;
            }

            if (tree->forwarding)
            {
                mstiMessage->flags |= (unsigned char) 0x20;
            }

            // b) to e)
            //mstiMessage->RegionalRootId       = tree->designatedPriority.RegionalRootId;
            memcpy(mstiMessage->RegionalRootId, tree->designatedPriority.RegionalRootId, sizeof(BRIDGE_ID));

            //mstiMessage->InternalRootPathCost = tree->designatedPriority.InternalRootPathCost;
            mstiMessage->InternalRootPathCost = htonl(tree->designatedPriority.InternalRootPathCost);

            //mstiMessage->BridgePriority       = bridge->trees[1 + mstiIndex]->GetBridgeIdentifier().GetPriorityWithoutMstid() >> 8;
            mstiMessage->BridgePriority         = GET_PRIORITY_FROM_IDENTIFIER(bridge->trees[1 + mstiIndex]->BridgeIdentifier);

            //mstiMessage->PortPriority         = tree->portId.GetPriority();
            mstiPortID = htons(tree->portId);
            mstiMessage->PortPriority           = GET_PRIORITY_FROM_IDENTIFIER(mstiPortID);
            // f)
            mstiMessage->RemainingHops          = tree->designatedTimes.remainingHops;

            mstiMessage++;
        }
    }

    if (bridge->ForceProtocolVersion < 3)
    {
        LOG(bridge, givenPort+1, -1, "TX RSTP BPDU to port {%d}:\r\n", 1 + givenPort);
#if STP_USE_LOG
        LOG_INDENT(bridge);
        DumpRstpBpdu(bridge, givenPort, -1, bpdu);
        LOG_UNINDENT(bridge);
#endif
    }
    else if (bridge->ForceProtocolVersion == 3)
    {
        LOG(bridge, givenPort+1, -1, "TX MSTP BPDU to port {%d}:\r\n", 1 + givenPort);
#if STP_USE_LOG
        LOG_INDENT(bridge);
        DumpMstpBpdu(bridge, givenPort, -1, bpdu);
        LOG_UNINDENT(bridge);
#endif
    }
    else
    {
        //assert(false); // not yet implemented for SPT
    }

#ifdef AIR_SUPPORT_MSTP
    ethernet_output_use_default_netif(ptr_pbuf, ptr_pbuf->len);
    pbuf_free(ptr_pbuf);
#endif
}


// ============================================================================
// 13.29.ab) - 13.29.29 in 802.1Q-2018
void txTcn(STP_BRIDGE* bridge, PortIndex givenPort, unsigned int timestamp)
{
    BPDU_HEADER     bpdu;
#ifdef AIR_SUPPORT_MSTP
    struct pbuf     *ptr_pbuf = NULL;
    UI32_T          bpdu_size;
#endif

    FLUSH_LOG (bridge);

    // 14.3.b) in 802.1Q-2018.
    bpdu.protocolId = 0;
    bpdu.protocolVersionId = 0;
    bpdu.bpduType = 0x80;

#ifdef AIR_SUPPORT_MSTP
    bpdu_size = sizeof(BPDU_HEADER);
    ptr_pbuf = pbuf_alloc(PBUF_TRANSPORT, bpdu_size, PBUF_RAM);
    if (NULL == ptr_pbuf)
    {
        return;
    }
    _bpdu_head_fill(givenPort, ptr_pbuf);
    memcpy(ptr_pbuf->payload, &bpdu, bpdu_size);
#endif

    LOG(bridge, givenPort+1, -1, "TX TCN BPDU to port {%d}:\r\n", 1 + givenPort);

#ifdef AIR_SUPPORT_MSTP
    ethernet_output_use_default_netif(ptr_pbuf, ptr_pbuf->len);
    pbuf_free(ptr_pbuf);
#endif
}

// ============================================================================
// 13.29.30 in 802.1Q-2018
void updtAgreement(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    //assert(false); // SPT not yet implemented
}

// ============================================================================
// 13.29.ac) - 13.29.31 in 802.1Q-2018
// Sets rcvdSTP TRUE if the BPDU received is a version 0 or version 1 TCN or a Config BPDU. Sets
// rcvdRSTP TRUE if the received BPDU is a RST BPDU or a MST BPDU.
// Note AG: we'll set rcvdRSTP also for a received SPT BPDU.
void updtBPDUVersion(STP_BRIDGE* bridge, PortIndex givenPort)
{
    if (NULL == bridge->ports[givenPort])
    {
        return;
    }

    switch (bridge->receivedBpduType)
    {
        case VALIDATED_BPDU_TYPE_STP_TCN:
        case VALIDATED_BPDU_TYPE_STP_CONFIG:
            bridge->ports[givenPort]->rcvdSTP = true;
            break;

        case VALIDATED_BPDU_TYPE_MST:
        case VALIDATED_BPDU_TYPE_RST:
        case VALIDATED_BPDU_TYPE_SPT:
            bridge->ports[givenPort]->rcvdRSTP = true;
            break;

        default:
            //assert(false);
            break;
    }
}

// ============================================================================
// 13.29.32 in 802.1Q-2018
void updtDigest(STP_BRIDGE* bridge, PortIndex givenPort)
{
    //assert(false); // SPT not yet implemented
}

// ============================================================================
// 13.29.ad) - 13.29.33 in 802.1Q-2018
// Updates rcvdInfoWhile (13.25). The value assigned to rcvdInfoWhile is three times the Hello Time, if either:
// a) Message Age, incremented by 1 second and rounded to the nearest whole second, does not exceed
//    Max Age and the information was received from a bridge external to the MST Region (rcvdInternal
//    FALSE); or
// b) remainingHops, decremented by one, is greater than zero and the information was received from a
//    bridge internal to the MST Region (rcvdInternal TRUE);
// and is zero otherwise.
//
// The values of Message Age, Max Age, remainingHops, and Hello Time used in these calculations are taken
// from the CIST's portTimes parameter (13.27.48) and are not changed by this procedure.
void updtRcvdInfoWhile(STP_BRIDGE* bridge, PortIndex givenPort, TreeIndex givenTree)
{
    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;

    const TIMES* cistTimes = &port->trees[CIST_INDEX]->portTimes;

    if (NULL == port)
    {
        return;
    }
    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }

    if (((cistTimes->MessageAge + 1 <= cistTimes->MaxAge) && (port->rcvdInternal == false))
        || (((int)cistTimes->remainingHops - 1 > 0) && port->rcvdInternal))
    {
        portTree->rcvdInfoWhile = 3 * cistTimes->HelloTime;
    }
    else
    {
        portTree->rcvdInfoWhile = 0;
    }
}

static void CalculateRootPathPriorityForPort(const STP_BRIDGE* bridge,
                                        unsigned int givenPort,
                                        TreeIndex givenTree,
                                        PRIORITY_VECTOR* rootPathPriorityOut)
{
    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;

    if (NULL == port)
    {
        return;
    }

    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }

    //*rootPathPriorityOut = portTree->portPriority;
    memcpy(rootPathPriorityOut, &portTree->portPriority, sizeof(PRIORITY_VECTOR));

    if (givenTree == CIST_INDEX)
    {
        // 13.10, page 486 in 802.1Q-2018
        // A root path priority vector for a Port can be calculated from a port priority vector that contains information
        // from a message priority vector, as follows:

        // Note AG: The standard references 13.29.8 (fromSameRegion), but that function tries to read the received BPDU
        // outside of STP_OnBpduReceived. I replaced fromSameRegion with rcvdInternal in the "if" below.
        if (!port->rcvdInternal)
        {
            // If the port priority vector was received from a Bridge in a different region (13.29.8), the External Port Path
            // Cost EPCPB is added to the External Root Path Cost component, and the Regional Root Identifier is set to
            // the value of the Bridge Identifier for the receiving Bridge. The Internal Root Path Cost component will have
            // been set to zero on reception.
            //      root path priority vector = {RD : ERCD + EPCPB : B : 0 : D : PD : PB}
            rootPathPriorityOut->ExternalRootPathCost += port->ExternalPortPathCost;
            //rootPathPriorityOut->RegionalRootId = bridge->trees[givenTree]->GetBridgeIdentifier();
            memcpy(rootPathPriorityOut->RegionalRootId, bridge->trees[givenTree]->BridgeIdentifier, sizeof(BRIDGE_ID));
            if (portTree->portPriority.InternalRootPathCost != 0)
            {
                return;
            }
        }
        else
        {
            // If the port priority vector was received from a Bridge in the same region (13.29.8), the Internal Port Path
            // Cost IPCPB is added to the Internal Root Path Cost component.
            //      root path priority vector = {RD : ERCD : RRD : IRCD + IPCPB : D : PD : PB)
            rootPathPriorityOut->InternalRootPathCost += portTree->InternalPortPathCost;
        }
    }
    else
    {
        // MSTI - 13.11, page 488 in 802.1Q-2018
        // A root path priority vector for a given MSTI can be calculated for a port that has received a port priority
        // vector from a bridge in the same region by adding the Internal Port Path Cost IPCPB to the Internal Root
        // Path Cost component.
        //			root path priority vector = {RRD : IRCD + IPCPB : D : PD : PB)
        rootPathPriorityOut->InternalRootPathCost += portTree->InternalPortPathCost;
    }
}

// 13.27.20 in 802.1Q-2018
static void CalculateDesignatedPriorityForPort(STP_BRIDGE* bridge, unsigned int givenPort, TreeIndex givenTree)
{
    BRIDGE_TREE* bridgeTree = bridge->trees[givenTree];
    PORT* port = bridge->ports[givenPort];
    PORT_TREE* portTree;

    if (NULL == port)
    {
        return;
    }

    if (NULL == bridgeTree)
    {
        return;
    }
    portTree = port->trees[givenTree];
    if (NULL == portTree)
    {
        return;
    }

    if (givenTree == CIST_INDEX)
    {
        // The designated priority vector for a port Q on bridge B is the root priority vector with B's Bridge Identifier
        // B substituted for the DesignatedBridgeID and Q's Port Identifier QB substituted for the DesignatedPortID
        // and RcvPortID components.
        //portTree->designatedPriority = bridgeTree->rootPriority;
        memcpy(&portTree->designatedPriority, &bridgeTree->rootPriority, sizeof(PRIORITY_VECTOR));
        //portTree->designatedPriority.DesignatedBridgeId = bridgeTree->GetBridgeIdentifier ();
        memcpy(portTree->designatedPriority.DesignatedBridgeId, bridgeTree->BridgeIdentifier, sizeof(BRIDGE_ID));
        portTree->designatedPriority.DesignatedPortId   = portTree->portId;

        // If Q is attached to a LAN that has one or more STP bridges attached (as
        // determined by the Port Protocol Migration state machine), B's Bridge Identifier B is also substituted for the
        // RRootID component.
        if (port->sendRSTP == false)
        {
            //portTree->designatedPriority.RegionalRootId = bridgeTree->GetBridgeIdentifier ();
            memcpy(portTree->designatedPriority.RegionalRootId, bridgeTree->BridgeIdentifier, sizeof(BRIDGE_ID));
        }
    }
    else
    {
        // MSTI
        // The designated priority vector for a port Q on bridge B is the root priority vector with B's Bridge Identifier
        // B substituted for the DesignatedBridgeID and Q's Port Identifier QB substituted for the DesignatedPortID
        // and RcvPortID components.
        //portTree->designatedPriority = bridgeTree->rootPriority;
        memcpy(&portTree->designatedPriority, &bridgeTree->rootPriority, sizeof(PRIORITY_VECTOR));
        //portTree->designatedPriority.DesignatedBridgeId = bridgeTree->GetBridgeIdentifier();
        memcpy(portTree->designatedPriority.DesignatedBridgeId, bridgeTree->BridgeIdentifier, sizeof(BRIDGE_ID));
        portTree->designatedPriority.DesignatedPortId   = portTree->portId;
    }
}

// ============================================================================
// 13.29.ae) - 13.29.34
void updtRolesTree(STP_BRIDGE* bridge, TreeIndex givenTree)
{
    // the SPT stuff is not implemented by this function
    if (bridge->ForceProtocolVersion > STP_VERSION_MULTIPLE_STP)
    {
        return;
    }

    BRIDGE_TREE* bridgeTree = bridge->trees[givenTree];
    unsigned int portIndex;

    if (NULL == bridgeTree)
    {
        return;
    }

    LOG(bridge, 0, givenTree, "Tree {%d}:\r\n", givenTree);
    LOG(bridge, 0, givenTree, "  BridgeID: {"BRIDGE_STR"}\r\n", BRIDGE_VALUE(bridgeTree->BridgeIdentifier));

    BRIDGE_ID previousCistRegionalRootIdentifier;// = bridgeTree->rootPriority.RegionalRootId;
    unsigned int previousCistExternalRootPathCost   = bridgeTree->rootPriority.ExternalRootPathCost;

    memcpy(previousCistRegionalRootIdentifier, bridgeTree->rootPriority.RegionalRootId, sizeof(BRIDGE_ID));
    // initialize this to our bridge priority
    //bridgeTree->rootPriority = bridgeTree->GetBridgePriority();
    memcpy(&bridgeTree->rootPriority, &bridgeTree->BridgePriority, sizeof(PRIORITY_VECTOR));
    //bridgeTree->rootPortId.Reset();
    bridgeTree->rootPortId = 0;
    //bridgeTree->rootTimes = bridgeTree->BridgeTimes;
    memcpy(&bridgeTree->rootTimes, &bridgeTree->BridgeTimes, sizeof(TIMES));

    PORT_TREE* rootPortTree = NULL;

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        PORT* port = bridge->ports[portIndex];
        PORT_TREE* portTree;

        if (NULL == port)
        {
            continue;
        }

        portTree = port->trees[givenTree];
        if (NULL == portTree)
        {
            continue;
        }
        LOG(bridge,portIndex+1, givenTree, "  Port {%d} update, tree %d\r\n", portIndex+1, givenTree);

        if (portTree->infoIs == INFO_IS_RECEIVED)
        {
            // a)
            PRIORITY_VECTOR rootPathPriority;
            CalculateRootPathPriorityForPort(bridge, portIndex, givenTree, &rootPathPriority);

            LOG(bridge, portIndex+1, givenTree, "  Port {%d} root path priority: {rootId: "BRIDGE_STR", exPathCost %u, "BRIDGE_STR", intPathCost %u}\r\n",
                portIndex+1, BRIDGE_VALUE(rootPathPriority.RootId),rootPathPriority.ExternalRootPathCost,
                BRIDGE_VALUE(rootPathPriority.RegionalRootId),rootPathPriority.InternalRootPathCost);

            // c)
            //if ((rootPathPriority.DesignatedBridgeId.GetAddress () != bridgeTree->GetBridgePriority ().DesignatedBridgeId.GetAddress ())
            if ((0 != memcmp(((unsigned char *)rootPathPriority.DesignatedBridgeId)+2, ((unsigned char *)bridgeTree->BridgePriority.DesignatedBridgeId)+2, 6))
                && (port->restrictedRole == false))
            {
                //if (rootPathPriority.IsBetterThan (bridgeTree->rootPriority)
                    //|| ((rootPathPriority == bridgeTree->rootPriority) && (portTree->portId.IsBetterThan (bridgeTree->rootPortId))))
                //if ((memcmp(&rootPathPriority, &bridgeTree->rootPriority, sizeof(PRIORITY_VECTOR)) < 0)
                if (IsBetterThan(&rootPathPriority, &bridgeTree->rootPriority)
                    || ((0 == memcmp(&rootPathPriority, &bridgeTree->rootPriority, sizeof(PRIORITY_VECTOR)))
                        //&& (memcmp(&portTree->portId, &bridgeTree->rootPortId, sizeof(Port_ID)) < 0)))
                        &&(portTree->portId < bridgeTree->rootPortId)))
                {
                    rootPortTree = portTree;

                    //bridgeTree->rootPriority = rootPathPriority;
                    memcpy(&bridgeTree->rootPriority, &rootPathPriority, sizeof(PRIORITY_VECTOR));
                    bridgeTree->rootPortId   = portTree->portId;

                    // d)
                    //bridgeTree->rootTimes = portTree->portTimes;
                    memcpy(&bridgeTree->rootTimes, &portTree->portTimes, sizeof(TIMES));
                    if (port->rcvdInternal == false)
                    {
                        bridgeTree->rootTimes.MessageAge++;
                    }
                    else
                    {
                        if (bridgeTree->rootTimes.remainingHops > 0)
                        {
                            bridgeTree->rootTimes.remainingHops--;
                        }
                    }
                }
            }
        }
    }

    LOG(bridge, 0, givenTree, "  bridge root priority : {rootId: "BRIDGE_STR", exPathCost %u, "BRIDGE_STR", intPathCost %u}\r\n",
        BRIDGE_VALUE(bridgeTree->rootPriority.RootId),bridgeTree->rootPriority.ExternalRootPathCost,
        BRIDGE_VALUE(bridgeTree->rootPriority.RegionalRootId),bridgeTree->rootPriority.InternalRootPathCost);

    LOG(bridge, 0, givenTree, "  root port = {0x%x}\r\n", bridgeTree->rootPortId);

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        PORT*      port;
        PORT_TREE* portTree;

        port = bridge->ports[portIndex];
        if (NULL == port)
        {
            continue;
        }
        portTree = port->trees[givenTree];
        if (NULL == portTree)
        {
            continue;
        }
        // e)
        CalculateDesignatedPriorityForPort(bridge, portIndex, givenTree);

        // f)
        portTree->designatedTimes = bridgeTree->rootTimes;

        LOG (bridge, portIndex+1, givenTree, "  Port {%u} designated priority : {rootId: "BRIDGE_STR", exPathCost %u, "BRIDGE_STR", intPathCost %u}\r\n",portIndex+1,
        BRIDGE_VALUE(portTree->designatedPriority.RootId), portTree->designatedPriority.ExternalRootPathCost,
        BRIDGE_VALUE(portTree->designatedPriority.RegionalRootId), portTree->designatedPriority.InternalRootPathCost);
    }

    // If the root priority vector for the CIST is recalculated, and has a different Regional Root Identifier than that
    // previously selected, and has or had a nonzero CIST External Root Path Cost, the syncMaster() procedure
    // (13.29.26) is invoked.
    if ((givenTree == CIST_INDEX)
        //&& (previousCistRegionalRootIdentifier != bridgeTree->rootPriority.RegionalRootId)
        && (0 != memcmp(previousCistRegionalRootIdentifier, bridgeTree->rootPriority.RegionalRootId, sizeof(BRIDGE_ID)))
        && ((bridgeTree->rootPriority.ExternalRootPathCost != 0) || (previousCistExternalRootPathCost != 0)))
    {
        syncMaster(bridge);
    }

    // The CIST, or MSTI Port Role for each port is assigned, and its port priority vector and timer information are
    // updated as specified in the remainder of this clause (13.41.2).

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        PORT* port;
        PORT_TREE* portTree;
        PORT_TREE* cistPortTree;

        port = bridge->ports[portIndex];
        if (NULL == port)
        {
            continue;
        }

        portTree = port->trees[givenTree];
        cistPortTree = port->trees[CIST_INDEX];
        if ((NULL == portTree) || (NULL == cistPortTree))
        {
            continue;
        }

        // If the port is Disabled (infoIs == Disabled), selectedRole is set to DisabledPort.
        if (portTree->infoIs == INFO_IS_DISABLED)
        {
            portTree->selectedRole = STP_PORT_ROLE_DISABLED;
        }

        // Otherwise, if this procedure was invoked for an MSTI or an SPT, for a port that is not Disabled, and that has
        // CIST port priority information that was received from a Bridge external to its Bridge's Region
        // (infoIs == Received and infoInternal == FALSE), then
        else if ((givenTree != CIST_INDEX)
                && ((cistPortTree->infoIs == INFO_IS_RECEIVED) && !port->infoInternal))
        {
            // g) If the selected CIST Port Role (calculated for the CIST prior to invoking this procedure for an MSTI
            //    or SPT) is RootPort, selectedRole is set to MasterPort
            if (cistPortTree->selectedRole == STP_PORT_ROLE_ROOT)
            {
                portTree->selectedRole = STP_PORT_ROLE_MASTER;
            }
#ifdef AIR_SUPPORT_MSTP
            else
            {
               /*  "At a Boundary Port frames allocated to the CIST and
                *   all MSTIs are forwarded or not forwarded alike.
                *   This is because Port Role assignments are such that
                *   if the CIST Port Role is Root Port, the MSTI Port Role
                *   will be Master Port, and if the CIST Port Role is
                *   Designated Port, Alternate Port, Backup Port,
                *   or Disabled Port, each MSTI's Port Role will be the same." */
                portTree->selectedRole = cistPortTree->selectedRole;
            }
#else
            // h) If selected CIST Port Role is AlternatePort, selectedRole is set to AlternatePort.
            if (cistPortTree->selectedRole == STP_PORT_ROLE_ALTERNATE)
            {
                portTree->selectedRole = STP_PORT_ROLE_ALTERNATE;
            }
#endif
            // i) Additionally, updtInfo is set if the port priority vector differs from the designated priority vector or
            //    the port's associated timer parameter differs from the one for the Root Port.
            //
            // Note AG: Problem in the standard: If we are the root bridge, we don't have a root port, so how are we
            // supposed to look at the "associated timer parameter" "for the Root Port"?
            // Let's look at the bridge times in this case.
            //if (portTree->portPriority != portTree->designatedPriority)
            if (0 != memcmp(&portTree->portPriority, &portTree->designatedPriority, sizeof(PRIORITY_VECTOR)))
            {
                portTree->updtInfo = true;
            }
            //else if ((rootPortTree != NULL) && (portTree->portTimes != rootPortTree->designatedTimes))
            else if ((rootPortTree != NULL) && (0 != memcmp(&portTree->portTimes, &rootPortTree->designatedTimes, sizeof(TIMES))))
            {
                portTree->updtInfo = true;
            }
            //else if ((rootPortTree == NULL) && (portTree->portTimes != bridgeTree->rootTimes))
            else if ((rootPortTree == NULL) && (0 != memcmp(&portTree->portTimes, &bridgeTree->rootTimes, sizeof(TIMES))))
            {
                portTree->updtInfo = true;
            }
        }

        // Otherwise, for the CIST for a port that is not Disabled and not internal to an SPT Region, or for an MSTI for
        // a port of that is not Disabled and whose CIST port priority information was not received from a Bridge
        // external to the Region (infoIs != Received or infoInternal == TRUE), the CIST or MSTI port role is assigned,
        // and the port priority vector and timer information updated as follows:
        //
        // "and not internal to an SPT Region" is not yet implemented.
        else if (((givenTree == CIST_INDEX)
                    && (cistPortTree->infoIs != INFO_IS_DISABLED))
                 || ((givenTree != CIST_INDEX)
                    && (cistPortTree->infoIs != INFO_IS_DISABLED)
                    && ((cistPortTree->infoIs != INFO_IS_RECEIVED)
                    || port->infoInternal)))
        {
            // j) If the port priority vector information was aged (infoIs = Aged), updtInfo is set and selectedRole is
            //    set to DesignatedPort.
            if (portTree->infoIs == INFO_IS_AGED)
            {
                portTree->updtInfo = true;
                portTree->selectedRole = STP_PORT_ROLE_DESIGNATED;
            }

            // k) If the port priority vector was derived from another port on the Bridge or from the Bridge itself as
            //    the Root Bridge (infoIs = Mine), selectedRole is set to DesignatedPort. Additionally, updtInfo is set if
            //    the port priority vector differs from the designated priority vector or the port's associated timer
            //    parameter(s) differ(s) from the Root Port's associated timer parameters.
            //
            // Note AG: see the note at condition i) above.
            else if (portTree->infoIs == INFO_IS_MINE)
            {
                portTree->selectedRole = STP_PORT_ROLE_DESIGNATED;

                //if (portTree->portPriority != portTree->designatedPriority)
                if (!_isEqual(&portTree->portPriority, &portTree->designatedPriority, sizeof(PRIORITY_VECTOR)))
                {
                    portTree->updtInfo = true;
                }
                //else if ((rootPortTree != NULL) && (portTree->portTimes != rootPortTree->designatedTimes))
                else if ((rootPortTree != NULL) && (!_isEqual(&portTree->portTimes, &rootPortTree->designatedTimes, sizeof(TIMES))))
                {
                    portTree->updtInfo = true;
                }
                //else if ((rootPortTree == NULL) && (portTree->portTimes != bridgeTree->rootTimes))
                else if ((rootPortTree == NULL) && (!_isEqual(&portTree->portTimes, &bridgeTree->rootTimes, sizeof(TIMES))))
                {
                    portTree->updtInfo = true;
                }
            }

            // l) If the port priority vector was received in a Configuration Message and is not aged
            //    (infoIs == Received), and the root priority vector is now derived from it, selectedRole is set to
            //    RootPort, and updtInfo is reset;
            else if ((portTree->infoIs == INFO_IS_RECEIVED) && (rootPortTree == portTree))
            {
                portTree->selectedRole = STP_PORT_ROLE_ROOT;
                portTree->updtInfo = false;
            }

            // m) If the port priority vector was received in a Configuration Message and is not aged
            //    (infoIs == Received), the root priority vector is not now derived from it, the designated priority
            //    vector is not better than the port priority vector, and the designated bridge and designated port
            //    components of the port priority vector do not reflect another port on this bridge, selectedRole is set
            //    to AlternatePort, and updtInfo is reset.
            //
            // Note AG: What exactly is this supposed to mean?
            // "the designated bridge and designated port components of the port priority vector do not reflect another port on this bridge"
            // Answer: Let's look at the DesignatedBridgeId component, but only at the _address field, not at _priority too,
            // to account for the case when the bridge priority was just changed by the user (for instance from 0x8000 to 0x9000)
            // and a BPDU with the old priority is still propagating through the network.
            else if ((portTree->infoIs == INFO_IS_RECEIVED)
                    && (rootPortTree != portTree)
                    //&& (portTree->designatedPriority.IsNotBetterThan (portTree->portPriority))
                    //&& (memcmp(&portTree->designatedPriority, &portTree->portPriority, sizeof(PRIORITY_VECTOR)) >= 0)
                    && (IsNotBetterThan(&portTree->designatedPriority, &portTree->portPriority))
                    //&& (portTree->portPriority.DesignatedBridgeId.GetAddress() != bridgeTree->GetBridgeIdentifier().GetAddress()))
                    && (0 != memcmp(((unsigned char *)portTree->portPriority.DesignatedBridgeId)+2, ((unsigned char *)bridgeTree->BridgeIdentifier)+2, 6)))
            {
                portTree->selectedRole = STP_PORT_ROLE_ALTERNATE;
                portTree->updtInfo = false;
            }
#ifdef AIR_SUPPORT_MSTP
            else if ((portTree->infoIs == INFO_IS_RECEIVED)
                    && (rootPortTree != portTree)
                    && _isEqual(&portTree->designatedPriority, &portTree->portPriority, sizeof(PRIORITY_VECTOR)))
            {
                /* This pkt is from myself */
                portTree->selectedRole = STP_PORT_ROLE_DESIGNATED;
                portTree->updtInfo = true;
                portTree->disputed = true;
            }
#endif
            // n) If the port priority vector was received in a Configuration Message and is not aged
            //    (infoIs == Received), the root priority vector is not now derived from it, the designated priority
            //    vector is not better than the port priority vector, and the designated bridge and designated port
            //    components of the port priority vector reflect another port on this bridge, selectedRole is set to
            //    BackupPort, and updtInfo is reset;
            else if ((portTree->infoIs == INFO_IS_RECEIVED)
                    && (rootPortTree != portTree)
                    //&& (portTree->designatedPriority.IsNotBetterThan (portTree->portPriority))
                    //&& (memcmp(&portTree->designatedPriority, &portTree->portPriority, sizeof(PRIORITY_VECTOR)) >= 0)
                    && (IsNotBetterThan(&portTree->designatedPriority, &portTree->portPriority))
                    //&& (portTree->portPriority.DesignatedBridgeId.GetAddress() == bridgeTree->GetBridgeIdentifier ().GetAddress()))
                    && (0 == memcmp(((unsigned char *)portTree->portPriority.DesignatedBridgeId)+2, ((unsigned char *)bridgeTree->BridgeIdentifier)+2, 6)))
            {
                portTree->selectedRole = STP_PORT_ROLE_BACKUP;
                portTree->updtInfo = false;
            }

            // o) If the port priority vector was received in a Configuration Message and is not aged
            //    (infoIs == Received), the root priority vector is not now derived from it, the designated priority
            //    vector is better than the port priority vector, selectedRole is set to DesignatedPort, and updtInfo is
            //    set.
            else if ((portTree->infoIs == INFO_IS_RECEIVED) && (rootPortTree != portTree)
                    //&& (portTree->designatedPriority.IsBetterThan (portTree->portPriority)))
                    //&& (memcmp(&portTree->designatedPriority, &portTree->portPriority, sizeof(PRIORITY_VECTOR)) < 0))
                    && (IsBetterThan(&portTree->designatedPriority, &portTree->portPriority)))
            {
                portTree->selectedRole = STP_PORT_ROLE_DESIGNATED;
                portTree->updtInfo = true;
            }
            else
            {
                // should we ever be here??
                //assert(false);
            }
        }

        LOG(bridge, portIndex+1, givenTree, "Port {%u}: Tree{%u}: selectedRole set to {%s}\r\n", 1 + portIndex, givenTree, GetPortRoleName(portTree->selectedRole));
    }
}

// ============================================================================
// 13.29.af) - 13.29.35
// This procedure sets selectedRole to DisabledPort for all ports of the bridge for a given tree (CIST, or MSTI).
void updtRolesDisabledTree(STP_BRIDGE* bridge, TreeIndex givenTree)
{
    unsigned int portIndex;

    for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
    {
        if (NULL == bridge->ports[portIndex])
        {
            continue;
        }

        if (NULL == bridge->ports[portIndex]->trees[givenTree])
        {
            continue;
        }
        bridge->ports[portIndex]->trees[givenTree]->selectedRole = STP_PORT_ROLE_DISABLED;
    }
}

#ifdef AIR_SUPPORT_MSTP
void updtPortStpMode(unsigned int portIndex, unsigned char mode)
{
    UI32_T lowerPort;

    if (((_port_mode >> (portIndex)) & 0x1) != (!!mode))
    {
        lowerPort = stp_db_get_lag_lower_port(portIndex+1);
        if ((portIndex+1) != lowerPort)
        {
            _port_mode &= ~(1 << (lowerPort - 1));
            _port_mode |= (!!mode) << (lowerPort - 1);
        }
        _port_mode &= ~(1 << (portIndex));
        _port_mode |= (!!mode) << (portIndex);
        stp_db_port_mode_update(_port_mode);
    }
}

inline unsigned char portRoleTransition(unsigned char role)
{
    switch (role)
    {
        case STP_PORT_ROLE_DISABLED:
            return MSTP_ROLE_DISABLED;
        case STP_PORT_ROLE_MASTER:
            return MSTP_ROLE_MASTER;
        case STP_PORT_ROLE_ROOT:
            return MSTP_ROLE_ROOT;
        case STP_PORT_ROLE_DESIGNATED:
            return MSTP_ROLE_DESIGNATED;
        case STP_PORT_ROLE_ALTERNATE:
            return MSTP_ROLE_ALTERNATE;
        case STP_PORT_ROLE_BACKUP:
            return MSTP_ROLE_BACKUP;
        default:
            break;
    }

    return MSTP_ROLE_DISABLED;
}

void updtCistPortRole(unsigned int portIndex, unsigned int treeIndex, unsigned char role)
{
    unsigned char updtRole;

    updtRole = portRoleTransition(role);

    if (CIST_INDEX == treeIndex)
    {
        stp_db_port_role_update((portIndex+1), updtRole);
    }
}

void updtPortInstanceRole(STP_BRIDGE* bridge, unsigned int portIndex)
{
    unsigned int    i;
    unsigned char   role[MSTP_INSTANCE_MAX_NUM] = {0};
    PORT            *ptr_port;
    PORT_TREE       *ptr_port_tree;

    ptr_port = bridge->ports[portIndex];
    if (NULL == ptr_port)
    {
        return;
    }

    for (i = 0; i < bridge->treeCount; i++)
    {
        ptr_port_tree = ptr_port->trees[i];
        if (NULL == ptr_port_tree)
        {
            role[i] = MSTP_ROLE_DISABLED;
            continue;
        }
        role[i] = portRoleTransition(ptr_port_tree->role);
    }

    mstp_db_port_role_update((portIndex+1), role);
}

void updtPortDisableRole(STP_BRIDGE* bridge, unsigned int portIndex)
{
    unsigned char   role[MSTP_INSTANCE_MAX_NUM] = {0};

    memset(role, MSTP_ROLE_DISABLED, sizeof(unsigned char) * MSTP_INSTANCE_MAX_NUM);
    stp_db_port_role_update((portIndex+1), role[0]);
    mstp_db_port_role_update((portIndex+1), role);
}

#endif
