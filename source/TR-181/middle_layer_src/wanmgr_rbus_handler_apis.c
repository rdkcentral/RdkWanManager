/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
/*
 * Copyright [2014] [Cisco Systems, Inc.]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     LICENSE-2.0" target="_blank" rel="nofollow">http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <rbus.h>
#include "wanmgr_data.h"
#include "wanmgr_net_utils.h"
#include "wanmgr_rbus_handler_apis.h"

static rbusHandle_t rbusHandle;

char componentName[] = "WANMANAGER";

unsigned int gSubscribersCount = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

ANSC_STATUS WanMgr_getUintParamValue (char * param, UINT * value)
{
    if ((param == NULL) || (value == NULL))
    {
        CcspTraceError(("%s %d: invalid args\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    if (rbus_getUint(rbusHandle, param, value) != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d: unbale to get value of param %s\n", __FUNCTION__, __LINE__, param));
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}


static void WanMgr_Rbus_EventReceiveHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    (void)handle;
    (void)subscription;

    const char* eventName = event->name;
    rbusValue_t valBuff = rbusObject_GetValue(event->data, NULL );

    if((valBuff == NULL) || (eventName == NULL))
    {
        CcspTraceError(("%s : FAILED , value is NULL\n",__FUNCTION__));
        return;
    }
    if (strcmp(eventName, WANMGR_DEVICE_NETWORKING_MODE) == 0)
    {
        CcspTraceInfo(("%s %d: change in %s\n", __FUNCTION__, __LINE__, eventName));
        UINT newValue = rbusValue_GetUInt32(valBuff);
        // Save here 
        WanMgr_Config_Data_t* pWanConfigData = WanMgr_GetConfigData_locked();
        if (pWanConfigData != NULL)
        {
            DML_WANMGR_CONFIG* pWanDmlData = &(pWanConfigData->data);
        
            if (newValue == 1)
            {
                pWanDmlData->DeviceNwMode = MODEM_MODE;
                CcspTraceInfo(("%s %d: DeviceNetworkMode changed to MODEM_MODE\n", __FUNCTION__, __LINE__));
            }
            else if (newValue == 0)
            {
                pWanDmlData->DeviceNwMode = GATEWAY_MODE;
                CcspTraceInfo(("%s %d: DeviceNetworkMode changed to GATEWAY_MODE\n", __FUNCTION__, __LINE__));
            }

            WanMgrDml_GetConfigData_release(pWanConfigData);
        }

        CcspTraceInfo(("%s:%d Received [%s:%u]\n",__FUNCTION__, __LINE__,eventName, newValue));
    }
    else
    {
        CcspTraceError(("%s:%d Unexpected Event Received [%s:%s]\n",__FUNCTION__, __LINE__,eventName));
    }
}


void WanMgr_SubscribeDML(void)
{
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    ret = rbusEvent_Subscribe(rbusHandle, WANMGR_DEVICE_NETWORKING_MODE, WanMgr_Rbus_EventReceiveHandler, NULL, 0);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, rbusError_ToString(ret), WANMGR_DEVICE_NETWORKING_MODE));
    }

    CcspTraceInfo(("WanMgr_SubscribeDML done\n"));
}

void WanMgr_UnSubscribeDML(void)
{
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    ret = rbusEvent_Unsubscribe(rbusHandle, WANMGR_DEVICE_NETWORKING_MODE);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, WANMGR_DEVICE_NETWORKING_MODE, rbusError_ToString(ret)));
    }

    CcspTraceInfo(("WanMgr_UnSubscribeDML done\n"));
}

/***********************************************************************
  WanMgr_Rbus_Init(): Initialize Rbus and data elements
 ***********************************************************************/
ANSC_STATUS WanMgr_Rbus_Init()
{
    int rc = ANSC_STATUS_FAILURE;

    rc = rbus_open(&rbusHandle, componentName);
    if (rc != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("WanMgr_Rbus_Init rbus initialization failed\n"));
        return rc;
    }

    return ANSC_STATUS_SUCCESS;
}

/*******************************************************************************
  WanMgr_RbusExit(): Unreg data elements and Exit
 ********************************************************************************/
ANSC_STATUS WanMgr_RbusExit()
{
    CcspTraceInfo(("%s %d - WanMgr_RbusExit called\n", __FUNCTION__, __LINE__ ));
    WanMgr_UnSubscribeDML();

    rbus_close(rbusHandle);
    return ANSC_STATUS_SUCCESS;
}
