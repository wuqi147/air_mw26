// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.32 from 802.1Q-2018
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
        case CHECKING_RSTP:     return "CHECKING_RSTP";
        case SELECTING_STP:     return "SELECTING_STP";
        case SENSING:           return "SENSING";
        default:                return "(undefined)";
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
        if (state == CHECKING_RSTP)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return CHECKING_RSTP;
    }
#ifdef AIR_SUPPORT_MSTP
    if (0 == state)
    {
        return CHECKING_RSTP;
    }
#endif

    // ------------------------------------------------------------------------
    // Check exit conditions from each state.

    if (state == CHECKING_RSTP)
    {
#ifdef AIR_SUPPORT_MSTP
        if ((port->mDelayWhile != bridge->MigrateTime) && !port->portEnabled)
        {
            return CHECKING_RSTP;
        }

        if (port->mDelayWhile == 0)
        {
            return SENSING;
        }
#else
        if (port->mDelayWhile == 0)
        {
            return SENSING;
        }

        if ((port->mDelayWhile != bridge->MigrateTime) && !port->portEnabled)
        {
            return CHECKING_RSTP;
        }
#endif
        return 0;
    }

    if (state == SELECTING_STP)
    {
        if ((port->mDelayWhile == 0) || !port->portEnabled || port->mcheck)
        {
            return SENSING;
        }

        return 0;
    }

    if (state == SENSING)
    {
#ifdef AIR_SUPPORT_MSTP
        if ((!port->portEnabled)
            || (port->mcheck)
            || ((rstpVersion(bridge) && (!port->sendRSTP) && (port->rcvdRSTP))))
        {
            return CHECKING_RSTP;
        }

        if (port->sendRSTP && port->rcvdSTP)
        {
            return SELECTING_STP;
        }
#else
        if (port->sendRSTP && port->rcvdSTP)
        {
            return SELECTING_STP;
        }

        if ((!port->portEnabled)
            || (port->mcheck)
            || ((rstpVersion(bridge) && (!port->sendRSTP) && (port->rcvdRSTP))))
        {
            return CHECKING_RSTP;
        }
#endif
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

    if (state == CHECKING_RSTP)
    {
        port->mcheck = false;
        port->sendRSTP = rstpVersion(bridge);
        port->mDelayWhile = bridge->MigrateTime;
    }
    else if (state == SELECTING_STP)
    {
        port->sendRSTP = false;
        port->mDelayWhile = bridge->MigrateTime;
    }
    else if (state == SENSING)
    {
        port->rcvdRSTP = port->rcvdSTP = false;
    }
    else
    {
        //assert(false);
    }

#ifdef AIR_SUPPORT_MSTP
    updtPortStpMode(pt->portIndex, port->sendRSTP);
#endif
}

const struct StateMachine ppm_stm =
{
    "PortProtocolMigration",
    &GetStateName,
    &CheckConditions,
    &InitState
};
