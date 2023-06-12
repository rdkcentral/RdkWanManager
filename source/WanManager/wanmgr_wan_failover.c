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
extern void WanMgr_ParallelScanSelectionProcess (void* arg);

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
static WcFailOverState_t Transition_ResetScan (WanMgr_FailOver_Controller_t * pFailOverController);

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
    pFailOverController->RestorationDelay = 0;
    pFailOverController->ActiveIfaceState = -1;
    pFailOverController->PhyState = WAN_IFACE_PHY_STATUS_UNKNOWN;
    /* Update group Interface */
    UINT TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    for (int i = 0 ; i < TotalIfaces; i++)
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(i);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((pWanIfaceData->Selection.Group - 1));
            if (pWanIfaceGroup != NULL)
            {
                pWanIfaceGroup->InterfaceAvailable |= (1<<i);
                WanMgrDml_GetIfaceGroup_release();
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
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
        pFailOverController->RestorationDelay = pWanConfigData->data.RestorationDelay;
        pFailOverController->ResetScan = pWanConfigData->data.ResetFailOverScan;
        pWanConfigData->data.ResetFailOverScan = false; //Reset Global data

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
                if(pWanIfaceData->Selection.Status == WAN_IFACE_ACTIVE)
                {
                    pWanIfaceData->Selection.Status = WAN_IFACE_SELECTED;
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
                if(pWanIfaceData->Selection.Status == WAN_IFACE_SELECTED)
                {
                    pWanIfaceData->Selection.Status = WAN_IFACE_ACTIVE;
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
    for(int i = 0; i < WanMgr_GetTotalNoOfGroups(); i++)
    {
        WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((i));
        if (pWanIfaceGroup != NULL)
        {
            if( pWanIfaceGroup->InterfaceAvailable && pWanIfaceGroup->State != STATE_GROUP_RUNNING)
            {
                pWanIfaceGroup->SelectionTimeOut = 0;
                UINT TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
                for( int uiLoopCount = 0; uiLoopCount < TotalIfaces; uiLoopCount++ )
                {
                    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
                    if(pWanDmlIfaceData != NULL)
                    {
                        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                        if((i+1) == pWanIfaceData->Selection.Group)
                        {
                            if(pWanIfaceData->Selection.Timeout > pWanIfaceGroup->SelectionTimeOut)
                            {
                                pWanIfaceGroup->SelectionTimeOut = pWanIfaceData->Selection.Timeout;
                            }
                        }
                        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    }
                }

                pWanIfaceGroup->ConfigChanged = FALSE; 

                int ret;
                switch (pWanIfaceGroup->Policy) 
                {
                    //TODO: NEW_DESIGN add other policies. 
#if defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
                    case PARALLEL_SCAN: 
                        CcspTraceInfo(("%s %d - Creating PARALLEL_SCAN selection thread for Group(%d) \n", __FUNCTION__, __LINE__, (i+1)));
                        ret = pthread_create( &pWanIfaceGroup->ThreadId, NULL, &WanMgr_ParallelScanSelectionProcess, (void *)(i+1));
                        break;
#endif
                    case AUTOWAN_MODE:
                    default:
                        pWanIfaceGroup->Policy = AUTOWAN_MODE; //set dafault mode AUTOWAN_MODE if policy not supported.  
                        CcspTraceInfo(("%s %d - Creating AUTOWAN_MODE selection thread for Group(%d) \n", __FUNCTION__, __LINE__, (i+1)));
                        ret = pthread_create( &pWanIfaceGroup->ThreadId, NULL, &WanMgr_AutoWanSelectionProcess, (void *)(i+1));
                        break;
                }

                if (ret != 0)
                {
                    CcspTraceError(("%s %d - Failed to Create Group(%d) Thread\n", __FUNCTION__, __LINE__, (i+1)));
                }else
                {
                    CcspTraceInfo(("%s %d - Successfully Created Group(%d) Thread Id :%lu \nSelectionTimeOut %d Interfaces_mask %d\n", __FUNCTION__, __LINE__, (i+1), pWanIfaceGroup->ThreadId, pWanIfaceGroup->SelectionTimeOut, pWanIfaceGroup->InterfaceAvailable));
                    pWanIfaceGroup->State = STATE_GROUP_RUNNING;
                }
            }
            WanMgrDml_GetIfaceGroup_release();
        }

    }
}

ANSC_STATUS UpdateLedStatus (WanMgr_FailOver_Controller_t* pFailOverController)
{
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    /* We have a Active Group */ 
    if(pFailOverController->CurrentActiveGroup != 0)
    {
        WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((pFailOverController->CurrentActiveGroup -1));
        if (pWanIfaceGroup != NULL)
        {
            /* Active group has a selected Ineterface */
            if(pWanIfaceGroup->SelectedInterface > 0)
            {
                WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked((pWanIfaceGroup->SelectedInterface - 1));
                if (pWanDmlIfaceData != NULL)
                {
                    DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                    /* Active Interface State Machine is running, Update the Wan/IP connection status of the interface to LED*/
                    if(pWanIfaceData->VirtIfList->eCurrentState != WAN_STATE_EXIT && pWanIfaceData->VirtIfList->eCurrentState != pFailOverController->ActiveIfaceState)
                    {
                        /* Update Only when state changed */
                        CcspTraceInfo(("%s %d Updating LED status of Selected Interface (%s)\n", __FUNCTION__, __LINE__,pWanIfaceData->DisplayName));
                        switch(pWanIfaceData->VirtIfList->eCurrentState)
                        {
                            case WAN_STATE_DUAL_STACK_ACTIVE:
                                wanmgr_sysevents_setWanState(WAN_IPV4_UP);
                                wanmgr_sysevents_setWanState(WAN_IPV6_UP);
                                break;

                            case WAN_STATE_IPV6_LEASED:
#ifdef FEATURE_MAPT
                                wanmgr_sysevents_setWanState(WAN_MAPT_DOWN);
#endif
                                wanmgr_sysevents_setWanState(WAN_IPV4_DOWN);
                                wanmgr_sysevents_setWanState(WAN_IPV6_UP);
                                break;

                            case WAN_STATE_IPV4_LEASED:
                                wanmgr_sysevents_setWanState(WAN_IPV6_DOWN);
                                wanmgr_sysevents_setWanState(WAN_IPV4_UP);

                                break;

#ifdef FEATURE_MAPT
                            case WAN_STATE_MAPT_ACTIVE:
                                wanmgr_sysevents_setWanState(WAN_MAPT_UP);
                                break;
#endif
                            default:
#ifdef FEATURE_MAPT
                                wanmgr_sysevents_setWanState(WAN_MAPT_DOWN);
#endif
                                wanmgr_sysevents_setWanState(WAN_IPV6_DOWN);
                                wanmgr_sysevents_setWanState(WAN_IPV4_DOWN);
                                wanmgr_sysevents_setWanState(WAN_LINK_UP_STATE);
                                break;
                        }
                        pFailOverController->ActiveIfaceState = pWanIfaceData->VirtIfList->eCurrentState;
                        pFailOverController->PhyState = WAN_IFACE_PHY_STATUS_UP; //If ISM running, set Phystate to UP
                    }
                    /* Active Interface State Machine is not running, Update the PHY connection status of the interface to LED */
                    else if(pWanIfaceData->VirtIfList->eCurrentState == WAN_STATE_EXIT && pWanIfaceData->BaseInterfaceStatus != pFailOverController->PhyState)
                    {
                        CcspTraceInfo(("%s %d Updating LED status of Selected Interface (%s)\n", __FUNCTION__, __LINE__,pWanIfaceData->DisplayName));

                        if(pWanIfaceData->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_UP)
                        {
                            wanmgr_sysevents_setWanState(WAN_LINK_UP_STATE);
                        }else if(pWanIfaceData->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_INITIALIZING && 
                                (strstr(pWanIfaceData->BaseInterface,"DSL") != NULL))
                        {
                            /* Update DSL_Training only for DSL connection */
                            wanmgr_sysevents_setWanState(DSL_TRAINING);

                        }else
                        {
                            wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);
                        }
                        pFailOverController->PhyState = pWanIfaceData->BaseInterfaceStatus;
                    }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }
            WanMgrDml_GetIfaceGroup_release();
        }
    }else
    {
        /* Active group is not selected, Update the PHY status of all Wan Interfaces  to LED*/
        DML_WAN_IFACE_PHY_STATUS BaseInterfaceStatus = WAN_IFACE_PHY_STATUS_DOWN;
        UINT uiTotalIfaces = 0;
        UINT uiLoopCount   = 0;

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
                    /* if any Wan Interface Phy is Up, set LED Wan link UP,
                       if all Wan Interfaces Phy is DOWN,  set LED Wan link DOWN */
                    if(pWanIfaceData->BaseInterfaceStatus > BaseInterfaceStatus && pWanIfaceData->BaseInterfaceStatus != WAN_IFACE_PHY_STATUS_UNKNOWN)
                    {
                        if(pWanIfaceData->BaseInterfaceStatus != WAN_IFACE_PHY_STATUS_INITIALIZING || 
                          (pWanIfaceData->BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_INITIALIZING && strstr(pWanIfaceData->BaseInterface,"DSL") != NULL))
                        {
                            /* Update DSL_Training (PHY_STATUS_INITIALIZING) only for DSL connection */
                            BaseInterfaceStatus = pWanIfaceData->BaseInterfaceStatus;
                        }
                    }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }
        }

        if(BaseInterfaceStatus != pFailOverController->PhyState)
        {
            CcspTraceInfo(("%s %d No Selected Interface. Updating BaseInterface status of all WanLink to LED\n", __FUNCTION__, __LINE__));
            if(BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_UP)
            {
                wanmgr_sysevents_setWanState(WAN_LINK_UP_STATE);
            }else if(BaseInterfaceStatus == WAN_IFACE_PHY_STATUS_INITIALIZING)
            {
                wanmgr_sysevents_setWanState(DSL_TRAINING);
            }else 
            {
                wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);
            }
        }
        pFailOverController->PhyState = BaseInterfaceStatus;
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

    CcspTraceInfo(("%s %d Updating LED status LinkDown \n", __FUNCTION__, __LINE__));
    wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);

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

    for(int i = 0; i < WanMgr_GetTotalNoOfGroups(); i++)
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
                    if((pWanIfaceData->VirtIfList->Status == WAN_IFACE_STATUS_STANDBY ||
                        pWanIfaceData->VirtIfList->Status == WAN_IFACE_STATUS_UP) &&
                        (!(pWanIfaceData->IfaceType == REMOTE_IFACE &&  // Check RemoteStatus also for REMOTE_IFACE
                        pWanIfaceData->VirtIfList->RemoteStatus != WAN_IFACE_STATUS_UP)))
                    {
                        highestValidGroup = (i+1);
                    }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }

            if(pWanIfaceGroup->ConfigChanged)
            {
                GroupChanged = true;
            }
            if(WaitForHigherGroup)
            {
                /* get the current time */
                memset(&(CurrentTime), 0, sizeof(struct timespec));
                clock_gettime(CLOCK_MONOTONIC_RAW, &(CurrentTime));
                if(difftime(CurrentTime.tv_sec, pFailOverController->GroupSelectionTimer.tv_sec) < pWanIfaceGroup->SelectionTimeOut)
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

    if(pFailOverController->HighestValidGroup)
    {
        if(WanMgr_ActivateGroup(pFailOverController->HighestValidGroup) != ANSC_STATUS_SUCCESS)
        {
            return STATE_FAILOVER_SCANNING_GROUP;
        }
        pFailOverController->CurrentActiveGroup = pFailOverController->HighestValidGroup;
    }else //We should never reach this case.
    {
        CcspTraceError(("%s %d HighestValidGroup not available.\n", __FUNCTION__, __LINE__));
    }

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
    if(pFailOverController->CurrentActiveGroup)
    {
        if(WanMgr_DeactivateGroup(pFailOverController->CurrentActiveGroup) != ANSC_STATUS_SUCCESS)
        {
            return STATE_FAILOVER_GROUP_ACTIVE;
        }
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

static WcFailOverState_t Transition_ResetScan (WanMgr_FailOver_Controller_t * pFailOverController)
{
    CcspTraceInfo(("%s %d  \n", __FUNCTION__, __LINE__));

    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    pFailOverController->CurrentActiveGroup =  0;
    pFailOverController->HighestValidGroup = 0;
    pFailOverController->ResetScan = false;

    //Update LED
    CcspTraceInfo(("%s %d Updating LED status (Ipv4/ipv6/mapt down) \n", __FUNCTION__, __LINE__));
#ifdef FEATURE_MAPT
    wanmgr_sysevents_setWanState(WAN_MAPT_DOWN);
#endif
    wanmgr_sysevents_setWanState(WAN_IPV6_DOWN);
    wanmgr_sysevents_setWanState(WAN_IPV4_DOWN);
    pFailOverController->ActiveIfaceState = -1;
    pFailOverController->PhyState = WAN_IFACE_PHY_STATUS_UNKNOWN;

    //Start the selectionTimer
    memset(&(pFailOverController->GroupSelectionTimer), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pFailOverController->GroupSelectionTimer));

    return STATE_FAILOVER_SCANNING_GROUP;
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

    if(pFailOverController->WanEnable == FALSE)
    {
        return STATE_FAILOVER_EXIT;
    }

    MarkHighPriorityGroup(pFailOverController, true);

    if( pFailOverController->HighestValidGroup)
    {
        return Transition_ActivateGroup(pFailOverController);
    }

    return STATE_FAILOVER_SCANNING_GROUP;
}

static WcFailOverState_t State_GroupActive (WanMgr_FailOver_Controller_t * pFailOverController)
{
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    if(pFailOverController->WanEnable == FALSE )
    {
        return Transition_DeactivateGroup(pFailOverController);
    }

    if(pFailOverController->ResetScan == TRUE )
    {
        return Transition_ResetScan(pFailOverController);
    }

    MarkHighPriorityGroup(pFailOverController, false);

    if(pFailOverController->HighestValidGroup)
    {
        bool CurrentActiveGroup_down = false;
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
                        if(pWanIfaceData->VirtIfList->Status != WAN_IFACE_STATUS_UP)
                        {
                            CurrentActiveGroup_down = true;
                        }
                        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    }
                }else
                {
                            CurrentActiveGroup_down = true;
                }
                WanMgrDml_GetIfaceGroup_release();
            }
        }else
        {
            CurrentActiveGroup_down = true;
        }

        if(CurrentActiveGroup_down && (pFailOverController->CurrentActiveGroup < pFailOverController->HighestValidGroup))
        {
            CcspTraceInfo(("%s %d : CurrentActiveGroup(%d) Down. Activating Next Highest available group (%d) \n", __FUNCTION__, __LINE__,
                                    pFailOverController->CurrentActiveGroup , pFailOverController->HighestValidGroup));
            return Transition_DeactivateGroup (pFailOverController); 
        }
        else if (pFailOverController->CurrentActiveGroup > pFailOverController->HighestValidGroup)
        {
            CcspTraceInfo(("%s %d :  Found Highest available group (%d). Starting Restoration Timer \n", __FUNCTION__, __LINE__,pFailOverController->HighestValidGroup));
            return Transition_Restoration (pFailOverController);
        }
    }

    return STATE_FAILOVER_GROUP_ACTIVE;
}

