// This file is part of the mstp-lib library, available at https://github.com/adigostin/mstp-lib
// Copyright (c) 2011-2020 Adi Gostin, distributed under Apache License v2.0.
//
// This header file is the entire library interface; you should
// have no need to include any other header file from the library.
//
// Find documentation for all STP functions in the "_help" directory in the source code tree.

#ifndef MSTP_LIB_H
#define MSTP_LIB_H

#ifndef STP_USE_LOG
#define STP_USE_LOG 0
#endif
#define MAX_STP_ITERATIONS_INSTANCE   (100)
#define MAX_STP_RUN_TIME              (2000) //2000ms
#define MAX_STP_ITERATIONS            (1000)
#define MSTP_NAME                     "mstp"
#define MSTP_MCID_DIGEST_LEN          (16)
#define MSTP_MCID_PADDING_LEN         (64)
#define MSTP_MAX_VID_NUM              (4096)
#define MSTP_INVALID_FID              (17)

#define HMAC_MD5_Init(context) ((void)0)
#define HMAC_MD5_Update(context, data, len) ((void)0)
#define HMAC_MD5_End(context) ((void)0)

STP_BRIDGE* STP_CreateBridge(unsigned int portCount,
                              unsigned int trunkCount,
                              unsigned int mstiCount,
                              unsigned int maxVlanNumber,
                              const unsigned char bridgeAddress[6]);
void STP_DestroyBridge(STP_BRIDGE* bridge);
void STP_StartBridge(STP_BRIDGE* bridge, unsigned int timestamp);
void STP_StopBridge(STP_BRIDGE* bridge, unsigned int timestamp);
bool STP_IsBridgeStarted(const STP_BRIDGE* bridge);

void STP_EnableLogging(STP_BRIDGE* bridge, bool enable);
bool STP_IsLoggingEnabled(const STP_BRIDGE* bridge);

unsigned int STP_GetPortCount(const STP_BRIDGE* bridge);
unsigned int STP_GetMstiCount(const STP_BRIDGE* bridge);

// ieee8021SpanningTreeVersion / dot1dStpVersion
STP_VERSION STP_GetStpVersion(const STP_BRIDGE* bridge);
void STP_SetStpVersion(STP_BRIDGE* bridge, STP_VERSION version, unsigned int timestamp);

// Call this when you receive a BPDU.
void STP_OnBpduReceived(STP_BRIDGE* bridge,
                            unsigned int portIndex,
                            const unsigned char* bpdu,
                            unsigned int bpduSize,
                            unsigned int timestamp);

// Call this every time the bridge's MAC address changes while STP is running.
void STP_SetBridgeAddress(STP_BRIDGE* bridge, const unsigned char* address, unsigned int timestamp);
const struct STP_BRIDGE_ADDRESS* STP_GetBridgeAddress(const STP_BRIDGE* bridge);

// Call these whenever one of the ports changes state (link up/down).
void STP_OnPortEnabled(STP_BRIDGE* bridge,
                        unsigned int portIndex,
                        unsigned int speedMegabitsPerSecond,
                        bool detectedPointToPointMAC,
                        unsigned int timestamp);
void STP_OnPortDisabled(STP_BRIDGE* bridge, unsigned int portIndex, unsigned int timestamp);

// Call this once a second.
void STP_OnOneSecondTick(STP_BRIDGE* bridge, unsigned int timestamp);

// ieee8021SpanningTreePriority / dot1dStpPriority (0-61440 in steps of 4096)
void            STP_SetBridgePriority(STP_BRIDGE* bridge,
                                    unsigned int treeIndex,
                                    unsigned short instanceId,
                                    unsigned short bridgePriority,
                                    unsigned int timestamp);
unsigned short STP_GetBridgePriority(const STP_BRIDGE* bridge, unsigned int treeIndex);

// ieee8021SpanningTreePortPriority / dot1dStpPortPriority (0-240 in steps of 16)
void          STP_SetPortPriority(STP_BRIDGE* bridge,
                                    unsigned int portIndex,
                                    unsigned int treeIndex,
                                    unsigned char portPriority,
                                    unsigned int timestamp);
unsigned char STP_GetPortPriority(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex);

unsigned short STP_GetPortIdentifier(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex);

