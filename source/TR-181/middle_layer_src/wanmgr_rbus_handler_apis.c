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

#ifdef RBUS_BUILD_FLAG_ENABLE
#include <rbus.h>
#include "wanmgr_data.h"
#include "wanmgr_net_utils.h"
#include "wanmgr_rbus_handler_apis.h"

static rbusHandle_t rbusHandle;

char componentName[32] = "WANMANAGER";

unsigned int gSubscribersCount = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char localCurrentActiveInterface[64] = {0};
char localCurrentStandbyInterface[64] = {0};

rbusError_t WanMgr_Rbus_SubscribeHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char *eventName, rbusFilter_t filter, int32_t interval, bool *autoPublish);
rbusError_t WanMgr_Rbus_getHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t *opts);
/***********************************************************************

  Data Elements declaration:

 ***********************************************************************/
rbusDataElement_t wanMgrRbusDataElements[NUM_OF_RBUS_PARAMS] = {
    {WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE,  RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
    {WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE, RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
};

rbusError_t WanMgr_Rbus_getHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t *opts)
{
    (void)handle;
    (void)opts;
    char const* name = rbusProperty_GetName(property);

    rbusValue_t value;
    rbusValue_Init(&value);

    if (strcmp(name, WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE) == 0)
    {
        rbusValue_SetString(value, localCurrentActiveInterface);
    }
    else if (strcmp(name, WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE) == 0)
    {
        rbusValue_SetString(value, localCurrentStandbyInterface);
    }
    else
    {
        CcspTraceWarning(("getHandler: Invalid Input\n"));
        return RBUS_ERROR_INVALID_INPUT;
    }

    rbusProperty_SetValue(property, value);
    rbusValue_Release(value);
    return RBUS_ERROR_SUCCESS;
}

ANSC_STATUS WanMgr_Rbus_getUintParamValue (char * param, UINT * value)
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


void WanMgr_Rbus_SubscribeDML(void)
{
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    ret = rbusEvent_Subscribe(rbusHandle, WANMGR_DEVICE_NETWORKING_MODE, WanMgr_Rbus_EventReceiveHandler, NULL, 0);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, rbusError_ToString(ret), WANMGR_DEVICE_NETWORKING_MODE));
    }

    CcspTraceInfo(("WanMgr_Rbus_SubscribeDML done\n"));
}

void WanMgr_Rbus_UnSubscribeDML(void)
{
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    ret = rbusEvent_Unsubscribe(rbusHandle, WANMGR_DEVICE_NETWORKING_MODE);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, WANMGR_DEVICE_NETWORKING_MODE, rbusError_ToString(ret)));
    }

    CcspTraceInfo(("WanMgr_Rbus_UnSubscribeDML done\n"));
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

    // Register data elements
    rc = rbus_regDataElements(rbusHandle, NUM_OF_RBUS_PARAMS, wanMgrRbusDataElements);

    if (rc != RBUS_ERROR_SUCCESS)
    {
        CcspTraceWarning(("rbus register data elements failed\n"));
        rbus_close(rbusHandle);
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
    rbus_unregDataElements(rbusHandle, NUM_OF_RBUS_PARAMS, wanMgrRbusDataElements);
    WanMgr_Rbus_UnSubscribeDML();

    rbus_close(rbusHandle);
    return ANSC_STATUS_SUCCESS;
}

/*******************************************************************************
  WanMgr_Rbus_EventPublishHandler(): publish rbus events
 ********************************************************************************/
