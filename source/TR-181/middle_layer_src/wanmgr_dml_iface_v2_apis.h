#if defined(WAN_MANAGER_UNIFICATION_ENABLED)
/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/
#ifndef  _WANMGR_DML_IFACE_V2_APIS_H_
#define  _WANMGR_DML_IFACE_V2_APIS_H_

#include "ansc_platform.h"

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.

    *  WanIf_GetEntryCount
    *  WanIf_GetEntry
    *  WanIf_AddEntry
    *  WanIf_DelEntry
    *  WanIf_GetParamStringValue
    *  WanIf_SetParamStringValue
    *  WanIf_GetParamBoolValue
    *  WanIf_SetParamBoolValue
    *  WanIf_Validate
    *  WanIf_Commit
    *  WanIf_Rollback

***********************************************************************/
ULONG WanIf_GetEntryCount(ANSC_HANDLE);
ANSC_HANDLE WanIf_GetEntry(ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber);
BOOL WanIf_IsUpdated(ANSC_HANDLE hInsContext);
ULONG WanIf_Synchronize(ANSC_HANDLE hInsContext);
ANSC_HANDLE WanIf_AddEntry(ANSC_HANDLE hInsContext, ULONG* pInsNumber);
ULONG WanIf_DelEntry(ANSC_HANDLE hInsContext, ANSC_HANDLE hInstance);
ULONG WanIf_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanIf_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL WanIf_GetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool);
BOOL WanIf_SetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
BOOL WanIf_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIf_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
BOOL WanIf_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanIf_Commit(ANSC_HANDLE hInsContext);
ULONG WanIf_Rollback(ANSC_HANDLE hInsContext);
BOOL WanIfCfg_IsUpdated (ANSC_HANDLE hInsContext);
/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.{i}.Selection.

    *  WanIfSelectionCfg_GetParamUlongValue
    *  WanIfSelectionCfg_SetParamUlongValue
    *  WanIfSelectionCfg_GetParamIntValue
    *  WanIfSelectionCfg_SetParamIntValue
    *  WanIfSelectionCfg_GetParamBoolValue
    *  WanIfSelectionCfg_SetParamBoolValue
    *  WanIfSelectionCfg_Validate
    *  WanIfSelectionCfg_Commit
    *  WanIfSelectionCfg_Rollback

***********************************************************************/

BOOL WanIfSelectionCfg_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIfSelectionCfg_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
BOOL WanIfSelectionCfg_GetParamIntValue(ANSC_HANDLE hInsContext, char* ParamName, int* pInt);
BOOL WanIfSelectionCfg_SetParamIntValue(ANSC_HANDLE hInsContext, char* ParamName, int iValue);
BOOL WanIfSelectionCfg_GetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool);
BOOL WanIfSelectionCfg_SetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
BOOL WanIfSelectionCfg_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanIfSelectionCfg_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfSelectionCfg_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.{i}.VirtualInterface.{i}.

    *  WanVirtualIf_GetEntryCount
    *  WanVirtualIf_GetEntry
    *  WanVirtualIf_IsUpdated
    *  WanVirtualIf_Synchronize
    *  WanVirtualIf_GetParamBoolValue
    *  WanVirtualIf_SetParamBoolValue
    *  WanVirtualIf_GetParamUlongValue
    *  WanVirtualIf_SetParamUlongValue
    *  WanVirtualIf_GetParamStringValue
    *  WanVirtualIf_SetParamStringValue
    *  WanVirtualIf_Validate
    *  WanVirtualIf_Commit
    *  WanVirtualIf_Rollback

***********************************************************************/

ULONG WanVirtualIf_GetEntryCount(ANSC_HANDLE);
ANSC_HANDLE WanVirtualIf_GetEntry(ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber);
BOOL WanVirtualIf_IsUpdated(ANSC_HANDLE hInsContext);
ULONG WanVirtualIf_Synchronize(ANSC_HANDLE hInsContext);
BOOL WanVirtualIf_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanVirtualIf_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
BOOL WanVirtualIf_GetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool);
BOOL WanVirtualIf_SetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
ULONG WanVirtualIf_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanVirtualIf_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL WanVirtualIf_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanVirtualIf_Commit(ANSC_HANDLE hInsContext);
ULONG WanVirtualIf_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.{i}.VirtualInterface.{i}.IP.

    *  WanIfIpCfg_GetParamUlongValue
    *  WanIfIpCfg_SetParamUlongValue
    *  WanIfIpCfg_GetParamStringValue
    *  WanIfIpCfg_SetParamStringValue
    *  WanIfIpCfg_Validate
    *  WanIfIpCfg_Commit
    *  WanIfIpCfg_Rollback

***********************************************************************/

BOOL WanIfIpCfg_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIfIpCfg_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
ULONG WanIfIpCfg_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanIfIpCfg_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL WanIfIpCfg_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanIfIpCfg_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfIpCfg_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.{i}.VirtualInterface.{i}.MAPT.

    *  WanIfMapt_GetParamUlongValue
    *  WanIfMapt_SetParamUlongValue
    *  WanIfMapt_GetParamStringValue
    *  WanIfMapt_SetParamStringValue
    *  WanIfMapt_Validate
    *  WanIfMapt_Commit
    *  WanIfMapt_Rollback

***********************************************************************/

