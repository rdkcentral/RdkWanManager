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
#ifndef  _WANMGR_DML_APIS_H_
#define  _WANMGR_DML_APIS_H_

#include "ansc_platform.h"

/***********************************************************************

 APIs for Object:

    InternetGatewayDevice.X_CISCO_COM_COSADataModel.PluginSampleObj.

    *  PluginSampleObj_GetBulkParamValues
    *  PluginSampleObj_SetBulkParamValues
    *  PluginSampleObj_Validate
    *  PluginSampleObj_Commit
    *  PluginSampleObj_Rollback

***********************************************************************/
BOOL WanManager_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanManager_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
ULONG WanManager_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanManager_Commit(ANSC_HANDLE hInsContext);
BOOL WanManager_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString);
LONG WanManager_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize);
BOOL WanManager_GetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool);
BOOL WanManager_SetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
#if defined(WAN_MANAGER_UNIFICATION_ENABLED)
BOOL WanManagerGroup_IsUpdated(ANSC_HANDLE hInsContext);
ULONG WanManagerGroup_Synchronize(ANSC_HANDLE hInsContext);
ULONG WanManagerGroup_GetEntryCount(ANSC_HANDLE hInsContext);
ANSC_HANDLE WanManagerGroup_GetEntry(ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber);
BOOL WanManagerGroup_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong);
BOOL WanManagerGroup_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue);
BOOL WanManagerGroup_GetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool);
BOOL WanManagerGroup_SetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue);
ULONG WanManagerGroup_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength);
ULONG WanManagerGroup_Commit(ANSC_HANDLE hInsContext);
#endif /* WAN_MANAGER_UNIFICATION_ENABLED */
#endif //_WANMGR_DML_APIS_H_