static WcFailOverState_t State_RestorationWait (WanMgr_FailOver_Controller_t * pFailOverController)
{
    if(pFailOverController == NULL)
    {
        CcspTraceError(("%s %d pFWController object is NULL \n", __FUNCTION__, __LINE__));
        return STATE_FAILOVER_ERROR;
    }

    if(pFailOverController->WanEnable == FALSE )
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
                if((pWanIfaceData->VirtIfList->Status != WAN_IFACE_STATUS_STANDBY &&
                            pWanIfaceData->VirtIfList->Status != WAN_IFACE_STATUS_UP))
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
                    if((pWanIfaceData->VirtIfList->Status != WAN_IFACE_STATUS_UP))
                    {
                        CurrentActiveGroup_Deactivated = true;
                        pFailOverController->CurrentActiveGroup = 0; //Reset the CurrentActiveGroup.
                    }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }else
            {
                CurrentActiveGroup_Deactivated = true;
                pFailOverController->CurrentActiveGroup = 0; //Reset the CurrentActiveGroup.
            }
            WanMgrDml_GetIfaceGroup_release();
        }
    }
    if(CurrentActiveGroup_Deactivated)
    {
        if(pFailOverController->WanEnable == FALSE)
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
        UpdateLedStatus(&FWController);
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
        for(int i = 0; i < WanMgr_GetTotalNoOfGroups(); i++)
        {
            WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked((i));
            if (pWanIfaceGroup != NULL)
            {
                if(pWanIfaceGroup->State == STATE_GROUP_RUNNING)
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