// ieee8021SpanningTreeRstpPortAdminEdgePort / dot1dStpPortAdminEdgePort / ieee8021MstpCistPortAdminEdgePort
void STP_SetPortAdminEdge(STP_BRIDGE* bridge, unsigned int portIndex, bool adminEdge, unsigned int timestamp);
bool STP_GetPortAdminEdge(const STP_BRIDGE* bridge, unsigned int portIndex);

// ieee8021SpanningTreeRstpPortAutoEdgePort / ieee8021MstpCistPortAutoEdgePort
void STP_SetPortAutoEdge(STP_BRIDGE* bridge, unsigned int portIndex, bool autoEdge, unsigned int timestamp);
bool STP_GetPortAutoEdge(const STP_BRIDGE* bridge, unsigned int portIndex);

// ----------------------------------------------------------------------------

// ieee8021BridgeBasePortAdminPointToPoint / dot1dStpPortAdminPointToPoint
void STP_SetAdminPointToPointMAC(STP_BRIDGE* bridge,
                                    unsigned int portIndex,
                                    STP_ADMIN_P2P adminPointToPointMAC,
                                    unsigned int timestamp);
STP_ADMIN_P2P STP_GetAdminPointToPointMAC(const STP_BRIDGE* bridge, unsigned int portIndex);
bool STP_GetDetectedPointToPointMAC(const STP_BRIDGE* bridge, unsigned int portIndex);
// ieee8021BridgeBasePortOperPointToPoint / dot1dStpPortOperPointToPoint
bool STP_GetOperPointToPointMAC(const STP_BRIDGE* bridge, unsigned int portIndex);

// ----------------------------------------------------------------------------

// Returns the port path cost calculated from the link speed passed to STP_OnPortEnabled,
// which the library uses for path cost calculations while the admin cost is not set or
// after it is reset back to zero. Useful for troubleshooting.
unsigned int STP_GetDetectedPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex);

// dot1dStpPortPathCost / ieee8021SpanningTreePortPathCost / ieee8021MstpCistPortCistPathCost (ExternalPortPathCost)
// Returns the port path cost currently used for calculations (AdminExternalPortPathCost if non-zero, otherwise DetectedPortPathCost).
unsigned int STP_GetExternalPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex);

// dot1dStpPortAdminPathCost / ieee8021SpanningTreeRstpPortAdminPathCost / ieee8021MstpCistPortAdminPathCost
void        STP_SetAdminExternalPortPathCost(STP_BRIDGE* bridge,
                                            unsigned int portIndex,
                                            unsigned int adminPortPathCost,
                                            unsigned int debugTimestamp);
unsigned int STP_GetAdminExternalPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex);

// ieee8021MstpPortPathCost (13.27.33 in 802.1Q-2018)
// Returns the port path cost currently used for calculations (AdminInternalPortPathCost if non-zero, otherwise DetectedPortPathCost).
unsigned int STP_GetInternalPortPathCost(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned treeIndex);

// ieee8021MstpPortAdminPathCost (13.27.33 in 802.1Q-2018)
void         STP_SetAdminInternalPortPathCost(STP_BRIDGE* bridge,
                                                    unsigned int portIndex,
                                                    unsigned int treeIndex,
                                                    unsigned int adminInternalPortPathCost,
                                                    unsigned int debugTimestamp);
unsigned int STP_GetAdminInternalPortPathCost(const STP_BRIDGE* bridge,
                                                    unsigned int portIndex,
                                                    unsigned int treeIndex);

//TODO: ieee8021MstpCistPathCost, ieee8021MstpRootPathCost (path costs to the root bridge)

// ----------------------------------------------------------------------------

bool STP_GetPortEnabled(const STP_BRIDGE* bridge, unsigned int portIndex);
STP_PORT_ROLE STP_GetPortRole(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex);
bool STP_GetPortLearning(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex);
bool STP_GetPortForwarding(const STP_BRIDGE* bridge, unsigned int portIndex, unsigned int treeIndex);
bool STP_GetPortOperEdge(const STP_BRIDGE* bridge, unsigned int portIndex);

