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
/* Selection Timer Start and End Global Variable */

static INT iInterfaceCount = 0;

/* Auto Wan policy */
typedef enum {
    STATE_AUTO_WAN_INTERFACE_DOWN = 0,
    STATE_AUTO_WAN_INTERFACE_WAIT,
    STATE_AUTO_WAN_INTERFACE_SM_WAIT,
    STATE_AUTO_WAN_INTERFACE_SCAN,
    STATE_AUTO_WAN_INTERFACE_REBOOT,
    STATE_AUTO_WAN_INTERFACE_ACTIVE
} WcAwPolicyState_t;

/* STATES */
static WcAwPolicyState_t State_WaitingForInterface(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t State_WaitingForInterfaceSM(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t State_ScaningInterface(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t State_Rebooting(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t State_WanInterfaceActive(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t State_WanInterfaceDown(WanMgr_Policy_Controller_t* pWanController);

/* TRANSITIONS */
static WcAwPolicyState_t Transition_Start(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_tryingNextInterface(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_InterfaceFound(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_ScaningNextInterface(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_ReconfiguringPlatform(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_ActivatingInterface(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_WanInterfaceDown(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_WanInterfaceUp(WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_ResetActiveInterface(WanMgr_Policy_Controller_t* pWanController);

/*********************************************************************************/
/**************************** ACTIONS ********************************************/
/*********************************************************************************/

/**********************************************************************
    description:
        This function is called to Set Wan Status of all Interface from
        InValid to Disabled.
**********************************************************************/
static bool WanMgr_ResetInterfaceScanning(void)
{
    bool bAllDone = TRUE;
    UINT uiLoopCount;
    UINT uiTotalIfaces = 0;

    //Get uiTotalIfaces
    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    if(!uiTotalIfaces)
    {
        bAllDone = FALSE;
    }

    iInterfaceCount = 0;

    if(uiTotalIfaces > 0)
    {
        for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
        {
            WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
            if(pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

                if (pWanIfaceData->Wan.Status == WAN_IFACE_STATUS_INVALID)
                {
                    pWanIfaceData->Wan.Status = WAN_IFACE_STATUS_DISABLED;
                    CcspTraceInfo(("%s-%d: Wan.Status=Disabled, Interface-Idx=%d, Interface-Name=%s\n",
                                           __FUNCTION__, __LINE__, uiLoopCount, pWanIfaceData->DisplayName));
                }

                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
        }
    }
    return bAllDone;
}

/**********************************************************************
    description:
        This function is Called to Indetify the Active Interface Before
       	CPE Reboot.	
**********************************************************************/
static void WanMgr_FindActiveInterface(WanMgr_Policy_Controller_t* pWanController)
{
    UINT uiLoopCount;
    UINT uiTotalIfaces = -1;

    //Get uiTotalIfaces
    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    if(uiTotalIfaces > 0)
    {
        // Check the policy to determine if any primary interface should be used for WAN
        if(pWanController->WanEnable == TRUE)
        {
            for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
            {
                WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
                if(pWanDmlIfaceData != NULL)
                {
                    DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                    if ((pWanIfaceData->Wan.Enable == TRUE) &&
                        (pWanIfaceData->Wan.ActiveLink == TRUE))
                    {
                        pWanController->activeInterfaceIdx = uiLoopCount;
                        iInterfaceCount++;
                        CcspTraceInfo(("%s-%d: Previous ActiveLink on Reboot, Interface %d, Interface-Name %s \n",
                                               __FUNCTION__, __LINE__, uiLoopCount, pWanIfaceData->DisplayName));
                        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                        break;
                    }

                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }
        }
    }
    return;
}

/**********************************************************************
    description:
        This function is Called to Select the Interface Based on Priority 
	, Wan Type, Global Wan Enable and Interface Wan Enable .
**********************************************************************/
static void WanMgr_Policy_Auto_SelectWANActive(WanMgr_Policy_Controller_t* pWanController)
{
    UINT uiLoopCount;
    UINT uiTotalIfaces = -1;
    INT iSelPrimaryInterface = -1;
    INT iSelSecondaryInterface = -1;
    INT iSelPrimaryPriority = DML_WAN_IFACE_PRIORITY_MAX;
    INT iSelSecondaryPriority = DML_WAN_IFACE_PRIORITY_MAX;

    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();

    if (iInterfaceCount >= uiTotalIfaces)
    {
        //Reset Interface Count to Start Interface Table polling From Starting Interface.
        WanMgr_ResetInterfaceScanning();
    }

    if(uiTotalIfaces > 0)
    {
        if(pWanController->WanEnable == TRUE)
        {
            for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
            {
                WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
                if(pWanDmlIfaceData != NULL)
                {
                    DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                    //Check Iface Wan Enable and Wan Status InValid
                    if ((pWanIfaceData->Wan.Enable == TRUE) && 
                        (pWanIfaceData->Wan.Priority >= 0) &&
                        (pWanIfaceData->Wan.Status != WAN_IFACE_STATUS_INVALID))
                    {
                        // pWanIfaceData - is Wan-Enabled & has valid Priority & Phy status
                        if(pWanIfaceData->Wan.Priority < iSelPrimaryPriority)
                        {
                            // move Primary interface as Secondary
                            iSelSecondaryInterface = iSelPrimaryInterface;
                            iSelSecondaryPriority = iSelPrimaryPriority;
                            // update Primary iface with high priority iface
                            iSelPrimaryInterface = uiLoopCount;
                            iSelPrimaryPriority = pWanIfaceData->Wan.Priority;
                        }
                        else if (pWanIfaceData->Wan.Priority < iSelSecondaryPriority)
                        {
                            // pWanIfaceData - has a priority greater the selected primary but lesser the secondar iface
                            iSelSecondaryInterface = uiLoopCount;
                            iSelSecondaryPriority = pWanIfaceData->Wan.Priority;
                        }
                    }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                }
            }
        }
    }
    if (iSelPrimaryInterface != -1)
    {
        //Finalize the Selected Interface for Primary Type.
        pWanController->activeInterfaceIdx = iSelPrimaryInterface;
        iInterfaceCount++;
	CcspTraceInfo(("%s-%d: Current Selected Interface=%d \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    }
    else if (iSelSecondaryInterface != -1)
    {
        //Finalize the Selected Interface for Secondary Type.
        pWanController->activeInterfaceIdx = iSelSecondaryInterface;
        iInterfaceCount++;
	CcspTraceInfo(("%s-%d: Current Selected Interface=%d \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    }
    return;
}

/**********************************************************************
    description:
        This function is called to wait for all Running Interface State
        Machine to be Killed/Stopped.
**********************************************************************/
static bool WanMgr_CheckAllIfacesDown(void)
{
    bool bAllDown = TRUE;
    UINT uiLoopCount;
    UINT uiTotalIfaces = 0;

    //Get uiTotalIfaces
    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    if(!uiTotalIfaces)
    {
        bAllDown = FALSE;
    }

    if(uiTotalIfaces > 0)
    {
        for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
        {
            WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
            if(pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

                if ((pWanIfaceData->Wan.Status != WAN_IFACE_STATUS_DISABLED) &&
                    (pWanIfaceData->Wan.Status != WAN_IFACE_STATUS_INVALID) &&
                    (pWanIfaceData->SelectionStatus == WAN_IFACE_NOT_SELECTED))
                {
                    bAllDown = FALSE;
                }

                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
        }
    }
    return bAllDown;
}

/**********************************************************************
    description:
        This function is called to set all CPE X RDK Interface UpStream 
	false to change WAN to LAN.
**********************************************************************/
static UINT WanMgr_AutoPolicy_Set_InterfaceUpstream(INT ActiveIface)
{
    bool bAllUpStreamSet = TRUE;
    UINT uiLoopCount;
    UINT uiTotalIfaces = 0;

    //Get uiTotalIfaces
    uiTotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    if(!uiTotalIfaces)
    {
        bAllUpStreamSet = FALSE;
    }

    if(uiTotalIfaces > 0)
    {
        for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
        {
            WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
            if(pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
                if ((pWanIfaceData->Phy.Path) && (uiLoopCount != ActiveIface))
                {
                    WanMgr_RdkBus_updateInterfaceUpstreamFlag(pWanIfaceData->Phy.Path, FALSE);
                }
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
        }
    }
    return bAllUpStreamSet;
}

/*********************************************************************************/
/************************** TRANSITIONS ******************************************/
/*********************************************************************************/
static WcAwPolicyState_t Transition_Start(WanMgr_Policy_Controller_t* pWanController)
{
    //Select The Interface Based on Boot time ActiveLink
    WanMgr_FindActiveInterface(pWanController);
    if (pWanController->activeInterfaceIdx == -1)
    {
        CcspTraceInfo(("%s-%d: No ActiveLink True, so trying to Select Interface in CPE Table\n", __FUNCTION__, __LINE__));
	//Select the Interface with Available Interface in CPE Table.
        WanMgr_Policy_Auto_SelectWANActive(pWanController);
    }
    wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);

    //Update The SelectedTimeOut for New SelectedInterface.
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pWanController->activeInterfaceIdx);
    if (pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
        pWanController->InterfaceSelectionTimeOut = pWanIfaceData->Wan.SelectionTimeout;
        CcspTraceInfo(("%s-%d: Selected New Interface, Interface-Idx=%d, Interface-Name=%s, SelectionTimeOut=%d \n",
                               __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx, pWanIfaceData->DisplayName,
                               pWanController->InterfaceSelectionTimeOut));
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }

    // Start Timer base on SelectiontimeOut DM.
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutStart));
    pWanController->SelectionTimeOutStart.tv_sec += pWanController->InterfaceSelectionTimeOut;

    CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_WAIT \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_WAIT;
}

static WcAwPolicyState_t Transition_tryingNextInterface(WanMgr_Policy_Controller_t* pWanController)
{
    //Select the Interface with Available Interface in CPE Table
    WanMgr_Policy_Auto_SelectWANActive(pWanController);

    //Update The SelectedTimeOut for New SelectedInterface.
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pWanController->activeInterfaceIdx);
    if (pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
        pWanController->InterfaceSelectionTimeOut = pWanIfaceData->Wan.SelectionTimeout;
        CcspTraceInfo(("%s-%d: Selected New Interface, Interface-Idx=%d, Interface-Name=%s, SelectionTimeOut=%d \n",
                               __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx, pWanIfaceData->DisplayName,
                               pWanController->InterfaceSelectionTimeOut));
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }

    // Start Timer base on SelectiontimeOut DM.
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutStart));
    pWanController->SelectionTimeOutStart.tv_sec += pWanController->InterfaceSelectionTimeOut;

    CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_WAIT \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_WAIT;
}

static WcAwPolicyState_t Transition_InterfaceFound(WanMgr_Policy_Controller_t* pWanController)
{
    WanMgr_IfaceSM_Controller_t wanIfCtrl;

    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Set ActiveLink For Selected Interface.
    if (pWanController->pWanActiveIfaceData)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanController->pWanActiveIfaceData->data);
        pWanIfaceData->SelectionStatus = WAN_IFACE_ACTIVE;
        WanMgr_IfaceSM_Init(&wanIfCtrl, pWanIfaceData->uiIfaceIdx);

        /* Starts an instance of the WAN Interface State Machine on
        the interface to begin configuring the WAN link */
        WanMgr_StartInterfaceStateMachine(&wanIfCtrl);
    }

    //Start Timer base on SelectiontimeOut DM.
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutStart));
    pWanController->SelectionTimeOutStart.tv_sec += pWanController->InterfaceSelectionTimeOut;

    CcspTraceInfo(("%s %d - State changed to STATE_AUTO_WAN_INTERFACE_SCAN \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_SCAN;
}

static WcAwPolicyState_t Transition_ScaningNextInterface(WanMgr_Policy_Controller_t* pWanController)
{
    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //To Stop Running Interface State Machine, Set False ActiveLink of Selected Interface.
    if (pWanController->pWanActiveIfaceData)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanController->pWanActiveIfaceData->data);
        //Set ActiveLink to TRUE
        if ((pWanIfaceData->Wan.Enable == TRUE) &&
            (pWanIfaceData->SelectionStatus == WAN_IFACE_ACTIVE))
        {
            pWanIfaceData->Wan.ActiveLink = FALSE;
            pWanIfaceData->SelectionStatus = WAN_IFACE_NOT_SELECTED;
            CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_SM_WAIT \n", __FUNCTION__, __LINE__));
            return STATE_AUTO_WAN_INTERFACE_SM_WAIT;
        }
    }

    //Select the Interface with Available Interface in CPE Table
    WanMgr_Policy_Auto_SelectWANActive(pWanController);

    //Update The SelectedTimeOut for New SelectedInterface.
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pWanController->activeInterfaceIdx);
    if (pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
        pWanController->InterfaceSelectionTimeOut = pWanIfaceData->Wan.SelectionTimeout;
        CcspTraceInfo(("%s-%d: Selected New Interface, Interface-Idx=%d, Interface-Name=%s, SelectionTimeOut=%d \n",
                               __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx, pWanIfaceData->DisplayName,
                               pWanController->InterfaceSelectionTimeOut));
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }

    //Start Timer base on SelectiontimeOut DM.
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutStart));
    pWanController->SelectionTimeOutStart.tv_sec += pWanController->InterfaceSelectionTimeOut;

    //Check If Phy.Status==Down, then Change State from Interfac Scaning to Interface Wait.
    if (pWanController->activeInterfaceIdx != -1)
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pWanController->activeInterfaceIdx);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

            //Set ActiveLink to TRUE
            if (pWanIfaceData->Phy.Status == WAN_IFACE_PHY_STATUS_DOWN)
            {
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                return STATE_AUTO_WAN_INTERFACE_WAIT;
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }

    CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_SCAN \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_SCAN;
}

static WcAwPolicyState_t Transition_ReconfiguringPlatform(WanMgr_Policy_Controller_t* pWanController)
{
    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Reset the SelectedInterface timer.
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));

    //Set Selected Interface ActiveLink false, If Interface State Machine Running for SelectedInterface. 
    if (pWanController->pWanActiveIfaceData)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanController->pWanActiveIfaceData->data);;
        if(pWanIfaceData->Wan.Status == WAN_IFACE_STATUS_DISABLED)
        {
            pWanIfaceData->SelectionStatus = WAN_IFACE_ACTIVE;
        }
        else
        {
            pWanIfaceData->Wan.ActiveLink = FALSE;
            pWanIfaceData->SelectionStatus = WAN_IFACE_NOT_SELECTED;
        }
    }

    CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_REBOOT \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_REBOOT;
}

static WcAwPolicyState_t Transition_ActivatingInterface(WanMgr_Policy_Controller_t* pWanController)
{
    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Reset The SelectedInterfaceTimeOut Timer to Zero.
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));

    //Set The Selected Interface ActiveLink to Presistant.
    if (pWanController->pWanActiveIfaceData)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanController->pWanActiveIfaceData->data);
        if (DmlSetWanActiveLinkInPSMDB(pWanController->activeInterfaceIdx, pWanIfaceData) != ANSC_STATUS_SUCCESS)
        {
            CcspTraceError(("%s-%d: Failed to set ActiveLink in PSM, SelectedInterface %d \n",
                                    __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
        }
    }

    CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_ACTIVE \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_ACTIVE;
}

static WcAwPolicyState_t Transition_WanInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
{
    CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_DOWN \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_DOWN;
}

static WcAwPolicyState_t Transition_WanInterfaceUp(WanMgr_Policy_Controller_t* pWanController)
{
    WanMgr_IfaceSM_Controller_t wanIfCtrl;

    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Set Selected Interface ActiveLink to True, and Start Interface State Machine for Selected Interface.
    if (pWanController->pWanActiveIfaceData)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanController->pWanActiveIfaceData->data);
        pWanIfaceData->SelectionStatus = WAN_IFACE_ACTIVE;
        WanMgr_IfaceSM_Init(&wanIfCtrl, pWanIfaceData->uiIfaceIdx);

        /* Starts an instance of the WAN Interface State Machine on
        the interface to begin configuring the WAN link */
        WanMgr_StartInterfaceStateMachine(&wanIfCtrl);
    }

    CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_ACTIVE \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_ACTIVE;
}

