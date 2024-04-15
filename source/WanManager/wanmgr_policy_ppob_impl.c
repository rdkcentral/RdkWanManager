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
    SELECTED_INTERFACE_UP,
    STATE_TEARING_DOWN,
    STATE_SM_EXIT
} WcPpobPolicyState_t;


/* STATES */
static WcPpobPolicyState_t State_SelectingWanInterface(WanMgr_Policy_Controller_t* pWanController);
static WcPpobPolicyState_t State_SelectedInterfaceDown(WanMgr_Policy_Controller_t* pWanController);
static WcPpobPolicyState_t State_SelectedInterfaceUp(WanMgr_Policy_Controller_t* pWanController);
static WcPpobPolicyState_t State_WaitingForInterfaceSMExit(WanMgr_Policy_Controller_t* pWanController);

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

               if(((difftime(SelectionTimeOutEnd.tv_sec, PPoB_SelectionTimer.tv_sec)) > pWanIfaceData->Selection.Timeout) && 
                   (pWanIfaceData->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_DOWN))
               {
                   pWanIfaceData->VirtIfList->Status = WAN_IFACE_STATUS_INVALID;
               }else
               {
                   pWanIfaceData->VirtIfList->Status = WAN_IFACE_STATUS_DISABLED;
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

                    if ((pWanIfaceData->Selection.Enable == TRUE) && 
                        (pWanIfaceData->Selection.Priority >= 0) &&
                        (pWanIfaceData->VirtIfList->Status != WAN_IFACE_STATUS_INVALID))
                    {
                        if((pWanController->AllowRemoteInterfaces == FALSE) && (pWanIfaceData->IfaceType == REMOTE_IFACE))
                        {
                            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                            continue;
                        }
                        // pWanIfaceData - is Wan-Enabled & has valid Priority & Phy status
                        if(pWanIfaceData->Selection.Priority < iSelPrimaryPriority)
                        {
                            // update Primary iface with high priority iface
                            iSelPrimaryInterface = uiLoopCount;
                            iSelPrimaryPriority = pWanIfaceData->Selection.Priority;
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
            if(pWanIfaceData->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_UP ||
               pWanIfaceData->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_INITIALIZING)
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

    // Set Selection.Status = ACTIVE, ActiveLink = TRUE & start Interface State Machine
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pWanController->activeInterfaceIdx);
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

        pWanIfaceData->Selection.Status = WAN_IFACE_ACTIVE;
        pWanIfaceData->Selection.ActiveLink = TRUE;

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

/*
 * WanMgr_ResetIfaceTable()
 * - iterates thorugh the interface table and sets INVALID interfaces as DISABLED
 */
static void WanMgr_ResetIfaceTable (WanMgr_Policy_Controller_t* pWanController)
{
    if ((pWanController == NULL) || (pWanController->TotalIfaces == 0))
    {
        return;
    }

    UINT uiLoopCount;
    for( uiLoopCount = 0; uiLoopCount < pWanController->TotalIfaces; uiLoopCount++ )
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

            if (pWanIfaceData->VirtIfList->Status == WAN_IFACE_STATUS_INVALID)
            {
                pWanIfaceData->VirtIfList->Status = WAN_IFACE_STATUS_DISABLED;
                CcspTraceInfo(("%s-%d: VirtIfList->Status=Disabled, Interface-Idx=%d, Interface-Name=%s\n",
                            __FUNCTION__, __LINE__, uiLoopCount, pWanIfaceData->DisplayName));
            }

            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }
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

    // Set Selection.Status = ACTIVE, ActiveLink = TRUE & start Interface State Machine
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

    // Set Selection.Status = ACTIVE, ActiveLink = TRUE & start Interface State Machine
    if (WanMgr_StartIfaceStateMachine (pWanController) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: unable to start interface state machine\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    return SELECTED_INTERFACE_UP;
}

static WcPpobPolicyState_t Transition_SelectedInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
{
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

    if(pWanController->WanEnable == FALSE || pWanController->GroupCfgChanged == TRUE)
    {
        return STATE_SM_EXIT;
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
        pWanController->GroupCfgChanged == TRUE ||
        pActiveInterface->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_DOWN ||
        pActiveInterface->Selection.Enable == FALSE)
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

    if(pWanController->WanEnable == FALSE || pWanController->GroupCfgChanged == TRUE)
    {
        return STATE_TEARING_DOWN;
    }

    if(pWanController->WanEnable == TRUE &&
       (pActiveInterface->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_UP ||
       pActiveInterface->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_INITIALIZING) &&
       pActiveInterface->VirtIfList->VLAN.Status == WAN_IFACE_LINKSTATUS_DOWN &&
       pActiveInterface->VirtIfList->Status == WAN_IFACE_STATUS_DISABLED)
    {
        return Transition_SelectedInterfaceUp(pWanController);
    }

    return SELECTED_INTERFACE_DOWN;
}

static WcPpobPolicyState_t State_WaitingForInterfaceSMExit(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pFixedInterface = NULL;

    if((pWanController != NULL) && (pWanController->pWanActiveIfaceData != NULL))
    {
        pFixedInterface = &(pWanController->pWanActiveIfaceData->data);
    }

    if(pFixedInterface == NULL)
    {
        return STATE_TEARING_DOWN;
    }

    /* Set ActiveLink to FALSE since Fixed Interface is changed */
    pFixedInterface->Selection.ActiveLink = FALSE;
    pFixedInterface->Selection.Status = WAN_IFACE_NOT_SELECTED;
    if(pFixedInterface->VirtIfList->Status != WAN_IFACE_STATUS_DISABLED)
    {
        return STATE_TEARING_DOWN;
    }

    WanMgr_ResetIfaceTable(pWanController);

    return STATE_SM_EXIT;
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
            WanPolicyCtrl.GroupCfgChanged = pWanConfigData->data.GroupCfgChanged;
            WanPolicyCtrl.AllowRemoteInterfaces = pWanConfigData->data.AllowRemoteInterfaces;

            WanMgrDml_GetConfigData_release(pWanConfigData);
        }
        WanPolicyCtrl.TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();

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
            case STATE_TEARING_DOWN:
                ppob_sm_state = State_WaitingForInterfaceSMExit(&WanPolicyCtrl);
                break;
            case STATE_SM_EXIT:
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

