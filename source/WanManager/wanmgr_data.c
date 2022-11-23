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


#include "wanmgr_data.h"


/******** WAN MGR DATABASE ********/
WANMGR_DATA_ST gWanMgrDataBase;


/******** WANMGR CONFIG FUNCTIONS ********/
WanMgr_Config_Data_t* WanMgr_GetConfigData_locked(void)
{
    WanMgr_Config_Data_t* pWanConfigData = &(gWanMgrDataBase.Config);

    //lock
    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        return pWanConfigData;
    }

    return NULL;
}

void WanMgrDml_GetConfigData_release(WanMgr_Config_Data_t* pWanConfigData)
{
    if(pWanConfigData != NULL)
    {
        pthread_mutex_unlock (&gWanMgrDataBase.gDataMutex);
    }
}

void WanMgr_SetConfigData_Default(DML_WANMGR_CONFIG* pWanDmlConfig)
{
    if(pWanDmlConfig != NULL)
    {
        pWanDmlConfig->Enable = TRUE;
        pWanDmlConfig->Policy = FIXED_MODE;
        pWanDmlConfig->ResetActiveInterface = FALSE;
        pWanDmlConfig->AllowRemoteInterfaces = FALSE;
        pWanDmlConfig->PolicyChanged = FALSE;
        memset(pWanDmlConfig->InterfaceAvailableStatus, 0, BUFLEN_64);
        memset(pWanDmlConfig->InterfaceActiveStatus, 0, BUFLEN_64);

        CcspTraceInfo(("%s %d: Setting GATEWAY Mode\n", __FUNCTION__, __LINE__));
        pWanDmlConfig->DeviceNwMode = GATEWAY_MODE;
        pWanDmlConfig->DeviceNwModeChanged = FALSE;
    }
}

/******** WANMGR IFACE CTRL FUNCTIONS ********/
void WanMgr_SetIfaceGroup_Default(WanMgr_IfaceGroup_t *pWanIfacegroup)
{

    if(pWanIfacegroup != NULL)
    {
        pWanIfacegroup->ulTotalNumbWanIfaceGroup = MAX_INTERFACE_GROUP;
        for(int i = 0; i < MAX_INTERFACE_GROUP; i++)
	{
            pWanIfacegroup->Group[i].ThreadId = 0;
            pWanIfacegroup->Group[i].GroupState = 0;
            pWanIfacegroup->Group[i].Interfaces = 0;
            pWanIfacegroup->Group[i].SelectedInterface = 0;
            pWanIfacegroup->Group[i].SelectedIfaceStatus = 0;
            pWanIfacegroup->Group[i].GroupIfaceListChanged = 0;
	}
    }
}


/******** WANMGR IFACE CTRL FUNCTIONS ********/
void WanMgr_SetIfaceCtrl_Default(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl)
{
    if(pWanIfaceCtrl != NULL)
    {
        pWanIfaceCtrl->ulTotalNumbWanInterfaces = 0;
        pWanIfaceCtrl->pIface = NULL;
    }
}


void WanMgr_IfaceCtrl_Delete(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl)
{

    if(pWanIfaceCtrl != NULL)
    {
        pWanIfaceCtrl->ulTotalNumbWanInterfaces = 0;
        if(pWanIfaceCtrl->pIface != NULL)
        {
            AnscFreeMemory(pWanIfaceCtrl->pIface);
            pWanIfaceCtrl->pIface = NULL;
        }
    }
}

/******** WANMGR IFACE FUNCTIONS ********/

ANSC_STATUS WanMgr_WanDataInit(void)
{
    ANSC_STATUS retStatus = ANSC_STATUS_FAILURE;
    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        retStatus = WanMgr_WanIfaceConfInit(pWanIfaceCtrl);


        WanMgrDml_GetIfaceData_release(NULL);
    }
    return retStatus;
}

