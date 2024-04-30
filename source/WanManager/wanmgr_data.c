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


#include "wanmgr_data.h"
#include "wanmgr_rdkbus_apis.h"

#if defined(WAN_MANAGER_UNIFICATION_ENABLED) && (defined (_XB6_PRODUCT_REQ_) || defined (_CBR2_PRODUCT_REQ_))
extern ANSC_STATUS WanMgr_CheckAndResetV2PSMEntries(UINT IfaceCount);
#endif

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
        pWanDmlConfig->ResetFailOverScan = FALSE;
        pWanDmlConfig->AllowRemoteInterfaces = FALSE;
        memset(pWanDmlConfig->InterfaceAvailableStatus, 0, BUFLEN_64);
        memset(pWanDmlConfig->InterfaceActiveStatus, 0, BUFLEN_64);
        memset(pWanDmlConfig->CurrentStatus, 0, sizeof(pWanDmlConfig->CurrentStatus));
        strncpy(pWanDmlConfig->CurrentStatus, "Down", sizeof(pWanDmlConfig->CurrentStatus) -1);
        memset(pWanDmlConfig->CurrentActiveInterface, 0, BUFLEN_64);

        CcspTraceInfo(("%s %d: Setting GATEWAY Mode\n", __FUNCTION__, __LINE__));
        pWanDmlConfig->DeviceNwMode = GATEWAY_MODE;
        pWanDmlConfig->DeviceNwModeChanged = FALSE;
        pWanDmlConfig->BootToWanUp = FALSE;

        /*In Modem/Extender Mode, CurrentActiveInterface should be always Mesh Interface Name*/
#if defined (RDKB_EXTENDER_ENABLED)
        char buf[BUFLEN_16] = {0};
        if( 0 == syscfg_get(NULL, SYSCFG_DEVICE_NETWORKING_MODE, buf, sizeof(buf)) )
        {   //1-Extender Mode 0-Gateway Mode
            if (atoi(buf) == (MODEM_MODE-1))
            {
                strncpy(pWanDmlConfig->CurrentActiveInterface, MESH_IFNAME, sizeof(MESH_IFNAME));
            }
        }
#endif

    }
}

