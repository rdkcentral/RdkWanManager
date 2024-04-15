#if !defined(WAN_MANAGER_UNIFICATION_ENABLED)
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
#ifndef  _WANMGR_DML_IFACE_APIS_H_
#define  _WANMGR_DML_IFACE_APIS_H_

#include "ansc_platform.h"

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.CPEInterface.

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
BOOL WanIf_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanIf_Commit(ANSC_HANDLE hInsContext);
ULONG WanIf_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.CPEInterface.{i}.Phy.

    *  WanIfPhy_GetParamStringValue
    *  WanIfPhy_SetParamStringValue
    *  WanIfPhy_GetParamUlongValue
    *  WanIfPhy_SetParamUlongValue
    *  WanIfPhy_Validate
    *  WanIfPhy_Commit
    *  WanIfPhy_Rollback

***********************************************************************/


ULONG WanIfPhy_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanIfPhy_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL WanIfPhy_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIfPhy_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
BOOL WanIfPhy_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanIfPhy_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfPhy_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.CPEInterface.{i}.Wan.

    *  WanIfCfg_GetParamUlongValue
    *  WanIfCfg_SetParamUlongValue
    *  WanIfCfg_GetParamIntValue
    *  WanIfCfg_SetParamIntValue
    *  WanIfCfg_GetParamBoolValue
    *  WanIfCfg_SetParamBoolValue
    *  WanIfCfg_GetParamStringValue
    *  WanIfCfg_SetParamStringValue
    *  WanIfCfg_Validate
    *  WanIfCfg_Commit
    *  WanIfCfg_Rollback

***********************************************************************/

BOOL WanIfCfg_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanIfCfg_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
BOOL WanIfCfg_GetParamIntValue(ANSC_HANDLE hInsContext, char* ParamName, int* pInt);
BOOL WanIfCfg_SetParamIntValue(ANSC_HANDLE hInsContext, char* ParamName, int iValue);
BOOL WanIfCfg_GetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool);
BOOL WanIfCfg_SetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
ULONG WanIfCfg_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanIfCfg_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
BOOL WanIfCfg_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanIfCfg_Commit(ANSC_HANDLE hInsContext);
ULONG WanIfCfg_Rollback(ANSC_HANDLE hInsContext);

/***********************************************************************

 APIs for Object:

    X_RDK_WanManager.CPEInterface.{i}.IP.

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

    X_RDK_WanManager.CPEInterface.{i}.MAPT.

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

    X_RDK_WanManager.CPEInterface.{i}.DSLite.

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

    Device.X_RDK_WanManager.CPEInterface.{i}.Marking.{i}.

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

    X_RDK_WanManager.CPEInterface.{i}.PPP.

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

#endif /* _WANMGR_DML_IFACE_APIS_H_ */
#endif /* !WAN_MANAGER_UNIFICATION_ENABLED */
