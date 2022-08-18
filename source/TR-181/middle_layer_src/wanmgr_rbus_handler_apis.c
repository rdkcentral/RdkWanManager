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
#include "dmsb_tr181_psm_definitions.h"
enum {
ENUM_PHY = 1,
ENUM_WAN_STATUS,
ENUM_WAN_LINKSTATUS
};

#define  ARRAY_SZ(x) (sizeof(x) / sizeof((x)[0]))
#define  MAC_ADDR_SIZE 18
static rbusHandle_t rbusHandle;

char componentName[32] = "WANMANAGER";

unsigned int gSubscribersCount = 0;
UINT  uiTotalIfaces = 0;

rbusError_t WanMgr_Rbus_SubscribeHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char *eventName, rbusFilter_t filter, int32_t interval, bool *autoPublish);
rbusError_t WanMgr_Rbus_getHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t *opts);
ANSC_STATUS WanMgr_Rbus_EventPublishHandler(char *dm_event, void *dm_value, rbusValueType_t valueType);

rbusError_t wanMgrDmlPublishEventHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish);
rbusError_t WanMgr_Interface_GetHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts);
rbusError_t WanMgr_Interface_SetHandler(rbusHandle_t handle, rbusProperty_t prop, rbusSetHandlerOptions_t* opts);
static void CPEInterface_AsyncMethodHandler( rbusHandle_t handle, char const* methodName, rbusError_t error, rbusObject_t params);
static int WanMgr_Remote_IfaceData_index(char *macAddress);

typedef struct
{
    char* name;
    bool  get_request;
    bool  subscription_request;
} RemoteDM_list;

/***********************************************************************

  Data Elements declaration:

 ***********************************************************************/
