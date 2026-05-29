// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.33 from 802.1Q-2018.
#include "mstp_base_types.h"
#include "mstp_port.h"
#include "mstp_bridge.h"
#include "mstp_procedures.h"
#include "mstp_sm.h"
#include <assert.h>

static const char* GetStateName(unsigned char state)
{
    switch (state)
    {
        case NOT_EDGE:  return "NOT_EDGE";
        case EDGE:      return "EDGE";
        case ISOLATED:  return "ISOLATED";
        default:        return "(undefined)";
    }
}

// ============================================================================

// Returns the new state, or 0 when no transition is to be made.
static unsigned char CheckConditions(const STP_BRIDGE* bridge, PortTreeArgs* pt, unsigned char state)
{
    PORT* port = bridge->ports[pt->portIndex];

    if (NULL == port)
    {
        return 0;
    }

    // ------------------------------------------------------------------------
    // Check global conditions.

    if (bridge->BEGIN && !port->AdminEdge)
    {
        if (state == NOT_EDGE)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return NOT_EDGE;
    }

    if (bridge->BEGIN && port->AdminEdge)
    {
        if (state == EDGE)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return EDGE;
    }

#ifdef AIR_SUPPORT_MSTP
    if ((0 == state) && (port->AdminEdge))
    {
        return EDGE;
    }

    if ((0 == state) && (!port->AdminEdge))
    {
        return NOT_EDGE;
    }
#endif

    // ------------------------------------------------------------------------
    // Check exit conditions from each state.

    if (state == NOT_EDGE)
    {
        // Note AG: I changed this condition slightly because it was looping endlessly between EDGE and NOT_EDGE when disconnecting
        // from the root bridge a port that was connected to a non-stp device and already forwarding and whose AutoEdge was true.
        // The condition specified in 802.1Q-2018 was:
        //
        //	if ((!port->portEnabled && port->AdminEdge) ||
        //		((port->edgeDelayWhile == 0) && port->AutoEdge && port->sendRSTP && port->trees [CIST_INDEX]->proposing))
        //
        if ((!port->portEnabled && port->AdminEdge) ||
                (port->portEnabled && (port->edgeDelayWhile == 0) && port->AutoEdge && port->sendRSTP && port->trees[CIST_INDEX]->proposing))
        {
            return EDGE;
        }

        if ((port->edgeDelayWhile == 0) && !port->AdminEdge && !port->AutoEdge && port->sendRSTP && port->trees[CIST_INDEX]->proposing && port->operPointToPointMAC)
        {
            return ISOLATED;
        }

        return 0;
    }

    if (state == EDGE)
    {
        // Note AG: I changed this condition slightly because it was looping endlessly between EDGE and NOT_EDGE when disconnecting
        // from the root bridge a port that was connected to a non-stp device and already forwarding and whose AutoEdge was true.
        // The condition specified in 802.1Q-2018 was:
        //
        //if (((!port->portEnabled || !port->AutoEdge) && !port->AdminEdge) || !port->operEdge)
        //
        if (((!port->portEnabled || !port->AutoEdge) && !port->AdminEdge) || (port->portEnabled && !port->operEdge))
        {
            return NOT_EDGE;
        }

        return 0;
    }

    if (state == ISOLATED)
    {
        if (port->AdminEdge || port->AutoEdge || !port->isolate || !port->operPointToPointMAC)
        {
            return NOT_EDGE;
        }

        return 0;
    }

    //assert (false);
    return 0;
}

// ============================================================================

static void InitState(STP_BRIDGE* bridge, PortTreeArgs* pt, unsigned char state, unsigned int timestamp)
{
    PORT* port = bridge->ports[pt->portIndex];

    if (NULL == port)
    {
        return;
    }

    if (state == EDGE)
    {
        port->operEdge = true;
        port->isolate = false;
    }
    else if (state == NOT_EDGE)
    {
        port->operEdge = false;
        port->isolate = false;
    }
    else if (state == ISOLATED)
    {
        port->operEdge = false;
        port->isolate = true;
    }
    else
    {
        //assert(false);
    }
}

const struct StateMachine bd_stm =
{
    "BridgeDetection",
    &GetStateName,
    &CheckConditions,
    &InitState,
};
