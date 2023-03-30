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

#ifndef  _WANMGR_DATA_H_
#define  _WANMGR_DATA_H_

#include <stdbool.h>
#include "ansc_platform.h"
#include "ansc_string_util.h"
#include "wanmgr_dml.h"
#include "wanmgr_rbus_handler_apis.h"
#include "wanmgr_rdkbus_utils.h"
#include "wanmgr_utils.h"

//Minimum SelectionTimeOut Value
#define SELECTION_TIMEOUT_DEFAULT_MIN 20
#define MAX_INTERFACE_GROUP           2
#define MAX_WAN_INTERFACE_ENTRY       32
#define REMOTE_INTERFACE_GROUP        2
//WAN CONFIG
typedef struct _WANMGR_CONFIG_DATA_
{
    DML_WANMGR_CONFIG       data;
} WanMgr_Config_Data_t;


//WAN IFACE
typedef struct _WANMGR_IFACE_DATA_
{
    DML_WAN_IFACE           data;
}WanMgr_Iface_Data_t;


typedef struct _WANMGR_IFACECTRL_DATA_
{
    UINT                        ulTotalNumbWanInterfaces;
    WanMgr_Iface_Data_t*        pIface;
    UINT                        update;
}WanMgr_IfaceCtrl_Data_t;

typedef struct _WANMGR_IFACE_GROUP_DATA_
{
    pthread_t          ThreadId;
    UINT               GroupState;
    UINT               InterfaceAvailable;
    UINT               SelectedInterface;
    BOOL               GroupIfaceListChanged;
    UINT               GroupSelectionTimeOut;
}WANMGR_IFACE_GROUP;

typedef struct _WANMGR_IFACE_GROUP_
{
    UINT                      ulTotalNumbWanIfaceGroup;
    WANMGR_IFACE_GROUP        Group[MAX_INTERFACE_GROUP];
}WanMgr_IfaceGroup_t;

typedef struct _WANMGR_DATA_ST_
{
    //Mutex
    pthread_mutex_t             gDataMutex;

    //WAN CONFIG
    WanMgr_Config_Data_t        Config;

    //WAN IFACE
    WanMgr_IfaceCtrl_Data_t     IfaceCtrl;

    //Iface Group
    WanMgr_IfaceGroup_t         IfaceGroup;
} WANMGR_DATA_ST;


//WAN CONFIG
WanMgr_Config_Data_t* WanMgr_GetConfigData_locked(void);
void WanMgrDml_GetConfigData_release(WanMgr_Config_Data_t* pWanConfigData);
void WanMgr_SetConfigData_Default(DML_WANMGR_CONFIG* pWanDmlConfig);

//WAN IFACE
UINT WanMgr_IfaceData_GetTotalWanIface(void);
WanMgr_Iface_Data_t* WanMgr_GetIfaceData_locked(UINT iface_index);
WanMgr_Iface_Data_t* WanMgr_GetIfaceDataByName_locked(char* iface_name);
void WanMgrDml_GetIfaceData_release(WanMgr_Iface_Data_t* pWanIfaceData);
void WanMgr_IfaceData_Init(WanMgr_Iface_Data_t* pIfaceData, UINT uiInstNumber);
ANSC_STATUS WanMgr_WanDataInit(void);
void WanMgr_GetIfaceAliasNameByIndex(UINT iface_index, char *AliasName);
UINT WanMgr_GetIfaceIndexByAliasName(char* AliasName);

//WAN IFACE CTRL
void WanMgr_SetIfaceCtrl_Default(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl);

// WAN MGR
void WanMgr_Data_Init(void);
ANSC_STATUS WanMgr_Data_Delete(void);

// WAN MGR REMOTE INTERFACE
ANSC_STATUS WanMgr_Remote_IfaceData_configure(char *remoteCPEMac, int *iface_index);
void WanMgr_Remote_IfaceData_Init(WanMgr_Iface_Data_t* pIfaceData, UINT iface_index);
#endif  //_WANMGR_DATA_H_
