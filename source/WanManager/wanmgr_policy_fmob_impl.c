/*
   If not stated otherwise in this file or this component's Licenses.txt file the
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

/* ---- Include Files ---------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "wanmgr_controller.h"
#include "wanmgr_data.h"
#include "wanmgr_rdkbus_utils.h"
#include "wanmgr_interface_sm.h"
#include "wanmgr_platform_events.h"

/* ---- Global Constants -------------------------- */
#define LOOP_TIMEOUT 500000 // timeout in milliseconds. This is the state machine loop interval


/* fixed mode policy states */
typedef enum {
    STATE_FIXING_WAN_INTERFACE = 0,
    STATE_FIXED_WAN_INTERFACE_DOWN,
    STATE_FIXED_WAN_INTERFACE_UP
} WcFmobPolicyState_t;


/* STATES */
static WcFmobPolicyState_t State_FixingWanInterface(WanMgr_Policy_Controller_t* pWanController);
static WcFmobPolicyState_t State_FixedWanInterfaceDown(WanMgr_Policy_Controller_t* pWanController);
static WcFmobPolicyState_t State_FixedWanInterfaceUp(WanMgr_Policy_Controller_t* pWanController);

/* TRANSITIONS */
static WcFmobPolicyState_t Transition_Start(WanMgr_Policy_Controller_t* pWanController);
static WcFmobPolicyState_t Transition_WanInterfaceFixed(WanMgr_Policy_Controller_t* pWanController);
static WcFmobPolicyState_t Transition_FixedInterfaceUp(WanMgr_Policy_Controller_t* pWanController);
static WcFmobPolicyState_t Transition_FixedInterfaceDown(WanMgr_Policy_Controller_t* pWanController);

/*********************************************************************************/
/**************************** ACTIONS ********************************************/
/*********************************************************************************/
static INT WanMgr_Policy_FM_SelectWANActive(void)
{
    UINT uiLoopCount;
    INT iActiveWanIdx = -1;
    UINT uiTotalIfaces = -1;
    INT  iActivePriority = DML_WAN_IFACE_PRIORITY_MAX;

    //Get uiTotalIfaces
    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    if(uiTotalIfaces > 0)
    {
        // Check the policy to determine if any primary interface should be used for WAN
        for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
        {
            WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
            if(pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                if ((pWanIfaceData->Wan.Enable == TRUE) && (pWanIfaceData->Wan.Type == WAN_IFACE_TYPE_PRIMARY))
                {
                    if(pWanIfaceData->Wan.Priority < iActivePriority)
                    {
                        if(pWanIfaceData->Wan.Priority >= 0)
                        {
                            iActiveWanIdx = uiLoopCount;
                            iActivePriority = pWanIfaceData->Wan.Priority;
                        }
                    }
                }

                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
        }
    }

    return iActiveWanIdx;
}


/*********************************************************************************/
/************************** TRANSITIONS ******************************************/
/*********************************************************************************/
static WcFmobPolicyState_t Transition_Start(WanMgr_Policy_Controller_t* pWanController)
{
    wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);
    CcspTraceInfo(("%s %d - State changed to STATE_FIXING_WAN_INTERFACE \n", __FUNCTION__, __LINE__));
    return STATE_FIXING_WAN_INTERFACE;
}

static WcFmobPolicyState_t Transition_WanInterfaceFixed(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pFixedInterface = NULL;

    if((pWanController != NULL) && (pWanController->pWanActiveIfaceData != NULL))
    {
        pFixedInterface = &(pWanController->pWanActiveIfaceData->data);
    }

    if(pFixedInterface == NULL)
    {
        return STATE_FIXING_WAN_INTERFACE;
    }

    //ActiveLink
    pFixedInterface->Wan.ActiveLink = TRUE;
    pFixedInterface->SelectionStatus = WAN_IFACE_ACTIVE;

    CcspTraceInfo(("%s %d - State changed to STATE_FIXED_WAN_INTERFACE_DOWN \n", __FUNCTION__, __LINE__));
    return STATE_FIXED_WAN_INTERFACE_DOWN;
}

static WcFmobPolicyState_t Transition_FixedInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
{
    CcspTraceInfo(("%s %d - State changed to STATE_FIXED_WAN_INTERFACE_DOWN \n", __FUNCTION__, __LINE__));
    wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);
    return STATE_FIXED_WAN_INTERFACE_DOWN;
}

static WcFmobPolicyState_t Transition_FixedInterfaceUp(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pFixedInterface = NULL;
    WanMgr_IfaceSM_Controller_t wanIfCtrl;

    if((pWanController != NULL) && (pWanController->pWanActiveIfaceData != NULL))
    {
        pFixedInterface = &(pWanController->pWanActiveIfaceData->data);
    }

    if(pFixedInterface == NULL)
    {
        return STATE_FIXING_WAN_INTERFACE;
    }

    /* Starts an instance of the WAN Interface State Machine on the interface to begin configuring the WAN link */
    WanMgr_IfaceSM_Init(&wanIfCtrl, pFixedInterface->uiIfaceIdx);
    WanMgr_StartInterfaceStateMachine(&wanIfCtrl);

    CcspTraceInfo(("%s %d - State changed to STATE_FIXED_WAN_INTERFACE_UP \n", __FUNCTION__, __LINE__));
    return STATE_FIXED_WAN_INTERFACE_UP;
}

