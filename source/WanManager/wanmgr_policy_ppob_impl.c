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

struct timespec PPoB_SelectionTimer;

typedef enum {
    SELECTING_WAN_INTERFACE = 0,
    SELECTED_INTERFACE_DOWN,
    SELECTED_INTERFACE_UP
} WcPpobPolicyState_t;


/* STATES */
static WcPpobPolicyState_t State_SelectingWanInterface(WanMgr_Policy_Controller_t* pWanController);
static WcPpobPolicyState_t State_SelectedInterfaceDown(WanMgr_Policy_Controller_t* pWanController);
static WcPpobPolicyState_t State_SelectedInterfaceUp(WanMgr_Policy_Controller_t* pWanController);

/* TRANSITIONS */
static WcPpobPolicyState_t Transition_Start(WanMgr_Policy_Controller_t* pWanController);
static WcPpobPolicyState_t Transition_WanInterfaceSelected(WanMgr_Policy_Controller_t* pWanController);
static WcPpobPolicyState_t Transition_SelectedInterfaceUp(WanMgr_Policy_Controller_t* pWanController);
static WcPpobPolicyState_t Transition_SelectedInterfaceDown(WanMgr_Policy_Controller_t* pWanController);

/*********************************************************************************/
/**************************** ACTIONS ********************************************/
/*********************************************************************************/
static UINT SelectionTimer()
{
    UINT uiTotalIfaces = -1;
    UINT uiLoopCount;
    //Get uiTotalIfaces
    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    if(uiTotalIfaces > 0)
    {
        for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
       {
           WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
           if(pWanDmlIfaceData != NULL)
           {
               DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
               //get current time
               struct timespec         SelectionTimeOutEnd;
               memset(&(SelectionTimeOutEnd), 0, sizeof(struct timespec));
               clock_gettime( CLOCK_MONOTONIC_RAW, &(SelectionTimeOutEnd));

               if(((difftime(SelectionTimeOutEnd.tv_sec, PPoB_SelectionTimer.tv_sec)) > pWanIfaceData->Wan.SelectionTimeout) && 
                   (pWanIfaceData->Phy.Status == WAN_IFACE_PHY_STATUS_DOWN))
               {
                   pWanIfaceData->Wan.Status = WAN_IFACE_STATUS_INVALID;
               }else
               {
                   pWanIfaceData->Wan.Status = WAN_IFACE_STATUS_DISABLED;
               }
               WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
           }
       }
    }
}

