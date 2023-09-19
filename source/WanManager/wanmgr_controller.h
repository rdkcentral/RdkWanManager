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


#ifndef _WANMGR_CONTROLLER_H_
#define _WANMGR_CONTROLLER_H_
#include "ansc_platform.h"
#include "ansc_load_library.h"
#include "wanmgr_data.h"

typedef  struct _WANMGR_POLICY_CONTROLLER_
{
    BOOL                    WanEnable;
    INT                     activeInterfaceIdx;
    INT                     selSecondaryInterfaceIdx;
    WanMgr_Iface_Data_t*    pWanActiveIfaceData;
    struct timespec         SelectionTimeOutStart;
    struct timespec         SelectionTimeOutEnd;
    UINT                    InterfaceSelectionTimeOut;
    UINT                    TotalIfaces;
    INT                     WanOperationalMode;
    BOOL                    GroupCfgChanged;
    BOOL                    GroupPersistSelectedIface;
    BOOL                    ResetSelectedInterface;
    BOOL                    AllowRemoteInterfaces;
    UINT                    GroupIfaceList;
    UINT                    GroupInst;
} WanMgr_Policy_Controller_t;


ANSC_STATUS WanController_Init_StateMachine(void);
ANSC_STATUS WanMgr_Controller_PolicyCtrlInit(WanMgr_Policy_Controller_t* pWanPolicyCtrl);


/* Policies routines */
/*
ANSC_STATUS WanMgr_Policy_FixedModePolicy(void);
ANSC_STATUS WanMgr_Policy_FixedModeOnBootupPolicy(void);
ANSC_STATUS WanMgr_Policy_PrimaryPriorityPolicy(void);
ANSC_STATUS WanMgr_Policy_PrimaryPriorityOnBootupPolicy(void);
*/
ANSC_STATUS WanMgr_Policy_AutoWan(void);
ANSC_STATUS WanMgr_Policy_AutoWanPolicy(void);

#endif /*_WANMGR_CONTROLLER_H_*/