/*********************************************************************************/
/**************************** STATES *********************************************/
/*********************************************************************************/
static WcFmobPolicyState_t State_FixingWanInterface(WanMgr_Policy_Controller_t* pWanController)
{
    if(pWanController == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }

    pWanController->activeInterfaceIdx = WanMgr_Policy_FM_SelectWANActive();
    if(pWanController->activeInterfaceIdx != -1)
    {
        return Transition_WanInterfaceFixed(pWanController);
    }

    return STATE_FIXING_WAN_INTERFACE;
}

static WcFmobPolicyState_t State_FixedWanInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
{
    int iLoopCount;
    INT iSelectWanIdx = -1;
    DML_WAN_IFACE* pFixedInterface = NULL;

    if((pWanController != NULL) && (pWanController->pWanActiveIfaceData != NULL))
    {
        pFixedInterface = &(pWanController->pWanActiveIfaceData->data);
    }

    if(pFixedInterface == NULL)
    {
        return STATE_FIXING_WAN_INTERFACE;
    }

    if( pWanController->WanEnable == TRUE &&
        (pFixedInterface->Phy.Status == WAN_IFACE_PHY_STATUS_UP ||
         pFixedInterface->Phy.Status == WAN_IFACE_PHY_STATUS_INITIALIZING) &&
        pFixedInterface->Wan.Status == WAN_IFACE_STATUS_DISABLED &&
        pFixedInterface->Wan.LinkStatus == WAN_IFACE_LINKSTATUS_DOWN )
    {
        return Transition_FixedInterfaceUp(pWanController);
    }

    return STATE_FIXED_WAN_INTERFACE_DOWN;
}

static WcFmobPolicyState_t State_FixedWanInterfaceUp(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pFixedInterface = NULL;

    if((pWanController != NULL) && (pWanController->pWanActiveIfaceData != NULL))
    {
        pFixedInterface = &(pWanController->pWanActiveIfaceData->data);
    }

    if(pFixedInterface == NULL)
    {
        return STATE_FIXED_WAN_INTERFACE_DOWN;
    }

    if( pWanController->WanEnable == FALSE ||
        pFixedInterface->Phy.Status == WAN_IFACE_PHY_STATUS_DOWN)
    {
        return Transition_FixedInterfaceDown(pWanController);
    }

    return STATE_FIXED_WAN_INTERFACE_UP;
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/* WanMgr_Policy_FixedModeOnBootupPolicy */
ANSC_STATUS WanMgr_Policy_FixedModeOnBootupPolicy(void)
{
    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__));

    //detach thread from caller stack
    pthread_detach(pthread_self());

    //policy variables
    ANSC_STATUS retStatus = ANSC_STATUS_SUCCESS;
    WanMgr_Policy_Controller_t    WanPolicyCtrl;
    WcFmobPolicyState_t fmob_sm_state;
    bool bRunning = true;

    // event handler
    int n = 0;
    struct timeval tv;

    if(WanMgr_Controller_PolicyCtrlInit(&WanPolicyCtrl) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d Policy Controller Error \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d  Fixed Mode On Bootup Policy Thread Starting \n", __FUNCTION__, __LINE__));

    // initialise state machine
    fmob_sm_state = Transition_Start(&WanPolicyCtrl); // do this first before anything else to init variables

    while (bRunning)
    {
        /* Wait up to 500 milliseconds */
        tv.tv_sec = 0;
        tv.tv_usec = LOOP_TIMEOUT;

        n = select(0, NULL, NULL, NULL, &tv);
        if (n < 0)
        {
            /* interrupted by signal or something, continue */
            continue;
        }

        //Update Wan config
        WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
        if(pWanConfigData != NULL)
        {
            WanPolicyCtrl.WanEnable = pWanConfigData->data.Enable;
            WanMgrDml_GetConfigData_release(pWanConfigData);
        }

        //Lock Iface Data
        WanPolicyCtrl.pWanActiveIfaceData = WanMgr_GetIfaceData_locked(WanPolicyCtrl.activeInterfaceIdx);

        // process state
        switch (fmob_sm_state)
        {
            case STATE_FIXING_WAN_INTERFACE:
                fmob_sm_state = State_FixingWanInterface(&WanPolicyCtrl);
                break;
            case STATE_FIXED_WAN_INTERFACE_DOWN:
                fmob_sm_state = State_FixedWanInterfaceDown(&WanPolicyCtrl);
                break;
            case STATE_FIXED_WAN_INTERFACE_UP:
                fmob_sm_state = State_FixedWanInterfaceUp(&WanPolicyCtrl);
                break;
            default:
                CcspTraceInfo(("%s %d - Case: default \n", __FUNCTION__, __LINE__));
                bRunning = false;
                retStatus = ANSC_STATUS_FAILURE;
                break;
        }

        //Release Lock Iface Data
        if(WanPolicyCtrl.pWanActiveIfaceData != NULL)
        {
            WanMgrDml_GetIfaceData_release(WanPolicyCtrl.pWanActiveIfaceData);
        }
    }

    CcspTraceInfo(("%s %d - Exit from state machine\n", __FUNCTION__, __LINE__));

    return retStatus;
}

