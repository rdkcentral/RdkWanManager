/*
 * If not stated otherwise in this file or this component's LICENSE file the
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


#include "wanmgr_interface_apis.h"


/**
 * @brief Starts the interface state machine to obtain a WAN lease on all virtual interfaces configured for the specified interface.
 *
 * @param[in] interfaceIndex Index of the WAN interface to start.
 * @param[in] selectionStatus Selection status for the interface. Possible values:
 *   - WAN_IFACE_UNKNOWN: Interface selection status is unknown.
 *   - WAN_IFACE_NOT_SELECTED: Interface is not selected.
 *   - WAN_IFACE_VALIDATING: Start WAN and move to standby state.
 *   - WAN_IFACE_SELECTED: Start WAN and move to standby state.
 *   - WAN_IFACE_ACTIVE: Start WAN and activate the interface if it obtains an IP address.
 *
 * @return Status of the operation.
 */
int WanMgr_StartWan(int interfaceIndex, WANMGR_IFACE_SELECTION selectionStatus)
{
    int ret = 0;

    CcspTraceInfo(("%s %d: Entering WanMgr_StartWan for interfaceIndex=%d, selectionStatus=%d\n", __FUNCTION__, __LINE__, interfaceIndex, selectionStatus));
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(interfaceIndex);
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
        if(pWanIfaceData->Selection.Enable == FALSE)
        {
            CcspTraceInfo(("%s %d: Interface %d is disabled. not Starting WAN interface SM\n", __FUNCTION__, __LINE__, interfaceIndex));
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            return -1;
        }

        if(selectionStatus != WAN_IFACE_UNKNOWN && selectionStatus != WAN_IFACE_NOT_SELECTED)
        {
            CcspTraceInfo(("%s %d: Setting Selection.Status = %d for interfaceIndex=%d\n", __FUNCTION__, __LINE__, selectionStatus, interfaceIndex));
            pWanIfaceData->Selection.Status = selectionStatus;
        }
        else
        {
            CcspTraceError(("%s %d: Invalid selection status %d for interfaceIndex=%d\n", __FUNCTION__, __LINE__, selectionStatus, interfaceIndex));
        }
        pWanIfaceData->VirtIfChanged = FALSE;

        for(int VirtId=0; VirtId < pWanIfaceData->NoOfVirtIfs; VirtId++)
        {
            DML_VIRTUAL_IFACE* p_VirtIf = WanMgr_getVirtualIface_locked(interfaceIndex, VirtId);
            if(p_VirtIf != NULL)
            {
                if (p_VirtIf->Enable == FALSE)
                {
                    CcspTraceError(("%s %d: virtual if(%d) : %s is not enabled. Not starting VISM\n", __FUNCTION__, __LINE__,p_VirtIf->VirIfIdx, p_VirtIf->Alias));
                    WanMgr_VirtualIfaceData_release(p_VirtIf);
                    continue;
                }

                if(p_VirtIf->Interface_SM_Running == TRUE)
                {
                    CcspTraceWarning(("%s %d: Virtual Interface SM is already running for virtual interface %d \n", __FUNCTION__, __LINE__, VirtId));
                    WanMgr_VirtualIfaceData_release(p_VirtIf);
                    continue;
                }
                WanMgr_VirtualIfaceData_release(p_VirtIf);
            }

            /*Start virtual Interface SM */
            WanMgr_IfaceSM_Controller_t wanIfCtrl;
            WanMgr_IfaceSM_Init(&wanIfCtrl, interfaceIndex, VirtId);

            if (WanMgr_StartInterfaceStateMachine(&wanIfCtrl) != 0)
            {
                CcspTraceError(("%s %d: Unable to start Virtual interface state machine \n", __FUNCTION__, __LINE__));
                ret = -1;
            }
        }
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }

    return ret;
}


/**
 * @brief Stop WAN .
 *
 * This function stops the WAN interface state machine and sets the selection status to not selected.
 *
 * @param[in] interfaceIndex Index of the WAN interface to Stop WAN.
 * @param[in] waitForTermination If true, waits for the interface state machine to terminate before returning.
 * @return Status of the operation.
 */
int WanMgr_StopWan(int interfaceIndex, bool waitForTermination)
{
    int ret = 0;

    CcspTraceInfo(("%s %d: Entering WanMgr_StopWan for interfaceIndex=%d waitForTermination=%d\n", __FUNCTION__, __LINE__, interfaceIndex, waitForTermination));
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(interfaceIndex);
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
        //Set Selection.Status to “NOT_SELECTED”, to teardown the  the iface sm thread 
        CcspTraceInfo(("%s %d: Selection.Status set to NOT_SELECTED. Tearing down iface state machine\n", __FUNCTION__, __LINE__));
        pWanIfaceData->Selection.Status = WAN_IFACE_NOT_SELECTED;

        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);   
    }

    if(waitForTermination)
    {
        // Wait for the interface state machine to terminate for 10 seconds
        CcspTraceInfo(("%s %d: Waiting for interface state machine to terminate\n", __FUNCTION__, __LINE__));
        int waited_ms = 0;
        while (waited_ms < 10000)
        {
            if (WanMgr_Get_ISM_RunningStatus(interfaceIndex) == FALSE)
            {
                break;
            }
            usleep(200 * 1000); // 200 ms
            waited_ms += 200;
        }

        if (WanMgr_Get_ISM_RunningStatus(interfaceIndex) == TRUE)
        {
            CcspTraceError(("%s %d: Failed to stop interface state machine for interfaceIndex=%d within 10 seconds\n", __FUNCTION__, __LINE__, interfaceIndex));
            ret = -1;
        }
    }

    return ret;
}

/**
 * @brief Activates the WAN interface specified by the given index.
 *
 * If the interface already has a WAN lease(STANDBY), this function activates it by configuring
 * the default route, firewall, DNS, and MAP-T settings of the device based on the lease,
 * and transitions the interface to the active state. If the interface does not have a WAN IP,
 * the function returns a failure.
 *
 * @param interfaceIndex The index of the WAN interface to activate.
 * @return Returns 0 on success, or a negative error code on failure.
 */
int WanMgr_ActivateInterface(int interfaceIndex);

/**
 * @brief Deactivates the WAN interface and deconfigures the default route, firewall, DNS, and WAN IP, moving the interface back to the standby state.
 *
 * This function Deactivates the specified WAN interface, removes its default route, firewall, DNS settings, and WAN IP configuration, and transitions the interface to the standby state.
 *
 * @param[in] interfaceIndex The index of the WAN interface to deactivate.
 * @return 0 on success, or a negative error code on failure.
 */
int WanMgr_DeactivateInterface(int interfaceIndex);