UINT WanMgr_IfaceData_GetTotalWanIface(void)
{
   UINT TotalIfaces = 0;
   if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
   {
       if(&(gWanMgrDataBase.IfaceCtrl) != NULL)
       {
           TotalIfaces = gWanMgrDataBase.IfaceCtrl.ulTotalNumbWanInterfaces;
       }
       WanMgrDml_GetIfaceData_release(NULL);
   }
   return TotalIfaces;
}

WanMgr_Iface_Data_t* WanMgr_GetIfaceData_locked(UINT iface_index)
{
    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(iface_index < pWanIfaceCtrl->ulTotalNumbWanInterfaces)
        {
            if(pWanIfaceCtrl->pIface != NULL)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[iface_index]);
                return pWanIfaceData;
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }

    return NULL;
}

WanMgr_Iface_Data_t* WanMgr_GetIfaceDataByName_locked(char* iface_name)
{
   UINT idx;

    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(pWanIfaceCtrl->pIface != NULL)
        {
            for(idx = 0; idx < pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[idx]);

                if(!strcmp(iface_name, pWanIfaceData->data.Wan.Name))
                {
                    return pWanIfaceData;
                }
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }

    return NULL;
}

void WanMgr_GetIfaceAliasNameByIndex(UINT iface_index, char *AliasName)
{
    if(pthread_mutex_lock(&(gWanMgrDataBase.gDataMutex)) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(iface_index < pWanIfaceCtrl->ulTotalNumbWanInterfaces)
        {
            if(pWanIfaceCtrl->pIface != NULL)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[iface_index]);
                strcpy(AliasName,pWanIfaceData->data.AliasName);
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }
    CcspTraceInfo(("%s-%d : AliasName[%s]\n", __FUNCTION__, __LINE__, AliasName));
}

