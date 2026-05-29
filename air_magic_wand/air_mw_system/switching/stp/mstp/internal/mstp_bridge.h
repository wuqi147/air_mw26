// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

// This file implements "13.26 Per Bridge variables" from 802.1Q-2018.

#ifndef MSTP_LIB_BRIDGE_H
#define MSTP_LIB_BRIDGE_H

#include "mstp_bpdu.h"

typedef struct BRIDGE_TREE_S
{
    // There is one instance per bridge of each of the following for the CIST, and one for each MSTI.
    BRIDGE_ID               BridgeIdentifier;   // 13.26.e) - 13.26.2
    Port_ID                 rootPortId;         // 13.26.h) - 13.26.9
    PRIORITY_VECTOR         BridgePriority;     // 13.26.f) - 13.26.3
    PRIORITY_VECTOR         rootPriority;       // 13.26.i) - 13.26.10

    TIMES                   BridgeTimes;        // 13.26.g) - 13.26.4
    TIMES                   rootTimes;          // 13.26.j) - 13.26.11

    unsigned char           portRoleSelectionState;
    unsigned short          instance;
    unsigned int            memBmp;
    unsigned char           pad;
}__attribute__((packed))BRIDGE_TREE;

typedef struct STP_BRIDGE_S
{
    BRIDGE_TREE**   trees;
    PORT**          ports;

    unsigned short* mstConfigTable; /* Do not use it */

    // This variable is supposed to be be accessed only while a received BPDU is being handled.
    // When there's no received BPDU, we set it to the invalid value NULL, to cause a crash on access and signal the programming error early.
    // (Note that the crash won't happen on some microcontrollers for which address 0 is
    //  readable/writeable, that's why we also have asserts all around the place).
    MSTP_BPDU*              receivedBpduContent;
    VALIDATED_BPDU_TYPE     receivedBpduType;
    PORT*                   receivedBpduPort;

#if STP_USE_LOG
    static const unsigned int LogIndentSize = 2;
    char* logBuffer;
    unsigned int logBufferMaxSize;
    unsigned int logBufferUsedSize;
    unsigned int logIndent;
    bool logLineStarting;
    bool loggingEnabled;
    int logCurrentPort;
    int logCurrentTree;
#endif
    // 13.26 Per bridge variables
    // There is one instance per bridge component of the following variable(s):
    STP_VERSION          ForceProtocolVersion;      // 13.26.a) - 13.26.5
    unsigned int         TxHoldCount;               // 13.26.b) - 13.26.12
    unsigned short       MigrateTime;               // 13.26.c) - 13.26.6
    unsigned char        portCount;
    unsigned char        treeCount;
    STP_MST_CONFIG_ID    MstConfigId;               // 13.26.d) - 13.26.7

    bool BEGIN; // Defined in 13.23.1 in 802.1Q-2005. Widely used but definition was removed subsequent versions of the standard.
    bool started; // Added by me. STP_StartBridge sets it, STP_StopBridge clears it.

    //STP_CALLBACKS callbacks;
    unsigned char  maxVlanNumber;
    unsigned char  pad;


    // The above parameters ((a) through (d)) are not modified by the operation of the spanning tree protocols, but
    // are treated as constants by the state machines. If ForceProtocolVersion or MSTConfigId are modified by
    // management, BEGIN shall be asserted for all state machines.

    // If the ISIS-SPB is implemented there is one instance per Bridge component, of the following variable(s),
    // with that single instance supporting all SPTs:
    // XXX agreementDigest; // 13.26.k) - 13.26.1
    // XXX AuxMstConfigId;  // 13.26.l) - 13.26.8

    //void* applicationContext;
}__attribute__((packed))STP_BRIDGE;
#endif
