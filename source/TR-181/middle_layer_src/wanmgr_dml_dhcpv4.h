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

#ifndef  _DHCPV4_DML_H
#define  _DHCPV4_DML_H

#include "wanmgr_apis.h"
/***********************************************************************

 APIs for Object:

    DHCPv4.

    *  DHCPv4_GetParamBoolValue
    *  DHCPv4_GetParamIntValue
    *  DHCPv4_GetParamUlongValue
    *  DHCPv4_GetParamStringValue

***********************************************************************/
BOOL DHCPv4_GetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool );
BOOL DHCPv4_GetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int* pInt );
BOOL DHCPv4_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* pUlong );
ULONG DHCPv4_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize );

/***********************************************************************

 APIs for Object:

    DHCPv4.Client.{i}.

    *  Client_GetEntryCount
    *  Client_GetEntry
    *  Client_AddEntry
    *  Client_DelEntry
    *  Client_GetParamBoolValue
    *  Client_GetParamIntValue
    *  Client_GetParamUlongValue
    *  Client_GetParamStringValue
    *  Client_SetParamBoolValue
    *  Client_SetParamIntValue
    *  Client_SetParamUlongValue
    *  Client_SetParamStringValue
    *  Client_Validate
    *  Client_Commit
    *  Client_Rollback

***********************************************************************/
ULONG Client_GetEntryCount ( ANSC_HANDLE );
ANSC_HANDLE Client_GetEntry ( ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber );
ANSC_HANDLE Client_AddEntry ( ANSC_HANDLE hInsContext, ULONG* pInsNumber );
ULONG Client_DelEntry ( ANSC_HANDLE hInsContext, ANSC_HANDLE hInstance );
BOOL Client_GetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool );
BOOL Client_GetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int* pInt );
BOOL Client_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* pUlong );
ULONG Client_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize );
BOOL Client_SetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue );
BOOL Client_SetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int value );
BOOL Client_SetParamUlongValue ( ANSC_HANDLE  hInsContext, char* ParamName, ULONG uValuepUlong );
BOOL Client_SetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* strValue );
BOOL Client_Validate ( ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength );
ULONG Client_Commit ( ANSC_HANDLE hInsContext );
ULONG Client_Rollback ( ANSC_HANDLE hInsContext );

/***********************************************************************

 APIs for Object:

    DHCPv4.Client.{i}.SentOption.{i}.

    *  SentOption_GetEntryCount
    *  SentOption_GetEntry
    *  SentOption_AddEntry
    *  SentOption_DelEntry
    *  SentOption_GetParamBoolValue
    *  SentOption_GetParamIntValue
    *  SentOption_GetParamUlongValue
    *  SentOption_GetParamStringValue
    *  SentOption_SetParamBoolValue
    *  SentOption_SetParamIntValue
    *  SentOption_SetParamUlongValue
    *  SentOption_SetParamStringValue
    *  SentOption_Validate
    *  SentOption_Commit
    *  SentOption_Rollback

***********************************************************************/
ULONG SentOption_GetEntryCount ( ANSC_HANDLE  );
ANSC_HANDLE SentOption_GetEntry ( ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber );
ANSC_HANDLE SentOption_AddEntry ( ANSC_HANDLE hInsContext, ULONG* pInsNumber );
ULONG SentOption_DelEntry ( ANSC_HANDLE hInsContext, ANSC_HANDLE hInstance );
BOOL SentOption_GetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool );
BOOL SentOption_GetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int* pInt );
BOOL SentOption_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* pUlong );
ULONG SentOption_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize );
BOOL SentOption_SetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue );
BOOL SentOption_SetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int value );
BOOL SentOption_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValuepUlong );
BOOL SentOption_SetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* strValue );
BOOL SentOption_Validate ( ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength );
ULONG SentOption_Commit ( ANSC_HANDLE hInsContext );
ULONG SentOption_Rollback ( ANSC_HANDLE hInsContext );

/***********************************************************************

 APIs for Object:

    DHCPv4.Client.{i}.ReqOption.{i}.

    *  ReqOption_GetEntryCount
    *  ReqOption_GetEntry
    *  ReqOption_AddEntry
    *  ReqOption_DelEntry
    *  ReqOption_GetParamBoolValue
    *  ReqOption_GetParamIntValue
    *  ReqOption_GetParamUlongValue
    *  ReqOption_GetParamStringValue
    *  ReqOption_SetParamBoolValue
    *  ReqOption_SetParamIntValue
    *  ReqOption_SetParamUlongValue
    *  ReqOption_SetParamStringValue
    *  ReqOption_Validate
    *  ReqOption_Commit
    *  ReqOption_Rollback

***********************************************************************/
ULONG ReqOption_GetEntryCount ( ANSC_HANDLE );
ANSC_HANDLE ReqOption_GetEntry ( ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber );
ANSC_HANDLE ReqOption_AddEntry ( ANSC_HANDLE hInsContext, ULONG* pInsNumber );
ULONG ReqOption_DelEntry ( ANSC_HANDLE  hInsContext, ANSC_HANDLE hInstance );
BOOL ReqOption_GetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool );
BOOL ReqOption_GetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int* pInt );
BOOL ReqOption_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* pUlong );
ULONG ReqOption_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize );
BOOL ReqOption_SetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue );
BOOL ReqOption_SetParamIntValue ( ANSC_HANDLE hInsContext, char* ParamName, int value );
BOOL ReqOption_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValuepUlong );
BOOL ReqOption_SetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* strValue );
BOOL ReqOption_Validate ( ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength );
ULONG ReqOption_Commit ( ANSC_HANDLE hInsContext );
ULONG ReqOption_Rollback ( ANSC_HANDLE hInsContext );
#endif
