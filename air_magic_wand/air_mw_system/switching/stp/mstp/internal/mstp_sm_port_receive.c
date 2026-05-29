// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.31 from 802.1Q-2018.
#include "mstp_base_types.h"
#include "mstp_port.h"
#include "mstp_bridge.h"
#include "mstp_procedures.h"
#include "mstp_conditions_and_params.h"
#include "mstp_sm.h"
#include <assert.h>

static const char* GetStateName(unsigned char state)
{
    switch (state)
    {
        case PR_DISCARD:   return "DISCARD";
        case PR_RECEIVE:   return "RECEIVE";
        default:           return "(undefined)";
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

    if ((bridge->BEGIN)
        || ((port->rcvdBpdu || (port->edgeDelayWhile != bridge->MigrateTime)) && !port->portEnabled))
    {
        if (state == PR_DISCARD)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return PR_DISCARD;
    }
#ifdef AIR_SUPPORT_MSTP
    if (0 == state)
    {
        return PR_DISCARD;
    }
#endif
    // ------------------------------------------------------------------------
    // Check exit conditions from each state.

    if (state == PR_DISCARD)
    {
        if ((port->rcvdBpdu) && (port->portEnabled) && (port->enableBPDUrx))
        {
            return PR_RECEIVE;
        }

        return 0;
    }

    if (state == PR_RECEIVE)
    {
        if ((port->rcvdBpdu) && (port->portEnabled) && (port->enableBPDUrx) && (!rcvdAnyMsg(bridge, pt->portIndex)))
        {
            return PR_RECEIVE;
        }

        return 0;
    }

    //assert(false);
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

    if (state == PR_DISCARD)
    {
        port->rcvdBpdu = port->rcvdRSTP = port->rcvdSTP = false;
#ifndef AIR_SUPPORT_MSTP
        port->agreedMisorder = true;
        port->agreedN = port->agreedND = port->agreeND = 0;
        port->agreeN = 1;
#endif
        clearAllRcvdMsgs (bridge, pt->portIndex);
        port->edgeDelayWhile = bridge->MigrateTime;
    }
    else if (state == PR_RECEIVE)
    {
        updtBPDUVersion (bridge, pt->portIndex);
        port->rcvdInternal = fromSameRegion(bridge, pt->portIndex);
        rcvMsgs(bridge, pt->portIndex);
        port->operEdge = port->isolate = port->rcvdBpdu = false;
        port->edgeDelayWhile = bridge->MigrateTime;
    }
    else
    {
        //assert(false);
    }
}

const struct StateMachine pr_stm =
{
    "PortReceive",
    &GetStateName,
    &CheckConditions,
    &InitState
};

