
// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.

#ifndef MSTP_LIB_PROCEDURES_H
#define MSTP_LIB_PROCEDURES_H

#ifdef AIR_SUPPORT_MSTP
#ifndef STP_LLC_SSAP
#define STP_LLC_SSAP 0x42
#endif
#ifndef STP_LLC_DSAP
#define STP_LLC_DSAP 0x42
#endif
#ifndef STP_LLC_CNTL
#define STP_LLC_CNTL 0x03
#endif

enum mstp_role {
    MSTP_ROLE_DISABLED,
    MSTP_ROLE_ROOT,
    MSTP_ROLE_DESIGNATED,
    MSTP_ROLE_ALTERNATE,
    MSTP_ROLE_BACKUP,
    MSTP_ROLE_MASTER,
};

enum mstp_state {
    MSTP_DISABLED,
    MSTP_LEARNING,
    MSTP_FORWARDING,
    MSTP_DISCARDING,
#ifdef AIR_SUPPORT_RSTP_SECURITY
    MSTP_ROOT_INC,
    MSTP_LOOP_INC,
    MSTP_GUARD_DOWN
#endif
};

#endif

bool betterorsameInfo(STP_BRIDGE*, PortIndex, TreeIndex, INFO_IS newInfoIs);
void clearAllRcvdMsgs(STP_BRIDGE*, PortIndex);
void clearReselectTree(STP_BRIDGE*, TreeIndex);
void disableForwarding(const STP_BRIDGE*, PortIndex, TreeIndex, unsigned int timestamp);
void disableLearning(const STP_BRIDGE*, PortIndex, TreeIndex, unsigned int timestamp);
void enableForwarding(const STP_BRIDGE*, PortIndex, TreeIndex, unsigned int timestamp);
void enableLearning(const STP_BRIDGE*, PortIndex, TreeIndex, unsigned int timestamp);
bool fromSameRegion(STP_BRIDGE*, PortIndex);
void newTcDetected(STP_BRIDGE*, PortIndex, TreeIndex);
void newTcWhile(STP_BRIDGE*, PortIndex, TreeIndex, unsigned int timestamp);
void pseudoRcvMsgs(STP_BRIDGE*, PortIndex);
RCVD_INFO rcvInfo(STP_BRIDGE*, PortIndex, TreeIndex);
void rcvMsgs(STP_BRIDGE*, PortIndex);
void rcvAgreements(STP_BRIDGE*, PortIndex);
void recordAgreement(STP_BRIDGE*, PortIndex, TreeIndex);
void recordDispute(STP_BRIDGE*, PortIndex, TreeIndex);
void recordMastered(STP_BRIDGE*, PortIndex, TreeIndex);
void recordPriority(STP_BRIDGE*, PortIndex, TreeIndex);
void recordProposal(STP_BRIDGE*, PortIndex, TreeIndex);
void recordTimes(STP_BRIDGE*, PortIndex, TreeIndex);
void setReRootTree(STP_BRIDGE*, TreeIndex);
void setSelectedTree(STP_BRIDGE*, TreeIndex);
void setSyncTree(STP_BRIDGE*, TreeIndex);
void setTcFlags(STP_BRIDGE*, PortIndex, TreeIndex);
void setTcPropTree(STP_BRIDGE*, PortIndex, TreeIndex);
void syncMaster(STP_BRIDGE*);
void txConfig(STP_BRIDGE*, PortIndex, unsigned int timestamp);
void txRstp(STP_BRIDGE*, PortIndex, unsigned int timestamp);
void txTcn(STP_BRIDGE*, PortIndex, unsigned int timestamp);
void updtAgreement(STP_BRIDGE*, PortIndex, TreeIndex);
void updtBPDUVersion(STP_BRIDGE*, PortIndex);
void updtDigest(STP_BRIDGE*, PortIndex);
void updtRcvdInfoWhile(STP_BRIDGE*, PortIndex, TreeIndex);
void updtRolesTree(STP_BRIDGE*, TreeIndex);
void updtRolesDisabledTree(STP_BRIDGE*, TreeIndex);

#ifdef AIR_SUPPORT_MSTP
void updtPortStpMode(unsigned int portIndex, unsigned char mode);
void updtCistPortRole(unsigned int portIndex, unsigned int treeIndex, unsigned char role);
void updtPortInstanceRole(STP_BRIDGE* bridge, unsigned int portIndex);
void updtPortDisableRole(STP_BRIDGE* bridge, unsigned int portIndex);
#endif

#endif
