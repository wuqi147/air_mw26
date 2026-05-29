// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.38 from 802.1Q-2018.
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
        case PST_DISCARDING:    return "DISCARDING";
        case PST_LEARNING:      return "LEARNING";
        case PST_FORWARDING:    return "FORWARDING";
        default:                return "(undefined)";
    }
}

// ============================================================================

// Returns the new state, or 0 when no transition is to be made.
static unsigned char CheckConditions(const STP_BRIDGE* bridge, PortTreeArgs* pt, unsigned char state)
{
    PortIndex givenPort = pt->portIndex;
    TreeIndex givenTree = pt->treeIndex;

    PORT* port = bridge->ports[givenPort];
    PORT_TREE* tree;

    if (NULL == port)
    {
        return 0;
    }
    tree = port->trees[givenTree];
    if (NULL == tree)
    {
        return 0;
    }

    // ------------------------------------------------------------------------
    // Check global conditions.

    if (bridge->BEGIN)
    {
        if (state == PST_DISCARDING)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return PST_DISCARDING;
    }
#ifdef AIR_SUPPORT_MSTP
    if (0 == state)
    {
        return PST_DISCARDING;
    }
#endif

    // ------------------------------------------------------------------------
    // Check exit conditions from each state.

    if (state == PST_DISCARDING)
    {
        if (tree->learn)
        {
            return PST_LEARNING;
        }
#ifdef AIR_SUPPORT_MSTP
        disableForwarding(bridge, givenPort, givenTree, 0);
#endif
        return 0;
    }

    if (state == PST_LEARNING)
    {
        if (!tree->learn)
        {
            return PST_DISCARDING;
        }

        if (tree->forward)
        {
            return PST_FORWARDING;
        }

        return 0;
    }

    if (state == PST_FORWARDING)
    {
        if (!tree->forward)
        {
            return PST_DISCARDING;
        }

        return 0;
    }

    //assert(false);
    return 0;
}

// ============================================================================

static void InitState(STP_BRIDGE* bridge, PortTreeArgs* pt, unsigned char state, unsigned int timestamp)
{
    PortIndex givenPort = pt->portIndex;
    TreeIndex givenTree = pt->treeIndex;

    PORT* port = bridge->ports[givenPort];
    PORT_TREE* tree;

    if (NULL == port)
    {
        return;
    }
    tree = port->trees[givenTree];
    if (NULL == tree)
    {
        return;
    }

    if (state == PST_DISCARDING)
    {
#if 0
        /* port stp status is disable, port state is forward */
        if (!port->is_administrative_bridge_port)
        {
            tree->learning = false;
            tree->forwarding = false;
            enableForwarding(bridge, givenPort, givenTree, timestamp);
            return;
        }
#endif
        disableLearning(bridge, givenPort, givenTree, timestamp);
        tree->learning = false;
        disableForwarding(bridge, givenPort, givenTree, timestamp);
        tree->forwarding = false;
    }
    else if (state == PST_LEARNING)
    {
        enableLearning(bridge, givenPort, givenTree, timestamp);
        tree->learning = true;
    }
    else if (state == PST_FORWARDING)
    {
        enableForwarding(bridge, givenPort, givenTree, timestamp);
        tree->forwarding = true;
    }
    else
    {
        //assert(false);
    }
}

const struct StateMachine pst_stm =
{
    "PortStateTransition",
    &GetStateName,
    &CheckConditions,
    &InitState
};

