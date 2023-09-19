/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef _WANMGR_DHCPV4_APIS_H_
#define _WANMGR_DHCPV4_APIS_H_
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include "wanmgr_dml.h"
#include "wanmgr_rdkbus_apis.h"
#include "wanmgr_rdkbus_common.h"
#include "wanmgr_rdkbus_utils.h"
#include "wanmgr_interface_sm.h"

#include "ansc_platform.h"
#include "ipc_msg.h"

#ifdef DHCPV4_CLIENT_TI_UDHCPC 
#define DHCPV4_CLIENT_NAME "ti_udhcpc"
#else
#define DHCPV4_CLIENT_NAME "udhcpc"
#endif
#define DHCPV4_ACTION_HANDLER "service_udhcpc"
#define  DML_ALIAS_NAME_LENGTH                 64

/**
 * @brief API to process DHCP state change event message.
 * @param msg - Pointer to msg_payload_t structure contains Dhcpv4 configuration as part of ipc message
 * @return ANSC_STATUS_SUCCESS upon success else error code returned.
 */
ANSC_STATUS wanmgr_handle_dhcpv4_event_data(DML_VIRTUAL_IFACE* pVirtIf);
ANSC_STATUS IPCPStateChangeHandler (DML_VIRTUAL_IFACE* pVirtIf);
void WanMgr_UpdateIpFromCellularMgr (WanMgr_IfaceSM_Controller_t* pWanIfaceCtrl);

/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/

#define  DML_DHCP_MAX_ENTRIES                  4
#define  DML_DHCP_MAX_RESERVED_ADDRESSES       8
#define  DML_DHCP_MAX_OPT_ENTRIES              8

#define _DEBUG_DHCPV4
#ifdef _DEBUG_DHCPV4
    #define ULOGF ulogf
#else   
    #define ULOGF
#endif

typedef  enum
_DML_DHCP_STATUS
{
    DML_DHCP_STATUS_Disabled               = 1,
    DML_DHCP_STATUS_Enabled,
    DML_DHCP_STATUS_Error_Misconfigured,
    DML_DHCP_STATUS_Error
}
DML_DHCP_STATUS, *PDML_DHCP_STATUS;


typedef  enum
_DML_DHCPC_STATUS
{
    DML_DHCPC_STATUS_Init                  = 1,
    DML_DHCPC_STATUS_Selecting,
    DML_DHCPC_STATUS_Requesting,
    DML_DHCPC_STATUS_Rebinding,
    DML_DHCPC_STATUS_Bound,
    DML_DHCPC_STATUS_Renewing
}
DML_DHCPC_STATUS, *PDML_DHCPC_STATUS;


typedef  struct
_COSA_DML_DHCP_OPT
{
    ULONG                           InstanceNumber;
    char                            Alias[512];

    BOOLEAN                         bEnabled;
    UCHAR                           Tag;
    UCHAR                           Value[255];
}
COSA_DML_DHCP_OPT,  *PCOSA_DML_DHCP_OPT;


/*
 *  DHCP Client
 */
typedef  struct
_DML_DHCPC_CFG
{
    ULONG                           InstanceNumber;
    char                            Alias[DML_ALIAS_NAME_LENGTH];

    BOOLEAN                         bEnabled;
    char                            Interface[DML_ALIAS_NAME_LENGTH]; /* IP interface name */
    BOOLEAN                         PassthroughEnable;
    char                            PassthroughDHCPPool[64];            /* DHCP server pool alias */
    char                            X_CISCO_COM_BootFileName[256];
}
DML_DHCPC_CFG,  *PDML_DHCPC_CFG;


typedef  struct
_DML_DHCPC_INFO
{
    DML_DHCP_STATUS                 Status;
    DML_DHCPC_STATUS                DHCPStatus;
    ANSC_IPV4_ADDRESS               IPAddress;
    ANSC_IPV4_ADDRESS               SubnetMask;
    ULONG                           NumIPRouters;
    ANSC_IPV4_ADDRESS               IPRouters[DML_DHCP_MAX_ENTRIES];
    ULONG                           NumDnsServers;
    ANSC_IPV4_ADDRESS               DNSServers[DML_DHCP_MAX_ENTRIES];
    int                             LeaseTimeRemaining;
    ANSC_IPV4_ADDRESS               DHCPServer;
}
DML_DHCPC_INFO,  *PDML_DHCPC_INFO;


