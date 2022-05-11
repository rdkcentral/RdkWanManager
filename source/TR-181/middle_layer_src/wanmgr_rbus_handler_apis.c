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
#include "wanmgr_data.h"
#include "wanmgr_net_utils.h"
#include "wanmgr_rbus_handler_apis.h"

static rbusHandle_t rbusHandle;

char componentName[32] = "WANMANAGER";

unsigned int gSubscribersCount = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

rbusError_t WanMgr_Rbus_SubscribeHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char *eventName, rbusFilter_t filter, int32_t interval, bool *autoPublish);
rbusError_t WanMgr_Rbus_getHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t *opts);
/***********************************************************************

  Data Elements declaration:

 ***********************************************************************/
rbusDataElement_t wanMgrRbusDataElements[NUM_OF_RBUS_PARAMS] = {
    {WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE,  RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
    {WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE, RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
    {WANMGR_CONFIG_WAN_INTERFACEAVAILABLESTATUS,RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
    {WANMGR_CONFIG_WAN_INTERFACEACTIVESTATUS,   RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
};

rbusError_t WanMgr_Rbus_getHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t *opts)
{
    (void)handle;
    (void)opts;
    char const* name = rbusProperty_GetName(property);

    rbusValue_t value;
    rbusValue_Init(&value);

    WanMgr_Config_Data_t* pWanConfigData = WanMgr_GetConfigData_locked();
    if (pWanConfigData != NULL)
    {
        DML_WANMGR_CONFIG* pWanDmlData = &(pWanConfigData->data);
        if (strcmp(name, WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE) == 0)
        {
            rbusValue_SetString(value, pWanDmlData->CurrentActiveInterface);
        }
        else if (strcmp(name, WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE) == 0)
        {
            rbusValue_SetString(value, pWanDmlData->CurrentStandbyInterface);
        }
        else if (strcmp(name, WANMGR_CONFIG_WAN_INTERFACEAVAILABLESTATUS) == 0)
        {
            rbusValue_SetString(value, pWanDmlData->InterfaceAvailableStatus);
        }
        else if (strcmp(name, WANMGR_CONFIG_WAN_INTERFACEACTIVESTATUS) == 0)
        {
            rbusValue_SetString(value, pWanDmlData->InterfaceActiveStatus);
        }
        else
        {
            WanMgrDml_GetConfigData_release(pWanConfigData);
            CcspTraceWarning(("WanMgr_Rbus_getHandler: Invalid Input\n"));
            rbusValue_Release(value);
            return RBUS_ERROR_INVALID_INPUT;
        }

        WanMgrDml_GetConfigData_release(pWanConfigData);
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

    CcspTraceInfo(("%s %d: %s = %d\n", __FUNCTION__, __LINE__, param, *value));

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

void WanMgr_Rbus_UpdateLocalWanDb(void)
{
    UINT ret = 0;

    // get DeviceNetworking Mode
    if (WanMgr_Rbus_getUintParamValue(WANMGR_DEVICE_NETWORKING_MODE, &ret) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: unable to fetch %s\n", __FUNCTION__, __LINE__, WANMGR_DEVICE_NETWORKING_MODE));
        return;
    }

    //Update Wan config
    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        if (ret == 1)
        {
            pWanConfigData->data.DeviceNwMode = MODEM_MODE;
        }
        else
        {
            pWanConfigData->data.DeviceNwMode = GATEWAY_MODE;
        }
        WanMgrDml_GetConfigData_release(pWanConfigData);
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
 WanMgr_IDM_Invoke():
Description:
    Send Invoke request to IDM
Arguments:
    idm_invoke_method_Params_t*

    struct list:
    IDM_MSG_OPERATION operation : DM GET/SET/SUBSCRIBE
    char Mac_dest[18]           : Destination device (identifier) MAC
    char param_name[128]        : DM name
    char param_value[2048]      : DM value
    char pComponent_name[128]   : Destination Component name (EX: eRT.com.cisco.spvtg.ccsp.wanmanager)
    char pBus_path[128]         : Destination Component bus path (EX : /com/cisco/spvtg/ccsp/wanmanager)
    uint timeout                : Timeout for async call back
    enum dataType_e type        : DM data type
    rbusMethodAsyncHandle_t asyncHandle : Async call back handler pointer
Return value:
    ANSC_STATUS

 ***********************************************************************/

ANSC_STATUS WanMgr_IDM_Invoke(idm_invoke_method_Params_t *IDM_request)
{
    rbusObject_t inParams;
    rbusValue_t value;
    int rc = RBUS_ERROR_SUCCESS;

    CcspTraceInfo(("%s %d - sendind rbus request to %s \n", __FUNCTION__, __LINE__,IDM_request->Mac_dest));

    rbusObject_Init(&inParams, NULL);

    rbusValue_Init(&value);
    rbusValue_SetString(value, IDM_request->Mac_dest );
    rbusObject_SetValue(inParams, "DEST_MAC_ADDR", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetString(value, IDM_request->param_name);
    rbusObject_SetValue(inParams, "paramName", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetString(value,IDM_request->param_value);
    rbusObject_SetValue(inParams, "paramValue", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetString(value, IDM_request->pComponent_name);
    rbusObject_SetValue(inParams, "pComponent", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetString(value, IDM_request->pBus_path);
    rbusObject_SetValue(inParams, "pBus", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetInt32(value, IDM_request->timeout);
    rbusObject_SetValue(inParams, "Timeout", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetInt32(value, IDM_request->type);
    rbusObject_SetValue(inParams, "DataType", value);
    rbusValue_Release(value);

    rbusValue_Init(&value);
    rbusValue_SetInt32(value, IDM_request->operation);
    rbusObject_SetValue(inParams, "Operation", value);
    rbusValue_Release(value);

    rc = rbusMethod_InvokeAsync(rbusHandle, "Device.X_RDK_Remote.Invoke()", inParams, IDM_request->asyncHandle, IDM_request->timeout);

    rbusObject_Release(inParams);
    if(rc == RBUS_ERROR_SUCCESS)
    {
        CcspTraceInfo(("%s %d -consumer: rbusMethod_Invoke(Device.X_RDK_Remote.Invoke()) success\n",__FUNCTION__, __LINE__));
        return ANSC_STATUS_SUCCESS;
    }else
    {
        CcspTraceInfo(("%s %d -consumer: rbusMethod_Invoke(Device.X_RDK_Remote.Invoke()) success\n",__FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;    
    }
}

#if 0
//********************************* <sample code start > **************************/
/* sample code for IDM Subscription call back */
#define RM_REMOTE_INVOKE "Device.X_RDK_Remote.Invoke()"
#define RM_NEW_DEVICE_FOUND "Device.X_RDK_Remote.DeviceChange"
int deviceFound = 0, Index;
static void WanMgr_IDM_InvokeSubsEventHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    (void)handle;
    (void)subscription;

    const char* eventName = event->name;
    if(strcmp(eventName,RM_REMOTE_INVOKE)==0)
    {
        /* All subscription events from remote device will be published using RM_REMOTE_INVOKE event. Check for source mac and parameter name in event->data */
        CcspTraceInfo(("%s %d - event %s received \n", __FUNCTION__, __LINE__, eventName));
        rbusValue_t value;
        value = rbusObject_GetValue(event->data, "Mac_source");
        CcspTraceInfo(("%s %d - from source MAC %s\n", __FUNCTION__, __LINE__,rbusValue_GetString(value, NULL)));

        value = rbusObject_GetValue(event->data, "param_name");
        CcspTraceInfo(("%s %d - param_name %s\n", __FUNCTION__, __LINE__,rbusValue_GetString(value, NULL)));

        value = rbusObject_GetValue(event->data, "operation");
        CcspTraceInfo(("%s %d - operation %d\n", __FUNCTION__, __LINE__,rbusValue_GetInt32(value)));

        value = rbusObject_GetValue(event->data, "param_value");
        CcspTraceInfo(("%s %d - param_value %s\n", __FUNCTION__, __LINE__,rbusValue_GetString(value, NULL)));
    }else if(strcmp(eventName, RM_NEW_DEVICE_FOUND)==0)
    {
        rbusValue_t value;
        value = rbusObject_GetValue(event->data, "Capabilities");
        CcspTraceInfo(("%s %d - from source MAC %s\n", __FUNCTION__, __LINE__,rbusValue_GetString(value, NULL)));

        value = rbusObject_GetValue(event->data, "Index");
        Index = rbusValue_GetUInt32(value);
        CcspTraceInfo(("%s %d - Index %d\n", __FUNCTION__, __LINE__,Index));
        deviceFound = 1;
    }

}

/* sample code for IDM Async call back */
static void TestAsyncMethodHandler(
        rbusHandle_t handle,
        char const* methodName,
        rbusError_t error,
        rbusObject_t params)
{
    (void)handle;


    CcspTraceInfo(("asyncMethodHandler called: %s  error=%d\n", methodName, error));
    if(error == RBUS_ERROR_SUCCESS)
    {
        CcspTraceInfo(("%s %d - asyncMethodHandler request success\n", __FUNCTION__, __LINE__));
        rbusObject_fwrite(params, 1, stdout);
    }else
    {
        CcspTraceInfo(("%s %d - asyncMethodHandler request failed\n", __FUNCTION__, __LINE__));
        return;
    }

    if(strcmp(methodName, "Device.X_RDK_Remote.Invoke()") == 0)
    {
        /* get the return values from IDM */
        rbusValue_t value;
        value = rbusObject_GetValue(params, "Mac_source");
        CcspTraceInfo(("%s %d - from source MAC %s\n", __FUNCTION__, __LINE__,rbusValue_GetString(value, NULL)));

        value = rbusObject_GetValue(params, "param_name");
        CcspTraceInfo(("%s %d - param_name %s\n", __FUNCTION__, __LINE__,rbusValue_GetString(value, NULL)));

        value = rbusObject_GetValue(params, "operation");
        CcspTraceInfo(("%s %d - operation %d\n", __FUNCTION__, __LINE__,rbusValue_GetInt32(value)));

        value = rbusObject_GetValue(params, "param_value");
        CcspTraceInfo(("%s %d - param_value %s\n", __FUNCTION__, __LINE__,rbusValue_GetString(value, NULL)));
    }

}

/* sample code for IDM get */
void IDM_Sample_code(void *arg)
{

    pthread_detach(pthread_self());

        CcspTraceInfo(("%s %d - wait for 30 sec \n", __FUNCTION__, __LINE__));
        sleep(30);

    //Subscribe to RM_REMOTE_INVOKE method. All subscription events from remote device will be published using RM_REMOTE_INVOKE event.
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    ret = rbusEvent_Subscribe(rbusHandle,RM_REMOTE_INVOKE , WanMgr_IDM_InvokeSubsEventHandler, NULL, 0);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, rbusError_ToString(ret), RM_REMOTE_INVOKE));
    }

    CcspTraceInfo(("%s %d - Subscribed %s,\n", __FUNCTION__, __LINE__,RM_REMOTE_INVOKE));

    ret = rbusEvent_Subscribe(rbusHandle,RM_NEW_DEVICE_FOUND , WanMgr_IDM_InvokeSubsEventHandler, NULL, 0);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, rbusError_ToString(ret), RM_NEW_DEVICE_FOUND));
    }

    CcspTraceInfo(("%s %d - Subscribed %s,\n", __FUNCTION__, __LINE__,RM_NEW_DEVICE_FOUND));

    //wait for device change
    while(!deviceFound)
    {
        CcspTraceInfo(("%s %d - wait for deviceFound \n", __FUNCTION__, __LINE__));
        sleep(5);       
    }
    
    deviceFound = 1;

    /* get remote device MAC address */
    char dmQuery[BUFLEN_256] = {0};
    char dmValue[BUFLEN_256] = {0};

    //TODO: Hardcoded remote device index. Get it from device change event
    snprintf(dmQuery, sizeof(dmQuery)-1, "Device.X_RDK_Remote.Device.%d.MAC",Index);
    if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
    {
        CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
    }

    CcspTraceInfo(("%s %d - dmValue %s \n", __FUNCTION__, __LINE__,dmValue));

    /* IDM get request*/
    idm_invoke_method_Params_t IDM_request;
    memset(&IDM_request,0, sizeof(idm_invoke_method_Params_t));
    /* Update request parameters */
    strcpy(IDM_request.Mac_dest,dmValue);
    strcpy(IDM_request.param_name, "Device.X_RDK_WanManager.InterfaceAvailableStatus");
    strcpy(IDM_request.pComponent_name, "eRT.com.cisco.spvtg.ccsp.wanmanager");
    strcpy(IDM_request.pBus_path, "/com/cisco/spvtg/ccsp/wanmanager");
    IDM_request.timeout = 20;
    IDM_request.operation = IDM_GET;
    IDM_request.asyncHandle = &TestAsyncMethodHandler; //pointer to callback
    WanMgr_IDM_Invoke(&IDM_request);

    sleep(5);
    /*IDM subcribe request */
    CcspTraceInfo(("%s %d - sendind rbus subscribe request: \n", __FUNCTION__, __LINE__));
    memset(&IDM_request,0, sizeof(idm_invoke_method_Params_t));
    /* Update request parameters */
    strcpy(IDM_request.Mac_dest,dmValue);
    strcpy(IDM_request.param_name, WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE);
    //Subscription events are notified using RM_REMOTE_INVOKE. IDM_request.asyncHandle and IDM_request.timeout are passed to avoid rbus error.
    IDM_request.timeout = 600;
    IDM_request.operation = IDM_SUBS;
    IDM_request.asyncHandle = &TestAsyncMethodHandler; //pointer to callback
    WanMgr_IDM_Invoke(&IDM_request);

    sleep(30);

    /* Publish WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE event */
    CcspTraceInfo(("%s %d - publish \n", __FUNCTION__, __LINE__));
    WanMgr_Rbus_String_EventPublish(WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE, "DSL,1|WANOE,0|");

    sleep(10);

    /* IDM set request*/
    CcspTraceInfo(("%s %d - sendind rbus set request: \n", __FUNCTION__, __LINE__));
    memset(&IDM_request,0, sizeof(idm_invoke_method_Params_t));
    char setString1[64]={0};
    snprintf(setString1,64,"from_tid_%lu-%d_WM",pthread_self(),getpid());
    /* Update request parameters */
    strcpy(IDM_request.Mac_dest,dmValue);
    strcpy(IDM_request.param_name, "Device.X_RDK_WanManager.CPEInterface.2.Name");
    strcpy(IDM_request.param_value, setString1);
    strcpy(IDM_request.pComponent_name, "eRT.com.cisco.spvtg.ccsp.wanmanager");
    strcpy(IDM_request.pBus_path, "/com/cisco/spvtg/ccsp/wanmanager");
    IDM_request.timeout = 20;
    IDM_request.type = ccsp_string;
    IDM_request.operation = IDM_SET;
    IDM_request.asyncHandle = &TestAsyncMethodHandler;
    WanMgr_IDM_Invoke(&IDM_request);

    /* Publish multiple WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE event */
    for(int i =0; i<=100; i++)
    {
        char setString[64]={0};
        //publish random string to remote device
        snprintf(setString,64,"Publish_%d == >%lu-_WM", i ,pthread_self());
        CcspTraceInfo(("%s %d - publish %s \n", __FUNCTION__, __LINE__,setString));

        WanMgr_Rbus_String_EventPublish(WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE, setString);

        sleep(15);
    }

    pthread_exit(NULL);

    return 0;

}
//********************************* <sample code end> **************************/
#endif

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
    if ((strcmp(dm_event, WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE) == 0) ||
        (strcmp(dm_event, WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE) == 0) ||
        (strcmp(dm_event, WANMGR_CONFIG_WAN_INTERFACEAVAILABLESTATUS) == 0) ||
        (strcmp(dm_event, WANMGR_CONFIG_WAN_INTERFACEACTIVESTATUS) == 0))
    {
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
