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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "wanmgr_controller.h"
#include "wanmgr_data.h"
#include "wanmgr_rdkbus_utils.h"
#include "wanmgr_interface_sm.h"
#include "wanmgr_platform_events.h"
#include "wanmgr_rdkbus_apis.h"

/* ---- Global Constants -------------------------- */
#define LOOP_TIMEOUT 500000 // timeout in milliseconds. This is the state machine loop interval


typedef enum {
    STATE_AUTO_WAN_INTERFACE_SELECTING = 0,
    STATE_AUTO_WAN_INTERFACE_WAITING,
    STATE_AUTO_WAN_INTERFACE_SCANNING,
    STATE_AUTO_WAN_INTERFACE_TEARDOWN,
    STATE_AUTO_WAN_INTERFACE_RECONFIGURATION,
    STATE_AUTO_WAN_REBOOT_PLATFORM,
    STATE_AUTO_WAN_INTERFACE_ACTIVE,
    STATE_AUTO_WAN_INTERFACE_DOWN,
    STATE_AUTO_WAN_ERROR,
    STATE_AUTO_WAN_TEARING_DOWN,
    STATE_AUTO_WAN_SM_EXIT
} WcAwPolicyState_t;

/* STATES */
static WcAwPolicyState_t State_SelectingInterface (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t State_WaitForInterface (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t State_ScanningInterface (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t State_WaitingForIfaceTearDown (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t State_InterfaceReconfiguration (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t State_RebootingPlatform (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t State_WanInterfaceActive (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t State_WanInterfaceDown (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t State_WaitingForInterfaceSMExit(WanMgr_Policy_Controller_t* pWanController);

/* TRANSITIONS */
static WcAwPolicyState_t Transition_Start (WanMgr_Policy_Controller_t* pWanController);
static WcAwPolicyState_t Transition_InterfaceSelected (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_InterfaceInvalid (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_TryingNextInterface (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_InterfaceFound (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_InterfaceDeselect (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_InterfaceValidated (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_RestartSelectionInterface (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_ReconfigurePlatform (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_ActivatingInterface (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transistion_WanInterfaceDown (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transistion_WanInterfaceUp (WanMgr_Policy_Controller_t * pWanController);
static WcAwPolicyState_t Transition_ResetActiveInterface (WanMgr_Policy_Controller_t * pWanController);

/*
 * WanMgr_SetActiveLink()
 * - sets the ActiveLink locallt and saves it to PSM
 */
static int WanMgr_SetActiveLink (WanMgr_Policy_Controller_t * pWanController, bool storeValue)
{
    if ((pWanController == NULL) || (pWanController->activeInterfaceIdx == -1)
            || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    // set ActiveLink in Interface data
    DML_WAN_IFACE* pIfaceData = &(pWanController->pWanActiveIfaceData->data);
    pIfaceData->Wan.ActiveLink = storeValue;

    // save ActiveLink value in PSM
    if (DmlSetWanActiveLinkInPSMDB(pWanController->activeInterfaceIdx, storeValue) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Failed to set ActiveLink in PSM, SelectedInterface %d \n",
                    __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    }
    return ANSC_STATUS_SUCCESS;
}

/*
 * WanMgr_GetPrevSelectedInterface()
 * - returns the interface index of previously selected interface
 */
static int WanMgr_GetPrevSelectedInterface (WanMgr_Policy_Controller_t* pWanController)
{
    if ((pWanController == NULL) || (pWanController->TotalIfaces == 0))
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return -1;
    }

    // select iface with ActiveLink = TRUE
    UINT uiLoopCount;
    int uiInterfaceIdx = -1;

    for( uiLoopCount = 0; uiLoopCount < pWanController->TotalIfaces; uiLoopCount++ )
        {
            WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
            if(pWanDmlIfaceData != NULL)
            {
                DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            if (pWanIfaceData->Wan.ActiveLink == TRUE)
            {
                if (pWanIfaceData->Wan.Enable != TRUE)
                {
                    CcspTraceInfo(("%s %d: Previous ActiveLink Interface Index:%d is Wan disabled, So setting ActiveLink to FALSE and saving it to PSM\n",
                                __FUNCTION__, __LINE__, uiLoopCount));
                    if (DmlSetWanActiveLinkInPSMDB(uiLoopCount, FALSE) != ANSC_STATUS_SUCCESS)
                    {
                        CcspTraceError(("%s-%d: Failed to set ActiveLink in PSM, SelectedInterface %d \n",
                                    __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
                    }
                    pWanIfaceData->Wan.ActiveLink = FALSE;
                    uiInterfaceIdx = -1;
                }
                else
                {
                    CcspTraceInfo(("%s %d: Previous ActiveLink Interface Index:%d\n",
                                __FUNCTION__, __LINE__, uiLoopCount));
                    uiInterfaceIdx = uiLoopCount;
                }
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                return uiInterfaceIdx;
                }

                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            }
        }
    return uiInterfaceIdx;
}

/*
 * WanMgr_Policy_Auto_GetHighPriorityIface()
 * - returns highest priority interface that is Wan.Enable == TRUE && Wan.Status == WAN_IFACE_STATUS_DISABLED
 */
static void WanMgr_Policy_Auto_GetHighPriorityIface(WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->WanEnable != TRUE)
        || (pWanController->TotalIfaces == 0))
    {
        CcspTraceError(("%s %d: Invalid args or Global Wan disabled\n", __FUNCTION__, __LINE__));
        return;
    }

    UINT uiLoopCount;
    INT iSelInterface = -1;
    INT iSelPriority = DML_WAN_IFACE_PRIORITY_MAX;

    for( uiLoopCount = 0; uiLoopCount < pWanController->TotalIfaces; uiLoopCount++ )
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            //Check Iface Wan Enable and Wan Status
            if ((pWanIfaceData->Wan.Enable == TRUE) && 
                    (pWanIfaceData->Wan.Priority >= 0) &&
                    (pWanIfaceData->Wan.Status == WAN_IFACE_STATUS_DISABLED))
            {
                if((pWanController->AllowRemoteInterfaces == FALSE) && (pWanIfaceData->Wan.IfaceType == REMOTE_IFACE))
                {
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    continue;
                }
                // pWanIfaceData - is Wan-Enabled & has valid Priority
                if(pWanIfaceData->Wan.Priority < iSelPriority)
                {
                    // update Primary iface with high priority iface
                    iSelInterface = uiLoopCount;
                    iSelPriority = pWanIfaceData->Wan.Priority;
                }
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }

    if (iSelInterface != -1)
    {
        pWanController->activeInterfaceIdx = iSelInterface;
        CcspTraceInfo(("%s %d: Current Selected iface index =%d \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    }
    
    return;
}

/*
 * WanMgr_UpdateControllerData()
 * - updates the controller data 
 * - this function is called every time in the begining of the loop,
         so that the states/transition can work with the current data
 */
static void WanMgr_UpdateControllerData (WanMgr_Policy_Controller_t* pWanController)
{
    if (pWanController == NULL)
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return;
    }

    //Update Wan config
    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        pWanController->WanEnable = pWanConfigData->data.Enable;
        pWanController->PolicyChanged = pWanConfigData->data.PolicyChanged;
        pWanController->AllowRemoteInterfaces = pWanConfigData->data.AllowRemoteInterfaces;

        WanMgrDml_GetConfigData_release(pWanConfigData);
    }
    pWanController->TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();

}

/*
 * WanMgr_ResetActiveLinkOnAllIface()
 * - reset ActiveLink to false and saves it in PSM
 * - this will prevent any newly connected high priority iface to become second ActiveLink, 
        in case ResetActiveLinkflag is set
 */
static int WanMgr_ResetActiveLinkOnAllIface (WanMgr_Policy_Controller_t* pWanController)
{
    if ((pWanController == NULL) || (pWanController->TotalIfaces == 0))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    UINT uiLoopCount;
    for (uiLoopCount = 0; uiLoopCount < pWanController->TotalIfaces; uiLoopCount++)
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            CcspTraceInfo(("%s %d: setting Interface index:%d, ActiveLink = FALSE and saving it in PSM \n", __FUNCTION__, __LINE__, uiLoopCount));
            pWanIfaceData->Wan.ActiveLink = FALSE;
            if (DmlSetWanActiveLinkInPSMDB(uiLoopCount, FALSE) != ANSC_STATUS_SUCCESS)
            {
                CcspTraceError(("%s %d: Failed to set ActiveLink in PSM, SelectedInterface %d \n",
                            __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                return ANSC_STATUS_FAILURE;
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
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
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return;
    }

    UINT uiLoopCount;
    for( uiLoopCount = 0; uiLoopCount < pWanController->TotalIfaces; uiLoopCount++ )
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

/*
 * WanMgr_CheckIfSelectedIfaceOnlyPossibleWanLink()
 * - checks if the selected iface is the only possible wan link
 * - if yes, returns TRUE, else returns FALSE
 */
static bool WanMgr_CheckIfSelectedIfaceOnlyPossibleWanLink (WanMgr_Policy_Controller_t* pWanController)
{
    if ((pWanController == NULL) || (pWanController->activeInterfaceIdx == -1)
        || (pWanController->TotalIfaces == 0))
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return FALSE;
    }

    UINT uiLoopCount;
    for( uiLoopCount = 0; uiLoopCount < pWanController->TotalIfaces; uiLoopCount++ )
    {
        if (uiLoopCount == pWanController->activeInterfaceIdx)
            continue;

        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

            if (pWanIfaceData->Wan.Enable == TRUE)
            {
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                return FALSE;
            }

            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }
    return TRUE; 

}

/*
 * WanMgr_CheckIfPlatformReconfiguringRequired()
 * - checks if Platform Reconfiguration is required
 * - if reconfig required, returns TRUE, else returns FALSE 
 */
static bool WanMgr_CheckIfPlatformReconfiguringRequired (WanMgr_Policy_Controller_t* pWanController)
{
    if ((pWanController == NULL) || (pWanController->TotalIfaces == 0))
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_INTERFACE_SELECTING;
    }

    UINT uiLoopCount;
    char dmQuery[BUFLEN_256] = {0};
    char dmValue[BUFLEN_256] = {0};

    for( uiLoopCount = 0; uiLoopCount < pWanController->TotalIfaces; uiLoopCount++ )
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);

        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

            if (pWanIfaceData->Wan.RebootOnConfiguration == FALSE)
            {
                // interface index: uiLoopCount doesnt not need reboot to configure
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                continue;
            }
            CcspTraceInfo(("%s %d: Checking interface index:%d, RebootOnConfiguration is set to TRUE\n", __FUNCTION__, __LINE__, uiLoopCount));

            snprintf(dmQuery, sizeof(dmQuery)-1, "%s%s", pWanIfaceData->Phy.Path, UPSTREAM_DM_SUFFIX);
            if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
            {
                CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                return FALSE;
            }

            if (uiLoopCount == pWanController->activeInterfaceIdx)
            {
                CcspTraceInfo(("%s %d: active interface index:%d, has interface upstream value: %s\n", __FUNCTION__, __LINE__, uiLoopCount, dmValue));
                // selected iface need to be Upstream WAN, else Platform reconfig required
                if (strncasecmp(dmValue, "true", 5) != 0)
                {
                    CcspTraceInfo(("%s %d:selected interface not upstream, so platform reconf required\n", __FUNCTION__, __LINE__));
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    return TRUE;
                }
            }
            else
            {
                // all other iface except selected interface needs to be downstream
                CcspTraceInfo(("%s %d: interface index:%d, has interface upstream value: %s\n", __FUNCTION__, __LINE__, uiLoopCount, dmValue));
                if (strncasecmp(dmValue, "false", 6) != 0)
                {
                    CcspTraceInfo(("%s %d: interface index:%d not upstream, so platform reconf required\n", __FUNCTION__, __LINE__, uiLoopCount));
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    return TRUE;
                }
            }
#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
            if(strncmp(pWanIfaceData->DisplayName, "WANOE", 6)==0)
            {
                memset(&dmValue, 0, sizeof(dmValue));
                memset(&dmQuery, 0, sizeof(dmQuery));
                snprintf(dmQuery, sizeof(dmQuery)-1, "%s%s", pWanIfaceData->Phy.Path,WAN_CONFIG_PORT_DM_SUFFIX);
                if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
                {
                    CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    return FALSE;
                }

                if((uiLoopCount == pWanController->activeInterfaceIdx) && (strncasecmp(dmValue, "true", 5) != 0))
                {
                    CcspTraceInfo(("%s %d:WANOE is selected but eth WAN HW configuration set to LAN, so platform reconf required\n", __FUNCTION__, __LINE__));
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    return TRUE;
                }else if((uiLoopCount != pWanController->activeInterfaceIdx) && (strncasecmp(dmValue, "true", 5) == 0))
                {

                    CcspTraceInfo(("%s %d:WANOE is not selected but eth WAN HW configuration set to WAN, so platform reconf required\n", __FUNCTION__, __LINE__));
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    return TRUE;
                }
            }
#endif
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }
    return FALSE;

}

/*
 * WanMgr_CheckIfIntfStateMachineRunning()
 * - checks if Interface state machine is running
 * - if yes, returns TRUE, else returns FALSE
 */
static bool WanMgr_CheckIfIntfStateMachineRunning (WanMgr_Policy_Controller_t* pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return FALSE;
    }

    DML_WAN_IFACE * pActiveInterface = &(pWanController->pWanActiveIfaceData->data); 
    if (pActiveInterface->Wan.Status != WAN_IFACE_STATUS_DISABLED)
    {
        return TRUE;
    }
    return FALSE;
}

/*
 * WanMgr_SetUpstreamOnlyForSelectedIntf()
 * - sets Upstream = TRUE for selected interface
 * - sets Upstream = FALSE for other interfaces
 */
static void WanMgr_SetUpstreamOnlyForSelectedIntf (WanMgr_Policy_Controller_t* pWanController)
{
    if ((pWanController == NULL) || (pWanController->activeInterfaceIdx == -1)
        || (pWanController->TotalIfaces == 0))
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return;
    }

    UINT uiLoopCount;
    char dmQuery[BUFLEN_256] = {0};
    char dmValue[BUFLEN_256] = {0};

    for( uiLoopCount = 0; uiLoopCount < pWanController->TotalIfaces; uiLoopCount++ )
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);

            snprintf(dmQuery, sizeof(dmQuery)-1, "%s%s", pWanIfaceData->Phy.Path, UPSTREAM_DM_SUFFIX);
            if (uiLoopCount == pWanController->activeInterfaceIdx)
            {
                snprintf(dmValue, sizeof(dmValue)-1, "%s", "true");
            }
            else
            {
                snprintf(dmValue, sizeof(dmValue)-1, "%s", "false");
            }
            WanMgr_RdkBus_SetParamValues(ETH_COMPONENT_NAME, ETH_COMPONENT_PATH, dmQuery, dmValue, ccsp_boolean, TRUE );
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }
    return; 

}

/*
 * WanMgr_GetResetActiveInterfaceFlag()
 * - returns the value of ResetActiveLinkFlag
 */
static bool WanMgr_GetResetActiveInterfaceFlag ()
{
    bool ret = FALSE;

    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        ret = pWanConfigData->data.ResetActiveInterface;
        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    return ret;
}

/*
 * WanMgr_StartIfaceStateMachine()
 * - starts the interface state machine
 * - if successful returns TRUE, else FALSE
 */
static int WanMgr_StartIfaceStateMachine (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    // Set SelectionStatus = ACTIVE & start Interface State Machine
    DML_WAN_IFACE * pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
    pActiveInterface->SelectionStatus = WAN_IFACE_ACTIVE;

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
 * Transition_Start()
 * - If the ActiveLink flag of any interface is set to TRUE, then that interface will be selected
 * - else, the interface with the highest priority in the table ("0" is the highest) will be selected. Wan scan timer is started
 */
static WcAwPolicyState_t Transition_Start (WanMgr_Policy_Controller_t* pWanController)
{
    if (pWanController == NULL)
    {
        CcspTraceError(("%s %d: Invalid args \n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if (pWanController->WanEnable == TRUE)
    {
        // select the previously used Active Link
        pWanController->activeInterfaceIdx = WanMgr_GetPrevSelectedInterface (pWanController);
        if (pWanController->activeInterfaceIdx == -1)
        {
            CcspTraceInfo(("%s %d: unable to select an interface from DB\n", __FUNCTION__, __LINE__));
            // No previous ActiveLink available, so select the highest priority interface
            WanMgr_Policy_Auto_GetHighPriorityIface(pWanController);
        }
    }

    wanmgr_sysevents_setWanState(WAN_LINK_DOWN_STATE);

    CcspTraceInfo(("%s %d: State changed to STATE_AUTO_WAN_INTERFACE_SELECTING \n", __FUNCTION__, __LINE__));

    return STATE_AUTO_WAN_INTERFACE_SELECTING;

}

/*
 * Transition_InterfaceSelected()
 * - Interface is Selected but not validated
 * - Start the timer, and go to the Wait for Interface State
 */
static WcAwPolicyState_t Transition_InterfaceSelected (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    // update the  controller SelectedTimeOut for new selected active iface
    DML_WAN_IFACE* pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
    pWanController->InterfaceSelectionTimeOut = pActiveInterface->Wan.SelectionTimeout;
    CcspTraceInfo(("%s %d: selected interface idx=%d, name=%s, selectionTimeOut=%d \n",
                __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx, pActiveInterface->DisplayName,
                pWanController->InterfaceSelectionTimeOut));

    // Start Timer based on SelectiontimeOut
    CcspTraceInfo(("%s %d: Starting timer for interface %d \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutStart));
    pWanController->SelectionTimeOutStart.tv_sec += pWanController->InterfaceSelectionTimeOut;

    CcspTraceInfo(("%s %d: State changed to STATE_AUTO_WAN_INTERFACE_WAITING \n", __FUNCTION__, __LINE__));

    return STATE_AUTO_WAN_INTERFACE_WAITING;
}

/*
 * Transition_InterfaceInvalid()
 * - called when interface failed to be validated before its SelectionTimeout
 * - Mark the selected interface as Invalid (Wan.Status = Invalid)
 * - Set ActiveLink to FALSE, and persist it in PSM
 * - Deselect the interface
 * - Return to Selecting Interface State
 */
static WcAwPolicyState_t Transition_InterfaceInvalid (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL)
        || (pWanController->activeInterfaceIdx == -1))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    // set Wan.Status = INVALID and Wan.ActiveLink as FALSE for selected interface before deselecting it
    DML_WAN_IFACE* pIfaceData = &(pWanController->pWanActiveIfaceData->data);
    CcspTraceInfo(("%s %d: setting Interface index:%d, Wan.Status=InValid \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    pIfaceData->Wan.Status = WAN_IFACE_STATUS_INVALID;

    // set ActiveLink = FALSE and save it to PSM
    CcspTraceInfo(("%s %d: setting Interface index:%d, ActiveLink = False and saving it in PSM \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    if (WanMgr_SetActiveLink (pWanController, FALSE) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Failed to set ActiveLink in PSM, SelectedInterface %d \n",
                    __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    }

    // deselect the interface
    CcspTraceInfo(("%s %d: de-selecting interface \n", __FUNCTION__, __LINE__));
    pWanController->activeInterfaceIdx = -1;

    return STATE_AUTO_WAN_INTERFACE_SELECTING;

}

/*
 * Transition_TryingNextInterface()
 * - no interface selected yet
 * - Select an interface with highest priority - iface with Wan.Status = Disabled
 * - if unable to select an interface, then mark all interface disabled
 * - got to Selecting Interface State in all cases
 */
static WcAwPolicyState_t Transition_TryingNextInterface (WanMgr_Policy_Controller_t * pWanController)
{
    if (pWanController == NULL)
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    WanMgr_Policy_Auto_GetHighPriorityIface(pWanController);

    if (pWanController->activeInterfaceIdx == -1)
    {
        CcspTraceInfo(("%s %d: No interface available to select. So resetting interface table\n", __FUNCTION__, __LINE__));
        WanMgr_ResetIfaceTable(pWanController);
    }

    CcspTraceInfo(("%s %d: selected interface %d\n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));

    return STATE_AUTO_WAN_INTERFACE_SELECTING;
}

/*
 * Transition_InterfaceFound()
 * selected iface is PHY UP
 * Set SelectionStatus to “Active” and start Interface State Machine thread
 * Go to Scanning Interface State
 */
static WcAwPolicyState_t Transition_InterfaceFound (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    // Set SelectionStatus = ACTIVE & start Interface State Machine
    if (WanMgr_StartIfaceStateMachine (pWanController) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: unable to start interface state machine\n", __FUNCTION__, __LINE__));
    }

    // update the  controller SelectedTimeOut for new selected active iface
    DML_WAN_IFACE* pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
    pWanController->InterfaceSelectionTimeOut = pActiveInterface->Wan.SelectionTimeout;
    CcspTraceInfo(("%s %d: selected interface idx=%d, name=%s, selectionTimeOut=%d \n",
                __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx, pActiveInterface->DisplayName,
                pWanController->InterfaceSelectionTimeOut));

    // Start Timer based on SelectiontimeOut
    CcspTraceInfo(("%s %d: Starting timer for interface %d \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutStart));
    pWanController->SelectionTimeOutStart.tv_sec += pWanController->InterfaceSelectionTimeOut;

    return STATE_AUTO_WAN_INTERFACE_SCANNING;
}

/*
 * Transition_InterfaceDeselect()
 * - Selected interface is Phy DOWN
 * - Set SelectionStatus to “NOT_SELECTED”, the iface sm thread teardown
 * - Go to Waiting Interface Teardown State 
 */
static WcAwPolicyState_t Transition_InterfaceDeselect (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    DML_WAN_IFACE * pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
    CcspTraceInfo(("%s %d: SelectionStatus set to NOT_SELECTED. Tearing down iface state machine\n", __FUNCTION__, __LINE__));
    pActiveInterface->SelectionStatus = WAN_IFACE_NOT_SELECTED;

    return STATE_AUTO_WAN_INTERFACE_TEARDOWN;

}

/*
 * Transition_InterfaceValidated()
 * selected interface is 
 * - Set ActiveLink to TRUE, and persist it in PSM
 * - Stop WAN scan timer
 * - Go to Interface Reconfiguration State
 */
static WcAwPolicyState_t Transition_InterfaceValidated (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL)) 
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    // Set ActiveLink to TRUE and store it in PSM
    CcspTraceInfo(("%s %d: setting Interface index:%d, ActiveLink = TRUE and saving it in PSM \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    if (WanMgr_SetActiveLink(pWanController, TRUE) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s-%d: Failed to set ActiveLink in PSM, SelectedInterface %d \n",
                    __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    }

    // stop timer
    CcspTraceInfo(("%s %d: stopping timer\n", __FUNCTION__, __LINE__));
    memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));

    return STATE_AUTO_WAN_INTERFACE_RECONFIGURATION;

}

/*
 * Transition_RestartSelectionInterface()
 * - Deselect the interface (previously selected)
 * - Reset table, interfaces marked as Invalid should be marked as Disabled
 * - Set ResetActiveLink flag is set to “FALSE”
 * - Go to Selecting Interface State
 */
static WcAwPolicyState_t Transition_RestartSelectionInterface (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    // reset all interfaces for selection
    CcspTraceInfo(("%s %d: So resetting interface table\n", __FUNCTION__, __LINE__));
    WanMgr_ResetIfaceTable(pWanController);

    // reset ActiveLink to false for all interfaces and save it in PSM
    if (WanMgr_ResetActiveLinkOnAllIface(pWanController) != ANSC_STATUS_SUCCESS)
    {   
        CcspTraceError(("%s %d: Unable to reset ActiveLink in all interfaces\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    // reset ResetActiveInterface to FALSE
    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
            pWanConfigData->data.ResetActiveInterface = FALSE;
        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    // deselect interface 
    CcspTraceInfo(("%s %d: de-selecting interface\n", __FUNCTION__, __LINE__));
    pWanController->activeInterfaceIdx = -1;

    return STATE_AUTO_WAN_INTERFACE_SELECTING;
}

/*
 * Transition_ReconfigurePlatform()
 * - need to reconfigure platform
 * - Set SelectionStatus to “NOT_SELECTED”, this will trigger the interface state machine thread teardown.
 * - Go to Rebooting Platform
 */
static WcAwPolicyState_t Transition_ReconfigurePlatform (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL)) 
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    // set SelectionStatus = WAN_IFACE_NOT_SELECTED, to tear down iface sm 
    DML_WAN_IFACE * pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
    pActiveInterface->SelectionStatus = WAN_IFACE_NOT_SELECTED;
    CcspTraceInfo(("%s %d: setting SelectionStatus for interface:%d as NOT_SELECTED \n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));

    return STATE_AUTO_WAN_REBOOT_PLATFORM;
}

/*
 * Transition_ActivatingInterface()
 * - Go to WAN Interface Active State
 */
static WcAwPolicyState_t Transition_ActivatingInterface (WanMgr_Policy_Controller_t * pWanController)
{
    CcspTraceInfo(("%s %d: moving to state State_WanInterfaceActive()\n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_ACTIVE;
}

/*
 * Transistion_WanInterfaceDown()
 */
static WcAwPolicyState_t Transistion_WanInterfaceDown (WanMgr_Policy_Controller_t * pWanController)
{
    if (pWanController == NULL)
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }
    CcspTraceInfo(("%s %d: moving to State_WanInterfaceDown()\n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_DOWN;
}

/*
 *  Transistion_WanInterfaceUp()
 * - Start Interface State Machine thread
 * - Go to WAN Interface Active State
 */
static WcAwPolicyState_t Transistion_WanInterfaceUp (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->activeInterfaceIdx == -1) || 
        (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if (WanMgr_CheckIfIntfStateMachineRunning(pWanController) == TRUE)
    {
        CcspTraceInfo(("%s %d: Waiting to start new interface state machine \n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_INTERFACE_DOWN;
    }
    else
    {
        if (WanMgr_StartIfaceStateMachine (pWanController) != ANSC_STATUS_SUCCESS)
        {
            CcspTraceError(("%s %d: unable to start interface state machine\n", __FUNCTION__, __LINE__));
        }
    }

    CcspTraceInfo(("%s %d: started interface state machine & moving to state State_WanInterfaceActive()\n", __FUNCTION__, __LINE__));
    return STATE_AUTO_WAN_INTERFACE_ACTIVE;
}

/*
 * Transition_ResetActiveInterface()
 * - Set SelectionStatus to “NOT_SELECTED”, this will trigger the interface state machine thread teardown
 */

static WcAwPolicyState_t Transition_ResetActiveInterface (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL)) 
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    DML_WAN_IFACE* pWanIfaceData = &(pWanController->pWanActiveIfaceData->data);
    pWanIfaceData->SelectionStatus = WAN_IFACE_NOT_SELECTED;
    CcspTraceInfo(("%s %d: SelectionStatus set to NOT_SELECTED. moving to State_WaitingForIfaceTearDown()\n", __FUNCTION__, __LINE__));

    return STATE_AUTO_WAN_INTERFACE_TEARDOWN;
}

/*
 * State_SelectingInterface()
 * - If some interface was selected, the Interface Selected Transition will be called
 * - Else, if no interface was selected, the Trying Next Interface Transition will be called
 */
static WcAwPolicyState_t State_SelectingInterface (WanMgr_Policy_Controller_t * pWanController)
{
    if (pWanController == NULL)
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if(pWanController->WanEnable == FALSE || pWanController->PolicyChanged == TRUE)
    {
        return STATE_AUTO_WAN_SM_EXIT;
    }

    if (pWanController->activeInterfaceIdx != -1)
    {
        CcspTraceInfo (("%s %d: Selected interface index:%d\n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
        return Transition_InterfaceSelected (pWanController);
    }

    return Transition_TryingNextInterface (pWanController);

}

/* State_WaitForInterface()
 * - If the Phy.Status flag is set to "UP" before the WAN scan timer expires, the Interface Found Transition will be called
 * - If the selected interface is the only Interface enabled (Wan.Enable = TRUE), stay in this state (Wait for Interface State)
 * - If the WAN scan timer expires (and the interface selected is not the only possible interface), the Interface Invalid Transition will be called
 */
static WcAwPolicyState_t State_WaitForInterface (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL)) 
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if(pWanController->WanEnable == FALSE || pWanController->PolicyChanged == TRUE)
    {
        return STATE_AUTO_WAN_INTERFACE_DOWN;
    }

    // check if Phy is UP
    DML_WAN_IFACE * pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
    if (pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_UP)
    {
        // Phy is UP for selected iface
        CcspTraceInfo(("%s %d: selected interface index:%d is PHY UP\n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
        CcspTraceInfo(("%s %d: stopping timer\n", __FUNCTION__, __LINE__));
        memset(&(pWanController->SelectionTimeOutStart), 0, sizeof(struct timespec));
        return Transition_InterfaceFound(pWanController);
    }

    // Check if timer expired for selected Interface & check if selected interface is not the only available wan link
    clock_gettime( CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutEnd));
    if((difftime(pWanController->SelectionTimeOutEnd.tv_sec, pWanController->SelectionTimeOutStart.tv_sec ) > 0)
        && (WanMgr_CheckIfSelectedIfaceOnlyPossibleWanLink(pWanController) == FALSE))
    {
        // timer expired for selected iface but there is another interface that can be used
        CcspTraceInfo(("%s %d: Validation Timer expired for interface index:%d and there is another iface that can be possibly used as Wan interface\n", 
            __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
        return Transition_InterfaceInvalid(pWanController);
    }

    return STATE_AUTO_WAN_INTERFACE_WAITING;

}

/*
 * State_ScanningInterface()
 * - If the Phy.Status flag is set to "DOWN", the Interface Deselected Transition will be called
 * - If the Wan.Status is set to "UP", indicating that the interface was validated, the Interface Validated Transition will be called
 * - If the selected interface is the only Interface enabled (Wan.Enable = TRUE), the Interface Validated Transition will be called
 * - If the WAN scan timer expires (and the interface selected is not the only possible interface), the Interface Deselected Transition will be called.
 */
static WcAwPolicyState_t State_ScanningInterface (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if(pWanController->WanEnable == FALSE || pWanController->PolicyChanged == TRUE)
    {
        return STATE_AUTO_WAN_INTERFACE_DOWN;
    }

    // If Phy is not UP, move to interface deselect transition
    DML_WAN_IFACE * pActiveInterface = &(pWanController->pWanActiveIfaceData->data); 
    if (pActiveInterface->Phy.Status != WAN_IFACE_PHY_STATUS_UP)
    { 
        CcspTraceInfo(("%s %d: selected interface index:%d is now PHY DOWN\n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
        return Transition_InterfaceDeselect(pWanController);
    }

    bool SelectedIfaceLastWanLink = WanMgr_CheckIfSelectedIfaceOnlyPossibleWanLink(pWanController);

    // checked if iface is validated or only interface enabled
    if ((pActiveInterface->Wan.Status == WAN_IFACE_STATUS_UP) || 
            (SelectedIfaceLastWanLink == TRUE))
    {
        CcspTraceInfo(("%s %d: Interface validated\n", __FUNCTION__, __LINE__));
        return Transition_InterfaceValidated(pWanController);
    }

    // if timer is expired and there is another iface that can be used as Wan, deselect interface
   if((pWanController->SelectionTimeOutStart.tv_sec > 0) && 
            (SelectedIfaceLastWanLink == FALSE))
   {
       clock_gettime( CLOCK_MONOTONIC_RAW, &(pWanController->SelectionTimeOutEnd));
       if(difftime(pWanController->SelectionTimeOutEnd.tv_sec, pWanController->SelectionTimeOutStart.tv_sec ) > 0)
       {
            CcspTraceInfo(("%s %d: Validation Timer expired for interface index:%d and there is another iface that can be possibly used as Wan interface\n", 
                __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
            return Transition_InterfaceDeselect(pWanController);
       }
    }

    return STATE_AUTO_WAN_INTERFACE_SCANNING;

}

/*
 * State_WaitingForIfaceTearDown()
 * - wait for iface state machine to go down
 * - After the Interface State Machine thread terminate
 *       check for ResetActiveInterface flag - if TRUE goto Restart Selection Transition
 *       else go to iface invalid transition
 */
static WcAwPolicyState_t State_WaitingForIfaceTearDown (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if(pWanController->WanEnable == FALSE || pWanController->PolicyChanged == TRUE)
    {
        return STATE_AUTO_WAN_INTERFACE_DOWN;
    }

    // check if iface sm is running
    if (WanMgr_CheckIfIntfStateMachineRunning(pWanController) == TRUE)
    {
        return STATE_AUTO_WAN_INTERFACE_TEARDOWN;
    }
    CcspTraceInfo(("%s %d: Iface state machine has exited\n", __FUNCTION__, __LINE__));

    // check ResetActiveInterface
    if (WanMgr_GetResetActiveInterfaceFlag() == TRUE)
    {
        CcspTraceInfo(("%s %d: ResetActiveInterface flag detected\n", __FUNCTION__, __LINE__));
        return Transition_RestartSelectionInterface(pWanController);
    }

    return Transition_InterfaceInvalid (pWanController);

}

/*
 * State_InterfaceReconfiguration()
 * Check the HW configstatus (WAN/LAN), if the current HW config is LAN and the Wan.RebootOnConfiguration is set to “TRUE”
 */
static WcAwPolicyState_t State_InterfaceReconfiguration (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL))
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if (WanMgr_CheckIfPlatformReconfiguringRequired (pWanController) == TRUE)
    { 
        CcspTraceInfo(("%s %d: Hardware reconfiguration required\n", __FUNCTION__, __LINE__));
        return Transition_ReconfigurePlatform (pWanController);
    }
    CcspTraceInfo(("%s %d: Hardware reconfiguration not required\n", __FUNCTION__, __LINE__));

    return Transition_ActivatingInterface (pWanController);
}

/*
 * State_RebootingPlatform ()
 * - If interface State Machine thread still up, stay in this state
 * - Send Interface.Upstream = TRUE to the selected interface
 * - Send Interface.Upstream = FALSE to interfaces not selected
 */
static WcAwPolicyState_t State_RebootingPlatform (WanMgr_Policy_Controller_t * pWanController)
{
    if (pWanController == NULL)
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    // check if interface state machine is still running
    if (WanMgr_CheckIfIntfStateMachineRunning(pWanController) == TRUE)
    {
        CcspTraceInfo(("%s %d: Iface state machine still running..\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_REBOOT_PLATFORM;
    }
    CcspTraceInfo(("%s %d: Iface state machine has exited\n", __FUNCTION__, __LINE__));

    CcspTraceInfo(("%s %d: setting upstream for active interface:%d and going for a reboot\n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
    WanMgr_SetUpstreamOnlyForSelectedIntf (pWanController);

    system ("reboot");

    return STATE_AUTO_WAN_REBOOT_PLATFORM;
}

/*
 * State_WanInterfaceActive()
 * - If the ResetActiveLink flag is set to "TRUE", the Reset Active Interface Transition will be called
 * - If the Phy.Status flag is set to "DOWN", the WAN Interface Down Transition will be called
 */
static WcAwPolicyState_t State_WanInterfaceActive (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL)) 
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if(pWanController->WanEnable == FALSE || pWanController->PolicyChanged == TRUE)
    {
        return STATE_AUTO_WAN_INTERFACE_DOWN;
    }

    // check ResetActiveInterface
    if (WanMgr_GetResetActiveInterfaceFlag() == TRUE)
    {
        CcspTraceInfo(("%s %d: ResetActiveInterface flag detected\n", __FUNCTION__, __LINE__));
        return Transition_ResetActiveInterface (pWanController);
    }

    // check if PHY is still UP
    DML_WAN_IFACE * pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
    if (pActiveInterface->Phy.Status != WAN_IFACE_PHY_STATUS_UP)
    {
        CcspTraceInfo(("%s %d: interface:%d is PHY down\n", __FUNCTION__, __LINE__, pWanController->activeInterfaceIdx));
        return Transistion_WanInterfaceDown (pWanController);
    }

    return STATE_AUTO_WAN_INTERFACE_ACTIVE;
}

/*
 * State_WanInterfaceDown ()
 * - If the ResetActiveLink flag is set to "TRUE", the Reset Active Interface Transition will be called
 * - If the Phy.Status flag is set to "UP", the WAN Interface Up Transition will be called
 */
static WcAwPolicyState_t State_WanInterfaceDown (WanMgr_Policy_Controller_t * pWanController)
{
    if ((pWanController == NULL) || (pWanController->pWanActiveIfaceData == NULL)) 
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return STATE_AUTO_WAN_ERROR;
    }

    if(pWanController->WanEnable == FALSE || pWanController->PolicyChanged == TRUE)
    {
        return STATE_AUTO_WAN_TEARING_DOWN;
    }

    // check ResetActiveInterface
    if (WanMgr_GetResetActiveInterfaceFlag() == TRUE)
    {
        CcspTraceInfo(("%s %d: ResetActiveInterface flag detected\n", __FUNCTION__, __LINE__));
        return Transition_ResetActiveInterface (pWanController);
    }

    // check if PHY is UP
    DML_WAN_IFACE * pActiveInterface = &(pWanController->pWanActiveIfaceData->data);
    if (pActiveInterface->Phy.Status == WAN_IFACE_PHY_STATUS_UP)
    {
        return Transistion_WanInterfaceUp (pWanController);
    }
    
    return STATE_AUTO_WAN_INTERFACE_DOWN;
}

static WcAwPolicyState_t State_WaitingForInterfaceSMExit(WanMgr_Policy_Controller_t* pWanController)
{
    DML_WAN_IFACE* pFixedInterface = NULL;

    if((pWanController != NULL) && (pWanController->pWanActiveIfaceData != NULL))
    {
        pFixedInterface = &(pWanController->pWanActiveIfaceData->data);
    }

    if(pFixedInterface == NULL)
    {
        return STATE_AUTO_WAN_TEARING_DOWN;
    }

    pFixedInterface->SelectionStatus = WAN_IFACE_NOT_SELECTED;

    if(WanMgr_CheckIfIntfStateMachineRunning(pWanController) == TRUE)
    {
        return STATE_AUTO_WAN_TEARING_DOWN;
    }

    WanMgr_ResetIfaceTable(pWanController);

    if(pWanController->PolicyChanged == TRUE)
    {
        WanMgr_ResetActiveLinkOnAllIface(pWanController);
    }
    return STATE_AUTO_WAN_SM_EXIT;

}


ANSC_STATUS WanMgr_Policy_AutoWanPolicy (void)
{
    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__));

    // detach thread from caller stack
    pthread_detach(pthread_self());

    bool bRunning = true;
    int n = 0;
    WanMgr_Policy_Controller_t    WanController;
    WcAwPolicyState_t aw_sm_state;
    int retStatus = ANSC_STATUS_SUCCESS;
    struct timeval tv;

    // initialising policy data
    if(WanMgr_Controller_PolicyCtrlInit(&WanController) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s %d: Policy Controller Error \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    // updates policy controller data
    WanMgr_UpdateControllerData (&WanController);

    CcspTraceInfo(("%s %d: Auto WAN Policy Thread Starting \n", __FUNCTION__, __LINE__));

    aw_sm_state = Transition_Start(&WanController); // do this first before anything else to init variables

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

        // updates policy controller data
        WanMgr_UpdateControllerData (&WanController);

        // lock Iface Data & update selected iface data in controller data
        WanController.pWanActiveIfaceData = WanMgr_GetIfaceData_locked(WanController.activeInterfaceIdx);
        if (WanController.pWanActiveIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(WanController.pWanActiveIfaceData->data);
            WanController.InterfaceSelectionTimeOut = pWanIfaceData->Wan.SelectionTimeout;
        }

        // process states
        switch (aw_sm_state)
        {
            case STATE_AUTO_WAN_INTERFACE_SELECTING:
                aw_sm_state = State_SelectingInterface(&WanController);
                break;
            case STATE_AUTO_WAN_INTERFACE_WAITING:
                aw_sm_state = State_WaitForInterface(&WanController);
                break;
            case STATE_AUTO_WAN_INTERFACE_SCANNING:
                aw_sm_state = State_ScanningInterface(&WanController);
                break;
            case STATE_AUTO_WAN_INTERFACE_TEARDOWN:
                aw_sm_state = State_WaitingForIfaceTearDown(&WanController);
                break;
            case STATE_AUTO_WAN_INTERFACE_RECONFIGURATION:
                aw_sm_state = State_InterfaceReconfiguration(&WanController);
                break;
            case STATE_AUTO_WAN_REBOOT_PLATFORM:
                aw_sm_state = State_RebootingPlatform(&WanController);
                break;
            case STATE_AUTO_WAN_INTERFACE_ACTIVE:
                aw_sm_state = State_WanInterfaceActive(&WanController);
                break;
            case STATE_AUTO_WAN_INTERFACE_DOWN:
                aw_sm_state = State_WanInterfaceDown(&WanController);
                break;
            case STATE_AUTO_WAN_TEARING_DOWN:
                aw_sm_state = State_WaitingForInterfaceSMExit(&WanController);
                break;
            case STATE_AUTO_WAN_SM_EXIT:
                bRunning = false;
                break;
            case STATE_AUTO_WAN_ERROR:
            default:
                CcspTraceInfo(("%s %d: Failure Case \n", __FUNCTION__, __LINE__));
                bRunning = false;
                retStatus = ANSC_STATUS_FAILURE;
                break;
        }

        // release lock iface data
        if(WanController.pWanActiveIfaceData != NULL)
        {
            WanMgrDml_GetIfaceData_release(WanController.pWanActiveIfaceData);
        }

    }

    CcspTraceInfo(("%s %d - Exit from state machine\n", __FUNCTION__, __LINE__));
    return retStatus;
}
