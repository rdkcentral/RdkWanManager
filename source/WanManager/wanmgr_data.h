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

//WAN CONFIG
typedef struct _WANMGR_CONFIG_DATA_
{
    DML_WANMGR_CONFIG       data;
    pthread_mutex_t         mDataMutex;
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
    pthread_mutex_t             mDataMutex;
}WanMgr_IfaceCtrl_Data_t;



typedef struct _WANMGR_DATA_ST_
{
    //WAN CONFIG
    WanMgr_Config_Data_t        Config;

    //WAN IFACE
    WanMgr_IfaceCtrl_Data_t     IfaceCtrl;
} WANMGR_DATA_ST;


//WAN CONFIG
WanMgr_Config_Data_t* WanMgr_GetConfigData_locked(void);
void WanMgrDml_GetConfigData_release(WanMgr_Config_Data_t* pWanConfigData);
void WanMgr_SetConfigData_Default(DML_WANMGR_CONFIG* pWanDmlConfig);

//WAN IFACE
WanMgr_Iface_Data_t* WanMgr_GetIfaceData_locked(UINT iface_index);
WanMgr_Iface_Data_t* WanMgr_GetIfaceDataByName_locked(char* iface_name);
void WanMgrDml_GetIfaceData_release(WanMgr_Iface_Data_t* pWanIfaceData);
void WanMgr_IfaceData_Init(WanMgr_Iface_Data_t* pIfaceData, UINT uiInstNumber);


//WAN IFACE CTRL
WanMgr_IfaceCtrl_Data_t* WanMgr_GetIfaceCtrl_locked(void);
void WanMgrDml_GetIfaceCtrl_release(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl);
void WanMgr_SetIfaceCtrl_Default(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl);

// WAN MGR
void WanMgr_Data_Init(void);
ANSC_STATUS WanMgr_Data_Delete(void);



#endif  //_WANMGR_DATA_H_