static void WanMgr_Policy_FM_SelectWANActive(WanMgr_Policy_Controller_t* pWanController, INT* pPrimaryInterface)
{
    UINT uiLoopCount;
    UINT uiTotalIfaces = -1;
    INT iSelPrimaryInterface = -1;
    INT iSelSecondaryInterface = -1;
    INT iSelPrimaryPriority = DML_WAN_IFACE_PRIORITY_MAX;
    UINT validPrimaryCount = 0;

    //Get uiTotalIfaces
    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    if(uiTotalIfaces > 0)
    {
        // Check the policy to determine if any primary interface should be used for WAN based on Priority
        if(pWanController->WanEnable == TRUE)
        {
            //Selection Timer
            SelectionTimer();
            for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
            {

                WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
                if(pWanDmlIfaceData != NULL)
                {
                    DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

                    if ((pWanIfaceData->Wan.Enable == TRUE) && 
                        (pWanIfaceData->Wan.Priority >= 0) &&
                        (pWanIfaceData->Wan.Status != WAN_IFACE_STATUS_INVALID))
                    {
                        // pWanIfaceData - is Wan-Enabled & has valid Priority & Phy status
                        if(pWanIfaceData->Wan.Priority < iSelPrimaryPriority)
                        {
                            // update Primary iface with high priority iface
                            iSelPrimaryInterface = uiLoopCount;
                            iSelPrimaryPriority = pWanIfaceData->Wan.Priority;
                        }
                    }

                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }
        }
    }

    if(iSelPrimaryInterface != -1)
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(iSelPrimaryInterface);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            if(pWanIfaceData->Phy.Status == WAN_IFACE_PHY_STATUS_UP ||
               pWanIfaceData->Phy.Status == WAN_IFACE_PHY_STATUS_INITIALIZING)
            {
                *pPrimaryInterface = iSelPrimaryInterface;
            }
            else
            {
                iSelPrimaryInterface = -1;
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }

    *pPrimaryInterface = iSelPrimaryInterface;
    return ;
}

static int WanMgr_StartIfaceStateMachine (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->activeInterfaceIdx == -1))
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    // Set SelectionStatus = ACTIVE, ActiveLink = TRUE & start Interface State Machine
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pWanController->activeInterfaceIdx);
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

        pWanIfaceData->SelectionStatus = WAN_IFACE_ACTIVE;
        pWanIfaceData->Wan.ActiveLink = TRUE;

        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }


    WanMgr_IfaceSM_Controller_t wanIfCtrl;
    WanMgr_IfaceSM_Init(&wanIfCtrl, pWanController->activeInterfaceIdx);
    if (WanMgr_StartInterfaceStateMachine(&wanIfCtrl) != 0)
    {
        CcspTraceError(("%s %d: Unable to start interface state machine \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;

}


/*********************************************************************************/
/************************** TRANSITIONS ******************************************/
/*********************************************************************************/
static WcPpobPolicyState_t Transition_Start(WanMgr_Policy_Controller_t* pWanController)
{
    if(pWanController == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }
    //Start the selectionTimer
    memset(&(PPoB_SelectionTimer), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(PPoB_SelectionTimer));

    wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);

    return SELECTING_WAN_INTERFACE;
}

static WcPpobPolicyState_t Transition_WanInterfaceSelected(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pActiveInterface = NULL;
    WanMgr_IfaceSM_Controller_t wanIfCtrl;

    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    // Set SelectionStatus = ACTIVE, ActiveLink = TRUE & start Interface State Machine
    if (WanMgr_StartIfaceStateMachine (pWanController) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: unable to start interface state machine\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    return SELECTED_INTERFACE_UP;
}

static WcPpobPolicyState_t Transition_SelectedInterfaceUp(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pActiveInterface = NULL;
    WanMgr_IfaceSM_Controller_t wanIfCtrl;

    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    // Set SelectionStatus = ACTIVE, ActiveLink = TRUE & start Interface State Machine
    if (WanMgr_StartIfaceStateMachine (pWanController) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: unable to start interface state machine\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    return SELECTED_INTERFACE_UP;
}

static WcPpobPolicyState_t Transition_SelectedInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
{
    wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);
    return SELECTED_INTERFACE_DOWN;
}

/*********************************************************************************/
/**************************** STATES *********************************************/
/*********************************************************************************/
static WcPpobPolicyState_t State_SelectingWanInterface(WanMgr_Policy_Controller_t* pWanController)
{
    int selectedPrimaryInterface = -1;

    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    WanMgr_Policy_FM_SelectWANActive(pWanController, &selectedPrimaryInterface);


    if(selectedPrimaryInterface != -1)
    {
        pWanController->activeInterfaceIdx = selectedPrimaryInterface;
        return Transition_WanInterfaceSelected(pWanController);
    }

    return SELECTING_WAN_INTERFACE;
}

static WcPpobPolicyState_t State_SelectedInterfaceUp(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pActiveInterface = NULL;

    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    if(pWanController->pWanActiveIfaceData == NULL)
    {
        return SELECTED_INTERFACE_DOWN;
    }

    pActiveInterface = &(pWanController->pWanActiveIfaceData->data);


    if( pWanController->WanEnable == FALSE ||
        pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_DOWN ||
        pActiveInterface->Wan.Enable == FALSE)
    {
        return Transition_SelectedInterfaceDown(pWanController);
    }

    /* TODO: Traffic to the WAN Interface has been idle for a time that exceeds
    the configured IdleTimeout value */
    //return Transition_SelectedInterfaceDown(pWanController);

    return SELECTED_INTERFACE_UP;
}

static WcPpobPolicyState_t State_SelectedInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pActiveInterface = NULL;

    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    if(pWanController->pWanActiveIfaceData == NULL)
    {
        return SELECTED_INTERFACE_DOWN;
    }

    pActiveInterface = &(pWanController->pWanActiveIfaceData->data);

    if(pWanController->WanEnable == TRUE &&
       (pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_UP ||
       pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_INITIALIZING) &&
       pActiveInterface->Wan.LinkStatus == WAN_IFACE_LINKSTATUS_DOWN &&
       pActiveInterface->Wan.Status == WAN_IFACE_STATUS_DISABLED)
    {
        return Transition_SelectedInterfaceUp(pWanController);
    }

    return SELECTED_INTERFACE_DOWN;
}


/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/* WanMgr_Policy_PrimaryPriorityOnBootupPolicy */
ANSC_STATUS WanMgr_Policy_PrimaryPriorityOnBootupPolicy(void)
{
    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__));

    //detach thread from caller stack
    pthread_detach(pthread_self());

    //policy variables
    ANSC_STATUS retStatus = ANSC_STATUS_SUCCESS;
    WanMgr_Policy_Controller_t    WanPolicyCtrl;
    WcPpobPolicyState_t ppob_sm_state;
    bool bRunning = true;

    // event handler
    int n = 0;
    struct timeval tv;


    if(WanMgr_Controller_PolicyCtrlInit(&WanPolicyCtrl) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d Policy Controller Error \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d  Primary Priority On Bootup Policy Thread Starting \n", __FUNCTION__, __LINE__));

    // initialise state machine
    ppob_sm_state = Transition_Start(&WanPolicyCtrl); // do this first before anything else to init variables

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
        switch (ppob_sm_state)
        {
            case SELECTING_WAN_INTERFACE:
                ppob_sm_state = State_SelectingWanInterface(&WanPolicyCtrl);
                break;
            case SELECTED_INTERFACE_DOWN:
                ppob_sm_state = State_SelectedInterfaceDown(&WanPolicyCtrl);
                break;
            case SELECTED_INTERFACE_UP:
                ppob_sm_state = State_SelectedInterfaceUp(&WanPolicyCtrl);
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

