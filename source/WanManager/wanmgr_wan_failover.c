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


/* ---- Include Files ---------------------------------------- */

#include "wanmgr_wan_failover.h"

#define FAILOVER_SM_LOOP_TIMEOUT 300000 // timeout 

extern void WanMgr_AutoWanSelectionProcess (void* arg);

/* SELECTION STATES */
static WcFailOverState_t State_ScanningGroup (WanMgr_FailOver_Controller_t * pFailOverController);
static WcFailOverState_t State_GroupActive (WanMgr_FailOver_Controller_t * pFailOverController);
static WcFailOverState_t State_RestorationWait (WanMgr_FailOver_Controller_t * pFailOverController);
static WcFailOverState_t State_DeactivateGroup (WanMgr_FailOver_Controller_t * pFailOverController);


/* TRANSITIONS */
static WcFailOverState_t Transition_Start (WanMgr_FailOver_Controller_t* pFailOverController);
static WcFailOverState_t Transition_ActivateGroup (WanMgr_FailOver_Controller_t * pFailOverController);
static WcFailOverState_t Transition_Restoration (WanMgr_FailOver_Controller_t * pFailOverController);
static WcFailOverState_t Transition_DeactivateGroup (WanMgr_FailOver_Controller_t * pFailOverController);
static WcFailOverState_t Transition_RestorationFail (WanMgr_FailOver_Controller_t * pFailOverController);

ANSC_STATUS MarkHighPriorityGroup (WanMgr_FailOver_Controller_t* pFailOverController, bool WaitForHigherGroup);
/*
 * WanMgr_FailOverCtrlInit()
 * Init FailoverDeatails
 */
ANSC_STATUS WanMgr_FailOverCtrlInit(WanMgr_FailOver_Controller_t* pFailOverController)
{
    if (pFailOverController == NULL)
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    pFailOverController->WanEnable = FALSE;
    pFailOverController->CurrentActiveGroup = 0;
    pFailOverController->HighestValidGroup = 0;
    pFailOverController->PolicyChanged = FALSE;
    pFailOverController->RestorationDelay = 0;

    /* Update group Interface */
    UINT GroupInterfaceAvailable[MAX_INTERFACE_GROUP] = {0};
    UINT TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    for (int i = 0 ; i < TotalIfaces; i++)
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(i);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            GroupInterfaceAvailable[(pWanIfaceData->Wan.Group - 1)] |= (1<<i);
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }
    for(int i = 0; i < MAX_INTERFACE_GROUP; i++)
    {
        WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((i));
        if (pWanIfaceGroup != NULL)
        {
            pWanIfaceGroup->InterfaceAvailable = GroupInterfaceAvailable[i];
            WanMgrDml_GetIfaceGroup_release();
        }
    }
}

/*
 * WanMgr_UpdateFOControllerData()
 * - updates the controller data
 * - this function is called every time in the begining of the loop,
 so that the states/transition can work with the current data
 */
static void WanMgr_UpdateFOControllerData (WanMgr_FailOver_Controller_t* pFailOverController)
{
    if (pFailOverController == NULL)
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return;
    }

    //Update Wan config
    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        pFailOverController->WanEnable = pWanConfigData->data.Enable;
        pFailOverController->PolicyChanged = pWanConfigData->data.PolicyChanged;
        pFailOverController->RestorationDelay = pWanConfigData->data.RestorationDelay;

        WanMgrDml_GetConfigData_release(pWanConfigData);
    }
}

/*
 * WanMgr_SetGroupSelectedIface()
 * - sets the Group Selected Interface.
 */
int WanMgr_SetGroupSelectedIface (UINT GroupInst, UINT IfaceInst)
{
    WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((GroupInst - 1));
    if (pWanIfaceGroup != NULL)
    {
        pWanIfaceGroup->SelectedInterface = IfaceInst;
        CcspTraceInfo(("%s %d  group(%d) SelectedInterface %d\n", __FUNCTION__, __LINE__, GroupInst, pWanIfaceGroup->SelectedInterface));
        WanMgrDml_GetIfaceGroup_release();
    }
    return ANSC_STATUS_SUCCESS;
}

/* WanMgr_DeactivateGroup()
 * Set the selected interface to WAN_IFACE_SELECTED if it is set to WAN_IFACE_ACTIVE
 */
