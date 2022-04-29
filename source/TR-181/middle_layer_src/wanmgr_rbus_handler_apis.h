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

#ifndef _WANMGR_RBUS_H_
#define _WANMGR_RBUS_H_

#include "ansc_platform.h"

#define WANMGR_DEVICE_NETWORKING_MODE     "Device.X_RDKCENTRAL-COM_DeviceControl.DeviceNetworkingMode"

ANSC_STATUS WanMgr_Rbus_Init();
ANSC_STATUS WanMgr_Rbus_Exit();
ANSC_STATUS WanMgr_Rbus_String_EventPublish(char *dm_event, char *dm_value);

#endif
