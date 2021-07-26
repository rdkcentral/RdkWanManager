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

#include "ansc_platform.h"
#include "wanmgr_controller.h"
#include "wanmgr_rdkbus_utils.h"
#include "wanmgr_data.h"

ANSC_STATUS WanController_Policy_Change(void)
{
    /* Wan policy changed. Cpe needs a restart! */
    FILE *fp = NULL;
    char value[25] = {0};
    char cmd[128] = {0};
    char acOutput[64] = {0};
    int seconds = 30;
    int rebootCount = 1;

    memset(value, 0, sizeof(value));
    fp = popen("syscfg get X_RDKCENTRAL-COM_LastRebootCounter", "r");
    if (fp == NULL) {
        return ANSC_STATUS_FAILURE;
    }
    pclose(fp);

    rebootCount = atoi(value);

    CcspTraceInfo(("Updating the last reboot reason and last reboot counter\n"));
    sprintf(cmd, "syscfg set X_RDKCENTRAL-COM_LastRebootReason Wan_Policy_Change ");
    system(cmd);
    sprintf(cmd, "syscfg set X_RDKCENTRAL-COM_LastRebootCounter %d ",rebootCount);
    system(cmd);
    system("syscfg commit");

    while(seconds > 0)
    {
        printf("...(%d)...\n", seconds);
        seconds -= 10;
        sleep(10);
    }

    system("/rdklogger/backupLogs.sh true");

    return ANSC_STATUS_SUCCESS;
}

/* WanController_Start_StateMachine() */
ANSC_STATUS WanController_Start_StateMachine(DML_WAN_POLICY swan_policy)
{
    ANSC_STATUS retStatus = ANSC_STATUS_FAILURE;
    DML_WAN_POLICY wan_policy = FIXED_MODE;
    int iErrorCode = 0;

    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__ ));

    //Get Policy
    WanMgr_Config_Data_t* pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        DML_WANMGR_CONFIG* pWanConfig = &(pWanConfigData->data);

        wan_policy = pWanConfig->Policy;

        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    //Starts wan controller threads
    switch (wan_policy) {
        case FIXED_MODE:
            retStatus = WanMgr_Policy_FixedModePolicy();
            break;

        case FIXED_MODE_ON_BOOTUP:
            retStatus = WanMgr_Policy_FixedModeOnBootupPolicy();
            break;

        case PRIMARY_PRIORITY:
            retStatus = WanMgr_Policy_PrimaryPriorityPolicy();
            break;

        case PRIMARY_PRIORITY_ON_BOOTUP:
            retStatus = WanMgr_Policy_PrimaryPriorityOnBootupPolicy();
            break;

        case MULTIWAN_MODE:
            break;

        case AUTOWAN_MODE: 
#if defined (_XB8_PRODUCT_REQ_)
            retStatus = WanMgr_Policy_AutoWan();
#else
            retStatus = WanMgr_Policy_AutoWanPolicy();
#endif
            break;
    }

    if( ANSC_STATUS_SUCCESS != retStatus )
    {
        CcspTraceInfo(("%s %d Error: Failed to start State Machine Thread error code: %d \n", __FUNCTION__, __LINE__, retStatus ));
        retStatus = ANSC_STATUS_SUCCESS;
    }

    WanController_Policy_Change();

    return retStatus;
}

/* WanController_Init_StateMachine */
ANSC_STATUS WanController_Init_StateMachine(void)
{
    int iTotalInterfaces = 0;
    DML_WAN_POLICY wan_policy;

    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__ ));

    // Get the configured wan policy
    if(WanMgr_RdkBus_getWanPolicy(&wan_policy) != ANSC_STATUS_SUCCESS) {
        CcspTraceInfo(("%s %d  Error: WanController_getWanPolicy() failed \n", __FUNCTION__, __LINE__ ));
        return ANSC_STATUS_FAILURE;
    }

    if(WanController_Start_StateMachine(wan_policy) != ANSC_STATUS_SUCCESS) {
        CcspTraceInfo(("%s %d Error: WanController_Start_StateMachine failed \n", __FUNCTION__, __LINE__ ));
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}


ANSC_STATUS WanMgr_Controller_PolicyCtrlInit(WanMgr_Policy_Controller_t* pWanPolicyCtrl)
{
    ANSC_STATUS retStatus = ANSC_STATUS_FAILURE;

    if(pWanPolicyCtrl != NULL)
    {
        pWanPolicyCtrl->WanEnable = FALSE;
        pWanPolicyCtrl->activeInterfaceIdx = -1;
        pWanPolicyCtrl->selSecondaryInterfaceIdx = -1;
        pWanPolicyCtrl->pWanActiveIfaceData = NULL;
        memset(&(pWanPolicyCtrl->SelectionTimeOutStart), 0, sizeof(struct timespec));
        memset(&(pWanPolicyCtrl->SelectionTimeOutEnd), 0, sizeof(struct timespec));
        pWanPolicyCtrl->InterfaceSelectionTimeOut = 0;

        retStatus = ANSC_STATUS_SUCCESS;
    }

   return retStatus;
}