static WcAwPolicyState_t Transition_ResetActiveInterface(WanMgr_Policy_Controller_t* pWanController)
{
    if(pWanController == NULL)
    {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Set Selected Interface ActiveLink to false, to make Running Interface State Machine Down.
    if (pWanController->pWanActiveIfaceData)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanController->pWanActiveIfaceData->data);
        pWanIfaceData->Wan.ActiveLink = FALSE;
        pWanIfaceData->SelectionStatus = WAN_IFACE_NOT_SELECTED;
        if (DmlSetWanActiveLinkInPSMDB(pWanController->activeInterfaceIdx, pWanIfaceData) != ANSC_STATUS_SUCCESS)
        {
            CcspTraceError(("%s-%d: Failed to set ActiveLink in PSM, SelectedInterface %d \n",
                                    __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
        }
    }

    //If there is reset on Selected Interface, then do clean all Wan.Status==InValid to Disabled.
    iInterfaceCount = 0;
    WanMgr_ResetInterfaceScanning();

    //Select the Interface with Available Interface in CPE Table
    WanMgr_Policy_Auto_SelectWANActive(pWanController);

    //Update The SelectedTimeOut for New SelectedInterface.
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pWanController->activeInterfaceIdx);
    if (pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
        pWanController->InterfaceSelectionTimeOut = pWanIfaceData->Wan.SelectionTimeout;
        CcspTraceInfo(("%s-%d: Selected New Interface, Interface-Idx=%d, Interface-Name=%s, SelectionTimeOut=%d \n",
                               __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx, pWanIfaceData->DisplayName,
                               pWanController->InterfaceSelectionTimeOut));
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }

    //Start Timer base on SelectiontimeOut DM.
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutStart));
    pWanController->SelectionTimeOutStart.tv_sec += pWanController->InterfaceSelectionTimeOut;

    CcspTraceInfo(("%s-%d: State changed to STATE_AUTO_WAN_INTERFACE_SCAN \n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_SCAN;
}
/*********************************************************************************/
/**************************** STATES *********************************************/
/*********************************************************************************/
static WcAwPolicyState_t State_WaitingForInterface(WanMgr_Policy_Controller_t* pWanController)
{
   DML_WAN_IFACE* pActiveInterface = NULL;

   if(pWanController == NULL)
   {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
   }

   //Check The TimeOut For Selected Interface.
   if(pWanController->SelectionTimeOutStart.tv_sec > 0)
   {
       clock_gettime( CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutEnd));
       if(difftime(pWanController->SelectionTimeOutEnd.tv_sec, pWanController->SelectionTimeOutStart.tv_sec ) > 0)
       {
           CcspTraceInfo(("%s-%d: SelectionTimeOut \n", __FUNCTION__, __LINE__));
           memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));

           if(pWanController->pWanActiveIfaceData)
           {
              pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
              CcspTraceInfo(("%s-%d: Set Interface %d, Wan.Status=InValid \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
              pActiveInterface->Wan.Status = WAN_IFACE_STATUS_INVALID;
           }

           return Transition_tryingNextInterface(pWanController);
       }
   }

   //If Phy.Status==up, Then Change the State to Scan Interface.
   if(pWanController->pWanActiveIfaceData)
   {
       pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
       if (pActiveInterface->Wan.Enable == TRUE)
       {
           if (pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_UP)
           {
               memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
               return Transition_InterfaceFound(pWanController);
           }
       }
       else
       {
           memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
           return Transition_tryingNextInterface(pWanController);
       }
   }

   return STATE_AUTO_WAN_INTERFACE_WAIT;
}

