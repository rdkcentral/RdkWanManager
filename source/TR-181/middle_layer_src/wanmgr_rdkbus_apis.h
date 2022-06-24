/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
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


#ifndef  _WANMGR_RDKBUS_APIS_H_
#define  _WANMGR_RDKBUS_APIS_H_

#include "wanmgr_apis.h"
#include "ccsp_psm_helper.h"
#include "ansc_platform.h"
#include "ansc_string_util.h"
#include "wanmgr_dml.h"
#include "wanmgr_data.h"

//PPP Manager
#define PPPMGR_COMPONENT_NAME       "eRT.com.cisco.spvtg.ccsp.pppmanager"
#define PPPMGR_DBUS_PATH            "/com/cisco/spvtg/ccsp/pppmanager"

#define PPP_INTERFACE_TABLE          "Device.PPP.Interface."
#define PPP_INTERFACE_INSTANCE       "Device.PPP.Interface.%d."
#define PPP_INTERFACE_ENABLE         "Device.PPP.Interface.%d.Enable"
#define PPP_INTERFACE_ALIAS          "Device.PPP.Interface.%d.Alias"
#define PPP_INTERFACE_LOWERLAYERS    "Device.PPP.Interface.%d.LowerLayers"
#define PPP_INTERFACE_USERNAME       "Device.PPP.Interface.%d.Username"
#define PPP_INTERFACE_PASSWORD       "Device.PPP.Interface.%d.Password"
#define PPP_INTERFACE_IPCP_ENABLE    "Device.PPP.Interface.%d.IPCPEnable"
#define PPP_INTERFACE_IPV6CP_ENABLE  "Device.PPP.Interface.%d.IPv6CPEnable"
#define PPP_INTERFACE_LINKTYPE       "Device.PPP.Interface.%d.X_RDK_LinkType"
#define PPP_IPCP_LOCAL_IPADDRESS     "Device.PPP.Interface.%d.IPCP.LocalIPAddress"
#define PPP_IPCP_REMOTEIPADDRESS     "Device.PPP.Interface.%d.IPCP.RemoteIPAddress"
#define PPP_IPCP_DNS_SERVERS         "Device.PPP.Interface.%d.IPCP.DNSServers"
#define PPP_IPV6CP_REMOTE_IDENTIFIER "Device.PPP.Interface.%d.IPv6CP.RemoteInterfaceIdentifier"

#define WANMGR_IFACE_WAN_STATUS                      "Device.X_RDK_WanManager.CPEInterface.%d.Wan.Status"

struct IFACE_INFO
{
    INT     Priority;
    CHAR    AvailableStatus[BUFLEN_64];
    CHAR    ActiveStatus[BUFLEN_64];
    CHAR    CurrentActive[BUFLEN_64];
    CHAR    CurrentStandby[BUFLEN_64];
    struct IFACE_INFO *next;
};

ANSC_STATUS WanMgr_WanConfigInit(void);
ANSC_STATUS DmlSetWanIfCfg( INT LineIndex, DML_WAN_IFACE* pstLineInfo );
ANSC_STATUS DmlAddMarking(ANSC_HANDLE hContext,DML_MARKING* pMarking);
ANSC_STATUS DmlDeleteMarking(ANSC_HANDLE hContext, DML_MARKING* pMarking);
ANSC_STATUS DmlSetMarking(ANSC_HANDLE hContext, DML_MARKING*   pMarking);
ANSC_STATUS WanMgr_WanIfaceConfInit(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl);
ANSC_STATUS WanMgr_WanIfaceMarkingInit (WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl);

ANSC_STATUS WanMgr_Publish_WanStatus(UINT IfaceIndex);
ANSC_STATUS DmlSetWanActiveLinkInPSMDB( UINT uiInterfaceIdx, bool flag );
ANSC_STATUS WanController_ClearWanConfigurationsInPSM();
ANSC_STATUS Update_Interface_Status();
#endif /* _WANMGR_RDKBUS_APIS_H_ */
