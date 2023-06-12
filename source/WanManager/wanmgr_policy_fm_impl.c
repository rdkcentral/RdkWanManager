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

/* Fixed mode policy */
typedef enum {
    STATE_FIXING_WAN_INTERFACE = 0,
    STATE_FIXED_WAN_INTERFACE_DOWN,
    STATE_FIXED_WAN_INTERFACE_UP,
    STATE_FIXED_WAN_TEARING_DOWN,
    STATE_FIXED_WAN_SM_EXIT
} WcFmPolicyState_t;


/* STATES */
static WcFmPolicyState_t State_FixingWanInterface(WanMgr_Policy_Controller_t* pWanController);
static WcFmPolicyState_t State_FixedWanInterfaceDown(WanMgr_Policy_Controller_t* pWanController);
static WcFmPolicyState_t State_FixedWanInterfaceUp(WanMgr_Policy_Controller_t* pWanController);
static WcFmPolicyState_t State_WaitingForInterfaceSMExit(WanMgr_Policy_Controller_t* pWanController);

/* TRANSITIONS */
static WcFmPolicyState_t Transition_Start(WanMgr_Policy_Controller_t* pWanController);
static WcFmPolicyState_t Transition_WanInterfaceFixed(WanMgr_Policy_Controller_t* pWanController);
static WcFmPolicyState_t Transition_FixedInterfaceDown(WanMgr_Policy_Controller_t* pWanController);
static WcFmPolicyState_t Transition_FixedInterfaceUp(WanMgr_Policy_Controller_t* pWanController);
static WcFmPolicyState_t Transition_FixedInterfaceChanged(WanMgr_Policy_Controller_t* pWanController);


/*********************************************************************************/
/**************************** ACTIONS ********************************************/
/*********************************************************************************/
static INT WanMgr_Policy_FM_SelectWANActive(WanMgr_Policy_Controller_t* pWanController)
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
                if (pWanIfaceData->Selection.Enable == TRUE)
                {
                        if((pWanController->AllowRemoteInterfaces == FALSE) && (pWanIfaceData->IfaceType == REMOTE_IFACE))
                        {
                            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                            continue;
                        }
                    if((pWanIfaceData->Selection.Priority >= 0) && (pWanIfaceData->Selection.Priority < iActivePriority))
                    {
                        iActiveWanIdx = uiLoopCount;
                        iActivePriority = pWanIfaceData->Selection.Priority;
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
static WcFmPolicyState_t Transition_Start(WanMgr_Policy_Controller_t* pWanController)
{
    return STATE_FIXING_WAN_INTERFACE;
}

static WcFmPolicyState_t Transition_WanInterfaceFixed(WanMgr_Policy_Controller_t* pWanController)
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
    pFixedInterface->Selection.ActiveLink = TRUE;
    pFixedInterface->Selection.Status = WAN_IFACE_ACTIVE;

    return STATE_FIXED_WAN_INTERFACE_DOWN;
}


static WcFmPolicyState_t Transition_FixedInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
{
    return STATE_FIXED_WAN_INTERFACE_DOWN;
}


static WcFmPolicyState_t Transition_FixedInterfaceUp(WanMgr_Policy_Controller_t* pWanController)
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

    /* Starts an instance of the WAN Interface State Machine on
    the interface to begin configuring the WAN link */
    WanMgr_IfaceSM_Init(&wanIfCtrl, pFixedInterface->uiIfaceIdx);
    WanMgr_StartInterfaceStateMachine(&wanIfCtrl);

    return STATE_FIXED_WAN_INTERFACE_UP;
}

static WcFmPolicyState_t Transition_FixedInterfaceChanged(WanMgr_Policy_Controller_t* pWanController)
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

    /* Sets VirtIfList->Status to DISABLED for the current active interface */
    pFixedInterface->VirtIfList->Status = WAN_IFACE_STATUS_DISABLED;

    /* Set ActiveLink to FALSE since Fixed Interface is changed */
    pFixedInterface->Selection.ActiveLink = FALSE;
    pFixedInterface->Selection.Status = WAN_IFACE_NOT_SELECTED;

    /* Set default value to active Interface to force controller pWanActiveIfaceData to be updated */
    pWanController->activeInterfaceIdx = -1;

    return STATE_FIXING_WAN_INTERFACE;
}


/*********************************************************************************/
/**************************** STATES *********************************************/
/*********************************************************************************/
static WcFmPolicyState_t State_FixingWanInterface(WanMgr_Policy_Controller_t* pWanController)
{
    if(pWanController == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }

    if(pWanController->WanEnable == FALSE || pWanController->GroupCfgChanged == TRUE)
    {
        return STATE_FIXED_WAN_SM_EXIT;
    }

    pWanController->activeInterfaceIdx = WanMgr_Policy_FM_SelectWANActive(pWanController);


    if(pWanController->activeInterfaceIdx != -1)
    {
        return Transition_WanInterfaceFixed(pWanController);
    }

    return STATE_FIXING_WAN_INTERFACE;
}

