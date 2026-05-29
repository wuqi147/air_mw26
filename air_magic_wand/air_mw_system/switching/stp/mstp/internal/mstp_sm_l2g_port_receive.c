// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.40 from 802.1Q-2018.
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
        case L2G_INIT:           return "INIT";
        case L2G_PSEUDO_RECEIVE: return "PSEUDO_RECEIVE";
        case L2G_DISCARD:        return "DISCARD";
        case L2G_L2GP:           return "L2GP";
        default:                 return "(undefined)";
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

    if ((bridge->BEGIN) || (!port->isL2gp) || (!port->portEnabled))
    {
        if (state == L2G_INIT)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return L2G_INIT;
    }
#ifdef AIR_SUPPORT_MSTP
    if (0 == state)
    {
        return L2G_INIT;
    }
#endif

    // ------------------------------------------------------------------------
    // Check exit conditions from each state.

    if (state == L2G_INIT)
    {
        return L2G_L2GP;
    }

    if (state == L2G_PSEUDO_RECEIVE)
    {
        return L2G_L2GP;
    }

    if (state == L2G_DISCARD)
    {
        return L2G_L2GP;
    }

    if (state == L2G_L2GP)
    {
        if (!port->enableBPDUrx && port->rcvdBpdu && !rcvdAnyMsg(bridge, pt->portIndex))
        {
            return L2G_DISCARD;
        }
#ifndef AIR_SUPPORT_MSTP
        if ((port->pseudoInfoHelloWhen == 0) && !rcvdAnyMsg(bridge, pt->portIndex))
        {
            return L2G_PSEUDO_RECEIVE;
        }
#endif
        return 0;
    }

    //assert(false);
    return 0;
}

static void InitState(STP_BRIDGE* bridge, PortTreeArgs* pt, unsigned char state, unsigned int timestamp)
{
    PORT* port = bridge->ports[pt->portIndex];

    if (NULL == port)
    {
        return;
    }

    if (state == L2G_INIT)
    {
#ifndef AIR_SUPPORT_MSTP
        port->pseudoInfoHelloWhen = 0;
#endif
    }
    else if (state == L2G_PSEUDO_RECEIVE)
    {
        port->rcvdInternal = true;
        pseudoRcvMsgs(bridge, pt->portIndex);
        port->edgeDelayWhile = bridge->MigrateTime;
#ifndef AIR_SUPPORT_MSTP
        port->pseudoInfoHelloWhen = HelloTime(bridge, pt->portIndex);
#endif
    }
    else if (state == L2G_DISCARD)
    {
        port->rcvdBpdu = false;
    }
    else if (state == L2G_L2GP)
    {
        //noting
    }
    else
    {
        //assert (false);
    }
}

const struct StateMachine l2g_stm =
{
    "L2GPortReceive",
    &GetStateName,
    &CheckConditions,
    &InitState
};
