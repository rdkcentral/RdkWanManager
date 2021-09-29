#ifndef _WANMGR_RDKBUS_UTILS_H_
#define _WANMGR_RDKBUS_UTILS_H_

/*
   If not stated otherwise in this file or this component's Licenses.txt file the
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


/**************************************************************************

    module: wan_controller_utils.c

    For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        State machine to manage a Wan Controller

    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        13/02/2020    initial revision.

**************************************************************************/


#include "dmsb_tr181_psm_definitions.h"
#include "ccsp_psm_helper.h"
#include "wanmgr_data.h"
#include "wanmgr_rdkbus_common.h"
#include "wanmgr_data.h"
#include "ansc_platform.h"
#include "platform_hal.h"

#include <syscfg.h>
#include <syscfg/syscfg.h>

//VLAN Agent
#define VLAN_DBUS_PATH                     "/com/cisco/spvtg/ccsp/vlanmanager"
#define VLAN_COMPONENT_NAME                "eRT.com.cisco.spvtg.ccsp.vlanmanager"
#define VLAN_ETHLINK_NOE_PARAM_NAME        "Device.X_RDK_Ethernet.LinkNumberOfEntries"
#define VLAN_ETHLINK_TABLE_NAME            "Device.X_RDK_Ethernet.Link."
#define VLAN_ETHLINK_REFRESH_PARAM_NAME    "Device.X_RDK_Ethernet.Link.%d.X_RDK_Refresh"
//XDSL Manager
#define DSL_COMPONENT_NAME "eRT.com.cisco.spvtg.ccsp.xdslmanager"
#define DSL_COMPONENT_PATH "/com/cisco/spvtg/ccsp/xdslmanager"
#define DSL_UPSTREAM_NAME ".Upstream"
//Eth Manager
#define ETH_COMPONENT_NAME "eRT.com.cisco.spvtg.ccsp.ethagent"
#define ETH_COMP_NAME_WITHOUTSUBSYSTEM "com.cisco.spvtg.ccsp.ethagent"
#define ETH_COMPONENT_PATH "/com/cisco/spvtg/ccsp/ethagent"
#define ETH_UPSTREAM_NAME ".Upstream"
#define ETH_X_RDK_REBOOTREQUIRED_PARAM_NAME     "Device.Ethernet.Interface.%d.X_RDK_RebootRequired"

//CM Agent
#define CMAGENT_COMPONENT_NAME "eRT.com.cisco.spvtg.ccsp.cm"
#define CMAGENT_COMP_NAME_WITHOUTSUBSYSTEM "com.cisco.spvtg.ccsp.cm"
#define CMAGENT_COMPONENT_PATH "/com/cisco/spvtg/ccsp/cm"


//General Param for interfaces
#define PARAM_NAME_REQUEST_PHY_STATUS "RequestPhyStatus"
#define PARAM_NAME_REQUEST_OPERATIONAL_STATUS "RequestOperationalStatus"
#define PARAM_NAME_CONFIGURE_WAN "ConfigureWan"
#define PARAM_NAME_CUSTOM_CONFIG_WAN "CustomWanConfigUpdate"
#define PARAM_NAME_POST_CFG_WAN_FINALIZE "PostCfgWanFinalize"

ANSC_STATUS WanMgr_RdkBus_SetParamValues( char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, BOOLEAN bCommit );
ANSC_STATUS WanMgr_RdkBus_GetParamValues( char *pComponent, char *pBus, char *pParamName, char *pReturnVal );

int WanMgr_RdkBus_GetParamValuesFromDB( char *pParamName, char *pReturnVal, int ReturnValLength );
int WanMgr_RdkBus_SetParamValuesToDB( char *pParamName, char *pParamVal );

ANSC_STATUS WanMgr_RdkBus_getWanPolicy(DML_WAN_POLICY *wan_policy);
ANSC_STATUS WanMgr_RdkBus_setWanPolicy(DML_WAN_POLICY wan_policy);
ANSC_STATUS WanMgr_RdkBus_updateInterfaceUpstreamFlag(char *phyPath, BOOL flag);
void* WanMgr_RdkBus_WanIfRefreshThread( void *arg );

ANSC_STATUS DmlGetInstanceByKeywordFromPandM(char *ifname, int *piInstanceNumber);
ANSC_STATUS WanMgr_RdkBus_SetRequestIfComponent(char *pPhyPath, char *pInputparamName, char *pInputParamValue, enum dataType_e type);
ANSC_STATUS WaitForInterfaceComponentReady(char *pPhyPath);
ANSC_STATUS WanMgr_RdkBus_Get_InterfaceRebootRequired(UINT IfaceIndex, BOOL *RebootRequired);
#endif /* _WANMGR_RDKBUS_UTILS_H_ */
