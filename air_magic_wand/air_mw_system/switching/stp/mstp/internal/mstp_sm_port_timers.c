// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.30 from 802.1Q-2018.
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
        case ONE_SECOND:    return "ONE_SECOND";
        case TICK:          return "TICK";
        default:            return "(undefined)";
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

    if (bridge->BEGIN)
    {
        if (state == ONE_SECOND)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return ONE_SECOND;
    }
#ifdef AIR_SUPPORT_MSTP
    if (0 == state)
    {
        return ONE_SECOND;
    }
#endif

    // ------------------------------------------------------------------------
    // Check exit conditions from each state.

    if (state == ONE_SECOND)
    {
        if (port->tick)
        {
            return TICK;
        }

        return 0;
    }

    if (state == TICK)
    {
        return ONE_SECOND;
    }

    //assert(false);
    return 0;
}

// ============================================================================

static void InitState(STP_BRIDGE* bridge, PortTreeArgs* pt, unsigned char state, unsigned int timestamp)
{
    PORT* port = bridge->ports[pt->portIndex];
    unsigned int treeIndex;
    PORT_TREE* portTree;

    if (NULL == port)
    {
        return;
    }

    if (state == ONE_SECOND)
    {
        port->tick = false;
    }
    else if (state == TICK)
    {
        if (port->helloWhen      > 0) port->helloWhen--;
        if (port->mDelayWhile    > 0) port->mDelayWhile--;
        if (port->edgeDelayWhile > 0) port->edgeDelayWhile--;
        if (port->txCount        > 0) port->txCount--;
        //if (port->pseudoInfoHelloWhen > 0) port->pseudoInfoHelloWhen--;

        for (treeIndex = 0; treeIndex < bridge->treeCount; treeIndex++)
        {
            portTree = port->trees[treeIndex];
            if (NULL == portTree)
            {
                continue;
            }

            if (portTree->tcWhile       > 0) portTree->tcWhile--;
            if (portTree->fdWhile       > 0) portTree->fdWhile--;
            if (portTree->rcvdInfoWhile > 0) portTree->rcvdInfoWhile--;
            if (portTree->rrWhile       > 0) portTree->rrWhile--;
            if (portTree->tcDetected    > 0) portTree->tcDetected--;
            if (portTree->rbWhile       > 0) portTree->rbWhile--;
        }
    }
}

const struct StateMachine ptimer_stm =
{
    "PortTimers",
    &GetStateName,
    &CheckConditions,
    &InitState
};

