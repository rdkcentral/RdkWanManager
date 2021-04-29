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
static WANMGR_DATA_ST gWanMgrDataBase;



/******** WANMGR CONFIG FUNCTIONS ********/
WanMgr_Config_Data_t* WanMgr_GetConfigData_locked(void)
{
    WanMgr_Config_Data_t* pWanConfigData = &(gWanMgrDataBase.Config);

    //lock
    if(pthread_mutex_lock(&(pWanConfigData->mDataMutex)) == 0)
    {
        return pWanConfigData;
    }

    return NULL;
}

void WanMgrDml_GetConfigData_release(WanMgr_Config_Data_t* pWanConfigData)
{
    if(pWanConfigData != NULL)
    {
        pthread_mutex_unlock (&(pWanConfigData->mDataMutex));
    }
}

void WanMgr_SetConfigData_Default(DML_WANMGR_CONFIG* pWanDmlConfig)
{
    if(pWanDmlConfig != NULL)
    {
        pWanDmlConfig->Enable = TRUE;
        pWanDmlConfig->Policy = FIXED_MODE;
        pWanDmlConfig->IdleTimeout = 0;
    }
}



/******** WANMGR IFACE CTRL FUNCTIONS ********/
WanMgr_IfaceCtrl_Data_t* WanMgr_GetIfaceCtrl_locked(void)
{
    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);

    //lock
    if(pthread_mutex_lock(&(pWanIfaceCtrl->mDataMutex)) == 0)
    {
        return pWanIfaceCtrl;
    }

    return NULL;
}

void WanMgrDml_GetIfaceCtrl_release(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl)
{
    if(pWanIfaceCtrl != NULL)
    {
        pthread_mutex_unlock (&(pWanIfaceCtrl->mDataMutex));
    }
}


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
WanMgr_Iface_Data_t* WanMgr_GetIfaceData_locked(UINT iface_index)
{
    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = WanMgr_GetIfaceCtrl_locked();
    if(pWanIfaceCtrl != NULL)
    {
        if(iface_index < pWanIfaceCtrl->ulTotalNumbWanInterfaces)
        {
            if(pWanIfaceCtrl->pIface != NULL)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[iface_index]);
                return pWanIfaceData;
            }
        }

        WanMgrDml_GetIfaceCtrl_release(pWanIfaceCtrl);
    }

    return NULL;
}

WanMgr_Iface_Data_t* WanMgr_GetIfaceDataByName_locked(char* iface_name)
{
   UINT idx;

    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = WanMgr_GetIfaceCtrl_locked();
    if(pWanIfaceCtrl != NULL)
    {
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

        WanMgrDml_GetIfaceCtrl_release(pWanIfaceCtrl);
    }

    return NULL;
}

void WanMgrDml_GetIfaceData_release(WanMgr_Iface_Data_t* pWanIfaceData)
{
    WanMgrDml_GetIfaceCtrl_release(&(gWanMgrDataBase.IfaceCtrl));
}

void WanMgr_IfaceData_Init(WanMgr_Iface_Data_t* pIfaceData, UINT iface_index)
{
    if(pIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanDmlIface = &(pIfaceData->data);

        pWanDmlIface->uiIfaceIdx = iface_index;
        pWanDmlIface->uiInstanceNumber = iface_index+1;
        memset(pWanDmlIface->Name, 0, 64);
        memset(pWanDmlIface->DisplayName, 0, 64);
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
        pWanDmlIface->Wan.ActiveLink = FALSE;
        pWanDmlIface->Wan.Status = WAN_IFACE_STATUS_DISABLED;
        pWanDmlIface->Wan.LinkStatus = WAN_IFACE_LINKSTATUS_DOWN;
        pWanDmlIface->Wan.Refresh = FALSE;
        pWanDmlIface->Wan.Validation.DiscoverOffer = FALSE;
        pWanDmlIface->Wan.Validation.SolicitAdvertise = FALSE;
        pWanDmlIface->Wan.Validation.RS_RA = FALSE;
        pWanDmlIface->Wan.Validation.PadiPado = FALSE;
        pWanDmlIface->DynamicTrigger.Enable = FALSE;
        pWanDmlIface->DynamicTrigger.Delay = 0;
        memset(pWanDmlIface->IP.Path, 0, 64);
        pWanDmlIface->IP.Ipv4Status = WAN_IFACE_IPV4_STATE_DOWN;
        pWanDmlIface->IP.Ipv6Status = WAN_IFACE_IPV6_STATE_DOWN;
        pWanDmlIface->IP.Ipv4Changed = FALSE;
        pWanDmlIface->IP.Ipv6Changed = FALSE;
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
    pthread_mutex_init(&(pWanMgrData->Config.mDataMutex), &(muttex_attr));

    /*** WAN IFACE ***/
    WanMgr_SetIfaceCtrl_Default(&(pWanMgrData->IfaceCtrl));
    pthread_mutex_init(&(pWanMgrData->IfaceCtrl.mDataMutex), &(muttex_attr));
}




ANSC_STATUS WanMgr_Data_Delete(void)
{
    ANSC_STATUS         result  = ANSC_STATUS_FAILURE;
    WANMGR_DATA_ST*     pWanMgrData = &gWanMgrDataBase;
    int idx;

    /*** WAN CONFIG ***/
    pthread_mutex_destroy(&(pWanMgrData->Config.mDataMutex));

    /*** WAN IFACE ***/

    /*** WAN IFACECTRL ***/
    WanMgr_IfaceCtrl_Delete(&(pWanMgrData->IfaceCtrl));
    pthread_mutex_destroy(&(pWanMgrData->IfaceCtrl.mDataMutex));

    return result;
}


