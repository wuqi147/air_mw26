// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements 13.34 from 802.1Q-2018.
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
        case TRANSMIT_INIT:     return "TRANSMIT_INIT";
        case TRANSMIT_PERIODIC: return "TRANSMIT_PERIODIC";
        case TRANSMIT_CONFIG:   return "TRANSMIT_CONFIG";
        case TRANSMIT_TCN:      return "TRANSMIT_TCN";
        case TRANSMIT_RSTP:     return "TRANSMIT_RSTP";
        case AGREE_SPT:         return "AGREE_SPT";
        case IDLE:              return "IDLE";
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

    if (bridge->BEGIN || !port->portEnabled || !port->enableBPDUtx)
    {
        if (state == TRANSMIT_INIT)
        {
            // The entry block for this state has been executed already.
            return 0;
        }

        return TRANSMIT_INIT;
    }
#ifdef AIR_SUPPORT_MSTP
    if (0 == state)
    {
        return TRANSMIT_INIT;
    }
#endif

    // ------------------------------------------------------------------------
    // Check exit conditions from each state.

    if (state == TRANSMIT_INIT)
    {
        return IDLE;
    }

    if (state == TRANSMIT_PERIODIC)
    {
        return IDLE;
    }

    if (state == TRANSMIT_CONFIG)
    {
        return IDLE;
    }

    if (state == TRANSMIT_TCN)
    {
        return IDLE;
    }

    if (state == TRANSMIT_RSTP)
    {
        return IDLE;
    }

    if (state == AGREE_SPT)
    {
        return IDLE;
    }

    if (state == IDLE)
    {
        if (allTransmitReady(bridge, pt->portIndex))
        {
            if (port->helloWhen == 0)
            {
                return TRANSMIT_PERIODIC;
            }

            if ((!port->sendRSTP) && (port->newInfo) && (cistDesignatedPort (bridge, pt->portIndex))
                    && (port->txCount < bridge->TxHoldCount) && (port->helloWhen != 0))
            {
                return TRANSMIT_CONFIG;
            }

            if ((!port->sendRSTP) && (port->newInfo) && (cistRootPort(bridge, pt->portIndex))
                && (port->txCount < bridge->TxHoldCount) && (port->helloWhen != 0))
            {
                return TRANSMIT_TCN;
            }

            if ((port->sendRSTP) && ((port->newInfo) || ((port->newInfoMsti) && !mstiMasterPort (bridge, pt->portIndex)))
                    && (port->txCount < bridge->TxHoldCount) && (port->helloWhen != 0))
            {
                return TRANSMIT_RSTP;
            }

#ifndef AIR_SUPPORT_MSTP
            if (spt(bridge) && (port->sendRSTP) && (allSptAgree(bridge)) && (!port->agreeDigestValid))
#else
            if (spt(bridge) && (port->sendRSTP) && (allSptAgree(bridge)))
#endif
            {
                return AGREE_SPT;
            }
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

    if (state == TRANSMIT_INIT)
    {
        port->newInfo = port->newInfoMsti = true;
        port->txCount = 0;
    }
    else if (state == TRANSMIT_PERIODIC)
    {
        // Note AG: Not clear in the standard: tcWhile of which tree? I'll assume they meant "CIST's tcWhile", since the whole expression is about the CIST.
        port->newInfo = (port->newInfo) || (cistDesignatedPort(bridge, pt->portIndex)
                        || (cistRootPort (bridge, pt->portIndex) && (port->trees[CIST_INDEX]->tcWhile != 0)));

        port->newInfoMsti = port->newInfoMsti || mstiDesignatedOrTCpropagatingRootPort(bridge, pt->portIndex);
    }
    else if (state == TRANSMIT_CONFIG)
    {
        port->newInfo = false;
        txConfig(bridge, (pt->portIndex+1), timestamp);
        port->txCount += 1;
        port->tcAck = false;
    }
    else if (state == TRANSMIT_TCN)
    {
        port->newInfo = false;
        txTcn(bridge, (pt->portIndex+1), timestamp);
        port->txCount += 1;
    }
    else if (state == TRANSMIT_RSTP)
    {
        port->newInfo = port->newInfoMsti = false;
        txRstp(bridge, (pt->portIndex+1), timestamp);
        port->txCount += 1;
        port->tcAck = false;
    }
    else if (state == AGREE_SPT)
    {
#ifndef AIR_SUPPORT_MSTP
        port->agreeDigestValid = true;
#endif
        port->newInfoMsti = true;
    }
    else if (state == IDLE)
    {
        port->helloWhen = HelloTime(bridge, pt->portIndex);
    }
    else
    {
        //assert(false);
    }
}


const struct StateMachine pt_stm =
{
    "PortTransmit",
    &GetStateName,
    &CheckConditions,
    &InitState
};