static ANSC_STATUS WanMgr_DeactivateGroup(UINT groupId)
{
    CcspTraceInfo(("%s %d Deactivating group %d \n", __FUNCTION__, __LINE__, groupId));
    WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((groupId - 1));
    if (pWanIfaceGroup != NULL)
    {
        if (pWanIfaceGroup->SelectedInterface)
        {
            WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((pWanIfaceGroup->SelectedInterface - 1));
            if (pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                if(pWanIfaceData->SelectionStatus == WAN_IFACE_ACTIVE)
                {
                    pWanIfaceData->SelectionStatus = WAN_IFACE_SELECTED;
                }
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
        }
        WanMgrDml_GetIfaceGroup_release();
    }
    return ANSC_STATUS_SUCCESS;
}

/* WanMgr_ActivateGroup()
 * Set the selected interface to WAN_IFACE_ACTIVE if it is set to WAN_IFACE_SELECTED
 */
static ANSC_STATUS WanMgr_ActivateGroup(UINT groupId)
{

    CcspTraceInfo(("%s %d Activating group %d \n", __FUNCTION__, __LINE__, groupId));
    WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((groupId - 1));
    if (pWanIfaceGroup != NULL)
    {
        if (pWanIfaceGroup->SelectedInterface)
        {
            WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((pWanIfaceGroup->SelectedInterface - 1));
            if (pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                if(pWanIfaceData->SelectionStatus == WAN_IFACE_SELECTED)
                {
                    pWanIfaceData->SelectionStatus = WAN_IFACE_ACTIVE;
                }
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
        }
        WanMgrDml_GetIfaceGroup_release();
    }

    return ANSC_STATUS_SUCCESS;
}

static void WanMgr_FO_IfaceGroupMonitor()
{
    for(int i = 0; i < MAX_INTERFACE_GROUP; i++)
    {
        WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((i));
        if (pWanIfaceGroup != NULL)
        {
            if( pWanIfaceGroup->InterfaceAvailable && pWanIfaceGroup->GroupState != STATE_GROUP_RUNNING)
            {
                pWanIfaceGroup->GroupSelectionTimeOut = 0;
                UINT TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
                for( int uiLoopCount = 0; uiLoopCount < TotalIfaces; uiLoopCount++ )
                {
                    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
                    if(pWanDmlIfaceData != NULL)
                    {
                        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                        if((i+1) == pWanIfaceData->Wan.Group)
                        {
                            if(pWanIfaceData->Wan.SelectionTimeout > pWanIfaceGroup->GroupSelectionTimeOut)
                            {
                                pWanIfaceGroup->GroupSelectionTimeOut = pWanIfaceData->Wan.SelectionTimeout;
                            }
                        }
                        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    }
                }

                pWanIfaceGroup->GroupIfaceListChanged = FALSE; 
                if (pthread_create( &pWanIfaceGroup->ThreadId, NULL, &WanMgr_AutoWanSelectionProcess, (void *)(i+1) ) != 0)
                {
                    CcspTraceError(("%s %d - Failed to Create Group(%d) Thread\n", __FUNCTION__, __LINE__, (i+1)));
                }else
                {
                    CcspTraceInfo(("%s %d - Successfully Created Group(%d) Thread Id :%lu \nGroupSelectionTimeOut %d Interfaces_mask %d\n", __FUNCTION__, __LINE__, (i+1), pWanIfaceGroup->ThreadId, pWanIfaceGroup->GroupSelectionTimeOut, pWanIfaceGroup->InterfaceAvailable));
                    pWanIfaceGroup->GroupState = STATE_GROUP_RUNNING;
                }
            }
            WanMgrDml_GetIfaceGroup_release();
        }

    }
}
/*********************************************************************************/
/************************** TRANSITIONS ******************************************/
/*********************************************************************************/

static WcFailOverState_t Transition_Start (WanMgr_FailOver_Controller_t* pFailOverController)
{
    CcspTraceInfo(("%s %d  \n", __FUNCTION__, __LINE__));

    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    pFailOverController->CurrentActiveGroup =  0;
    pFailOverController->HighestValidGroup = 0;

    //Start the selectionTimer
    memset(&(pFailOverController->GroupSelectionTimer), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pFailOverController->GroupSelectionTimer));

    return STATE_FAILOVER_SCANNING_GROUP;
}

ANSC_STATUS MarkHighPriorityGroup (WanMgr_FailOver_Controller_t* pFailOverController, bool WaitForHigherGroup)
{
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    UINT highestValidGroup = 0;
    bool GroupChanged = false;
    bool HigherGroupAvailable = false;
    struct timespec CurrentTime;

    for(int i = 0; i < MAX_INTERFACE_GROUP; i++)
    {
        WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((i));
        if (pWanIfaceGroup != NULL)
        {
            if (pWanIfaceGroup->SelectedInterface && !highestValidGroup && (!WaitForHigherGroup || !HigherGroupAvailable))
            {
                WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((pWanIfaceGroup->SelectedInterface - 1));
                if (pWanDmlIfaceData != NULL)
                {
                    DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                    if((pWanIfaceData->Wan.Status == WAN_IFACE_STATUS_STANDBY ||
                        pWanIfaceData->Wan.Status == WAN_IFACE_STATUS_UP) &&
                        (!(pWanIfaceData->Wan.IfaceType == REMOTE_IFACE &&  // Check RemoteStatus also for REMOTE_IFACE
                        pWanIfaceData->Wan.RemoteStatus != WAN_IFACE_STATUS_UP)))
                    {
                        highestValidGroup = (i+1);
                    }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }

            if(pWanIfaceGroup->GroupIfaceListChanged)
            {
                GroupChanged = true;
            }
            if(WaitForHigherGroup)
            {
                /* get the current time */
                memset(&(CurrentTime), 0, sizeof(struct timespec));
                clock_gettime(CLOCK_MONOTONIC_RAW, &(CurrentTime));
                if(difftime(CurrentTime.tv_sec, pFailOverController->GroupSelectionTimer.tv_sec) < pWanIfaceGroup->GroupSelectionTimeOut)
                {
                    HigherGroupAvailable = true;

                }
            }

            WanMgrDml_GetIfaceGroup_release();
        }
    }

    /* If group Change inprogress, don't select highestValidGroup */
    if(!GroupChanged)
    {
        pFailOverController->HighestValidGroup = highestValidGroup;
    }else
    {
       CcspTraceInfo(("%s %d: Group list change is in progress. Don't select Interface now. \n", __FUNCTION__, __LINE__));
       pFailOverController->HighestValidGroup = 0; 
    }

    if(pFailOverController->CurrentActiveGroup)
    {
        WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((pFailOverController->CurrentActiveGroup -1));
        if (pWanIfaceGroup != NULL)
        {
            if(pWanIfaceGroup->SelectedInterface)
            {
                WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((pWanIfaceGroup->SelectedInterface - 1));
                if (pWanDmlIfaceData != NULL)
                {
                    DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                    if((pWanIfaceData->Wan.Status != WAN_IFACE_STATUS_UP))
                    {
                        pWanIfaceData->SelectionStatus = WAN_IFACE_SELECTED;
                        pFailOverController->CurrentActiveGroup = 0;
                    }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }else
            {
                pFailOverController->CurrentActiveGroup = 0;
            }
            WanMgrDml_GetIfaceGroup_release();
        }
    }
    return ANSC_STATUS_SUCCESS;
}

static WcFailOverState_t Transition_ActivateGroup (WanMgr_FailOver_Controller_t * pFailOverController)
{
    CcspTraceInfo(("%s %d  \n", __FUNCTION__, __LINE__));
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    if(WanMgr_ActivateGroup(pFailOverController->HighestValidGroup) != ANSC_STATUS_SUCCESS)
    {
        return STATE_FAILOVER_SCANNING_GROUP;
    }
    pFailOverController->CurrentActiveGroup = pFailOverController->HighestValidGroup;

    return STATE_FAILOVER_GROUP_ACTIVE;
}

static WcFailOverState_t Transition_DeactivateGroup (WanMgr_FailOver_Controller_t * pFailOverController)
{
    CcspTraceInfo(("%s %d  \n", __FUNCTION__, __LINE__));
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    if(WanMgr_DeactivateGroup(pFailOverController->CurrentActiveGroup) != ANSC_STATUS_SUCCESS)
    {
        return STATE_FAILOVER_GROUP_ACTIVE;
    }

    return STATE_FAILOVER_DEACTIVATE_GROUP;

}

static WcFailOverState_t Transition_Restoration (WanMgr_FailOver_Controller_t * pFailOverController)
{
    CcspTraceInfo(("%s %d  \n", __FUNCTION__, __LINE__));
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    //Start the selectionTimer. Using selection timer as restration timer.
    memset(&(pFailOverController->GroupSelectionTimer), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pFailOverController->GroupSelectionTimer));

    return STATE_FAILOVER_RESTORATION_WAIT;
}

static WcFailOverState_t Transition_RestorationFail (WanMgr_FailOver_Controller_t * pFailOverController)
{
    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__));
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    /* Do nothing */
    return STATE_FAILOVER_GROUP_ACTIVE;
}
/*********************************************************************************/
/**************************** STATES *********************************************/
/*********************************************************************************/