typedef  struct
_DML_DHCPC_FULL
{
    DML_DHCPC_CFG              Cfg;
    DML_DHCPC_INFO             Info;
}
DML_DHCPC_FULL, *PDML_DHCPC_FULL;


typedef  struct
_DML_DHCPC_REQ_OPT
{
    ULONG                           InstanceNumber;
    char                            Alias[DML_ALIAS_NAME_LENGTH];

    BOOLEAN                         bEnabled;
    ULONG                           Order;
    UCHAR                           Tag;
    UCHAR                           Value[255];
}
DML_DHCPC_REQ_OPT,  *PDML_DHCPC_REQ_OPT;

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/

ANSC_STATUS
WanMgr_DmlDhcpInit
    (
        ANSC_HANDLE                 hDml,
        PANSC_HANDLE                phContext
    );

/*
 *  DHCP Client
 */
ULONG
WanMgr_DmlDhcpcGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    );

ANSC_STATUS
WanMgr_DmlDhcpcGetEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        PDML_DHCPC_FULL        pEntry
    );

ANSC_STATUS
WanMgr_DmlDhcpcSetValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    );

ANSC_HANDLE
WanMgr_DmlDhcpcAddEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber
    );

ANSC_STATUS
WanMgr_DmlDhcpcDelEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber
    );

ANSC_STATUS
WanMgr_DmlDhcpcSetCfg
    (
        ANSC_HANDLE                 hContext,
        PDML_DHCPC_CFG         pCfg        /* Identified by InstanceNumber */
    );

ANSC_STATUS
WanMgr_DmlDhcpcGetCfg
    (
        ANSC_HANDLE                 hContext,
        PDML_DHCPC_CFG         pCfg        /* Identified by InstanceNumber */
    );

ANSC_STATUS
WanMgr_DmlDhcpcGetInfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PDML_DHCPC_INFO        pInfo
    );

ANSC_STATUS
WanMgr_DmlDhcpcRenew
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber
    );

/*
 *  DHCP Client Send/Req Option
 *
 *  The options are managed on top of a DHCP client,
 *  which is identified through pClientAlias
 */
ULONG
WanMgr_DmlDhcpcGetNumberOfSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber
    );

ANSC_STATUS
WanMgr_DmlDhcpcGetSentOptionbyInsNum
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PCOSA_DML_DHCP_OPT          pEntry
    );

ANSC_STATUS
WanMgr_DmlDhcpcGetSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulIndex,
        PCOSA_DML_DHCP_OPT          pEntry
    );

ANSC_STATUS
WanMgr_DmlDhcpcSetSentOptionValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    );

ANSC_STATUS
WanMgr_DmlDhcpcAddSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PCOSA_DML_DHCP_OPT          pEntry
    );

ANSC_STATUS
WanMgr_DmlDhcpcDelSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulInstanceNumber
    );

ANSC_STATUS
WanMgr_DmlDhcpcSetSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PCOSA_DML_DHCP_OPT          pEntry        /* Identified by InstanceNumber */
    );

ULONG
WanMgr_DmlDhcpcGetNumberOfReqOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber
    );

ANSC_STATUS
WanMgr_DmlDhcpcGetReqOptionbyInsNum
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PDML_DHCPC_REQ_OPT     pEntry
    );

ANSC_STATUS
WanMgr_DmlDhcpcGetReqOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulIndex,
        PDML_DHCPC_REQ_OPT     pEntry
    );

ANSC_STATUS
WanMgr_DmlDhcpcSetReqOptionValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    );

ANSC_STATUS
WanMgr_DmlDhcpcAddReqOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PDML_DHCPC_REQ_OPT     pEntry
    );

ANSC_STATUS
WanMgr_DmlDhcpcDelReqOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulInstanceNumber
    );

ANSC_STATUS
WanMgr_DmlDhcpcSetReqOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PDML_DHCPC_REQ_OPT     pEntry        /* Identified by InstanceNumber */
    );



#endif //_WANMGR_DHCPV4_APIS_H_