static WcAwPolicyState_t State_WaitingForInterfaceSM(WanMgr_Policy_Controller_t* pWanController)
{
   DML_WAN_IFACE* pActiveInterface = NULL;

   if(pWanController == NULL)
   {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
   }

   if(pWanController->pWanActiveIfaceData)
   {
       pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
       if (pActiveInterface->Wan.Status == WAN_IFACE_STATUS_DISABLED)
       {
           CcspTraceInfo(("%s-%d: Interface %d, Set Wan.Status=InValid \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
           pActiveInterface->Wan.Status = WAN_IFACE_STATUS_INVALID;
           return Transition_ScaningNextInterface(pWanController);
       }
   }

   return STATE_AUTO_WAN_INTERFACE_SM_WAIT;
}

static WcAwPolicyState_t State_ScaningInterface(WanMgr_Policy_Controller_t* pWanController)
{

   DML_WAN_IFACE* pActiveInterface = NULL;

   if(pWanController == NULL)
   {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
   }

   //If Global Wan Enable is false, then Select the next Interface.
   if(pWanController->WanEnable == FALSE)
   {
      return Transition_ScaningNextInterface(pWanController);
   }

   //Check If SelectedInterface TimeOut Happened.
   if(pWanController->SelectionTimeOutStart.tv_sec > 0)
   {
       clock_gettime( CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutEnd));
       if(difftime(pWanController->SelectionTimeOutEnd.tv_sec, pWanController->SelectionTimeOutStart.tv_sec ) > 0)
       {
           CcspTraceInfo(("%s-%d: SelectionTimeOut \n", __FUNCTION__, __LINE__));
           memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
           return Transition_ScaningNextInterface(pWanController);
       }
   }

   //Check If any Running Interface State Machine.
   if((pWanController->pWanActiveIfaceData) && (WanMgr_CheckAllIfacesDown()))
   {
       pActiveInterface = &(pWanController->pWanActiveIfaceData->data);

       //Check If Interface Wan Enable==false, Or Phy.Status==Down.
       if ((pActiveInterface->Wan.Enable == FALSE) ||
           (pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_DOWN))
       {
           pActiveInterface->Wan.ActiveLink = FALSE;
           pActiveInterface->SelectionStatus = WAN_IFACE_NOT_SELECTED;
           return Transition_ScaningNextInterface(pWanController);
       }

       //To check If any Instance of Interface State Machine Running.
       if ((pActiveInterface->Wan.Enable == TRUE) &&
           (pActiveInterface->SelectionStatus == WAN_IFACE_NOT_SELECTED))
       {
           memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
           return Transition_InterfaceFound(pWanController);
       }

       //Check If Interface Wan Enable==true, and Wan.Status-==up, Change the state to Interface Active.
       if ((pActiveInterface->Wan.Enable == TRUE) &&
           (pActiveInterface->Wan.Status == WAN_IFACE_STATUS_UP))
       {
           pActiveInterface->Wan.ActiveLink = TRUE;
           memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));

           //if interface is not configured for WAN, do it. this check is done by DM Device.Ethernet.X_RDK_Interface.{i}.UpStream
           BOOL UpStream = FALSE;
           if (!WanMgr_AutoPolicy_Set_InterfaceUpstream(pWanController->activeInterfaceIdx))
           {
               CcspTraceError(("%s-%d: Failed to set Upstream data model \n", __FUNCTION__, __LINE__));
           }

           //Check If RebootOnConfiguration==true, and check if RebootRequired==true, then reboot the CPE.
           if (pActiveInterface->Wan.RebootOnConfiguration == TRUE)
           {
               BOOL RebootRequired = FALSE;
               if (WanMgr_RdkBus_Get_InterfaceRebootRequired(pWanController->activeInterfaceIdx, &RebootRequired) != ANSC_STATUS_SUCCESS)
               {
                   CcspTraceError(("%s-%d: Failed to get Reboot Required data model, SelectedInterface %d, RebootRequired %d \n",
                                        __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx, RebootRequired));
               }

               //If RebootRequired==true, then Reboot CPE.
               if (RebootRequired == TRUE)
               {
                   return Transition_ReconfiguringPlatform(pWanController);
               }
           }
           return Transition_ActivatingInterface(pWanController);
       }
   }

    return STATE_AUTO_WAN_INTERFACE_SCAN;
}

