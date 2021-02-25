/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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

#include "wanmgr_platform_events.h"
#include "wanmgr_sysevents.h"
#include "platform_hal.h"


ANSC_STATUS WanMgr_UpdatePlatformStatus(eWanMgrPlatformStatus platform_status)
{
    ANSC_STATUS retStatus = ANSC_STATUS_SUCCESS;

#ifdef _HUB4_PRODUCT_REQ_

    char region[BUFLEN_16] = {0};

    switch(platform_status)
    {
        case WANMGR_DISCONNECTED:
            wanmgr_sysevents_setWanLedState(LED_OFF_STR);
            break;
        case WANMGR_LINK_UP:
            wanmgr_sysevents_setWanLedState(LED_FLASHING_AMBER_STR);
            break;
        case WANMGR_LINK_V6UP_V4DOWN:
	    if(platform_hal_GetRouterRegion(region) == RETURN_OK)
            {
                if(strncmp(region, "IT", sizeof(region)) == 0)
                {
                    wanmgr_sysevents_setWanLedState(LED_FLASHING_GREEN_STR);
                }
                else
                {
                    wanmgr_sysevents_setWanLedState(LED_SOLID_AMBER_STR);
                }
            }
            break;
	case WANMGR_LINK_V4UP_V6DOWN:
	    if(platform_hal_GetRouterRegion(region) == RETURN_OK)
            {
                if(strncmp(region, "IT", sizeof(region)) == 0)
                {
                    wanmgr_sysevents_setWanLedState(LED_SOLID_AMBER_STR);
                }
                else
                {
                    wanmgr_sysevents_setWanLedState(LED_SOLID_GREEN_STR);
                }
            }
            break;
        case WANMGR_CONNECTING:
            wanmgr_sysevents_setWanLedState(LED_SOLID_AMBER_STR);
            break;
        case WANMGR_CONNECTED:
            wanmgr_sysevents_setWanLedState(LED_SOLID_GREEN_STR);
            break;
        default:
            retStatus = ANSC_STATUS_FAILURE;
            break;

    }
#endif //_HUB4_PRODUCT_REQ_

    return retStatus;
}
