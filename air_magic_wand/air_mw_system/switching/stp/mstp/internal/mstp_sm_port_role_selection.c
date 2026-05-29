// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.36 from 802.1Q-2018.
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
        case INIT_TREE:     return "INIT_TREE";
        case ROLE_SELECTION:return "ROLE_SELECTION";
        default:            return "(undefined)";
    }
}

// ============================================================================

// Returns the new state, or 0 when no transition is to be made.
static unsigned char CheckConditions(const STP_BRIDGE* bridge, PortTreeArgs* pt, unsigned char state)
{
    unsigned int portIndex;

    // ------------------------------------------------------------------------
    // Check global conditions.

    if (bridge->BEGIN)
    {
        if (state == INIT_TREE)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return INIT_TREE;
    }

#ifdef AIR_SUPPORT_MSTP
    if (0 == state)
    {
        return INIT_TREE;
    }
#endif

    // ------------------------------------------------------------------------
    // Check exit conditions from each state.

    if (state == INIT_TREE)
    {
        return ROLE_SELECTION;
    }

    if (state == ROLE_SELECTION)
    {
        for (portIndex = 0; portIndex < bridge->portCount; portIndex++)
        {
            if (NULL == bridge->ports[portIndex])
            {
                continue;
            }
            if (NULL == bridge->ports[portIndex]->trees[pt->treeIndex])
            {
                continue;
            }
            if (bridge->ports[portIndex]->trees[pt->treeIndex]->reselect)
            {
                return ROLE_SELECTION;
            }
        }

        return 0;
    }

    //assert (false);
    return 0;
}

// ============================================================================

static void InitState(STP_BRIDGE* bridge, PortTreeArgs* pt, unsigned char state, unsigned int timestamp)
{
    if (state == INIT_TREE)
    {
        updtRolesDisabledTree(bridge, pt->treeIndex);
    }
    else if (state == ROLE_SELECTION)
    {
        clearReselectTree(bridge, pt->treeIndex);
        updtRolesTree(bridge, pt->treeIndex);
        setSelectedTree(bridge, pt->treeIndex);
    }
    else
    {
        //assert(false);
    }
}

const struct StateMachine prs_stm =
{
    "PortRoleSelection",
    &GetStateName,
    &CheckConditions,
    &InitState
};