static WcFailOverState_t State_ScanningGroup (WanMgr_FailOver_Controller_t * pFailOverController)
{
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    if(pFailOverController->WanEnable == FALSE ||
            pFailOverController->PolicyChanged == TRUE)
    {
        return STATE_FAILOVER_EXIT;
    }

    if( pFailOverController->HighestValidGroup)
    {
        return Transition_ActivateGroup(pFailOverController);
    }

    MarkHighPriorityGroup(pFailOverController, true);

    return STATE_FAILOVER_SCANNING_GROUP;
}

static WcFailOverState_t State_GroupActive (WanMgr_FailOver_Controller_t * pFailOverController)
{
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    if(pFailOverController->WanEnable == FALSE ||
            pFailOverController->PolicyChanged == TRUE)
    {
        return Transition_DeactivateGroup(pFailOverController);
    }

    if(pFailOverController->HighestValidGroup)
    {
        if(!pFailOverController->CurrentActiveGroup)
        {
            CcspTraceInfo(("%s %d : CurrentActiveGroup Down. Activating Highest available group \n", __FUNCTION__, __LINE__));
            return Transition_ActivateGroup(pFailOverController);
        }
        else if(pFailOverController->CurrentActiveGroup < pFailOverController->HighestValidGroup)
        {
            CcspTraceInfo(("%s %d :  Activating Highest available group \n", __FUNCTION__, __LINE__));
            return Transition_DeactivateGroup (pFailOverController); 
        }
        else if (pFailOverController->CurrentActiveGroup > pFailOverController->HighestValidGroup)
        {
            CcspTraceInfo(("%s %d :  Found Highest available group. Starting Restoration Timer \n", __FUNCTION__, __LINE__));
            return Transition_Restoration (pFailOverController);
        }
    }

    MarkHighPriorityGroup(pFailOverController, false);
    return STATE_FAILOVER_GROUP_ACTIVE;
}

