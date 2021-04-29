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


#ifndef _WANMGR_PLATFORM_EVENTS_H_
#define _WANMGR_PLATFORM_EVENTS_H_
/* ---- Global Types -------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include "wanmgr_rdkbus_common.h"

#ifdef _HUB4_PRODUCT_REQ_

#define CONSOLE_LOG_FILE "/rdklogs/logs/Consolelog.txt.0"
#define LOG_CONSOLE(fmt ...)     {\
                                        FILE     *fp        = NULL;\
                                        fp = fopen ( CONSOLE_LOG_FILE, "a+");\
                                        if (fp)\
                                        {\
                                            fprintf(fp,fmt);\
                                            fclose(fp);\
                                        }\
                               }\


#else

#define LOG_CONSOLE()

#endif //_HUB4_PRODUCT_REQ_


typedef enum
{
    WANMGR_DISCONNECTED = 1,
    WANMGR_LINK_UP,
    WANMGR_LINK_V6UP_V4DOWN,
    WANMGR_LINK_V4UP_V6DOWN,
    WANMGR_CONNECTING,
    WANMGR_CONNECTED
} eWanMgrPlatformStatus;


/***************************************************************************
 * @brief Function used to inform the platform the status of WAN Manager
 * @param  platform_status Current WAN Manager status
 * @return ANSC_STATUS_SUCCESS upon success else returned error code.
 ***************************************************************************/
ANSC_STATUS WanMgr_UpdatePlatformStatus(eWanMgrPlatformStatus platform_status);

#endif /*_WANMGR_PLATFORM_EVENTS_H_*/