UINT WanMgr_GetIfaceIndexByAliasName(char* AliasName)
{
   UINT index = 0;

    if (strlen(AliasName) <= 0)
    {
       return index;
    }

    if(pthread_mutex_lock(&(gWanMgrDataBase.gDataMutex)) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(pWanIfaceCtrl->pIface != NULL)
        {
            for(int idx = 0; idx < pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[idx]);
                if((strlen(pWanIfaceData->data.AliasName) > 0) && (strstr(AliasName, pWanIfaceData->data.AliasName) != NULL))
                {
                    index = idx + 1;
                }
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }

    CcspTraceInfo(("%s-%d : Alias(%s) index[%d]\n", __FUNCTION__, __LINE__, AliasName,index));
    return index;
}


void WanMgrDml_GetIfaceData_release(WanMgr_Iface_Data_t* pWanIfaceData)
{
    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
    if(pWanIfaceCtrl != NULL)
    {
        pthread_mutex_unlock (&gWanMgrDataBase.gDataMutex);
    }
}

WANMGR_IFACE_GROUP* WanMgr_GetIfaceGroup_locked(UINT iface_index)
{
    if(pthread_mutex_lock(&(gWanMgrDataBase.gDataMutex)) == 0)
    {
        WanMgr_IfaceGroup_t* pWanIfaceGroup = &(gWanMgrDataBase.IfaceGroup);
        if(iface_index < pWanIfaceGroup->ulTotalNumbWanIfaceGroup)
        {
            if(pWanIfaceGroup->Group != NULL)
            {
                WANMGR_IFACE_GROUP* pWanIfacegroup = &(pWanIfaceGroup->Group[iface_index]);
                return pWanIfacegroup;
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }

    return NULL;
}

void WanMgrDml_GetIfaceGroup_release(void)
{
    WanMgr_IfaceGroup_t* pWanIfaceGroup = &(gWanMgrDataBase.IfaceGroup);
    if(pWanIfaceGroup != NULL)
    {
        pthread_mutex_unlock (&gWanMgrDataBase.gDataMutex);
    }
}

void WanMgr_IfaceData_Init(WanMgr_Iface_Data_t* pIfaceData, UINT iface_index)
{
    if(pIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanDmlIface = &(pIfaceData->data);

        pWanDmlIface->MonitorOperStatus = FALSE;
        pWanDmlIface->WanConfigEnabled = FALSE;
        pWanDmlIface->CustomConfigEnable = FALSE;
        memset(pWanDmlIface->CustomConfigPath,0,sizeof(pWanDmlIface->CustomConfigPath));
        memset(pWanDmlIface->RemoteCPEMac, 0, sizeof(pWanDmlIface->RemoteCPEMac));
        pWanDmlIface->Wan.OperationalStatus = WAN_OPERSTATUS_UNKNOWN;
        pWanDmlIface->uiIfaceIdx = iface_index;
        pWanDmlIface->uiInstanceNumber = iface_index+1;
        memset(pWanDmlIface->Name, 0, 64);
        memset(pWanDmlIface->DisplayName, 0, 64);
        memset(pWanDmlIface->AliasName, 0, 64);
        memset(pWanDmlIface->Phy.Path, 0, 64);
        pWanDmlIface->Phy.Status = WAN_IFACE_PHY_STATUS_DOWN;
        memset(pWanDmlIface->Wan.Name, 0, 64);
        pWanDmlIface->Wan.Enable = FALSE;
        pWanDmlIface->Wan.Priority = -1;
        pWanDmlIface->Wan.Type = WAN_IFACE_TYPE_UNCONFIGURED;
        pWanDmlIface->Wan.SelectionTimeout = 0;
        pWanDmlIface->Wan.EnableMAPT = FALSE;
        pWanDmlIface->Wan.EnableDSLite = FALSE;
        pWanDmlIface->Wan.EnableIPoE = FALSE;
        pWanDmlIface->Wan.EnableDHCP = TRUE;    // DHCP is enabled by default
        pWanDmlIface->Wan.RefreshDHCP = FALSE;        // RefreshDHCP is set when there is a change in EnableDHCP
        pWanDmlIface->Wan.IfaceType = LOCAL_IFACE;    // InterfaceType is Local by default
        pWanDmlIface->Wan.ActiveLink = FALSE;
	pWanDmlIface->SelectionStatus = WAN_IFACE_NOT_SELECTED;
        pWanDmlIface->Wan.Status = WAN_IFACE_STATUS_DISABLED;
        pWanDmlIface->Wan.RemoteStatus = WAN_IFACE_STATUS_DISABLED;
        pWanDmlIface->Wan.LinkStatus = WAN_IFACE_LINKSTATUS_DOWN;
        pWanDmlIface->Wan.Refresh = FALSE;
        pWanDmlIface->Wan.RebootOnConfiguration = FALSE;
	pWanDmlIface->Wan.Group = 1;
        memset(pWanDmlIface->IP.Path, 0, 64);
        pWanDmlIface->IP.Ipv4Status = WAN_IFACE_IPV4_STATE_DOWN;
        pWanDmlIface->IP.Ipv6Status = WAN_IFACE_IPV6_STATE_DOWN;
        pWanDmlIface->IP.Ipv4Changed = FALSE;
        pWanDmlIface->IP.Ipv6Changed = FALSE;
#ifdef FEATURE_IPOE_HEALTH_CHECK
        pWanDmlIface->IP.Ipv4Renewed = FALSE;
        pWanDmlIface->IP.Ipv6Renewed = FALSE;
#endif
        memset(&(pWanDmlIface->IP.Ipv4Data), 0, sizeof(WANMGR_IPV4_DATA));
        memset(&(pWanDmlIface->IP.Ipv6Data), 0, sizeof(WANMGR_IPV6_DATA));
        pWanDmlIface->IP.pIpcIpv4Data = NULL;
        pWanDmlIface->IP.pIpcIpv6Data = NULL;
        pWanDmlIface->MAP.MaptStatus = WAN_IFACE_MAPT_STATE_DOWN;
        memset(pWanDmlIface->MAP.Path, 0, 64);
        pWanDmlIface->MAP.MaptChanged = FALSE;
        memset(pWanDmlIface->DSLite.Path, 0, 64);
        pWanDmlIface->DSLite.Status = WAN_IFACE_DSLITE_STATE_DOWN;
        pWanDmlIface->DSLite.Changed = FALSE;
        pWanDmlIface->PPP.LinkStatus = WAN_IFACE_PPP_LINK_STATUS_DOWN;
        pWanDmlIface->PPP.LCPStatus = WAN_IFACE_LCP_STATUS_DOWN;
        pWanDmlIface->PPP.IPCPStatus = WAN_IFACE_IPCP_STATUS_DOWN;
        pWanDmlIface->PPP.IPV6CPStatus = WAN_IFACE_IPV6CP_STATUS_DOWN;
        pWanDmlIface->InterfaceScanStatus = WAN_IFACE_STATUS_NOT_SCANNED;

        pWanDmlIface->Sub.PhyStatusSub = 0;
	pWanDmlIface->Sub.WanStatusSub = 0;
	pWanDmlIface->Sub.WanLinkStatusSub = 0;
    }
}


void WanMgr_Remote_IfaceData_Init(WanMgr_Iface_Data_t* pIfaceData, UINT iface_index)
{
    static int remotePriority = 100;
    CcspTraceInfo(("%s %d - Enter \n", __FUNCTION__, __LINE__));
    if(pIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanDmlIface = &(pIfaceData->data);

        pWanDmlIface->MonitorOperStatus = FALSE;
        pWanDmlIface->WanConfigEnabled = FALSE;
        pWanDmlIface->CustomConfigEnable = FALSE;
        memset(pWanDmlIface->RemoteCPEMac, 0, sizeof(pWanDmlIface->RemoteCPEMac));
        memset(pWanDmlIface->CustomConfigPath,0,sizeof(pWanDmlIface->CustomConfigPath));
        pWanDmlIface->Wan.OperationalStatus = WAN_OPERSTATUS_UNKNOWN;
        pWanDmlIface->uiIfaceIdx = iface_index;
        pWanDmlIface->uiInstanceNumber = iface_index+1;
        memset(pWanDmlIface->Name, 0, 64);
        strcpy(pWanDmlIface->Name, REMOTE_INTERFACE_NAME); // Remote name by default
        memset(pWanDmlIface->DisplayName, 0, 64);
        memset(pWanDmlIface->AliasName, 0, 64);
        memset(pWanDmlIface->Phy.Path, 0, 64);
        pWanDmlIface->Phy.Status = WAN_IFACE_PHY_STATUS_DOWN;
        memset(pWanDmlIface->Wan.Name, 0, 64);
        pWanDmlIface->Wan.Enable = TRUE; // Remote wan Enable by default
        pWanDmlIface->Wan.Priority = remotePriority;
        pWanDmlIface->Wan.Type = WAN_IFACE_TYPE_UNCONFIGURED;
        pWanDmlIface->Wan.SelectionTimeout = 0;
        pWanDmlIface->Wan.EnableMAPT = FALSE;
        pWanDmlIface->Wan.EnableDSLite = FALSE;
        pWanDmlIface->Wan.EnableIPoE = FALSE;
        pWanDmlIface->Wan.EnableDHCP = TRUE;    // DHCP is enabled by default
        pWanDmlIface->Wan.RefreshDHCP = FALSE;        // RefreshDHCP is set when there is a change in EnableDHCP
        pWanDmlIface->Wan.IfaceType = REMOTE_IFACE;    // InterfaceType is Remote by default
        pWanDmlIface->Wan.ActiveLink = FALSE;
        pWanDmlIface->SelectionStatus = WAN_IFACE_NOT_SELECTED;
        pWanDmlIface->Wan.Status = WAN_IFACE_STATUS_DISABLED;
        pWanDmlIface->Wan.LinkStatus = WAN_IFACE_LINKSTATUS_DOWN;
        pWanDmlIface->Wan.Refresh = FALSE;
        pWanDmlIface->Wan.RebootOnConfiguration = FALSE;
        pWanDmlIface->Wan.Group = 2;
        memset(pWanDmlIface->IP.Path, 0, 64);
        pWanDmlIface->IP.Ipv4Status = WAN_IFACE_IPV4_STATE_DOWN;
        pWanDmlIface->IP.Ipv6Status = WAN_IFACE_IPV6_STATE_DOWN;
        pWanDmlIface->IP.Ipv4Changed = FALSE;
        pWanDmlIface->IP.Ipv6Changed = FALSE;
#ifdef FEATURE_IPOE_HEALTH_CHECK
        pWanDmlIface->IP.Ipv4Renewed = FALSE;
        pWanDmlIface->IP.Ipv6Renewed = FALSE;
#endif
        memset(&(pWanDmlIface->IP.Ipv4Data), 0, sizeof(WANMGR_IPV4_DATA));
        memset(&(pWanDmlIface->IP.Ipv6Data), 0, sizeof(WANMGR_IPV6_DATA));
        pWanDmlIface->IP.pIpcIpv4Data = NULL;
        pWanDmlIface->IP.pIpcIpv6Data = NULL;
        pWanDmlIface->MAP.MaptStatus = WAN_IFACE_MAPT_STATE_DOWN;
        memset(pWanDmlIface->MAP.Path, 0, 64);
        pWanDmlIface->MAP.MaptChanged = FALSE;
        memset(pWanDmlIface->DSLite.Path, 0, 64);
        pWanDmlIface->DSLite.Status = WAN_IFACE_DSLITE_STATE_DOWN;
        pWanDmlIface->DSLite.Changed = FALSE;
        pWanDmlIface->PPP.LinkStatus = WAN_IFACE_PPP_LINK_STATUS_DOWN;
        pWanDmlIface->PPP.LCPStatus = WAN_IFACE_LCP_STATUS_DOWN;
        pWanDmlIface->PPP.IPCPStatus = WAN_IFACE_IPCP_STATUS_DOWN;
        pWanDmlIface->PPP.IPV6CPStatus = WAN_IFACE_IPV6CP_STATUS_DOWN;
        remotePriority++;
    }
}

/******** WAN MGR DATA FUNCTIONS ********/
void WanMgr_Data_Init(void)
{
    WANMGR_DATA_ST*         pWanMgrData = &gWanMgrDataBase;
    pthread_mutexattr_t     muttex_attr;

    //Initialise mutex attributes
    pthread_mutexattr_init(&muttex_attr);
    pthread_mutexattr_settype(&muttex_attr, PTHREAD_MUTEX_RECURSIVE);

    /*** WAN CONFIG ***/
    WanMgr_SetConfigData_Default(&(pWanMgrData->Config.data));

    /*** WAN IFACE ***/
    WanMgr_SetIfaceCtrl_Default(&(pWanMgrData->IfaceCtrl));

    WanMgr_SetIfaceGroup_Default(&(pWanMgrData->IfaceGroup));

    pthread_mutex_init(&(gWanMgrDataBase.gDataMutex), &(muttex_attr));
}

ANSC_STATUS WanMgr_Data_Delete(void)
{
    ANSC_STATUS         result  = ANSC_STATUS_FAILURE;
    WANMGR_DATA_ST*     pWanMgrData = &gWanMgrDataBase;

    /*** WAN IFACECTRL ***/
    WanMgr_IfaceCtrl_Delete(&(pWanMgrData->IfaceCtrl));

    pthread_mutex_destroy(&(gWanMgrDataBase.gDataMutex));
    return result;
}

ANSC_STATUS WanMgr_Remote_IfaceData_configure(char *remoteCPEMac, int  *iface_index)
{
    ANSC_STATUS ret = ANSC_STATUS_FAILURE;
    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        WanMgr_Iface_Data_t*  pIfaceData = NULL;
        if (pWanIfaceCtrl->ulTotalNumbWanInterfaces < MAX_WAN_INTERFACE_ENTRY)
        {
            pIfaceData = &(pWanIfaceCtrl->pIface[pWanIfaceCtrl->ulTotalNumbWanInterfaces]);
            WanMgr_Remote_IfaceData_Init(pIfaceData, pWanIfaceCtrl->ulTotalNumbWanInterfaces);
            DML_WAN_IFACE* pWanDmlIface = &(pIfaceData->data);
            strcpy(pWanDmlIface->RemoteCPEMac, remoteCPEMac);
            *iface_index = pWanIfaceCtrl->ulTotalNumbWanInterfaces;
            pWanIfaceCtrl->ulTotalNumbWanInterfaces = pWanIfaceCtrl->ulTotalNumbWanInterfaces + 1;
            gWanMgrDataBase.IfaceCtrl.update = 0;
            WanMgrDml_GetIfaceData_release(NULL);
            ret = ANSC_STATUS_SUCCESS;
        }
        else
        {
            CcspTraceInfo(("%s %d - Wan Interface Entries has reached its limit = [%d]\n", __FUNCTION__, __LINE__, MAX_WAN_INTERFACE_ENTRY));
        }
    }
    return ret;
}

ANSC_STATUS WanMgr_UpdatePrevData ()
{
    if (access(WANMGR_RESTART_INFO_FILE, F_OK) != 0)
    {
        return ANSC_STATUS_FAILURE;
    }

    UINT uiLoopCount;
    int uiInterfaceIdx = -1;

    UINT TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    for( uiLoopCount = 0; uiLoopCount < TotalIfaces; uiLoopCount++ )
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
#if defined (_XB6_PRODUCT_REQ_) || defined (_CBR2_PRODUCT_REQ_)
            WanMgr_RestartUpdateCfg_Bool (WAN_ENABLE_CUSTOM_CONFIG_PARAM_NAME, uiLoopCount, &pWanIfaceData->CustomConfigEnable);
            WanMgr_RestartUpdateCfg_Bool (WAN_CONFIGURE_WAN_ENABLE_PARAM_NAME, uiLoopCount, &pWanIfaceData->WanConfigEnabled);
            WanMgr_RestartUpdateCfg_Bool (WAN_ENABLE_OPER_STATUS_MONITOR_PARAM_NAME, uiLoopCount, &pWanIfaceData->MonitorOperStatus);
            WanMgr_RestartUpdateCfg (WAN_CUSTOM_CONFIG_PATH_PARAM_NAME, uiLoopCount, pWanIfaceData->CustomConfigPath, sizeof(pWanIfaceData->CustomConfigPath));
            WanMgr_RestartUpdateCfg (WAN_NAME_PARAM_NAME, uiLoopCount, pWanIfaceData->Wan.Name, sizeof(pWanIfaceData->Wan.Name));
#endif            
            WanMgr_RestartUpdateCfg (WAN_PHY_PATH_PARAM_NAME, uiLoopCount, pWanIfaceData->Phy.Path, sizeof(pWanIfaceData->Phy.Path));
            if(pWanIfaceData->PPP.Enable == TRUE)
            {
                CcspTraceInfo(("%s %d - Update PPP path\n", __FUNCTION__, __LINE__));
                WanMgr_RestartUpdateCfg (WAN_PPP_PATH_PARAM_NAME, uiLoopCount, pWanIfaceData->PPP.Path, sizeof(pWanIfaceData->PPP.Path));
            }
            #if !defined(AUTOWAN_ENABLE)
            WanMgr_RestartGetPhyStatus(pWanIfaceData);
            #endif
            
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }   
    #ifdef RBUS_BUILD_FLAG_ENABLE
    WanMgr_RestartUpdateRemoteIface();
    #endif
    return ANSC_STATUS_SUCCESS;

}