static WcFailOverState_t State_RestorationWait (WanMgr_FailOver_Controller_t * pFailOverController)
{
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    if(pFailOverController->WanEnable == FALSE ||
            pFailOverController->PolicyChanged == TRUE)
    {
        return Transition_DeactivateGroup(pFailOverController);
    }

    bool HighestValidGroup_Down = false;

    WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((pFailOverController->HighestValidGroup -1));
    if (pWanIfaceGroup != NULL)
    {
        if (pWanIfaceGroup->SelectedInterface )
        {
            WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((pWanIfaceGroup->SelectedInterface - 1));
            if (pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                if((pWanIfaceData->Wan.Status != WAN_IFACE_STATUS_STANDBY &&
                            pWanIfaceData->Wan.Status != WAN_IFACE_STATUS_UP))
                {
                    HighestValidGroup_Down = true;
                }
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
        }   
        WanMgrDml_GetIfaceGroup_release();
    }
    if(HighestValidGroup_Down)
    {
        return Transition_RestorationFail(pFailOverController);
    }

    /* get the current time */
    struct timespec CurrentTime;
    memset(&(CurrentTime), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(CurrentTime));
    if(difftime(CurrentTime.tv_sec, pFailOverController->GroupSelectionTimer.tv_sec) > pFailOverController->RestorationDelay) 
    {
        CcspTraceInfo(("%s %d : Restoration timer expired. Deactivating current group and activating high priority group. \n", __FUNCTION__, __LINE__));
        return Transition_DeactivateGroup(pFailOverController);
    }

    return STATE_FAILOVER_RESTORATION_WAIT;
}

static WcFailOverState_t State_DeactivateGroup (WanMgr_FailOver_Controller_t * pFailOverController)
{
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    bool CurrentActiveGroup_Deactivated = false;
    if(!pFailOverController->CurrentActiveGroup)
    {
        CurrentActiveGroup_Deactivated = true;
    }else
    {
        WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((pFailOverController->CurrentActiveGroup -1));
        if (pWanIfaceGroup != NULL)
        {
            if (pWanIfaceGroup->SelectedInterface )
            {
                WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((pWanIfaceGroup->SelectedInterface - 1));
                if (pWanDmlIfaceData != NULL)
                {
                    DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                    if((pWanIfaceData->Wan.Status != WAN_IFACE_STATUS_UP))
                    {
                        CurrentActiveGroup_Deactivated = true;
                    }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }else
            {
                CurrentActiveGroup_Deactivated = true;
            }
            WanMgrDml_GetIfaceGroup_release();
        }
    }
    if(CurrentActiveGroup_Deactivated)
    {
        if(pFailOverController->WanEnable == FALSE ||
                pFailOverController->PolicyChanged == TRUE)
        {
            return STATE_FAILOVER_EXIT;
        }
        return Transition_ActivateGroup(pFailOverController);
    }

    return STATE_FAILOVER_DEACTIVATE_GROUP;
}


/*********************************************************************************/
/**************************** SM THREAD ******************************************/
/*********************************************************************************/

ANSC_STATUS WanMgr_FailOverThread (void)
{
    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__));
    pthread_detach(pthread_self());

    bool bRunning = true;
    int n = 0;
    struct timeval loopTimer;
    WcFailOverState_t fo_sm_state;
    WanMgr_FailOver_Controller_t  FWController;

    // initialising Failover data
    if(WanMgr_FailOverCtrlInit(&FWController) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Policy Controller Error \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }


    WanMgr_UpdateFOControllerData(&FWController);
    //    WanMgr_IfaceGroupMonitor();
    WanMgr_FO_IfaceGroupMonitor();

    fo_sm_state = Transition_Start(&FWController);

    while (bRunning)
    {
        /* Wait up to 500 milliseconds */
        loopTimer.tv_sec = 0;
        loopTimer.tv_usec = FAILOVER_SM_LOOP_TIMEOUT;

        n = select(0, NULL, NULL, NULL, &loopTimer);
        if (n < 0)
        {
            /* interrupted by signal or something, continue */
            continue;
        }

        WanMgr_UpdateFOControllerData(&FWController);
        WanMgr_FO_IfaceGroupMonitor();
        // process states
        switch (fo_sm_state)
        {
            case STATE_FAILOVER_SCANNING_GROUP:
                fo_sm_state = State_ScanningGroup(&FWController);
                break;
            case STATE_FAILOVER_GROUP_ACTIVE:
                fo_sm_state = State_GroupActive(&FWController);
                break;
            case STATE_FAILOVER_RESTORATION_WAIT:
                fo_sm_state = State_RestorationWait(&FWController);
                break;
            case STATE_FAILOVER_DEACTIVATE_GROUP:
                fo_sm_state = State_DeactivateGroup(&FWController);
                break;
            case STATE_FAILOVER_EXIT:
                bRunning = false;
                break;
            case STATE_FAILOVER_ERROR:
            default:
                CcspTraceInfo(("%s %d: Failure Case \n", __FUNCTION__, __LINE__));
                bRunning = false;
                break;
        }

    }

    /* Wait for All selection thread to exit */
    while(true)
    {
        bool selectionThreadRunning = false;
        for(int i = 0; i < MAX_INTERFACE_GROUP; i++)
        {
            WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((i));
            if (pWanIfaceGroup != NULL)
            {
                if(pWanIfaceGroup->GroupState == STATE_GROUP_RUNNING)
                {
                    selectionThreadRunning = true;
                }
                WanMgrDml_GetIfaceGroup_release();
            }
        }
        if(!selectionThreadRunning)
        {
            break;
        }
    }
    CcspTraceInfo(("%s %d - Exit from Failover Thread\n", __FUNCTION__, __LINE__));
    return ANSC_STATUS_SUCCESS;
}

