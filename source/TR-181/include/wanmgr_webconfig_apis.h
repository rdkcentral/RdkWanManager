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


#ifndef  _WANMGR_WAN_APIS_H
#define  _WANMGR_WAN_APIS_H

/* ---- Include Files ---------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include "ansc_platform.h"
#include "ansc_string_util.h"

 /***********************************
     Actual definition declaration
 ************************************/
 /*
     Wan Part
 */

#if defined(WAN_MANAGER_UNIFICATION_ENABLED)
#define TABLE_NAME_WAN_INTERFACE           "Device.X_RDK_WanManager.Interface."
#define PARAM_NAME_WAN_REFRESH             "Reset"
#else
#define TABLE_NAME_WAN_INTERFACE           "Device.X_RDK_WanManager.CPEInterface."
#define PARAM_NAME_WAN_REFRESH             "Refresh"
#endif 

#define TABLE_NAME_WAN_MARKING             "Marking."

#define PARAM_NAME_MARK_ALIAS              "Alias"
#define PARAM_NAME_IF_NAME                 "Name"
#define PARAM_NAME_ETHERNET_PRIORITY_MARK  "EthernetPriorityMark"
#define PARAM_NAME_ALLOW_REMOTE_IFACE      "AllowRemoteInterfaces"

ANSC_STATUS WanMgrDmlWanDataSet(const void *pData, size_t len);
ANSC_STATUS WanMgrDmlWanWebConfigInit( void );
int WanMgr_WanData_Rollback_Handler(void);
size_t WanMgr_WanData_Timeout_Handler(size_t numOfEntries);
void WanMgr_WanData_Free_Resources(void *arg);

#endif /* _WANMGR_WAN_APIS_H */