ANSC_STATUS WanMgr_Rbus_EventPublishHandler(char *dm_event, void *dm_value, rbusValueType_t valueType)
{
    rbusEvent_t event;
    rbusObject_t rdata;
    rbusValue_t value;

    if(dm_event == NULL || dm_value == NULL)
    {
        CcspTraceInfo(("%s %d - Failed publishing\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    rbusValue_Init(&value);
    rbusObject_Init(&rdata, NULL);

    rbusObject_SetValue(rdata, dm_event, value);
    CcspTraceInfo(("%s %d - dm_event[%s] valueType[%d]\n", __FUNCTION__, __LINE__, dm_event,valueType));
    switch(valueType)
    {
        case RBUS_BOOLEAN:
            rbusValue_SetBoolean(value, (*(bool*)(dm_value)));
            CcspTraceInfo(("%s %d - dm_value[%s]\n", __FUNCTION__, __LINE__, (dm_value)?"True":"False"));
            break;
        case RBUS_UINT64:
            rbusValue_SetUInt64(value, (*(int*)(dm_value)));
            CcspTraceInfo(("%s %d - dm_value[%d]\n", __FUNCTION__, __LINE__, dm_value));
            break;
        case RBUS_STRING:
            rbusValue_SetString(value, (char*)dm_value);
            CcspTraceInfo(("%s %d - dm_value[%s]\n", __FUNCTION__, __LINE__, dm_value));
            break;
        default:
            CcspTraceInfo(("%s %d - Cannot identify valueType %d\n", __FUNCTION__, __LINE__, valueType));
            return ANSC_STATUS_FAILURE;
    }

    event.name = dm_event;
    event.data = rdata;
    event.type = RBUS_EVENT_GENERAL;
    if(rbusEvent_Publish(rbusHandle, &event) != RBUS_ERROR_SUCCESS) {
        CcspTraceInfo(("%s %d - event pusblishing failed for type %d\n", __FUNCTION__, __LINE__, valueType));
        return ANSC_STATUS_FAILURE;
    }
    CcspTraceInfo(("%s %d - Successfully Pusblished event for event %s \n", __FUNCTION__, __LINE__, dm_event));
    rbusValue_Release(value);
    rbusObject_Release(rdata);

    return ANSC_STATUS_SUCCESS;
}
/*******************************************************************************
  WanMgr_Rbus_String_EventPublish(): publish rbus string events
 ********************************************************************************/
ANSC_STATUS WanMgr_Rbus_String_EventPublish(char *dm_event, char *dm_value)
{
    if (strcmp(dm_event, WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE) == 0)
    {
        strncpy(localCurrentActiveInterface,dm_value,sizeof(localCurrentActiveInterface));
        return WanMgr_Rbus_EventPublishHandler(dm_event, dm_value, RBUS_STRING);
    }
    else if (strcmp(dm_event, WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE) == 0)
    {
        strncpy(localCurrentStandbyInterface,dm_value,sizeof(localCurrentStandbyInterface));
        return WanMgr_Rbus_EventPublishHandler(dm_event, dm_value, RBUS_STRING);
    }
    else
    {
        CcspTraceWarning(("WanMgr_Rbus_String_EventPublish: Invalid Input\n"));
        return ANSC_STATUS_FAILURE;
    }
}

/***********************************************************************
  Event subscribe handler API for objects:
 ***********************************************************************/
rbusError_t WanMgr_Rbus_SubscribeHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char *eventName, rbusFilter_t filter, int32_t interval, bool *autoPublish)
{
    (void)handle;
    (void)filter;
    (void)interval;

    *autoPublish = false;

    CcspTraceWarning(("WanMgr_Rbus_SubscribeHandler called.\n"));

    if ((strcmp(eventName, WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE) == 0) ||
        (strcmp(eventName, WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE) == 0))
    {
        if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
        {
            gSubscribersCount += 1;
        }
        else
        {
            if (gSubscribersCount > 0)
            {
                gSubscribersCount -= 1;
            }
        }
        CcspTraceWarning(("Subscribers count changed, new value=%d\n", gSubscribersCount));
    }
    else
    {
        CcspTraceWarning(("provider: eventSubHandler unexpected eventName %s\n", eventName));
    }
    return RBUS_ERROR_SUCCESS;
}
#endif //RBUS_BUILD_FLAG_ENABLE