static WcFmPolicyState_t State_FixedWanInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
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

    if(pWanController->WanEnable == FALSE || pWanController->GroupCfgChanged == TRUE)
    {
        return STATE_FIXED_WAN_TEARING_DOWN;
    }

    if((pFixedInterface->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_UP ||
        pFixedInterface->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_INITIALIZING) &&
        pWanController->WanEnable &&
        pFixedInterface->VirtIfList->Status == WAN_IFACE_STATUS_DISABLED &&
        pFixedInterface->VirtIfList->VLAN.Status == WAN_IFACE_LINKSTATUS_DOWN)
    {
        return Transition_FixedInterfaceUp(pWanController);
    }

    if( pFixedInterface->Selection.Enable != TRUE)
    {
        return Transition_FixedInterfaceChanged(pWanController);
    }

    /* Selection.Priority of the Fixed Interface is no longer the highest of all Primary interfaces */
    iSelectWanIdx = WanMgr_Policy_FM_SelectWANActive(pWanController);

    if(iSelectWanIdx != pWanController->activeInterfaceIdx)
    {
        return Transition_FixedInterfaceChanged(pWanController);
    }

    return STATE_FIXED_WAN_INTERFACE_DOWN;
}

static WcFmPolicyState_t State_FixedWanInterfaceUp(WanMgr_Policy_Controller_t* pWanController)
{
    INT iSelectWanIdx = -1;
    DML_WAN_IFACE* pFixedInterface = NULL;

    if((pWanController != NULL) && (pWanController->pWanActiveIfaceData != NULL))
    {
        pFixedInterface = &(pWanController->pWanActiveIfaceData->data);
    }

    if(pFixedInterface == NULL)
    {
        return STATE_FIXED_WAN_INTERFACE_DOWN;
    }

    if( pFixedInterface->BaseInterfaceStatus  == WAN_IFACE_PHY_STATUS_DOWN || pWanController->WanEnable == FALSE || pWanController->GroupCfgChanged == TRUE)
    {
        return Transition_FixedInterfaceDown(pWanController);
    }

    if( pFixedInterface->Selection.Enable == FALSE )
    {
        return Transition_FixedInterfaceChanged(pWanController);
    }

    /* Selection.Priority of the Fixed Interface is no longer the highest of all Primary interfaces */
    iSelectWanIdx = WanMgr_Policy_FM_SelectWANActive(pWanController);

    if(iSelectWanIdx != pWanController->activeInterfaceIdx)
    {
        return Transition_FixedInterfaceChanged(pWanController);
    }

    return STATE_FIXED_WAN_INTERFACE_UP;
}

static WcFmPolicyState_t State_WaitingForInterfaceSMExit(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pFixedInterface = NULL;

    if((pWanController != NULL) && (pWanController->pWanActiveIfaceData != NULL))
    {
        pFixedInterface = &(pWanController->pWanActiveIfaceData->data);
    }

    if(pFixedInterface == NULL)
    {
        return STATE_FIXED_WAN_TEARING_DOWN;
    }

    pFixedInterface->Selection.ActiveLink = FALSE;
    pFixedInterface->Selection.Status = WAN_IFACE_NOT_SELECTED;

    if(pFixedInterface->VirtIfList->Status != WAN_IFACE_STATUS_DISABLED)
    {
        return STATE_FIXED_WAN_TEARING_DOWN;
    }

    return STATE_FIXED_WAN_SM_EXIT;
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/* WanMgr_Policy_FixedModePolicy */
ANSC_STATUS WanMgr_Policy_FixedModePolicy(void)
{
    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__));

    //detach thread from caller stack
    pthread_detach(pthread_self());

    //policy variables
    ANSC_STATUS retStatus = ANSC_STATUS_SUCCESS;
    WanMgr_Policy_Controller_t    WanPolicyCtrl;
    WcFmPolicyState_t fm_sm_state;
    bool bRunning = true;

    // event handler
    int n = 0;
    struct timeval tv;


    if(WanMgr_Controller_PolicyCtrlInit(&WanPolicyCtrl) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d Policy Controller Error \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d  Fixed Mode Policy Thread Starting \n", __FUNCTION__, __LINE__));

    // initialise state machine
    fm_sm_state = Transition_Start(&WanPolicyCtrl); // do this first before anything else to init variables

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
            WanPolicyCtrl.GroupCfgChanged = pWanConfigData->data.GroupCfgChanged;
            WanPolicyCtrl.AllowRemoteInterfaces = pWanConfigData->data.AllowRemoteInterfaces;

            WanMgrDml_GetConfigData_release(pWanConfigData);
        }

        //Lock Iface Data
        WanPolicyCtrl.pWanActiveIfaceData = WanMgr_GetIfaceData_locked(WanPolicyCtrl.activeInterfaceIdx);

        // process state
        switch (fm_sm_state)
        {
            case STATE_FIXING_WAN_INTERFACE:
                fm_sm_state = State_FixingWanInterface(&WanPolicyCtrl);
                break;
            case STATE_FIXED_WAN_INTERFACE_UP:
                fm_sm_state = State_FixedWanInterfaceUp(&WanPolicyCtrl);
                break;
            case STATE_FIXED_WAN_INTERFACE_DOWN:
                fm_sm_state = State_FixedWanInterfaceDown(&WanPolicyCtrl);
                break;
            case STATE_FIXED_WAN_TEARING_DOWN:
                fm_sm_state = State_WaitingForInterfaceSMExit(&WanPolicyCtrl);
                break;
            case STATE_FIXED_WAN_SM_EXIT:
                bRunning = false;
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