rbusDataElement_t wanMgrRbusDataElements[NUM_OF_RBUS_PARAMS] = {
    {WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE,  RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
    {WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE, RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
    {WANMGR_CONFIG_WAN_INTERFACEAVAILABLESTATUS,RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
    {WANMGR_CONFIG_WAN_INTERFACEACTIVESTATUS,   RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY, {WanMgr_Rbus_getHandler, NULL, NULL, NULL, WanMgr_Rbus_SubscribeHandler, NULL}},
};

rbusDataElement_t wanMgrIfacePublishElements[] = {
    {WANMGR_INFACE, RBUS_ELEMENT_TYPE_TABLE, {NULL, NULL, NULL, NULL, NULL, NULL}},
    {WANMGR_INFACE_WAN_ENABLE, RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY,
    {WanMgr_Interface_GetHandler, WanMgr_Interface_SetHandler, NULL, NULL, wanMgrDmlPublishEventHandler, NULL}},
    {WANMGR_INFACE_ALIASNAME, RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY,
    {WanMgr_Interface_GetHandler, WanMgr_Interface_SetHandler, NULL, NULL, wanMgrDmlPublishEventHandler, NULL}},
    {WANMGR_INFACE_PHY_STATUS, RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY,
    {WanMgr_Interface_GetHandler, WanMgr_Interface_SetHandler, NULL, NULL, wanMgrDmlPublishEventHandler, NULL}},
    {WANMGR_INFACE_WAN_STATUS, RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY,
    {WanMgr_Interface_GetHandler, WanMgr_Interface_SetHandler, NULL, NULL, wanMgrDmlPublishEventHandler, NULL}},
    {WANMGR_INFACE_WAN_LINKSTATUS, RBUS_ELEMENT_TYPE_EVENT | RBUS_ELEMENT_TYPE_PROPERTY,
    {WanMgr_Interface_GetHandler, WanMgr_Interface_SetHandler, NULL, NULL, wanMgrDmlPublishEventHandler, NULL}},
};

RemoteDM_list RemoteDMs[] = {
    {"Device.X_RDK_WanManager.CPEInterface.1.DisplayName",TRUE, FALSE},
    {"Device.X_RDK_WanManager.CPEInterface.1.AliasName",TRUE, FALSE},
    {"Device.X_RDK_WanManager.CPEInterface.1.Phy.Status",TRUE,TRUE},
    {"Device.X_RDK_WanManager.CPEInterface.1.Wan.Status",TRUE,TRUE},
    {"Device.X_RDK_WanManager.CPEInterface.1.Wan.LinkStatus",TRUE,TRUE},
};

static void WanMgr_EnumToString(UINT Enum, UINT EnumType, char* String)
{
    char *Ptr = NULL;

    if (EnumType == ENUM_PHY)
    {
        Ptr = ((Enum == WAN_IFACE_PHY_STATUS_DOWN)? "Down":((Enum == WAN_IFACE_PHY_STATUS_INITIALIZING)?
                "Initializing":((Enum == WAN_IFACE_PHY_STATUS_UP)? "Up":"Unknown")));
    }
    else if (EnumType == ENUM_WAN_STATUS)
    {
        Ptr = ((Enum == WAN_IFACE_STATUS_INITIALISING)? "Initialising":
               ((Enum == WAN_IFACE_STATUS_VALIDATING)? "Validating":
               ((Enum == WAN_IFACE_STATUS_UP)? "Up":
               ((Enum == WAN_IFACE_STATUS_STANDBY)? "Standby":
               ((Enum == WAN_IFACE_STATUS_INVALID)? "Invalid":"Down")))));
    }
    else if (EnumType == ENUM_WAN_LINKSTATUS)
    {
        Ptr = ((Enum == WAN_IFACE_LINKSTATUS_CONFIGURING)? "Configuring":((Enum == WAN_IFACE_LINKSTATUS_UP)? "Up":"Down"));
    }
    if (Ptr != NULL)
    {
        AnscCopyString(String , Ptr);
    }
}

static void WanMgr_StringToEnum(UINT *Enum, UINT EnumType, char* String)
{

    if (EnumType == ENUM_PHY)
    {
        *Enum = (strcmp(String ,"Down") == 0)? WAN_IFACE_PHY_STATUS_DOWN:
                ((strcmp(String, "Initializing") == 0)? WAN_IFACE_PHY_STATUS_INITIALIZING:
                ((strcmp(String, "Up") == 0))? WAN_IFACE_PHY_STATUS_UP:WAN_IFACE_PHY_STATUS_UNKNOWN);
    }
    else if (EnumType == ENUM_WAN_STATUS)
    {
        *Enum = ((strcmp(String, "Initialising") == 0)? WAN_IFACE_STATUS_INITIALISING:
                ((strcmp(String, "Validating")== 0)? WAN_IFACE_STATUS_VALIDATING:
                ((strcmp(String, "Up") == 0)? WAN_IFACE_STATUS_UP:
                ((strcmp(String, "Standby") == 0)? WAN_IFACE_STATUS_STANDBY:
                ((strcmp(String, "Invalid") == 0)? WAN_IFACE_STATUS_INVALID:WAN_IFACE_STATUS_DISABLED)))));
    }
    else if (EnumType == ENUM_WAN_LINKSTATUS)
    {
        *Enum = ((strcmp(String, "Configuring") == 0)? WAN_IFACE_LINKSTATUS_CONFIGURING:
                ((strcmp(String, "Up") == 0)? WAN_IFACE_LINKSTATUS_UP:WAN_IFACE_LINKSTATUS_DOWN));
    }
}

rbusError_t wanMgrDmlPublishEventHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish)
{
    char *subscribe_action = NULL;
    UINT index = 0;
    char AliasName[64] = {0};

    CcspTraceInfo(("%s %d - Event %s has been subscribed from subscribed\n", __FUNCTION__, __LINE__,eventName ));
    subscribe_action = action == RBUS_EVENT_ACTION_SUBSCRIBE ? "subscribe" : "unsubscribe";
    CcspTraceInfo(("%s %d - action=%s \n", __FUNCTION__, __LINE__, subscribe_action ));

    if(eventName == NULL)
    {
        CcspTraceInfo(("%s %d - Property get name is NULL\n", __FUNCTION__, __LINE__));
        return RBUS_ERROR_BUS_ERROR;
    }


    sscanf(eventName, "Device.X_RDK_WanManager.CPEInterface.%d.", &index);

    if(index  == 0)
    {
        sscanf(eventName, "Device.X_RDK_WanManager.CPEInterface.%s.", &AliasName);
        index = WanMgr_GetIfaceIndexByAliasName(AliasName);
    }
    if(index <= 0)
    {
        CcspTraceError(("%s %d - Invalid index\n", __FUNCTION__, __LINE__));
        return RBUS_ERROR_INVALID_INPUT;
    }

    WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((index - 1));
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanDmlIface = &(pWanDmlIfaceData->data);

        if(strstr(eventName, ".Phy.Status"))
        {
            if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
            {
                if (pWanDmlIface->Sub.PhyStatusSub == 0)
                {
                    pWanDmlIface->Sub.PhyStatusSub = 1;
                }
                else
                {
                    pWanDmlIface->Sub.PhyStatusSub++;
                }
               CcspTraceInfo(("%s-%d : PhyStatus Sub(%d) \n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.PhyStatusSub));
            }
            else
            {
                if (pWanDmlIface->Sub.PhyStatusSub)
                    pWanDmlIface->Sub.PhyStatusSub--;
               CcspTraceInfo(("%s-%d : PhyStatus UnSub(%d) \n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.PhyStatusSub));
            }
        }
        if(strstr(eventName, ".Wan.Status"))
        {
            if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
            {
                if (pWanDmlIface->Sub.WanStatusSub == 0)
                {
                    pWanDmlIface->Sub.WanStatusSub = 1;
                }
                else
                {
                    pWanDmlIface->Sub.WanStatusSub++;
                }
                CcspTraceInfo(("%s-%d : WanStatus Sub(%d) \n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanStatusSub));
            }
            else
            {
                if (pWanDmlIface->Sub.WanStatusSub)
                    pWanDmlIface->Sub.WanStatusSub--;
                CcspTraceInfo(("%s-%d : WanStatus UnSub(%d) \n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanStatusSub));
            }
        }
       else if(strstr(eventName, ".Wan.LinkStatus"))
        {
            if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
            {
                if (pWanDmlIface->Sub.WanLinkStatusSub == 0)
                {
                    pWanDmlIface->Sub.WanLinkStatusSub = 1;
                }
                else
                {
                    pWanDmlIface->Sub.WanLinkStatusSub++;
                }
                CcspTraceInfo(("%s-%d : WanLinkStatus Sub(%d) \n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanLinkStatusSub));
            }
            else
            {
                if (pWanDmlIface->Sub.WanLinkStatusSub)
                    pWanDmlIface->Sub.WanLinkStatusSub--;
                CcspTraceInfo(("%s-%d : WanLinkStatus UnSub(%d) \n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanLinkStatusSub));
            }
        }
        else if(strstr(eventName, ".Wan.Enable"))
        {
            if (action == RBUS_EVENT_ACTION_SUBSCRIBE)
            {
                if (pWanDmlIface->Sub.WanEnableSub == 0)
                {
                    pWanDmlIface->Sub.WanEnableSub = 1;
                }
                else
                {
                    pWanDmlIface->Sub.WanEnableSub++;
                }
                CcspTraceInfo(("%s-%d : WanEnable Sub(%d) \n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanEnableSub));
            }
            else
            {
                if (pWanDmlIface->Sub.WanEnableSub)
                    pWanDmlIface->Sub.WanEnableSub--;
                CcspTraceInfo(("%s-%d : WanEnable UnSub(%d) \n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanEnableSub));
            }
        }
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }
    return RBUS_ERROR_SUCCESS;
}

rbusError_t WanMgr_Interface_GetHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts)
{
    char const* name = rbusProperty_GetName(property);
    rbusValue_t value;
    UINT index = 0;
    char AliasName[64] = {0};
    rbusError_t ret = RBUS_ERROR_SUCCESS;

    if(name == NULL)
    {
        CcspTraceInfo(("%s %d - Property get name is NULL\n", __FUNCTION__, __LINE__));
        return RBUS_ERROR_BUS_ERROR;
    }

    rbusValue_Init(&value);


    sscanf(name, "Device.X_RDK_WanManager.CPEInterface.%d.", &index);

    if(index  == 0)
    {
        sscanf(name, "Device.X_RDK_WanManager.CPEInterface.%s.", &AliasName);
        index = WanMgr_GetIfaceIndexByAliasName(AliasName);
    }
    if(index <= 0)
    {
        CcspTraceError(("%s %d - Invalid index\n", __FUNCTION__, __LINE__));
        return RBUS_ERROR_INVALID_INPUT;
    }

    WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((index - 1));
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanDmlIface = &(pWanDmlIfaceData->data);

        if(strstr(name, ".Phy.Status"))
        {
            char String[20] = {0};
            WanMgr_EnumToString(pWanDmlIface->Phy.Status, ENUM_PHY, String);
            rbusValue_SetString(value, String);
        }
        else if(strstr(name, ".Wan.Status"))
        {
            char String[20] = {0};
            if(pWanDmlIface->Wan.IfaceType == REMOTE_IFACE && pWanDmlIface->Wan.RemoteStatus != WAN_IFACE_STATUS_UP)
            {
                WanMgr_EnumToString(pWanDmlIface->Wan.RemoteStatus, ENUM_WAN_STATUS, String);
            }else
            {
                WanMgr_EnumToString(pWanDmlIface->Wan.Status, ENUM_WAN_STATUS, String);
            }
            rbusValue_SetString(value, String);
        }
        else if(strstr(name, ".Wan.LinkStatus"))
        {
            char String[20] = {0};
            WanMgr_EnumToString(pWanDmlIface->Wan.LinkStatus, ENUM_WAN_LINKSTATUS, String);
            rbusValue_SetString(value, String);
        }
        else if(strstr(name, ".Wan.Enable"))
        {
            BOOL Enable = pWanDmlIface->Wan.Enable;
            rbusValue_SetBoolean(value, Enable);
        }
        else if(strstr(name, ".AliasName"))
        {
            rbusValue_SetString(value, pWanDmlIface->AliasName);
        }
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }
    rbusProperty_SetValue(property, value);

    rbusValue_Release(value);

    return ret;
}

rbusError_t WanMgr_Interface_SetHandler(rbusHandle_t handle, rbusProperty_t prop, rbusSetHandlerOptions_t* opts)
{
    (void)opts;
    char const* name = rbusProperty_GetName(prop);
    rbusValue_t value = rbusProperty_GetValue(prop);
    rbusValueType_t type = rbusValue_GetType(value);
    rbusError_t ret = RBUS_ERROR_SUCCESS;
    UINT index = 0;
    char AliasName[64] = {0};

    if(name == NULL)
    {
        CcspTraceInfo(("%s %d - Property get name is NULL\n", __FUNCTION__, __LINE__));
        return RBUS_ERROR_BUS_ERROR;
    }


    sscanf(name, "Device.X_RDK_WanManager.CPEInterface.%d.", &index);

    if(index  == 0)
    {
        sscanf(name, "Device.X_RDK_WanManager.CPEInterface.%s.", &AliasName);
        index = WanMgr_GetIfaceIndexByAliasName(AliasName);
    }
    if(index <= 0)
    {
        CcspTraceError(("%s %d - Invalid index\n", __FUNCTION__, __LINE__));
        return RBUS_ERROR_INVALID_INPUT;
    }

    WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((index - 1));
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanDmlIface = &(pWanDmlIfaceData->data);
        if(strstr(name, ".Phy.Status"))
        {
            if (type == RBUS_STRING)
            {
                char String[20] = {0};
                AnscCopyString(String , rbusValue_GetString(value, NULL));
                WanMgr_StringToEnum(&pWanDmlIface->Phy.Status, ENUM_PHY, String);
                if (pWanDmlIface->Sub.PhyStatusSub)
                {
                    CcspTraceInfo(("%s-%d : PhyStatus Publish Event, SubCount(%d)\n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.PhyStatusSub));
                    WanMgr_Rbus_EventPublishHandler(name, &String, type);
                }
            }
            else
            {
                ret = RBUS_ERROR_INVALID_INPUT;
            }
        }
        else if(strstr(name, ".Wan.Status"))
        {
            if (type == RBUS_STRING)
            {
                char String[20] = {0};
                AnscCopyString(String , rbusValue_GetString(value, NULL));
                WanMgr_StringToEnum(&pWanDmlIface->Wan.Status, ENUM_WAN_STATUS, String);
                if (pWanDmlIface->Sub.WanStatusSub)
                {
                    CcspTraceInfo(("%s-%d : WanStatus Publish Event, SubCount(%d)\n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanStatusSub));
                    WanMgr_Rbus_EventPublishHandler(name, &String, type);
                }
            }
            else
            {
                ret = RBUS_ERROR_INVALID_INPUT;
            }
        }
        else if(strstr(name, ".Wan.LinkStatus"))
        {
            if (type == RBUS_STRING)
            {
                char String[20] = {0};
                AnscCopyString(String , rbusValue_GetString(value, NULL));
                WanMgr_StringToEnum(&pWanDmlIface->Wan.LinkStatus, ENUM_WAN_LINKSTATUS, String);
                if (pWanDmlIface->Sub.WanLinkStatusSub)
                {
                    CcspTraceInfo(("%s-%d : WanLinkStatus Publish Event, SubCount(%d)\n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanLinkStatusSub));
                    WanMgr_Rbus_EventPublishHandler(name, &String, type);
                }
            }
            else
            {
                ret = RBUS_ERROR_INVALID_INPUT;
            }
        }
        else if(strstr(name, ".Wan.Enable"))
        {
            if (type == RBUS_BOOLEAN)
            {
                BOOL Enable = FALSE;
                char param_name[256] = {0};
                _ansc_sprintf(param_name, PSM_WANMANAGER_IF_ENABLE, index);
                if(rbusValue_GetBoolean(value))
                {
                    pWanDmlIface->Wan.Enable = TRUE;
                    WanMgr_RdkBus_SetParamValuesToDB(param_name,"TRUE");
                    Enable = TRUE;
                }
                else
                {
                    pWanDmlIface->Wan.Enable = FALSE;
                    WanMgr_RdkBus_SetParamValuesToDB(param_name,"FALSE");
                    Enable = FALSE;
                }
                if (pWanDmlIface->Sub.WanEnableSub)
                {
                    CcspTraceInfo(("%s-%d : WanEnable Publish Event, SubCount(%d)\n", __FUNCTION__, __LINE__, pWanDmlIface->Sub.WanEnableSub));
                    WanMgr_Rbus_EventPublishHandler(name, &Enable, type);
                }
            }
            else
            {
                ret = RBUS_ERROR_INVALID_INPUT;
            }
        }
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }
    return ret;
}

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
    int  cpeInterfaceIndex   = -1;

    if((eventName == NULL))
    {
        CcspTraceError(("%s : FAILED , value is NULL\n",__FUNCTION__));
        return;
    }
    if (strcmp(eventName, WANMGR_DEVICE_NETWORKING_MODE) == 0)
    {
        rbusValue_t valBuff = rbusObject_GetValue(event->data, NULL );
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
    else if (strcmp(eventName, X_RDK_REMOTE_DEVICECHANGE) == 0)
    {
        CcspTraceInfo(("%s:%d Received [%s] \n",__FUNCTION__, __LINE__,eventName));

        rbusValue_t value;
        value = rbusObject_GetValue(event->data, "Capabilities");
        CcspTraceInfo(("%s %d - Capabilities %s\n", __FUNCTION__, __LINE__,rbusValue_GetString(value, NULL)));

        value = rbusObject_GetValue(event->data, "Index");
        UINT Index = rbusValue_GetUInt32(value);
        CcspTraceInfo(("%s %d - Index %d\n", __FUNCTION__, __LINE__,Index));

        value = rbusObject_GetValue(event->data, "Mac_addr");
        char *remoteMac = malloc(MAC_ADDR_SIZE + 1);

        if(remoteMac == NULL  )
        {
            CcspTraceInfo(("%s %d - Memory allocation failed \n", __FUNCTION__, __LINE__));
            return;
        }
        char *Mac = rbusValue_GetString(value, NULL);
        if(Mac != NULL)
        {
            strncpy(remoteMac,Mac,MAC_ADDR_SIZE);
            CcspTraceInfo(("%s %d - from source MAC %s\n", __FUNCTION__, __LINE__, remoteMac));
        }else
        {
            CcspTraceInfo(("%s %d - Mac_addr get failed \n", __FUNCTION__, __LINE__));
            if(remoteMac != NULL)
            {
                free(remoteMac);
            }
            return;
        }

        if (Index > 0)
        {
            DEVICE_NETWORKING_MODE  DeviceMode;
            WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
            if(pWanConfigData != NULL)
            {
                DeviceMode = pWanConfigData->data.DeviceNwMode;
                WanMgrDml_GetConfigData_release(pWanConfigData);

                if (DeviceMode == GATEWAY_MODE)
                {
                    CcspTraceInfo(("%s %d -DeviceNwMode set to GATEWAY_MODE. Configure remote Iface\n", __FUNCTION__, __LINE__));
                    WanMgr_WanRemoteIfaceConfigure(remoteMac);
                }else
                    CcspTraceInfo(("%s %d -DeviceNwMode is not GATEWAY_MODE. Do not configure remote Iface\n", __FUNCTION__, __LINE__));
            }
        }

    }
    else if(strcmp(eventName, X_RDK_REMOTE_INVOKE) == 0)
    {
        CcspTraceInfo(("%s:%d Received [%s] \n",__FUNCTION__, __LINE__,eventName));

        rbusValue_t value;
        value = rbusObject_GetValue(event->data, "Mac_source");
        char *pMac = rbusValue_GetString(value, NULL);
        CcspTraceInfo(("%s %d - from source MAC %s\n", __FUNCTION__, __LINE__, pMac));

        cpeInterfaceIndex =  WanMgr_Remote_IfaceData_index(pMac);

        if (cpeInterfaceIndex >= 0)
        {
            value = rbusObject_GetValue(event->data, "param_name");
            char *pParamName = rbusValue_GetString(value, NULL);
            CcspTraceInfo(("%s %d - param_name %s\n", __FUNCTION__, __LINE__, pParamName));

            value = rbusObject_GetValue(event->data, "param_value");
            char *pValue = rbusValue_GetString(value, NULL);
            CcspTraceInfo(("%s %d - param_value %s\n", __FUNCTION__, __LINE__, pValue));

            WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked(cpeInterfaceIndex);
            if(pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

                if( strstr(pParamName, ".Phy.Status") != NULL )
                {
                    WanMgr_StringToEnum(&pWanIfaceData->Phy.Status, ENUM_PHY, pValue);
                }

                if( strstr(pParamName, ".Wan.LinkStatus") != NULL )
                {
                    WanMgr_StringToEnum(&pWanIfaceData->Wan.LinkStatus, ENUM_WAN_LINKSTATUS, pValue);
                    if(pWanIfaceData->Wan.LinkStatus == WAN_IFACE_LINKSTATUS_UP)
                    {
                        strncpy(pWanIfaceData->Wan.Name,REMOTE_INTERFACE_NAME,sizeof(pWanIfaceData->Wan.Name));
                    }
                }
                if( strstr(pParamName, ".Wan.Status") != NULL )
                {
                    WanMgr_StringToEnum(&pWanIfaceData->Wan.RemoteStatus, ENUM_WAN_STATUS, pValue);
                }
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
        else
        {
            CcspTraceInfo(("%s %d - Remote Interface Not found \n", __FUNCTION__, __LINE__));
        }
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

    ret = rbusEvent_Subscribe(rbusHandle, WANMGR_DEVICE_NETWORKING_MODE, WanMgr_Rbus_EventReceiveHandler, NULL, 60);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, rbusError_ToString(ret), WANMGR_DEVICE_NETWORKING_MODE));
    }

    ret = rbusEvent_Subscribe(rbusHandle, X_RDK_REMOTE_DEVICECHANGE, WanMgr_Rbus_EventReceiveHandler, NULL, 60);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, rbusError_ToString(ret), X_RDK_REMOTE_DEVICECHANGE));
    }

    ret = rbusEvent_Subscribe(rbusHandle, X_RDK_REMOTE_INVOKE, WanMgr_Rbus_EventReceiveHandler, NULL, 60);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, rbusError_ToString(ret), X_RDK_REMOTE_INVOKE));
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

    ret = rbusEvent_Unsubscribe(rbusHandle, X_RDK_REMOTE_DEVICECHANGE);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, X_RDK_REMOTE_DEVICECHANGE, rbusError_ToString(ret)));
    }

    ret = rbusEvent_Unsubscribe(rbusHandle, X_RDK_REMOTE_INVOKE);
    if(ret != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Subscribe %s, Error=%s \n", __FUNCTION__, __LINE__, X_RDK_REMOTE_INVOKE, rbusError_ToString(ret)));
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

    rc = rbusMethod_InvokeAsync(rbusHandle, X_RDK_REMOTE_INVOKE, inParams, IDM_request->asyncHandle, IDM_request->timeout);

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


/***********************************************************************
  WanMgr_Rbus_Init(): Initialize Rbus and data elements
 ***********************************************************************/

ANSC_STATUS WanMgr_Rbus_Init()
{
    int rc = ANSC_STATUS_FAILURE;

    char AliasName[64] = {0};
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

    rc = rbus_regDataElements(rbusHandle, ARRAY_SZ(wanMgrIfacePublishElements), wanMgrIfacePublishElements);
    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Failed to Register Iface Table 1 Elements, Error=%d \n", __FUNCTION__, __LINE__, rc));
    }

    if(DmlGetTotalNoOfWanInterfaces(&uiTotalIfaces) != ANSC_STATUS_SUCCESS)
    {
        return ANSC_STATUS_FAILURE;
    }

    for (int i = 0; i < uiTotalIfaces; i++)
    {
        WanMgr_GetIfaceAliasNameByIndex(i,AliasName);
        rc = rbusTable_registerRow(rbusHandle, WANMGR_INFACE_TABLE, (i+1), AliasName);
        if(rc != RBUS_ERROR_SUCCESS)
        {
            CcspTraceError(("%s %d - Iterface(%d) Table (%s) UnRegistartion failed, Error=%d \n", __FUNCTION__, __LINE__, i, WANMGR_INFACE_TABLE, rc));
            return rc;
        }
        else
        {
             CcspTraceInfo(("%s %d - Iterface(%d) Table (%s) Registartion Successfully, AliasName(%s)\n", __FUNCTION__, __LINE__, i, WANMGR_INFACE_TABLE, AliasName));
        }
        memset(AliasName,0,64);
     }

    return ANSC_STATUS_SUCCESS;
}

/*******************************************************************************
  WanMgr_RbusExit(): Unreg data elements and Exit
 ********************************************************************************/
ANSC_STATUS WanMgr_RbusExit()
{
    int rc = ANSC_STATUS_FAILURE;
    char param_name[256] = {0};

    CcspTraceInfo(("%s %d - WanMgr_RbusExit called\n", __FUNCTION__, __LINE__ ));
    rbus_unregDataElements(rbusHandle, NUM_OF_RBUS_PARAMS, wanMgrRbusDataElements);

    rbus_unregDataElements(rbusHandle, wanMgrIfacePublishElements, wanMgrIfacePublishElements);
    for (int i = 0; i < uiTotalIfaces; i++)
    {
        memset(param_name, 0, sizeof(param_name));
        _ansc_sprintf(param_name, "%s.%d", WANMGR_INFACE_TABLE, (i+1));
        rc = rbusTable_unregisterRow(rbusHandle, WANMGR_INFACE_TABLE);
        if(rc != RBUS_ERROR_SUCCESS)
        {
            CcspTraceError(("%s %d - Iterface(%d) Table (%s) Registartion failed, Error=%d \n", __FUNCTION__, __LINE__, i, param_name, rc));
            return rc;
        }
        else
        {
            CcspTraceInfo(("%s %d - Iterface(%d) Table (%s) Registartion Successfully\n", __FUNCTION__, __LINE__, i, param_name));
        }
    }

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
            CcspTraceInfo(("%s %d - dm_value[%s]\n", __FUNCTION__, __LINE__, (*(bool*)(dm_value))?"True":"False"));
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
  WanMgr_Rbus_String_EventPublish_OnValueChange(): publish rbus events on value change
 ********************************************************************************/
ANSC_STATUS WanMgr_Rbus_String_EventPublish_OnValueChange(char *dm_event, void *prev_dm_value, void *dm_value)
{
    rbusEvent_t event;
    rbusObject_t rdata;
    rbusValue_t Value, preValue, byVal;
    int rc = ANSC_STATUS_FAILURE;

    if(dm_event == NULL || dm_value == NULL)
    {
        CcspTraceInfo(("%s %d - Failed publishing\n", __FUNCTION__, __LINE__));
        return rc;
    }

    rbusValue_Init(&Value);
    rbusValue_SetString(Value, (char*)dm_value);

    rbusValue_Init(&preValue);
    rbusValue_SetString(preValue, (char*)prev_dm_value);

    rbusValue_Init(&byVal);
    rbusValue_SetString(byVal, componentName);

    rbusObject_Init(&rdata, NULL);
    rbusObject_SetValue(rdata, "value", Value);
    rbusObject_SetValue(rdata, "oldValue", preValue);
    rbusObject_SetValue(rdata, "by", byVal);

    event.name = dm_event;
    event.data = rdata;
    event.type = RBUS_EVENT_VALUE_CHANGED;

    CcspTraceInfo(("%s %d - dm_event[%s],prev_dm_value[%s],dm_value[%s]\n", __FUNCTION__, __LINE__, dm_event, prev_dm_value, dm_value));

    if(rbusEvent_Publish(rbusHandle, &event) != RBUS_ERROR_SUCCESS) 
    {
        CcspTraceInfo(("%s %d - event publishing failed for type\n", __FUNCTION__, __LINE__));
    }
    else
    {
        CcspTraceInfo(("%s %d - Successfully Published event for event %s \n", __FUNCTION__, __LINE__, dm_event));
        rc = ANSC_STATUS_SUCCESS;
    }

    rbusValue_Release(Value);
    rbusValue_Release(preValue);
    rbusValue_Release(byVal);
    rbusObject_Release(rdata);
    return rc;
}

/*******************************************************************************
  WanMgr_Rbus_String_EventPublish(): publish rbus string events
 ********************************************************************************/
ANSC_STATUS WanMgr_Rbus_String_EventPublish(char *dm_event, void *dm_value)
{
    if (strstr(dm_event, ".Wan.Status"))
    {
       char String[20] = {0};
       WanMgr_EnumToString((*(UINT *)dm_value), ENUM_WAN_STATUS, String);
        CcspTraceInfo(("%s-%d : WanStatus(%s)(%d) Publish\n", __FUNCTION__, __LINE__, String, (*(UINT *)dm_value)));
        return WanMgr_Rbus_EventPublishHandler(dm_event, &String, RBUS_STRING);
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

void WanMgr_WanRemoteIfaceConfigure_thread(void *arg);

ANSC_STATUS WanMgr_WanRemoteIfaceConfigure(char *remoteMac)
{
    pthread_t                threadId;
    int                      iErrorCode     = 0;

    iErrorCode = pthread_create( &threadId, NULL, &WanMgr_WanRemoteIfaceConfigure_thread, remoteMac);
    if( 0 != iErrorCode )
    {
        CcspTraceInfo(("%s %d - Failed to start WanMgr_WanRemoteIfaceConfigure_thread EC:%d\n", __FUNCTION__, __LINE__, iErrorCode ));
        if(remoteMac != NULL)
        {
            free(remoteMac);
        }
        return ANSC_STATUS_FAILURE;
    }
    else
    {
        CcspTraceInfo(("%s %d - WanMgr_WanRemoteIfaceConfigure_thread Started Successfully\n", __FUNCTION__, __LINE__ ));
    }
        return ANSC_STATUS_SUCCESS;
}
void WanMgr_WanRemoteIfaceConfigure_thread(void *arg)
{
    int  cpeInterfaceIndex   = -1;
    int  newInterfaceIndex   = -1;
    int  rc = ANSC_STATUS_FAILURE;
    char *remoteMac =  (char*)arg;
    char AliasName[64] = {0};

    CcspTraceInfo(("%s %d - Enter \n", __FUNCTION__, __LINE__));

    pthread_detach(pthread_self());

    CcspTraceInfo(("%s %d - remoteMac %s \n", __FUNCTION__, __LINE__,remoteMac));
    // check the interface table and return index of the match
    cpeInterfaceIndex =  WanMgr_Remote_IfaceData_index(remoteMac);

    if(cpeInterfaceIndex >= 0)
    {
        CcspTraceInfo(("%s %d - [%s] MAC  Already have an Entry \n", __FUNCTION__, __LINE__, remoteMac));
        if(remoteMac != NULL)
        {
            free(remoteMac);
        }
        return ANSC_STATUS_SUCCESS;
    }

    // Initialise remote CPE's wan interface with default
    WanMgr_Iface_Data_t * pIfaceData = WanMgr_Remote_IfaceData_configure(remoteMac, &newInterfaceIndex);

    if(pIfaceData == NULL)
    {
        CcspTraceInfo(("%s %d - Failed to configure remote Wan Interface Entry.\n", __FUNCTION__, __LINE__));
        if(remoteMac != NULL)
        {
            free(remoteMac);
        }
        return ANSC_STATUS_FAILURE;
    }

    idm_invoke_method_Params_t IDM_request;
    /* IDM get request*/
    for (int i =0; i< ARRAY_SZ(RemoteDMs); i++)
    {
        if(RemoteDMs[i].get_request)
        {
            /* IDM get request*/
            CcspTraceInfo(("%s %d - Requesting  %s \n", __FUNCTION__, __LINE__,RemoteDMs[i]));
            memset(&IDM_request,0, sizeof(idm_invoke_method_Params_t));

            /* Update request parameters */
            strcpy(IDM_request.Mac_dest,remoteMac);
            strcpy(IDM_request.param_name, RemoteDMs[i].name);
            strcpy(IDM_request.pComponent_name, "eRT.com.cisco.spvtg.ccsp.wanmanager");
            strcpy(IDM_request.pBus_path, "/com/cisco/spvtg/ccsp/wanmanager");
            IDM_request.timeout = 30;
            IDM_request.operation = IDM_GET;
            IDM_request.asyncHandle = &CPEInterface_AsyncMethodHandler; //pointer to callback

            WanMgr_IDM_Invoke(&IDM_request);
        }
    }

    sleep(3);
    /* IDM subscription request*/
    for (int i =0; i< ARRAY_SZ(RemoteDMs); i++)
    {
        if(RemoteDMs[i].subscription_request)
        {
            /* IDM get request*/
            CcspTraceInfo(("%s %d - Requesting  subscription %s \n", __FUNCTION__, __LINE__,RemoteDMs[i]));
            memset(&IDM_request,0, sizeof(idm_invoke_method_Params_t));

            /* Update request parameters */
            strcpy(IDM_request.Mac_dest,remoteMac);
            strcpy(IDM_request.param_name, RemoteDMs[i].name);
            IDM_request.timeout = 600;
            IDM_request.operation = IDM_SUBS;
            IDM_request.asyncHandle = &CPEInterface_AsyncMethodHandler; //pointer to callback

            WanMgr_IDM_Invoke(&IDM_request);
        }
    }

    WanMgr_GetIfaceAliasNameByIndex(newInterfaceIndex,AliasName);

    CcspTraceError(("%s %d - Iterface(%d) AliasName[%s]\n", __FUNCTION__, __LINE__, newInterfaceIndex, AliasName));
    rc = rbusTable_registerRow(rbusHandle, WANMGR_INFACE_TABLE, (newInterfaceIndex+1), AliasName);

    if(rc != RBUS_ERROR_SUCCESS)
    {
        CcspTraceError(("%s %d - Iterface(%d) Table (%s) UnRegistartion failed, Error=%d \n", __FUNCTION__, __LINE__, (newInterfaceIndex+1), WANMGR_INFACE_TABLE, rc));
        if(remoteMac != NULL)
        {
            free(remoteMac);
        }
        return rc;
    }
    else
    {
        CcspTraceInfo(("%s %d - Iterface(%d) Table (%s) Registartion Successfully AliasName[%s]\n", __FUNCTION__, __LINE__, (newInterfaceIndex+1), WANMGR_INFACE_TABLE,AliasName));
    }

    if(remoteMac != NULL)
    {
        free(remoteMac);
    }
    pthread_exit(NULL);

    return ANSC_STATUS_SUCCESS;
}

static void CPEInterface_AsyncMethodHandler(
    rbusHandle_t handle,
    char const* methodName,
    rbusError_t error,
    rbusObject_t params)
{
    (void)handle;
    int cpeInterfaceIndex = -1;

    CcspTraceInfo(("%s %d - asyncMethodHandler called: %s  error=%d\n", __FUNCTION__, __LINE__, methodName, error));
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
        char *pMac = rbusValue_GetString(value, NULL);
        CcspTraceInfo(("%s %d - from source MAC %s\n", __FUNCTION__, __LINE__, pMac));

        cpeInterfaceIndex =  WanMgr_Remote_IfaceData_index(pMac);

        if (cpeInterfaceIndex >= 0)
        {
            value = rbusObject_GetValue(params, "param_name");
            char *pParamName = rbusValue_GetString(value, NULL);
            CcspTraceInfo(("%s %d - param_name %s\n", __FUNCTION__, __LINE__, pParamName));

            value = rbusObject_GetValue(params, "param_value");
            char *pValue = rbusValue_GetString(value, NULL);
            CcspTraceInfo(("%s %d - param_value %s\n", __FUNCTION__, __LINE__, pValue));

            WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked(cpeInterfaceIndex);
            if(pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

                if( strstr(pParamName, ".Name") != NULL)
                {
                    strcpy( pWanIfaceData->Name, pValue );
                }

                if( strstr(pParamName, ".DisplayName") != NULL)
                {
                    char DisplayName[64] = {0};
                    snprintf(DisplayName, sizeof(DisplayName), "REMOTE_%s" , pValue);
                    strcpy( pWanIfaceData->DisplayName, DisplayName );
                }

                if( strstr(pParamName, ".AliasName") != NULL)
                {
                    char AliasName[64] = {0};
                    snprintf(AliasName, sizeof(AliasName), "REMOTE_%s" , pValue);
                    strcpy( pWanIfaceData->AliasName, AliasName );
                }

                if( strstr(pParamName, ".Phy.Status") != NULL )
                {
                    WanMgr_StringToEnum(&pWanIfaceData->Phy.Status, ENUM_PHY, pValue);
                }

                if( strstr(pParamName, ".Wan.LinkStatus") != NULL )
                {
                    WanMgr_StringToEnum(&pWanIfaceData->Wan.LinkStatus, ENUM_WAN_LINKSTATUS, pValue);
                    if(pWanIfaceData->Wan.LinkStatus == WAN_IFACE_LINKSTATUS_UP)
                    {
                        strncpy(pWanIfaceData->Wan.Name, REMOTE_INTERFACE_NAME, sizeof(pWanIfaceData->Wan.Name));
                    }
                }
                if( strstr(pParamName, ".Wan.Status") != NULL )
                {
                    WanMgr_StringToEnum(&pWanIfaceData->Wan.RemoteStatus, ENUM_WAN_STATUS, pValue);
                }
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
        else
        {
            CcspTraceInfo(("%s %d - Remote Interface Not found \n", __FUNCTION__, __LINE__));
        }
    }
}

static int WanMgr_Remote_IfaceData_index(char *macAddress)
{
    int cpeInterfaceIndex = -1;
    UINT uiTotalIfaces = 0;
    UINT uiLoopCount   = 0;

    CcspTraceInfo(("%s %d - index search for macAddress=[%s]\n", __FUNCTION__, __LINE__, macAddress));

    //Get uiTotalIfaces
    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();

    if(uiTotalIfaces > 0)
    {
        for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
        {

            WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
            if(pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                if((0 == strncasecmp(pWanIfaceData->RemoteCPEMac, macAddress, BUFLEN_128)) && (pWanIfaceData->Wan.IfaceType == REMOTE_IFACE))
                {
                    cpeInterfaceIndex = uiLoopCount ;
                }

                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
            if(cpeInterfaceIndex >= 0)
            {
                break;
            }
        }
    }
    CcspTraceInfo(("%s %d - End cpeInterfaceIndex=[%d]\n", __FUNCTION__, __LINE__, cpeInterfaceIndex));
    return cpeInterfaceIndex;
}

ANSC_STATUS WanMgr_RestartUpdateRemoteIface()
{
    char dmQuery[BUFLEN_256] = {0};
    char dmValue[BUFLEN_256] = {0};
    int numofRemoteEntries = 0;

    CcspTraceInfo(("%s %d - Enter \n", __FUNCTION__, __LINE__));

    DEVICE_NETWORKING_MODE  DeviceMode;
    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        DeviceMode = pWanConfigData->data.DeviceNwMode;
        WanMgrDml_GetConfigData_release(pWanConfigData);

        if (DeviceMode == GATEWAY_MODE)
        {
            CcspTraceInfo(("%s %d -DeviceNwMode set to GATEWAY_MODE. Configure remote Iface\n", __FUNCTION__, __LINE__));
        }
        else
        {
            CcspTraceInfo(("%s %d -DeviceNwMode is not GATEWAY_MODE. Do not configure remote Iface\n", __FUNCTION__, __LINE__));
            return ANSC_STATUS_SUCCESS;
        }
    }

    // get mac of connected remote CPE
    snprintf(dmQuery, sizeof(dmQuery)-1, X_RDK_REMOTE_DEVICE_NUM_OF_ENTRIES);

    if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
    {
        CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
        return ANSC_STATUS_FAILURE;
    }

    numofRemoteEntries = atoi(dmValue);
    CcspTraceInfo(("%s %d - numofRemoteEntries = %d\n", __FUNCTION__, __LINE__,numofRemoteEntries));

    //IDM first entry will be local device.
    if(numofRemoteEntries > 1)
    {
        for( int i = 2; i <= numofRemoteEntries; i++)
        {
            memset(dmQuery, 0, sizeof(dmQuery));
            memset(dmValue, 0, sizeof(dmValue));
            snprintf(dmQuery, sizeof(dmQuery)-1, X_RDK_REMOTE_DEVICE_MAC, i);
            if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
            {
                CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
            }
            CcspTraceInfo(("%s %d - Remote device MAC %s \n", __FUNCTION__, __LINE__,dmValue));
            char *remoteMac = malloc(MAC_ADDR_SIZE + 1);
            strncpy(remoteMac, dmValue, MAC_ADDR_SIZE+1);
            WanMgr_WanRemoteIfaceConfigure(remoteMac);
        }
    }
    return ANSC_STATUS_SUCCESS;
}
#endif //RBUS_BUILD_FLAG_ENABLE
