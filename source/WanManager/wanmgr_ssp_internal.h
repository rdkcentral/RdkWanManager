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
/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/


/*********************************************************************************

    description:

        This is the template file of wanmgr_ssp_internal.h for XxxxSsp.
        Please replace "XXXX" with your own ssp name with the same up/lower cases.

    ------------------------------------------------------------------------------

    revision:

        09/08/2011    initial revision.

**********************************************************************************/

#ifndef  _WANMGR_SSP_INTERNAL_H_
#define  _WANMGR_SSP_INTERNAL_H_

#define  COMMON_COMPONENT_HEALTH_Red                   1
#define  COMMON_COMPONENT_HEALTH_Yellow                2
#define  COMMON_COMPONENT_HEALTH_Green                 3

#define  COMMON_COMPONENT_STATE_Initializing           1
#define  COMMON_COMPONENT_STATE_Running                2
#define  COMMON_COMPONENT_STATE_Blocked                3
#define  COMMON_COMPONENT_STATE_Paused                 3

#define  COMMON_COMPONENT_FREERESOURCES_PRIORITY_High  1
#define  COMMON_COMPONENT_FREERESOURCES_PRIORITY_Low   2

#define  COMPONENT_ID_WANMANAGER                             "com.cisco.spvtg.ccsp.wanmanager"
#define  COMPONENT_NAME_WANMANAGER                          "com.cisco.spvtg.ccsp.wanmanager"
#define  COMPONENT_VERSION_WANMANAGER                        1
#define  COMPONENT_PATH_WANMANAGER                           "/com/cisco/spvtg/ccsp/wanmanager"
#define  WAN_COMPONENT_NAME                                  "eRT.com.cisco.spvtg.ccsp.wanmanager"

#define  MESSAGE_BUS_CONFIG_FILE                            "msg_daemon.cfg"

typedef  struct _COMPONENT_COMMON_WANMANAGER
{
    char*                           Name;
    ULONG                           Version;
    char*                           Author;
    ULONG                           Health;
    ULONG                           State;

    BOOL                            LogEnable;
    ULONG                           LogLevel;

    ULONG                           MemMaxUsage;
    ULONG                           MemMinUsage;
    ULONG                           MemConsumed;
} COMPONENT_COMMON_WANMANAGER,  *PCOMPONENT_COMMON_WANMANAGER;

#define ComponentCommonDmInit(component_com_wanmanager)                                          \
        {                                                                                  \
            AnscZeroMemory(component_com_wanmanager, sizeof(COMPONENT_COMMON_WANMANAGER));             \
            component_com_wanmanager->Name        = NULL;                                        \
            component_com_wanmanager->Version     = 1;                                           \
            component_com_wanmanager->Author      = NULL;                                        \
            component_com_wanmanager->Health      = COMMON_COMPONENT_HEALTH_Red;            \
            component_com_wanmanager->State       = COMMON_COMPONENT_STATE_Running;         \
            if(g_iTraceLevel >= CCSP_TRACE_LEVEL_EMERGENCY)                                \
                component_com_wanmanager->LogLevel = (ULONG) g_iTraceLevel;                      \
            component_com_wanmanager->LogEnable   = TRUE;                                        \
            component_com_wanmanager->MemMaxUsage = 0;                                           \
            component_com_wanmanager->MemMinUsage = 0;                                           \
            component_com_wanmanager->MemConsumed = 0;                                           \
        }


#define  ComponentCommonDmClean(component_com_wanmanager)                                        \
         {                                                                                  \
            if ( component_com_wanmanager->Name )                                                \
            {                                                                               \
                AnscFreeMemory(component_com_wanmanager->Name);                                  \
            }                                                                               \
                                                                                            \
            if ( component_com_wanmanager->Author )                                              \
            {                                                                               \
                AnscFreeMemory(component_com_wanmanager->Author);                                \
            }                                                                               \
         }


#define  ComponentCommonDmFree(component_com_wanmanager)                                         \
         {                                                                                  \
            ComponentCommonDmClean(component_com_wanmanager);                                    \
            AnscFreeMemory(component_com_wanmanager);                                            \
         }

int  cmd_dispatch(int  command);


ANSC_STATUS ssp_create();
ANSC_STATUS ssp_engage();
ANSC_STATUS ssp_cancel();


char* ssp_CcdIfGetComponentName(ANSC_HANDLE hThisObject);
ULONG ssp_CcdIfGetComponentVersion(ANSC_HANDLE hThisObject);
char* ssp_CcdIfGetComponentAuthor(ANSC_HANDLE hThisObject);
ULONG ssp_CcdIfGetComponentHealth(ANSC_HANDLE hThisObject);
ULONG ssp_CcdIfGetComponentState(ANSC_HANDLE hThisObject);

BOOL ssp_CcdIfGetLoggingEnabled(ANSC_HANDLE hThisObject);
ANSC_STATUS ssp_CcdIfSetLoggingEnabled(ANSC_HANDLE hThisObject, BOOL bEnabled);
ULONG ssp_CcdIfGetLoggingLevel(ANSC_HANDLE hThisObject);
ANSC_STATUS ssp_CcdIfSetLoggingLevel(ANSC_HANDLE hThisObject, ULONG LogLevel);

ULONG ssp_CcdIfGetMemMaxUsage(ANSC_HANDLE hThisObject);
ULONG ssp_CcdIfGetMemMinUsage(ANSC_HANDLE hThisObject);
ULONG ssp_CcdIfGetMemConsumed(ANSC_HANDLE hThisObject);
ANSC_STATUS ssp_CcdIfApplyChanges(ANSC_HANDLE hThisObject);


#endif //_WANMGR_SSP_INTERNAL_H_
