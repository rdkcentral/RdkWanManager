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

#ifndef _WANMGR_RBUS_H_
#define _WANMGR_RBUS_H_
#ifdef RBUS_BUILD_FLAG_ENABLE
#include "ansc_platform.h"

#define NUM_OF_RBUS_PARAMS                           2
#define WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE     "Device.X_RDK_WanManager.CurrentActiveInterface"
#define WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE    "Device.X_RDK_WanManager.CurrentStandbyInterface"
#define WANMGR_DEVICE_NETWORKING_MODE                "Device.X_RDKCENTRAL-COM_DeviceControl.DeviceNetworkingMode"

ANSC_STATUS WanMgr_Rbus_Init();
ANSC_STATUS WanMgr_Rbus_Exit();
ANSC_STATUS WanMgr_Rbus_String_EventPublish(char *dm_event, char *dm_value);
ANSC_STATUS WanMgr_Rbus_getUintParamValue(char * param, UINT * value);
#endif //RBUS_BUILD_FLAG_ENABLE
#endif