/******** WANMGR IFACE CTRL FUNCTIONS ********/
void WanMgr_SetIfaceGroup_Default(WanMgr_IfaceGroup_t *pWanIfacegroup)
{

    if(pWanIfacegroup != NULL)
    {
        pWanIfacegroup->ulTotalNumbWanIfaceGroup = MAX_INTERFACE_GROUP;
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

ANSC_STATUS WanMgr_Group_Configure()
{
    ANSC_STATUS ret = ANSC_STATUS_FAILURE;
    CcspTraceInfo(("%s %d Initialize Wan Group Conf \n", __FUNCTION__, __LINE__));
    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceGroup_t *pWanIfacegroup = &(gWanMgrDataBase.IfaceGroup);

        if(pWanIfacegroup != NULL)
        {
            DmlGetTotalNoOfGroups(&(pWanIfacegroup->ulTotalNumbWanIfaceGroup));
            CcspTraceInfo(("%s %d - Total no of Groups %d\n",__FUNCTION__,__LINE__,pWanIfacegroup->ulTotalNumbWanIfaceGroup));

            pWanIfacegroup->Group = (WANMGR_IFACE_GROUP *) AnscAllocateMemory( sizeof(WANMGR_IFACE_GROUP) * pWanIfacegroup->ulTotalNumbWanIfaceGroup);
            if(pWanIfacegroup->Group == NULL)
            {
                CcspTraceError(("%s %d: AnscAllocateMemory failed \n", __FUNCTION__, __LINE__));
                return ret;
            }
            for(int i = 0; i < pWanIfacegroup->ulTotalNumbWanIfaceGroup; i++)
            {
                pWanIfacegroup->Group[i].groupIdx = i;
                pWanIfacegroup->Group[i].ThreadId = 0;
                pWanIfacegroup->Group[i].State = STATE_GROUP_STOPPED;
                pWanIfacegroup->Group[i].InterfaceAvailable = 0;
                pWanIfacegroup->Group[i].SelectedInterface = 0;
                pWanIfacegroup->Group[i].SelectionTimeOut = 0;
                pWanIfacegroup->Group[i].ActivationCount = 0;
                pWanIfacegroup->Group[i].ConfigChanged = FALSE;
                pWanIfacegroup->Group[i].PersistSelectedIface = FALSE;
                pWanIfacegroup->Group[i].ResetSelectedInterface = FALSE;
                pWanIfacegroup->Group[i].InitialScanComplete = FALSE;
                pWanIfacegroup->Group[i].Policy = AUTOWAN_MODE;
                WanMgr_Read_GroupConf_FromPSM(&(pWanIfacegroup->Group[i]), i);
                CcspTraceInfo(("%s %d Group[%d] Policy : %d \n", __FUNCTION__, __LINE__, i, pWanIfacegroup->Group[i].Policy));
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }
    return ANSC_STATUS_SUCCESS;
}


/******** WANMGR IFACE FUNCTIONS ********/
ANSC_STATUS WanMgr_VirtIfConfVLAN(DML_VIRTUAL_IFACE *p_VirtIf, UINT Ifid)
{
    if(p_VirtIf ==NULL)
    {
        CcspTraceError(("%s %d Invalid memory \n", __FUNCTION__, __LINE__));
    }

    for(int i =0; i < p_VirtIf->VLAN.NoOfInterfaceEntries; i++)
    {
        DML_VLAN_IFACE_TABLE* p_VlanIf = (DML_VLAN_IFACE_TABLE *) AnscAllocateMemory( sizeof(DML_VLAN_IFACE_TABLE));
        if(p_VlanIf == NULL)
        {
            CcspTraceError(("%s %d: AnscAllocateMemory failed \n", __FUNCTION__, __LINE__));
            return ANSC_STATUS_FAILURE;
        }
        p_VlanIf->Index = i;
        p_VlanIf->VirIfIdx = p_VirtIf->VirIfIdx;
        p_VlanIf->baseIfIdx = p_VirtIf->baseIfIdx;

        memset(p_VlanIf->Interface,0, sizeof(p_VlanIf->Interface));

        char param_value[256]={0};
        char param_name[512]={0};
        int retPsmGet = CCSP_SUCCESS;
        _ansc_sprintf(param_name, PSM_WANMANAGER_IF_VIRIF_VLAN_INTERFACE_ENTRY, (Ifid+1), (p_VirtIf->VirIfIdx +1),(i+1));
        retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
        AnscCopyString(p_VlanIf->Interface, param_value);

        CcspTraceInfo(("%s %d Adding Vlan Interface entry %d \n", __FUNCTION__, __LINE__, p_VlanIf->Index));
        WanMgr_AddVirtVlanIfToList(&(p_VirtIf->VLAN.InterfaceList), p_VlanIf);
    }

    for(int i =0; i < p_VirtIf->VLAN.NoOfMarkingEntries; i++)
    {
        DML_VIRTIF_MARKING* p_Marking = (DML_VIRTIF_MARKING *) AnscAllocateMemory( sizeof(DML_VIRTIF_MARKING));
        if(p_Marking == NULL)
        {
            CcspTraceError(("%s %d: AnscAllocateMemory failed \n", __FUNCTION__, __LINE__));
            return ANSC_STATUS_FAILURE;
        }
        p_Marking->VirtMarkingInstanceNumber = i;
        p_Marking->VirIfIdx = p_VirtIf->VirIfIdx;
        p_Marking->baseIfIdx = p_VirtIf->baseIfIdx;
        p_Marking->Entry = 0;

        char param_value[256]={0};
        char param_name[512]={0};
        int retPsmGet = CCSP_SUCCESS;
        _ansc_sprintf(param_name, PSM_WANMANAGER_IF_VIRIF_VLAN_MARKING_ENTRY, (Ifid+1), (p_VirtIf->VirIfIdx +1),(i+1));
        retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
        if (retPsmGet == CCSP_SUCCESS)
        {
            _ansc_sscanf(param_value, "%d", &(p_Marking->Entry));
        }

        CcspTraceInfo(("%s %d Adding Marking entry %d \n", __FUNCTION__, __LINE__, p_Marking->VirtMarkingInstanceNumber));
        WanMgr_AddVirtMarkingToList(&(p_VirtIf->VLAN.VirtMarking),p_Marking);
    }
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS WanMgr_WanIfaceConfInit(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl)
{
    CcspTraceInfo(("%s %d Initialize Wan Iface Conf \n", __FUNCTION__, __LINE__));
    if(pWanIfaceCtrl != NULL)
    {
        ANSC_STATUS result;
        UINT        uiTotalIfaces;
        UINT        idx;

        result = DmlGetTotalNoOfWanInterfaces(&uiTotalIfaces);
        if(result == ANSC_STATUS_FAILURE) {
            return ANSC_STATUS_FAILURE;
        }

        pWanIfaceCtrl->pIface = (WanMgr_Iface_Data_t*) AnscAllocateMemory( sizeof(WanMgr_Iface_Data_t) * MAX_WAN_INTERFACE_ENTRY);
        if( NULL == pWanIfaceCtrl->pIface )
        {
            return ANSC_STATUS_FAILURE;
        }

        pWanIfaceCtrl->ulTotalNumbWanInterfaces = uiTotalIfaces;
#if defined(WAN_MANAGER_UNIFICATION_ENABLED) && (defined (_XB6_PRODUCT_REQ_) || defined (_CBR2_PRODUCT_REQ_))
        WanMgr_CheckAndResetV2PSMEntries(uiTotalIfaces);
#endif

        //Memset all memory
        memset( pWanIfaceCtrl->pIface, 0, ( sizeof(WanMgr_Iface_Data_t) * MAX_WAN_INTERFACE_ENTRY ) );

        //Get static interface configuration from PSM data store
        for( idx = 0 ; idx < uiTotalIfaces ; idx++ )
        {
            WanMgr_Iface_Data_t*  pIfaceData  = &(pWanIfaceCtrl->pIface[idx]);
            WanMgr_IfaceData_Init(pIfaceData, idx);
            get_Wan_Interface_ParametersFromPSM((idx+1), &(pIfaceData->data));

            CcspTraceInfo(("%s %d No Of Virtual Interfaces %d \n", __FUNCTION__, __LINE__, pIfaceData->data.NoOfVirtIfs));

            for(int i=0; i< pIfaceData->data.NoOfVirtIfs; i++)
            {
                DML_VIRTUAL_IFACE* p_VirtIf = (DML_VIRTUAL_IFACE *) AnscAllocateMemory( sizeof(DML_VIRTUAL_IFACE) );
                if(p_VirtIf == NULL)
                {
                    CcspTraceError(("%s %d: AnscAllocateMemory failed \n", __FUNCTION__, __LINE__));
                    return ANSC_STATUS_FAILURE;
                }
                WanMgr_VirtIface_Init(p_VirtIf, i);
                p_VirtIf->baseIfIdx = idx; //Add base interface index 
                get_Virtual_Interface_FromPSM((idx+1), i , p_VirtIf);
                CcspTraceInfo(("%s %d Adding %d \n", __FUNCTION__, __LINE__, p_VirtIf->VirIfIdx));
                WanMgr_VirtIfConfVLAN(p_VirtIf, idx);
                WanMgr_AddVirtualToList(&(pIfaceData->data.VirtIfList), p_VirtIf);
            }
        }
        // initialize
        pWanIfaceCtrl->update = 0;
    }

    return ANSC_STATUS_SUCCESS;
}

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

ANSC_STATUS WanMgr_WanConfigInit(void)
{
    ANSC_STATUS retStatus = ANSC_STATUS_FAILURE;

    //Wan Configuration init
    WanMgr_Config_Data_t* pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        retStatus = WanMgr_WanConfInit(&(pWanConfigData->data));

        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    if(retStatus != ANSC_STATUS_SUCCESS)
    {
        return retStatus;
    }


    WanMgr_Group_Configure();
    //Wan Interface Configuration init
    retStatus = WanMgr_WanDataInit();
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

DML_VIRTUAL_IFACE* WanMgr_getVirtualIface_locked(UINT baseIfidx, UINT virtIfIdx)
{
    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(baseIfidx < pWanIfaceCtrl->ulTotalNumbWanInterfaces)
        {
            if(pWanIfaceCtrl->pIface != NULL)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[baseIfidx]);
                return WanMgr_getVirtualIfaceById(pWanIfaceData->data.VirtIfList, virtIfIdx);
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }
    return NULL;
}

DML_VLAN_IFACE_TABLE* WanMgr_getVlanIface_locked(UINT baseIfidx, UINT virtIfIdx, UINT vlanid)
{
    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(baseIfidx < pWanIfaceCtrl->ulTotalNumbWanInterfaces)
        {
            if(pWanIfaceCtrl->pIface != NULL)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[baseIfidx]);
                return WanMgr_getVirtVlanIfById((WanMgr_getVirtualIfaceById(pWanIfaceData->data.VirtIfList, virtIfIdx))->VLAN.InterfaceList, vlanid);
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }
    return NULL;
}

void WanMgr_VlanIfaceMutex_release(DML_VLAN_IFACE_TABLE* vlanIf)
{
    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
    if(pWanIfaceCtrl != NULL)
    {
        pthread_mutex_unlock (&gWanMgrDataBase.gDataMutex);
    }
}

DML_VIRTIF_MARKING* WanMgr_getVirtualMarking_locked(UINT baseIfidx, UINT virtIfIdx, UINT Markingid)
{
    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(baseIfidx < pWanIfaceCtrl->ulTotalNumbWanInterfaces)
        {
            if(pWanIfaceCtrl->pIface != NULL)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[baseIfidx]);
                return WanMgr_getVirtMakingById((WanMgr_getVirtualIfaceById(pWanIfaceData->data.VirtIfList, virtIfIdx))->VLAN.VirtMarking, Markingid);
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }
    return NULL;
}

void WanMgr_VirtMarkingMutex_release(DML_VIRTIF_MARKING* virtMarking)
{
    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
    if(pWanIfaceCtrl != NULL)
    {
        pthread_mutex_unlock (&gWanMgrDataBase.gDataMutex);
    }
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

                if(!strcmp(iface_name, pWanIfaceData->data.VirtIfList->Name)) //get the fist interface name
                {
                    return pWanIfaceData;
                }
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }

    return NULL;
}

DML_VIRTUAL_IFACE* WanMgr_GetVirtualIfaceByName_locked(char* iface_name)
{
    UINT idx;
    if(iface_name == NULL || strlen(iface_name) <=0)
    {
        return NULL;
    }

    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(pWanIfaceCtrl->pIface != NULL)
        {
            for(idx = 0; idx < pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[idx]);
                DML_VIRTUAL_IFACE* virIface = pWanIfaceData->data.VirtIfList;
                while(virIface != NULL)
                {
                    if(!strcmp(iface_name,virIface->Name))
                    {
                        return virIface;
                    }
                    virIface = virIface->next;
                }
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }

    return NULL;
}

ANSC_STATUS WanMgr_AddVirtVlanIfToList(DML_VLAN_IFACE_TABLE** head, DML_VLAN_IFACE_TABLE *newNode)
{
    if(NULL == newNode)
        return ANSC_STATUS_FAILURE;

    CcspTraceInfo(("%s %d Adding %d \n", __FUNCTION__, __LINE__, newNode->Index));
    if (*head == NULL)
    {
        *head = newNode;
        return ANSC_STATUS_SUCCESS;
    }

    DML_VLAN_IFACE_TABLE *last = *head;
    while (last->next != NULL)
        last = last->next;

    last->next = newNode;
    return ANSC_STATUS_SUCCESS;
}

DML_VLAN_IFACE_TABLE* WanMgr_getVirtVlanIfById(DML_VLAN_IFACE_TABLE* vlanIf, UINT inst)
{
    if (vlanIf == NULL) {
        return NULL;
    }

    while(vlanIf != NULL)
    {
        if(vlanIf->Index == inst)
        {
            return vlanIf;
        }
        vlanIf = vlanIf->next;
    }
    return NULL;
}

ANSC_STATUS WanMgr_AddVirtMarkingToList(DML_VIRTIF_MARKING** head, DML_VIRTIF_MARKING *newNode)
{
    if(NULL == newNode)
        return ANSC_STATUS_FAILURE;

    CcspTraceInfo(("%s %d Adding %d \n", __FUNCTION__, __LINE__, newNode->VirtMarkingInstanceNumber));
    if (*head == NULL)
    {
        *head = newNode;
        return ANSC_STATUS_SUCCESS;
    }

    DML_VIRTIF_MARKING* last = *head;
    while (last->next != NULL)
        last = last->next;

    last->next = newNode;
    return ANSC_STATUS_SUCCESS;
}

DML_VIRTIF_MARKING* WanMgr_getVirtMakingById(DML_VIRTIF_MARKING* marking, UINT inst)
{
    if (marking == NULL) {
        return NULL;
    }

    while(marking != NULL)
    {
        if(marking->VirtMarkingInstanceNumber == inst)
        {
            return marking;
        }
        marking = marking->next;
    }
    return NULL;
}

ANSC_STATUS WanMgr_AddVirtualToList(DML_VIRTUAL_IFACE** head, DML_VIRTUAL_IFACE *newNode)
{
    if(NULL == newNode)
        return ANSC_STATUS_FAILURE;

    CcspTraceInfo(("%s %d Adding %d \n", __FUNCTION__, __LINE__, newNode->VirIfIdx));
    if (*head == NULL) 
    {
        *head = newNode;
        return ANSC_STATUS_SUCCESS;
    }

    DML_VIRTUAL_IFACE* last = *head;
    while (last->next != NULL)
        last = last->next;

    last->next = newNode;
    return ANSC_STATUS_SUCCESS;
}

DML_VIRTUAL_IFACE* WanMgr_getVirtualIfaceById(DML_VIRTUAL_IFACE* virIface, UINT inst)
{
    if (virIface == NULL) {
        return NULL;
    }

    while(virIface != NULL)
    {
        if(virIface->VirIfIdx == inst)
        {
            return virIface;
        }
        virIface = virIface->next;
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

void WanMgr_VirtualIfaceData_release(DML_VIRTUAL_IFACE* pVirtIf)
{
    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
    if(pWanIfaceCtrl != NULL)
    {
        pthread_mutex_unlock (&gWanMgrDataBase.gDataMutex);
    }
}

UINT WanMgr_GetTotalNoOfGroups()
{
    UINT groupCount = MAX_INTERFACE_GROUP; 
    if(pthread_mutex_lock(&(gWanMgrDataBase.gDataMutex)) == 0)
    {
                WanMgr_IfaceGroup_t* pWanIfaceGroup = &(gWanMgrDataBase.IfaceGroup);
        groupCount = pWanIfaceGroup->ulTotalNumbWanIfaceGroup;
        WanMgrDml_GetIfaceData_release(NULL);

    }
    return groupCount;
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
        pWanDmlIface->uiIfaceIdx = iface_index;
        pWanDmlIface->uiInstanceNumber = iface_index+1;
        memset(pWanDmlIface->Name, 0, 64);
        memset(pWanDmlIface->DisplayName, 0, 64);
        memset(pWanDmlIface->AliasName, 0, 64);
        memset(pWanDmlIface->BaseInterface, 0, 64);
        pWanDmlIface->BaseInterfaceStatus = WAN_IFACE_PHY_STATUS_DOWN;
        pWanDmlIface->VirtIfChanged = FALSE;
        pWanDmlIface->Selection.Enable = FALSE;
        pWanDmlIface->Selection.Priority = -1;
        pWanDmlIface->Selection.Timeout = 0;
        pWanDmlIface->IfaceType = LOCAL_IFACE;    // InterfaceType is Local by default
        pWanDmlIface->Selection.ActiveLink = FALSE;
	    pWanDmlIface->Selection.Status = WAN_IFACE_NOT_SELECTED;
        pWanDmlIface->Selection.RequiresReboot = FALSE;
        pWanDmlIface->Selection.RebootTriggerStatus = FALSE;
	    pWanDmlIface->Selection.Group = 1;
        pWanDmlIface->InterfaceScanStatus = WAN_IFACE_STATUS_NOT_SCANNED;

        pWanDmlIface->Sub.BaseInterfaceStatusSub = 0;
	    pWanDmlIface->Sub.WanStatusSub = 0;
	    pWanDmlIface->Sub.WanLinkStatusSub = 0;

        pWanDmlIface->NoOfVirtIfs = 1; 
        pWanDmlIface->Type = WAN_IFACE_TYPE_UNCONFIGURED;
    }
}

void WanMgr_VirtIface_Init(DML_VIRTUAL_IFACE * pVirtIf, UINT iface_index)
{
    CcspTraceInfo(("%s %d Initialize Wan Virtual Iface Conf \n", __FUNCTION__, __LINE__));
    if(pVirtIf == NULL)
    {    
        CcspTraceError(("%s %d: Invalid memory \n", __FUNCTION__, __LINE__));
    }
    pVirtIf->VirIfIdx = iface_index;
    memset(pVirtIf->Name, 0, sizeof(pVirtIf->Name));
    memset(pVirtIf->Alias, 0, sizeof(pVirtIf->Alias));

    pVirtIf->OperationalStatus = WAN_OPERSTATUS_UNKNOWN;
    pVirtIf->EnableMAPT = FALSE;
    pVirtIf->EnableDSLite = FALSE;
    pVirtIf->EnableIPoE = FALSE;
    pVirtIf->IP.RefreshDHCP = FALSE;        // RefreshDHCP is set when there is a change in IP source
    pVirtIf->IP.RestartV6Client = FALSE;
    pVirtIf->Status = WAN_IFACE_STATUS_DISABLED;
    pVirtIf->RemoteStatus = WAN_IFACE_STATUS_DISABLED;
    pVirtIf->VLAN.Status = WAN_IFACE_LINKSTATUS_DOWN;
    pVirtIf->VLAN.Enable = FALSE;
    pVirtIf->VLAN.NoOfMarkingEntries = 0;
    pVirtIf->VLAN.Timeout = 0;
    pVirtIf->VLAN.ActiveIndex = -1;
    pVirtIf->VLAN.NoOfInterfaceEntries = 0;
    memset(pVirtIf->VLAN.VLANInUse,0, sizeof(pVirtIf->VLAN.VLANInUse));
    pVirtIf->Reset = FALSE;
    memset(pVirtIf->IP.Interface, 0, 64);
    pVirtIf->IP.Ipv4Status = WAN_IFACE_IPV4_STATE_DOWN;
    pVirtIf->IP.Ipv6Status = WAN_IFACE_IPV6_STATE_DOWN;
    pVirtIf->IP.IPv4Source= DML_WAN_IP_SOURCE_DHCP;
    pVirtIf->IP.IPv6Source = DML_WAN_IP_SOURCE_DHCP;
#ifdef FEATURE_TAD_HEALTH_CHECK //FEATURE_IPOE_HEALTH_CHECK feature is enabled 
    CcspTraceInfo(("%s %d IP.ConnectivityCheckType set to WAN_CONNECTIVITY_TYPE_TAD \n", __FUNCTION__, __LINE__));
    pVirtIf->IP.ConnectivityCheckType = WAN_CONNECTIVITY_TYPE_TAD;
#else /* FEATURE_TAD_HEALTH_CHECK */
    pVirtIf->IP.ConnectivityCheckType = WAN_CONNECTIVITY_TYPE_NO_CHECK;
#endif /* FEATURE_TAD_HEALTH_CHECK */
    pVirtIf->IP.Ipv4ConnectivityStatus = WAN_CONNECTIVITY_DOWN; 
    pVirtIf->IP.Ipv6ConnectivityStatus = WAN_CONNECTIVITY_DOWN;
    pVirtIf->IP.Mode = DML_WAN_IP_MODE_DUAL_STACK;
    pVirtIf->IP.Ipv4Changed = FALSE;
    pVirtIf->IP.Ipv6Changed = FALSE;
    pVirtIf->IP.PreferredMode = DUAL_STACK_MODE;
    pVirtIf->IP.SelectedMode = DUAL_STACK_MODE;
    pVirtIf->IP.ModeForceEnable = FALSE;
    pVirtIf->IP.SelectedModeTimerStatus = NOTSTARTED;
    memset(&(pVirtIf->IP.SelectedModeTimerStart), 0, sizeof(struct timespec));
    pVirtIf->IP.Ipv4Renewed = FALSE;
    pVirtIf->IP.Ipv6Renewed = FALSE;
    memset(&(pVirtIf->IP.Ipv4Data), 0, sizeof(WANMGR_IPV4_DATA));
    memset(&(pVirtIf->IP.Ipv6Data), 0, sizeof(WANMGR_IPV6_DATA));
    pVirtIf->IP.pIpcIpv4Data = NULL;
    pVirtIf->IP.pIpcIpv6Data = NULL;
    pVirtIf->MAP.MaptStatus = WAN_IFACE_MAPT_STATE_DOWN;
    memset(pVirtIf->MAP.Path, 0, 64);
    pVirtIf->MAP.MaptChanged = FALSE;
#ifdef FEATURE_MAPT
    memset(&(pVirtIf->MAP.MaptConfig), 0, sizeof(WANMGR_MAPT_CONFIG_DATA));
#endif
    memset(pVirtIf->DSLite.Path, 0, 64);
    pVirtIf->DSLite.Status = WAN_IFACE_DSLITE_STATE_DOWN;
    pVirtIf->DSLite.Changed = FALSE;
    pVirtIf->PPP.Enable = FALSE;
    pVirtIf->PPP.LinkStatus = WAN_IFACE_PPP_LINK_STATUS_DOWN;
    pVirtIf->PPP.IPCPStatus = WAN_IFACE_IPCP_STATUS_DOWN;
    pVirtIf->PPP.IPV6CPStatus = WAN_IFACE_IPV6CP_STATUS_DOWN;
    pVirtIf->PPP.IPCPStatusChanged = FALSE;
    pVirtIf->PPP.IPV6CPStatusChanged = FALSE;
    memset(pVirtIf->PPP.Interface,0, sizeof(pVirtIf->PPP.Interface));

}

void WanMgr_Remote_IfaceData_Update(WanMgr_Iface_Data_t* pIfaceData, UINT iface_index)
{
    static int remotePriority = 100;
    CcspTraceInfo(("%s %d - Enter \n", __FUNCTION__, __LINE__));
    if(pIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanDmlIface = &(pIfaceData->data);
        strcpy(pWanDmlIface->Name, REMOTE_INTERFACE_NAME); // Remote name by default
        strcpy(pWanDmlIface->DisplayName, "REMOTE"); // Remote name by default
        pWanDmlIface->Selection.Enable = TRUE; // Remote wan Enable by default
        pWanDmlIface->Selection.Priority = remotePriority;
        pWanDmlIface->IfaceType = REMOTE_IFACE;    // InterfaceType is Remote by default
        pWanDmlIface->Selection.Group = REMOTE_INTERFACE_GROUP;
        /*Update GroupCfgChanged */

        WANMGR_IFACE_GROUP* pWanIfaceGroup = WanMgr_GetIfaceGroup_locked(pWanDmlIface->Selection.Group - 1);
        if (pWanIfaceGroup != NULL)
        {
            CcspTraceInfo(("%s %d Group(%d) configuration changed  \n", __FUNCTION__, __LINE__, pWanDmlIface->Selection.Group));
            /* Add interface to remote group Available interface list */
            pWanIfaceGroup->InterfaceAvailable |= (1<<pWanDmlIface->uiIfaceIdx);
            if(pWanIfaceGroup->State != STATE_GROUP_DEACTIVATED)
            {
                //If GROUP is DEACTIVATED, setting ConfigChanged flag is not required.
                pWanIfaceGroup->ConfigChanged = TRUE;
            }
            WanMgrDml_GetIfaceGroup_release();
            pWanIfaceGroup = NULL;
        }
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

            WanMgr_IfaceData_Init(pIfaceData, pWanIfaceCtrl->ulTotalNumbWanInterfaces);

            for(int i=0; i< pIfaceData->data.NoOfVirtIfs; i++)
            {
                DML_VIRTUAL_IFACE* p_VirtIf = (DML_VIRTUAL_IFACE *) AnscAllocateMemory( sizeof(DML_VIRTUAL_IFACE) );
                if(p_VirtIf == NULL)
                {
                    CcspTraceError(("%s %d: AnscAllocateMemory failed \n", __FUNCTION__, __LINE__));
                    return ANSC_STATUS_FAILURE;
                }
                WanMgr_VirtIface_Init(p_VirtIf, i);
                *iface_index = pWanIfaceCtrl->ulTotalNumbWanInterfaces;
                p_VirtIf->baseIfIdx = *iface_index; //Add base interface index 
                p_VirtIf->Enable = TRUE;
                p_VirtIf->IP.IPv6Source = DML_WAN_IP_SOURCE_STATIC;
                strncpy(p_VirtIf->Name, REMOTE_INTERFACE_NAME, sizeof(p_VirtIf->Name));
                CcspTraceInfo(("%s %d - Adding Remote Interface Index = [%d]\n", __FUNCTION__, __LINE__,p_VirtIf->baseIfIdx));
                WanMgr_AddVirtualToList(&(pIfaceData->data.VirtIfList), p_VirtIf);
            }
            WanMgr_Remote_IfaceData_Update(pIfaceData, pWanIfaceCtrl->ulTotalNumbWanInterfaces);

            DML_WAN_IFACE* pWanDmlIface = &(pIfaceData->data);
            strcpy(pWanDmlIface->RemoteCPEMac, remoteCPEMac);
            pWanIfaceCtrl->ulTotalNumbWanInterfaces = pWanIfaceCtrl->ulTotalNumbWanInterfaces + 1;
            gWanMgrDataBase.IfaceCtrl.update = 0;
            WanMgrDml_GetIfaceData_release(NULL);
            ret = ANSC_STATUS_SUCCESS;
        }
        else
        {
            CcspTraceError(("%s %d - Wan Interface Entries has reached its limit = [%d]\n", __FUNCTION__, __LINE__, MAX_WAN_INTERFACE_ENTRY));
        }
    }
    return ret;
}

ANSC_STATUS WanMgr_UpdatePrevData ()
{
#if !defined(WAN_MANAGER_UNIFICATION_ENABLED)
    if (access(WANMGR_RESTART_INFO_FILE, F_OK) != 0)
    {
        return ANSC_STATUS_FAILURE;
    }
#endif

    // update Wan Manager config data
    // update BootToWanUp flag
    char buff[64] = {0};
    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_NTP_TIME_SYNC, buff, sizeof(buff));
    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if(pWanConfigData != NULL)
    {
        if (strncmp(buff, NTP_TIME_SYNCD, strlen(NTP_TIME_SYNCD)) == 0)
        {
            CcspTraceInfo(("%s %d: already ntp has been synced, so Wan was UP before\n", __FUNCTION__, __LINE__));
            pWanConfigData->data.BootToWanUp = TRUE;
        }
        else
        {
            CcspTraceInfo(("%s %d: No ntp sync in device, so setting Boot to Wan up to false\n", __FUNCTION__, __LINE__))
            pWanConfigData->data.BootToWanUp = FALSE;
        }
        WanMgrDml_GetConfigData_release(pWanConfigData);
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
#if (defined (_XB6_PRODUCT_REQ_) || defined (_CBR2_PRODUCT_REQ_)) && !defined(WAN_MANAGER_UNIFICATION_ENABLED)
            //TODO: Rework for WAN_MANAGER_UNIFICATION_ENABLED
            WanMgr_RestartUpdateCfg_Bool (WAN_ENABLE_CUSTOM_CONFIG_PARAM_NAME, uiLoopCount, &pWanIfaceData->CustomConfigEnable);
            WanMgr_RestartUpdateCfg_Bool (WAN_CONFIGURE_WAN_ENABLE_PARAM_NAME, uiLoopCount, &pWanIfaceData->WanConfigEnabled);
            WanMgr_RestartUpdateCfg_Bool (WAN_ENABLE_OPER_STATUS_MONITOR_PARAM_NAME, uiLoopCount, &pWanIfaceData->MonitorOperStatus);
            WanMgr_RestartUpdateCfg (WAN_CUSTOM_CONFIG_PATH_PARAM_NAME, uiLoopCount, pWanIfaceData->CustomConfigPath, sizeof(pWanIfaceData->CustomConfigPath));
            WanMgr_RestartUpdateCfg (WAN_NAME_PARAM_NAME, uiLoopCount, pWanIfaceData->VirtIfList->Name, sizeof(pWanIfaceData->VirtIfList->Name));
#endif            
            WanMgr_GetBaseInterfaceStatus(pWanIfaceData);
            
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }   
    #if defined(RBUS_BUILD_FLAG_ENABLE) && defined(FEATURE_RDKB_INTER_DEVICE_MANAGER)
    WanMgr_RestartUpdateRemoteIface();
    #endif
    return ANSC_STATUS_SUCCESS;

}

/* WanMgr_GetVirtIfDataByAlias_locked () 
 * This function checks virtual interfaces of all Interfaces and retuns if Virtual Alias is found.
 */
DML_VIRTUAL_IFACE* WanMgr_GetVirtIfDataByAlias_locked(char* Alias)
{
    if (Alias == NULL)
    {
        CcspTraceError(("%s %d: invalid args\n", __FUNCTION__, __LINE__));
        return NULL;
    }
    UINT idx;

    if(pthread_mutex_lock(&gWanMgrDataBase.gDataMutex) == 0)
    {
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        if(pWanIfaceCtrl->pIface != NULL)
        {
            for(idx = 0; idx < pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++)
            {
                WanMgr_Iface_Data_t* pWanIfaceData = &(pWanIfaceCtrl->pIface[idx]);
                DML_VIRTUAL_IFACE* virIface = pWanIfaceData->data.VirtIfList;
                while(virIface != NULL)
                {
                    if(!strcmp(Alias,virIface->Alias))
                    {
                        return virIface;
                    }
                    virIface = virIface->next;
                }
            }
        }
        WanMgrDml_GetIfaceData_release(NULL);
    }

    return NULL;
}