BOOL WanIfMapt_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIfMapt_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
ULONG WanIfMapt_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanIfMapt_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL WanIfMapt_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanIfMapt_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfMapt_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.{i}.VirtualInterface.{i}.DSLite.

    *  WanIfDSLite_GetParamUlongValue
    *  WanIfDSLite_SetParamUlongValue
    *  WanIfDSLite_GetParamStringValue
    *  WanIfDSLite_SetParamStringValue
    *  WanIfDSLite_Validate
    *  WanIfDSLite_Commit
    *  WanIfDSLite_Rollback

***********************************************************************/

BOOL WanIfDSLite_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIfDSLite_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
ULONG WanIfDSLite_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanIfDSLite_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL WanIfDSLite_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanIfDSLite_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfDSLite_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    Device.X_RDK_WanManager.Interface.{i}.Marking.{i}.

    *  Marking_GetEntryCount
    *  Marking_GetEntry
    *  Marking_AddEntry
    *  Marking_DelEntry
    *  Marking_GetParamUlongValue
    *  Marking_GetParamStringValue
    *  Marking_GetParamIntValue
    *  Marking_SetParamIntValue
    *  Marking_SetParamUlongValue
    *  Marking_SetParamStringValue
    *  Marking_Validate
    *  Marking_Commit
    *  Marking_Rollback

***********************************************************************/

ULONG Marking_GetEntryCount(ANSC_HANDLE hInsContext);
ANSC_HANDLE Marking_GetEntry(ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber);
ANSC_HANDLE Marking_AddEntry(ANSC_HANDLE hInsContext, ULONG* pInsNumber);
ULONG Marking_DelEntry(ANSC_HANDLE hInsContext, ANSC_HANDLE hInstance);
BOOL Marking_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
ULONG Marking_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL Marking_GetParamIntValue(ANSC_HANDLE hInsContext, char* ParamName, int* pInt);
BOOL Marking_SetParamIntValue(ANSC_HANDLE hInsContext, char* ParamName, int iValue);
BOOL Marking_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
BOOL Marking_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL Marking_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG Marking_Commit(ANSC_HANDLE hInsContext);
ULONG Marking_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.{i}.VirtualInterface.{i}.PPP.

    *  WanIfPPPCfg_GetParamUlongValue
    *  WanIfPPPCfg_SetParamUlongValue
    *  WanIfPPPCfg_GetParamStringValue
    *  WanIfPPPCfg_SetParamStringValue
    *  WanIfPPPCfg_GetParamBoolValue
    *  WanIfPPPCfg_SetParamBoolValue
    *  WanIfPPPCfg_Validate
    *  WanIfPPPCfg_Commit
    *  WanPPPIpCfg_Rollback

***********************************************************************/

BOOL WanIfPPPCfg_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIfPPPCfg_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
ULONG WanIfPPPCfg_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanIfPPPCfg_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
ULONG WanIfPPPCfg_GetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool);
BOOL WanIfPPPCfg_SetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
BOOL WanIfPPPCfg_Validate(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
ULONG WanIfPPPCfg_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfPPPCfg_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.{i}.VirtualInterface.{i}.VLAN.{i}.

    *  WanIfVlanCfg_GetEntryCount
    *  WanIfVlanCfg_GetEntry
    *  WanIfVlanCfg_IsUpdated
    *  WanIfVlanCfg_Synchronize
    *  WanIfVlanCfg_GetParamStringValue
    *  WanIfVlanCfg_SetParamStringValue
    *  WanIfVlanCfg_Validate
    *  WanIfVlanCfg_Commit
    *  WanIfVlanCfg_Rollback

***********************************************************************/

ULONG WanIfVlanCfg_GetEntryCount(ANSC_HANDLE);
ANSC_HANDLE WanIfVlanCfg_GetEntry(ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber);
BOOL WanIfVlanCfg_IsUpdated(ANSC_HANDLE hInsContext);
ULONG WanIfVlanCfg_Synchronize(ANSC_HANDLE hInsContext);
ULONG WanIfVlanCfg_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanIfVlanCfg_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL WanIfVlanCfg_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIfVlanCfg_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
BOOL WanIfVlanCfg_Validate(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
ULONG WanIfVlanCfg_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfVlanCfg_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.Interface.{i}.VirtualInterface.{i}.VLAN.Marking{i}.

    *  WanIfVlanMarking_GetEntryCount
    *  WanIfVlanMarking_GetEntry
    *  WanIfVlanMarking_GetParamIntValue
    *  WanIfVlanMarking_SetParamIntValue
    *  WanIfVlanMarking_Validate
    *  WanIfVlanMarking_Commit
    *  WanIfVlanMarking_Rollback

***********************************************************************/

ULONG WanIfVlanMarking_GetEntryCount(ANSC_HANDLE hInsContext);
ANSC_HANDLE WanIfVlanMarking_GetEntry(ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber);
BOOL WanIfVlanMarking_GetParamIntValue(ANSC_HANDLE hInsContext, char* ParamName, int* pInt);
BOOL WanIfVlanMarking_SetParamIntValue(ANSC_HANDLE hInsContext, char* ParamName, int iValue);
BOOL WanIfVlanMarking_Validate(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
ULONG WanIfVlanMarking_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfVlanMarking_Rollback(ANSC_HANDLE hInsContext);

#endif /* _WANMGR_DML_IFACE_V2_APIS_H_ */
#endif /* WAN_MANAGER_UNIFICATION_ENABLED */