void STP_GetDefaultMstConfigName(const unsigned char bridgeAddress[6], char nameOut[18]);
void STP_SetMstConfigName(STP_BRIDGE* bridge, const char* name, unsigned int timestamp);
void STP_SetMstConfigRevisionLevel(STP_BRIDGE* bridge, unsigned short revisionLevel, unsigned int debugTimestamp);

typedef struct STP_CONFIG_TABLE_ENTRY_S
{
    unsigned char unused;
    unsigned char treeIndex; // 0=CIST, 1=MSTI1, 2=MSTI2...
}STP_CONFIG_TABLE_ENTRY;

void STP_SetMstConfigTable(STP_BRIDGE* bridge,
                                const STP_CONFIG_TABLE_ENTRY* entries,
                                unsigned int entryCount,
                                unsigned int timestamp);
void STP_SetMstConfigTableEntry(STP_BRIDGE* bridge,
                                unsigned int vlanNumber,
                                unsigned int treeIndex,
                                unsigned int timestamp);
const STP_CONFIG_TABLE_ENTRY* STP_GetMstConfigTable(STP_BRIDGE* bridge, unsigned int* entryCountOut);
unsigned char STP_GetMaxVlanNumber(const STP_BRIDGE* bridge);
unsigned int STP_GetTreeIndexFromVlanNumber(const STP_BRIDGE* bridge, unsigned int vlanNumber);
const STP_MST_CONFIG_ID* STP_GetMstConfigId(const STP_BRIDGE* bridge);

const char* STP_GetPortRoleString(STP_PORT_ROLE portRole);
const char* STP_GetVersionString(STP_VERSION version);
const char* STP_GetAdminP2PString(STP_ADMIN_P2P adminP2P);

void STP_GetRootPriorityVector(const STP_BRIDGE* bridge, unsigned int treeIndex, unsigned char priorityVectorOut[36]);
void STP_GetRootTimes(const STP_BRIDGE* bridge,
                       unsigned int treeIndex,
                       unsigned short* forwardDelayOutOrNull,
                       unsigned short* helloTimeOutOrNull,
                       unsigned short* maxAgeOutOrNull,
                       unsigned short* messageAgeOutOrNull,
                       unsigned char* remainingHopsOutOrNull);

bool STP_IsCistRoot(const STP_BRIDGE* bridge);
bool STP_IsRegionalRoot(const STP_BRIDGE* bridge, unsigned int treeIndex);

// ieee8021SpanningTreeBridgeHelloTime / dot1dStpBridgeHelloTime
void STP_SetBridgeHelloTime(STP_BRIDGE* bridge, unsigned int helloTime, unsigned int timestamp);
unsigned int STP_GetBridgeHelloTime(const STP_BRIDGE* bridge);

// ieee8021SpanningTreeHelloTime / dot1dStpHelloTime
unsigned int STP_GetHelloTime(const STP_BRIDGE* bridge);

// ieee8021SpanningTreeBridgeMaxAge / dot1dStpBridgeMaxAge
void STP_SetBridgeMaxAge(STP_BRIDGE* bridge, unsigned short maxAge, unsigned int timestamp);
unsigned short STP_GetBridgeMaxAge(const STP_BRIDGE* bridge);

// ieee8021SpanningTreeMaxAge / dot1dStpMaxAge
unsigned int STP_GetMaxAge(const STP_BRIDGE* bridge);

// ieee8021SpanningTreeBridgeForwardDelay / dot1dStpBridgeForwardDelay
void STP_SetBridgeForwardDelay(STP_BRIDGE* bridge, unsigned short forwardDelay, unsigned int timestamp);
unsigned short STP_GetBridgeForwardDelay(const STP_BRIDGE* bridge);
// ieee8021SpanningTreeForwardDelay / dot1dStpForwardDelay
unsigned short STP_GetForwardDelay(const STP_BRIDGE* bridge);

void STP_SetTxHoldCount(STP_BRIDGE* bridge, unsigned short txHoldCound, unsigned int timestamp);
unsigned short STP_GetTxHoldCount(const STP_BRIDGE* bridge);
unsigned int STP_GetTxCount(const STP_BRIDGE* bridge, unsigned int portIndex);

void  STP_SetApplicationContext(STP_BRIDGE* bridge, void* applicationContext);
void* STP_GetApplicationContext(const STP_BRIDGE* bridge);

#endif
