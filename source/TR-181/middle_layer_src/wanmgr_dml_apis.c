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

#include "wanmgr_dml_apis.h"

#include "wanmgr_data.h"
#include "wanmgr_controller.h"
#include "wanmgr_rdkbus_utils.h"


BOOL
WanManager_GetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong)
{
    BOOL ret = FALSE;

    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        DML_WANMGR_CONFIG* pWanDmlData = &(pWanConfigData->data);

        if(AnscEqualString(ParamName, "Policy", TRUE))
        {
            *puLong= pWanDmlData->Policy;
            ret = TRUE;
        }

        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    return ret;
}

BOOL
WanManager_SetParamUlongValue(ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue)
{
    BOOL ret = FALSE;
    ANSC_STATUS retStatus;

    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        DML_WANMGR_CONFIG* pWanDmlData = &(pWanConfigData->data);

        if(AnscEqualString(ParamName, "Policy", TRUE))
        {
            retStatus = WanMgr_RdkBus_setWanPolicy((DML_WAN_POLICY)uValue);
            if(retStatus == ANSC_STATUS_SUCCESS)
            {
                if(pWanDmlData->Policy != uValue)
                {
                    pWanDmlData->Policy = uValue;
                    pWanDmlData->PolicyChanged = TRUE;
                }
                ret = TRUE;
            }
        }

        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    return ret;
}

BOOL
WanManager_GetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool)
{
    BOOL ret = FALSE;

    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        DML_WANMGR_CONFIG* pWanDmlData = &(pWanConfigData->data);
        if(AnscEqualString(ParamName, "Enable", TRUE))
        {
            *pBool= pWanDmlData->Enable;
            ret = TRUE;
        }

        if(AnscEqualString(ParamName, "ResetActiveInterface", TRUE))
        {
            *pBool= pWanDmlData->ResetActiveInterface;
            ret = TRUE;
        }

        if(AnscEqualString(ParamName, "AllowRemoteInterfaces", TRUE))
        {
            *pBool= pWanDmlData->AllowRemoteInterfaces;
            ret = TRUE;
        }

        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    return ret;
}

BOOL WanManager_SetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pString)
{
    BOOL ret = FALSE;
    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();

    if(!pWanConfigData)
    {
        CcspTraceError(("%s: Failed to get wan config structure..\n",__FUNCTION__));
    }
    else if( AnscEqualString(ParamName, "Data", TRUE) )
    {
        char *webConf = NULL;
        int webSize = 0;

        webConf = AnscBase64Decode(pString, &webSize);
        if(!webConf)
        {
            CcspTraceError(("%s: Failed to decode webconfig blob..\n",__FUNCTION__));
            WanMgrDml_GetConfigData_release(pWanConfigData);
            return ret;
        }
        if ( ANSC_STATUS_SUCCESS == WanMgrDmlWanDataSet(webConf,webSize) )
        {
            CcspTraceInfo(("%s Success in parsing web config blob..\n",__FUNCTION__));
            ret = TRUE;
        }
        else
        {
            CcspTraceError(("%s Failed to parse webconfig blob..\n",__FUNCTION__));
        }
        AnscFreeMemory(webConf);
        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    return ret;
}

LONG WanManager_GetParamStringValue(ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pulSize)
{
    if( AnscEqualString(ParamName, "Data", TRUE) )
    {
        /* Data value should be empty for all get */
        snprintf(pValue, pulSize, "%s", "");
        return 0;
    }

    return -1;
}

BOOL WanManager_SetParamBoolValue(ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue)
{
    BOOL ret = FALSE;

    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        DML_WANMGR_CONFIG* pWanDmlData = &(pWanConfigData->data);


        if(AnscEqualString(ParamName, "Enable", TRUE))
        {
            WanMgr_RdkBus_setWanEnableToPsm(bValue);
            pWanDmlData->Enable = bValue;
            ret = TRUE;
        }

        if(AnscEqualString(ParamName, "ResetActiveInterface", TRUE))
        {
            pWanDmlData->ResetActiveInterface = bValue;
            ret = TRUE;
        }

        if(AnscEqualString(ParamName, "AllowRemoteInterfaces", TRUE))
        {
            pWanDmlData->AllowRemoteInterfaces = bValue;
            ret = TRUE;
        }

        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    return ret;
}
ULONG WanManager_Validate(ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength)
{
    return TRUE;
}

ULONG WanManager_Commit(ANSC_HANDLE hInsContext)
{
    ULONG ret = 0;

    return ret;
}
