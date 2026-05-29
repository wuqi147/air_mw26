// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

#ifndef MSTP_LIB_STP_SM_H
#define MSTP_LIB_STP_SM_H

typedef struct PortAndTree
{
    PortIndex portIndex;
    TreeIndex treeIndex;
}PortTreeArgs;

struct StateMachine
{
    const char* smName;
    const char* (*getStateName) (unsigned char state);
    unsigned char (*checkConditions) (const STP_BRIDGE* bridge, PortTreeArgs* portTreeArgs, unsigned char state);
    void (*initState) (STP_BRIDGE* bridge, PortTreeArgs* portTreeArgs, unsigned char state, unsigned int timestamp);
};

enum STP_FLUSH_FDB_TYPE
{
    STP_FLUSH_FDB_TYPE_IMMEDIATE,
    STP_FLUSH_FDB_TYPE_RAPID_AGEING,
};

enum TC_State {
    ACTIVE = 1,
    INACTIVE,
    LEARNING,
    DETECTED,
    NOTIFIED_TCN,
    NOTIFIED_TC,
    PROPAGATING,
    ACKNOWLEDGED,
};

enum PTIMER_State {
    ONE_SECOND = 1,
    TICK,
};

enum PPM_State {
    CHECKING_RSTP = 1,
    SELECTING_STP,
    SENSING,
};

enum PR_State {
    PR_DISCARD = 1,
    PR_RECEIVE,
};

enum BD_State {
    NOT_EDGE = 1,
    EDGE,
    ISOLATED,
};

enum PI_State {
    DISABLED = 1,
    AGED,
    UPDATE,
    SUPERIOR_DESIGNATED,
    REPEATED_DESIGNATED,
    INFERIOR_DESIGNATED,
    NOT_DESIGNATED,
    OTHER,
    PI_CURRENT,
    RECEIVE,
};

enum PRS_State {
    INIT_TREE = 1,
    ROLE_SELECTION,
};

enum PRT_State {
    INIT_PORT = 1,
    DISABLE_PORT,
    DISABLED_PORT,

    MASTER_PORT,
    MASTER_PROPOSED,
    MASTER_AGREED,
    MASTER_SYNCED,
    MASTER_RETIRED,
    MASTER_FORWARD,
    MASTER_LEARN,
    MASTER_DISCARD,

    ROOT_PORT,
    ROOT_PROPOSED,
    ROOT_AGREED,
    ROOT_SYNCED,
    REROOT,
    ROOT_FORWARD,
    ROOT_LEARN,
    REROOTED,
    ROOT_DISCARD,

    DESIGNATED_PROPOSE,
    DESIGNATED_AGREE,
    DESIGNATED_SYNCED,
    DESIGNATED_RETIRED,
    DESIGNATED_FORWARD,
    DESIGNATED_LEARN,
    DESIGNATED_DISCARD,
    DESIGNATED_PORT,

    ALTERNATE_PROPOSED,
    ALTERNATE_AGREED,
    BLOCK_PORT,
    BACKUP_PORT,
    ALTERNATE_PORT,
};

enum PST_State {
    PST_DISCARDING = 1,
    PST_LEARNING,
    PST_FORWARDING,
};

enum L2G_State {
    L2G_INIT = 1,
    L2G_PSEUDO_RECEIVE,
    L2G_DISCARD,
    L2G_L2GP,
};

enum PT_State {
    TRANSMIT_INIT = 1,
    TRANSMIT_PERIODIC,
    TRANSMIT_CONFIG,
    TRANSMIT_TCN,
    TRANSMIT_RSTP,
    AGREE_SPT,
    IDLE,
};

// ============================================================================

const struct StateMachine bd_stm;
const struct StateMachine l2g_stm;
const struct StateMachine pi_stm;
const struct StateMachine ppm_stm;
const struct StateMachine pr_stm;
const struct StateMachine prs_stm;
const struct StateMachine prt_stm;
const struct StateMachine pst_stm;
const struct StateMachine ptimer_stm;
const struct StateMachine pt_stm;
const struct StateMachine tc_stm;

#endif