static WcAwPolicyState_t State_Rebooting(WanMgr_Policy_Controller_t* pWanController)
{
   if(pWanController == NULL)
   {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
   }

   //Check If all Interface is down, then Reboot the CPE.
   if (WanMgr_CheckAllIfacesDown())
   {
       //Perform Reboot here with sysevent
       if (pWanController->pWanActiveIfaceData)
       {
           DML_WAN_IFACE* pWanIfaceData = &(pWanController->pWanActiveIfaceData->data);;
           pWanIfaceData->Wan.ActiveLink = FALSE;
           pWanIfaceData->SelectionStatus = WAN_IFACE_NOT_SELECTED;
           if (DmlSetWanActiveLinkInPSMDB(pWanController->activeInterfaceIdx, pWanIfaceData) != ANSC_STATUS_SUCCESS)
           {
               CcspTraceError(("%s-%d: Failed to set ActiveLink in PSM, SelectedInterface %d \n",
                                       __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
           }
       }
       CcspTraceInfo(("%s %d Rebooting CPE \n", __FUNCTION__, __LINE__));
       system("reboot");
   }

    return STATE_AUTO_WAN_INTERFACE_REBOOT;
}

static WcAwPolicyState_t State_WanInterfaceActive(WanMgr_Policy_Controller_t* pWanController)
{
   DML_WAN_IFACE* pActiveInterface = NULL;

   if(pWanController == NULL)
   {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
   }

   //Check If Global Wan Enable is false, then change state to Interface Down
   if(pWanController->WanEnable == FALSE)
   {
      return Transition_WanInterfaceDown(pWanController);
   }

   //Check If Wan.Status==disabled, and Phy.Status==Down, then change status to Interface Down.
   if(pWanController->pWanActiveIfaceData)
   {
       pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
       if ((pActiveInterface->Wan.Status == WAN_IFACE_STATUS_DISABLED) ||
           (pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_DOWN))
       {
           return Transition_WanInterfaceDown(pWanController);
       }
   }

   //If ResetActiveInterface==true, then Make Interface State Machine Down and Re-Select the New Interface.
   WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
   if(pWanConfigData != NULL)
   {
       if (pWanConfigData->data.ResetActiveInterface == TRUE)
       {
           pWanConfigData->data.ResetActiveInterface = FALSE;
           WanMgrDml_GetConfigData_release(pWanConfigData);
           return Transition_ResetActiveInterface(pWanController);
       }
       WanMgrDml_GetConfigData_release(pWanConfigData);
   }

   return STATE_AUTO_WAN_INTERFACE_ACTIVE;
}

static WcAwPolicyState_t State_WanInterfaceDown(WanMgr_Policy_Controller_t* pWanController)
{
   DML_WAN_IFACE* pActiveInterface = NULL;

   if(pWanController == NULL)
   {
        CcspTraceError(("%s %d pWanController object is NULL \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
   }

   //If Global Wan Enable==false, then Change the state to Interface Down.
   if(pWanController->WanEnable == FALSE)
   {
      return STATE_AUTO_WAN_INTERFACE_DOWN;
   }

   //Check The Phy.Status==UP, then Change the State to Active Interface.
   if(pWanController->pWanActiveIfaceData)
   {
       pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
       if (pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_UP)
       {
           return Transition_WanInterfaceUp(pWanController);
       }
   }

   //If ResetActiveInterface DM is True, Then Interface State Machine should be Down, and Re-Select the Next Interface.
   WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
   if(pWanConfigData != NULL)
   {
       if (pWanConfigData->data.ResetActiveInterface == TRUE)
       {
           pWanConfigData->data.ResetActiveInterface = FALSE;
           WanMgrDml_GetConfigData_release(pWanConfigData);
           return Transition_ResetActiveInterface(pWanController);
       }
       WanMgrDml_GetConfigData_release(pWanConfigData);
   }

   return STATE_AUTO_WAN_INTERFACE_DOWN;
}

/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/* WanMgr_Policy_AutoWanPolicy */
ANSC_STATUS WanMgr_Policy_AutoWanPolicy(void)
{
    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__));

    //detach thread from caller stack
    pthread_detach(pthread_self());

    //policy variables
    ANSC_STATUS retStatus = ANSC_STATUS_SUCCESS;
    WanMgr_Policy_Controller_t    WanPolicyCtrl;
    WcAwPolicyState_t pp_sm_state;
    bool bRunning = true;

    // event handler
    int n = 0;
    struct timeval tv;

    if(WanMgr_Controller_PolicyCtrlInit(&WanPolicyCtrl) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d Policy Controller Error \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d  Auto WAN Policy Thread Starting \n", __FUNCTION__, __LINE__));

    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        WanPolicyCtrl.WanEnable = pWanConfigData->data.Enable;
        WanMgrDml_GetConfigData_release(pWanConfigData);
    }
    // initialise state machine
    pp_sm_state = Transition_Start(&WanPolicyCtrl); // do this first before anything else to init variables

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
        if (WanPolicyCtrl.pWanActiveIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(WanPolicyCtrl.pWanActiveIfaceData->data);
            WanPolicyCtrl.InterfaceSelectionTimeOut = pWanIfaceData->Wan.SelectionTimeout;
        }
        // process state
        switch (pp_sm_state)
        {
            case STATE_AUTO_WAN_INTERFACE_WAIT:
                pp_sm_state = State_WaitingForInterface(&WanPolicyCtrl);
                break;
            case STATE_AUTO_WAN_INTERFACE_SM_WAIT:
                pp_sm_state = State_WaitingForInterfaceSM(&WanPolicyCtrl);
                break;
            case STATE_AUTO_WAN_INTERFACE_SCAN:
                pp_sm_state = State_ScaningInterface(&WanPolicyCtrl);
                break;
            case STATE_AUTO_WAN_INTERFACE_REBOOT:
                pp_sm_state = State_Rebooting(&WanPolicyCtrl);
                break;
            case STATE_AUTO_WAN_INTERFACE_ACTIVE:
                pp_sm_state = State_WanInterfaceActive(&WanPolicyCtrl);
                break;
            case STATE_AUTO_WAN_INTERFACE_DOWN:
                pp_sm_state = State_WanInterfaceDown(&WanPolicyCtrl);
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
