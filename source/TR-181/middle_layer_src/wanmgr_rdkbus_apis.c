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

/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

//!!!  This code assumes that all data structures are the SAME in middle-layer APIs and HAL layer APIs
//!!!  So it uses casting from one to the other
#include "wanmgr_rdkbus_apis.h"
#include "dmsb_tr181_psm_definitions.h"
#include "wanmgr_net_utils.h"
#ifdef RBUS_BUILD_FLAG_ENABLE
#include "wanmgr_rbus_handler_apis.h"
#endif //RBUS_BUILD_FLAG_ENABLE
//
#define PSM_ENABLE_STRING_TRUE  "TRUE"
#define PSM_ENABLE_STRING_FALSE  "FALSE"
#define PPP_LINKTYPE_PPPOA "PPPoA"
#define PPP_LINKTYPE_PPPOE "PPPoE"

#define MESH_IFNAME        "br-home"

#define DATA_SKB_MARKING_LOCATION "/tmp/skb_marking.conf"
#define WAN_DBUS_PATH             "/com/cisco/spvtg/ccsp/wanmanager"
#define WAN_COMPONENT_NAME        "eRT.com.cisco.spvtg.ccsp.wanmanager"
#define MARKING_TABLE             "Device.X_RDK_WanManager.CPEInterface.%d.Marking."

extern WANMGR_DATA_ST gWanMgrDataBase;
extern char g_Subsystem[32];
extern ANSC_HANDLE bus_handle;

static int get_Wan_Interface_ParametersFromPSM(ULONG instancenum, DML_WAN_IFACE* p_Interface)
{
    int retPsmGet = CCSP_SUCCESS;
    char param_value[256];
    char param_name[512];

    p_Interface->uiInstanceNumber = instancenum;

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_ENABLE, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.Enable = TRUE;
        }
        else
        {
             p_Interface->Wan.Enable = FALSE;
        }
    }
    else
    {
        p_Interface->Wan.Enable = FALSE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_ACTIVELINK, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.ActiveLink = TRUE;
        }
        else
        {
             p_Interface->Wan.ActiveLink = FALSE;
        }
    }
    else
    {
        p_Interface->Wan.ActiveLink = FALSE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_NAME, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        AnscCopyString(p_Interface->Name, param_value);
        AnscCopyString(p_Interface->Wan.Name, param_value);
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_DISPLAY_NAME, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        AnscCopyString(p_Interface->DisplayName, param_value);
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_ALIAS_NAME, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        AnscCopyString(p_Interface->AliasName, param_value);
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_TYPE, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        _ansc_sscanf(param_value, "%d", &(p_Interface->Wan.Type));
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_PRIORITY, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        _ansc_sscanf(param_value, "%d", &(p_Interface->Wan.Priority));
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_SELECTIONTIMEOUT, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        _ansc_sscanf(param_value, "%d", &(p_Interface->Wan.SelectionTimeout));
    }
    if (p_Interface->Wan.SelectionTimeout < SELECTION_TIMEOUT_DEFAULT_MIN)
    {
        p_Interface->Wan.SelectionTimeout = SELECTION_TIMEOUT_DEFAULT_MIN;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_GROUP, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        _ansc_sscanf(param_value, "%d", &(p_Interface->Wan.Group));
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_ENABLE_MAPT, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.EnableMAPT = TRUE;
        }
        else
        {
             p_Interface->Wan.EnableMAPT = FALSE;
        }
    }
    else
    {
        p_Interface->Wan.EnableMAPT = FALSE;
    }

    // Device.X_RDK_WanManager.CPEInterface.%d.Wan.EnableDHCP
    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_ENABLE_DHCP, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_FALSE) == 0)
        {
             p_Interface->Wan.EnableDHCP = FALSE;
        }
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_ENABLE_DSLITE, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.EnableDSLite = TRUE;
        }
        else
        {
             p_Interface->Wan.EnableDSLite = FALSE;
        }
    }
    else
    {
        p_Interface->Wan.EnableDSLite = FALSE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_ENABLE_IPOE, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.EnableIPoE = TRUE;
        }
        else
        {
             p_Interface->Wan.EnableIPoE = FALSE;
        }
    }
    else
    {
        p_Interface->Wan.EnableIPoE = FALSE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_REBOOTONCONFIGURATION, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.RebootOnConfiguration = TRUE;
        }
        else
        {
             p_Interface->Wan.RebootOnConfiguration = FALSE;
        }
    }
    else
    {
        p_Interface->Wan.RebootOnConfiguration = FALSE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_PPP_ENABLE, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->PPP.Enable = TRUE;
        }
        else
        {
             p_Interface->PPP.Enable = FALSE;
        }
    }
    else
    {
        p_Interface->PPP.Enable = FALSE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_PPP_IPCP_ENABLE, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->PPP.IPCPEnable = TRUE;
        }
        else
        {
             p_Interface->PPP.IPCPEnable = FALSE;
        }
    }
    else
    {
        p_Interface->PPP.IPCPEnable = FALSE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_PPP_IPV6CP_ENABLE, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->PPP.IPV6CPEnable = TRUE;
        }
        else
        {
             p_Interface->PPP.IPV6CPEnable = FALSE;
        }
    }
    else
    {
        p_Interface->PPP.IPV6CPEnable = FALSE;
    }

    _ansc_memset(param_name, 0, sizeof(param_name));
    _ansc_memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_PPP_LINKTYPE, instancenum);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PPP_LINKTYPE_PPPOA) == 0)
        {
             p_Interface->PPP.LinkType = WAN_IFACE_PPP_LINK_TYPE_PPPoA;
        }
        else if(strcmp(param_value, PPP_LINKTYPE_PPPOE) == 0)
        {
             p_Interface->PPP.LinkType = WAN_IFACE_PPP_LINK_TYPE_PPPoE;
        }
    }
    else
    {
        p_Interface->PPP.LinkType = WAN_IFACE_PPP_LINK_TYPE_PPPoA;
    }

    return ANSC_STATUS_SUCCESS;
}

static int write_Wan_Interface_ParametersFromPSM(ULONG instancenum, DML_WAN_IFACE* p_Interface)
{
    int retPsmSet = CCSP_SUCCESS;
    char param_name[256] = {0};
    char param_value[256] = {0};

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));

    if(p_Interface->Wan.Enable)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_ENABLE, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_value, "%d", p_Interface->Wan.Type );
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_TYPE, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_value, "%d", p_Interface->Wan.SelectionTimeout );
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_SELECTIONTIMEOUT, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_value, "%d", p_Interface->Wan.Group );
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_GROUP, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->Wan.EnableMAPT)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_ENABLE_MAPT, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->Wan.EnableDSLite)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_ENABLE_DSLITE, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->Wan.EnableIPoE)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_ENABLE_IPOE, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->Wan.RebootOnConfiguration)
    {
        _ansc_sprintf(param_value, PSM_ENABLE_STRING_TRUE);
    }
    else
    {
        _ansc_sprintf(param_value, PSM_ENABLE_STRING_FALSE);
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_REBOOTONCONFIGURATION, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);


    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->PPP.Enable)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_PPP_ENABLE, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->PPP.LinkType == WAN_IFACE_PPP_LINK_TYPE_PPPoA)
    {
        _ansc_sprintf(param_value, "PPPoA");
    }
    else if(p_Interface->PPP.LinkType == WAN_IFACE_PPP_LINK_TYPE_PPPoE)
    {
        _ansc_sprintf(param_value, "PPPoE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_PPP_LINKTYPE, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->PPP.IPCPEnable)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_PPP_IPCP_ENABLE, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->PPP.IPV6CPEnable)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_PPP_IPV6CP_ENABLE, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_value, "%d", p_Interface->Wan.Priority );
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_PRIORITY, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));
    if(p_Interface->Wan.EnableDHCP)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_WAN_ENABLE_DHCP, instancenum);
    WanMgr_RdkBus_SetParamValuesToDB(param_name, param_value);

    return ANSC_STATUS_SUCCESS;
}




/* DmlWanGetPSMRecordValue() */
static int
DmlWanGetPSMRecordValue
    (
         char *pPSMEntry,
         char *pOutputString
    )
{
    int   retPsmGet = CCSP_SUCCESS;
    char  strValue[256]  = {0};

    //Validate buffer
    if( ( NULL == pPSMEntry ) && ( NULL == pOutputString ) )
    {
        CcspTraceError(("%s %d Invalid buffer\n",__FUNCTION__,__LINE__));
        return retPsmGet;
    }

    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB( pPSMEntry, strValue, sizeof(strValue) );
    if ( retPsmGet == CCSP_SUCCESS )
    {
        //Copy till end of the string
        snprintf( pOutputString, strlen( strValue ) + 1, "%s", strValue );
    }

    return retPsmGet;
}

/* DmlWanSetPSMRecordValue() */
static int
DmlWanSetPSMRecordValue
    (
         char *pPSMEntry,
         char *pSetString
    )
{
    int   retPsmGet = CCSP_SUCCESS;

    //Validate buffer
    if( ( NULL == pPSMEntry ) && ( NULL == pSetString ) )
    {
        CcspTraceError(("%s %d Invalid buffer\n",__FUNCTION__,__LINE__));
        return retPsmGet;
    }

    retPsmGet = WanMgr_RdkBus_SetParamValuesToDB(pPSMEntry,pSetString);

    return retPsmGet;
}

/* DmlWanDeletePSMRecordValue() */
static int
DmlWanDeletePSMRecordValue
    (
         char *pPSMEntry
    )
{
    int   retPsmGet = CCSP_SUCCESS;

    //Validate buffer
    if( NULL == pPSMEntry )
    {
        CcspTraceError(("%s %d Invalid buffer\n",__FUNCTION__,__LINE__));
        return retPsmGet;
    }

    retPsmGet = PSM_Del_Record( bus_handle, g_Subsystem, pPSMEntry );

    return retPsmGet;
}


#ifdef FEATURE_802_1P_COS_MARKING

#ifdef _HUB4_PRODUCT_REQ_
static void AddSkbMarkingToConfFile(UINT data_skb_mark)
{
   FILE * fp = fopen(DATA_SKB_MARKING_LOCATION, "w+");
   if (!fp)
   {
      AnscTraceError(("%s Error writing skb mark\n", __FUNCTION__));
   }
   else
   {
      fprintf(fp, "data_skb_marking %d\n",data_skb_mark);
      fclose(fp);
   }
}
#endif

/* DmlWanIfMarkingInit() */
ANSC_STATUS WanMgr_WanIfaceMarkingInit ()
{
    INT iLoopCount  = 0;

    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
    //Validate received buffer
    if( NULL == pWanIfaceCtrl )
    {
        CcspTraceError(("%s %d - Invalid buffer\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Initialise Marking Params
    for( iLoopCount = 0; iLoopCount < pWanIfaceCtrl->ulTotalNumbWanInterfaces; iLoopCount++ )
    {
            ULONG                    ulIfInstanceNumber = 0;
            char                     acPSMQuery[128]    = { 0 },
                                     acPSMValue[64]     = { 0 };

            //Interface instance number
            ulIfInstanceNumber = iLoopCount + 1;

            //Query marking list for corresponding interface
            snprintf( acPSMQuery, sizeof( acPSMQuery ), PSM_MARKING_LIST, ulIfInstanceNumber );
            if ( ( CCSP_SUCCESS == DmlWanGetPSMRecordValue( acPSMQuery, acPSMValue ) ) && \
                 ( strlen( acPSMValue ) > 0 ) )
            {
                char acTmpString[64] = { 0 };
                char *token          = NULL;


                //Parse PSM output
                snprintf( acTmpString, sizeof( acTmpString ), acPSMValue );

                //split marking table value
                token = strtok( acTmpString, "-" );

                //check and add
                while ( token != NULL )
                {
                   char aTableName[512] = {0};
                   ULONG                                ulInstanceNumber  = 0;

                   /* Insert into marking table */
                    snprintf(aTableName, sizeof(aTableName), MARKING_TABLE, ulIfInstanceNumber);
                    if (CCSP_SUCCESS == CcspBaseIf_AddTblRow(bus_handle,WAN_COMPONENT_NAME,WAN_DBUS_PATH, 0, aTableName,&ulInstanceNumber))
                    {
                          DML_MARKING    Marking;
                          DML_MARKING*   p_Marking = &Marking;

                           char acTmpMarkingData[ 32 ] = { 0 };
                           /* a) SKBPort is derived from InstanceNumber.
                              b) Since InstanceNumber starts with Zero, to make it non-zero (add 1)
                              c) Voice packet get EthernetPriotityMark based on SKBPort.*/
                           p_Marking->InstanceNumber = ulInstanceNumber + 1;

                           //Stores into tmp buffer
                           snprintf( acTmpMarkingData, sizeof( acTmpMarkingData ), "%s", token );

                           //Get Alias from PSM
                           memset( acPSMQuery, 0, sizeof( acPSMQuery ) );
                           memset( acPSMValue, 0, sizeof( acPSMValue ) );

                           snprintf( acPSMQuery, sizeof( acPSMQuery ), PSM_MARKING_ALIAS, ulIfInstanceNumber, acTmpMarkingData );
                           if ( ( CCSP_SUCCESS == DmlWanGetPSMRecordValue( acPSMQuery, acPSMValue ) ) && \
                                ( strlen( acPSMValue ) > 0 ) )
                           {
                                snprintf( p_Marking->Alias, sizeof( p_Marking->Alias ), "%s", acPSMValue );
                           }

                           //Get SKB Port from PSM
                           memset( acPSMQuery, 0, sizeof( acPSMQuery ) );
                           memset( acPSMValue, 0, sizeof( acPSMValue ) );

                           snprintf( acPSMQuery, sizeof( acPSMQuery ), PSM_MARKING_SKBPORT, ulIfInstanceNumber, acTmpMarkingData );
                           if ( ( CCSP_SUCCESS == DmlWanGetPSMRecordValue( acPSMQuery, acPSMValue ) ) && \
                                ( strlen( acPSMValue ) > 0 ) )
                           {
                                p_Marking->SKBPort = atoi( acPSMValue );

                                //Re-adjust SKB Port if it is not matching with instance number
                                if ( p_Marking->InstanceNumber != p_Marking->SKBPort )
                                {
                                    p_Marking->SKBPort = p_Marking->InstanceNumber;

                                    //Set SKB Port into PSM
                                    memset( acPSMValue, 0, sizeof( acPSMValue ) );

                                    snprintf( acPSMValue, sizeof( acPSMValue ), "%u", p_Marking->SKBPort );
                                    DmlWanSetPSMRecordValue( acPSMQuery, acPSMValue );
                                }
                           }

                           //Get SKB Mark from PSM
                           memset( acPSMQuery, 0, sizeof( acPSMQuery ) );
                           memset( acPSMValue, 0, sizeof( acPSMValue ) );

                           snprintf( acPSMQuery, sizeof( acPSMQuery ), PSM_MARKING_SKBMARK, ulIfInstanceNumber, acTmpMarkingData );
                           if ( ( CCSP_SUCCESS == DmlWanGetPSMRecordValue( acPSMQuery, acPSMValue ) ) && \
                                ( strlen( acPSMValue ) > 0 ) )
                            {
                                /*
                                 * Re-adjust SKB Mark
                                 *
                                 * 0x100000 * InstanceNumber(1,2,3, etc)
                                 * 1048576 is decimal equalent to 0x100000 hexa decimal
                                 */
                                p_Marking->SKBMark = ( p_Marking->InstanceNumber ) * ( 1048576 );

                                //Set SKB Port into PSM
                                memset( acPSMValue, 0, sizeof( acPSMValue ) );

                                snprintf( acPSMValue, sizeof( acPSMValue ), "%u", p_Marking->SKBMark );
                                DmlWanSetPSMRecordValue( acPSMQuery, acPSMValue );
                            }

                            //Get Ethernet Priority Mark from PSM
                            memset( acPSMQuery, 0, sizeof( acPSMQuery ) );
                            memset( acPSMValue, 0, sizeof( acPSMValue ) );

                            snprintf( acPSMQuery, sizeof( acPSMQuery ), PSM_MARKING_ETH_PRIORITY_MASK, ulIfInstanceNumber, acTmpMarkingData );
                            if ( ( CCSP_SUCCESS == DmlWanGetPSMRecordValue( acPSMQuery, acPSMValue ) ) && \
                                    ( strlen( acPSMValue ) > 0 ) )
                            {
                                p_Marking->EthernetPriorityMark = atoi( acPSMValue );
                            }

                            CcspTraceInfo(("%s - Name[%s] Data[%s,%u,%u,%d]\n", __FUNCTION__, acTmpMarkingData, p_Marking->Alias, p_Marking->SKBPort, p_Marking->SKBMark, p_Marking->EthernetPriorityMark));

                            Marking_UpdateInitValue(pWanIfaceCtrl->pIface,ulIfInstanceNumber-1,ulInstanceNumber,p_Marking);
#ifdef _HUB4_PRODUCT_REQ_
                            /* Adding skb mark to config file if alis is 'DATA', so that udhcpc could use it to mark dhcp packets */
                            if(0 == strncmp(p_Marking->Alias, "DATA", 4))
                            {
                                AddSkbMarkingToConfFile(p_Marking->SKBMark);
                            }
#endif
                       }

                   token = strtok( NULL, "-" );
                }
            }

    }

    return ANSC_STATUS_SUCCESS;
}

/* * SListPushMarkingEntryByInsNum() */
ANSC_STATUS
SListPushMarkingEntryByInsNum
    (
        PSLIST_HEADER               pListHead,
        PCONTEXT_LINK_OBJECT   pLinkContext
    )
{
    ANSC_STATUS                 returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT   pLineContextEntry = (PCONTEXT_LINK_OBJECT)NULL;
    PSINGLE_LINK_ENTRY          pSLinkEntry       = (PSINGLE_LINK_ENTRY       )NULL;
    ULONG                       ulIndex           = 0;

    if ( pListHead->Depth == 0 )
    {
        AnscSListPushEntryAtBack(pListHead, &pLinkContext->Linkage);
    }
    else
    {
        pSLinkEntry = AnscSListGetFirstEntry(pListHead);

        for ( ulIndex = 0; ulIndex < pListHead->Depth; ulIndex++ )
        {
            pLineContextEntry = ACCESS_CONTEXT_LINK_OBJECT(pSLinkEntry);
            pSLinkEntry       = AnscSListGetNextEntry(pSLinkEntry);

            if ( pLinkContext->InstanceNumber < pLineContextEntry->InstanceNumber )
            {
                AnscSListPushEntryByIndex(pListHead, &pLinkContext->Linkage, ulIndex);

                return ANSC_STATUS_SUCCESS;
            }
        }

        AnscSListPushEntryAtBack(pListHead, &pLinkContext->Linkage);
    }

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
DmlCheckAndProceedMarkingOperations
    (
        ANSC_HANDLE         hContext,
        DML_MARKING*   pMarking,
        DML_WAN_MARKING_DML_OPERATIONS enMarkingOp
    )
{
    char    acPSMQuery[128]    = { 0 },
            acPSMValue[64]     = { 0 };
    ULONG   ulIfInstanceNumber = 0;

    //Validate param
    if ( NULL == pMarking )
    {
        CcspTraceError(("%s %d Invalid Buffer\n", __FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Find the Marking entry in PSM
    ulIfInstanceNumber = pMarking->ulWANIfInstanceNumber;

    //Query marking list for corresponding interface
    snprintf( acPSMQuery, sizeof( acPSMQuery ), PSM_MARKING_LIST, ulIfInstanceNumber );
    if ( CCSP_SUCCESS == DmlWanGetPSMRecordValue( acPSMQuery, acPSMValue ) )
    {
        char     acTmpString[64]          = { 0 },
                 acFoundMarkingRecord[64] = { 0 };
        char     *token                   = NULL;
        BOOL     IsMarkingRecordFound     = FALSE;

        //Parse PSM output
        snprintf( acTmpString, sizeof( acTmpString ), acPSMValue );

        //split marking table value
        token = strtok( acTmpString, "-" );

        //check and add
        while ( token != NULL )
        {
            if( 0 == strcmp( pMarking->Alias, token ) )
            {
                IsMarkingRecordFound = TRUE;
                snprintf( acFoundMarkingRecord, sizeof( acFoundMarkingRecord ), "%s", token );
                break;
            }

            token = strtok( NULL, "-" );
        }

        /*
         *
         * Note:
         * ----
         * If record found when add then reject that process
         * If record not found when add then needs to create new entry and update fields and LIST
         *
         * If record not found when delete then reject that process
         * If record found when delete then needs to delete all corresponding fields in DB and update LIST
         *
         * If record not found when update then reject that process
         * If record found when update then needs to update fields only not LIST
         *
         */
        switch( enMarkingOp )
        {
             case WAN_MARKING_ADD:
             {
                char    acPSMRecEntry[64],
                        acPSMRecValue[64];

                if( TRUE == IsMarkingRecordFound )
                {
                    CcspTraceError(("%s %d - Failed to add since record(%s) already exists!\n",__FUNCTION__,__LINE__,acFoundMarkingRecord));
                    return ANSC_STATUS_FAILURE;
                }

                //Set LIST into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_LIST, ulIfInstanceNumber );

                //Check whether already LIST is having another MARKING or not.
                if( 0 < strlen( acPSMValue ) )
                {
                    snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%s-%s", acPSMValue, pMarking->Alias );
                }
                else
                {
                    snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%s", pMarking->Alias );
                }

                //Check set is proper or not
                if ( CCSP_SUCCESS != DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue ) )
                {
                    CcspTraceError(("%s %d Failed to set PSM record %s\n", __FUNCTION__,__LINE__,acPSMRecEntry));
                    return ANSC_STATUS_FAILURE;
                }

                //Set Alias into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_ALIAS, ulIfInstanceNumber, pMarking->Alias );
                snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%s", pMarking->Alias );
                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                //Set SKBPort into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_SKBPORT, ulIfInstanceNumber, pMarking->Alias );

                /* a) SKBPort is derived from InstanceNumber.
                   b) Since InstanceNumber starts with Zero, to make it non-zero (add 1)
                   c) Voice packet get EthernetPriotityMark based on SKBPort.*/

                pMarking->InstanceNumber = pMarking->InstanceNumber + 1;

                /*
                 * Generate SKB port
                 *
                 * Stores the SKB Port for the entry. This is auto-generated for each entry starting from "1".
                 * Its value matches the instance index.
                 */

                pMarking->SKBPort = pMarking->InstanceNumber;

                snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%u", pMarking->SKBPort );
                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                //Set SKBMark into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_SKBMARK, ulIfInstanceNumber, pMarking->Alias );

                /*
                 * Generate SKB Mark
                 *
                 * Stores the SKB Mark for the entry. This is auto-generated for each entry starting from "0x100000".
                 * Its value increments by "0x100000", so the next would be "0x200000", then "0x300000", etc...
                 *
                 * 0x100000 * InstanceNumber(1,2,3, etc)
                 * 1048576 is decimal equalent to 0x100000 hexa decimal
                 */
                pMarking->SKBMark = ( pMarking->InstanceNumber ) * ( 1048576 );
                snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%u", pMarking->SKBMark );
                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                //Set Ethernet Priority Mark into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_ETH_PRIORITY_MASK, ulIfInstanceNumber, pMarking->Alias );
                snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%d", pMarking->EthernetPriorityMark );
                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                CcspTraceInfo(("%s Marking table(%s) and records added successfully\n",__FUNCTION__,pMarking->Alias));
             }
             break; /* * WAN_MARKING_ADD */

             case WAN_MARKING_UPDATE:
             {
                char    acPSMRecEntry[64],
                        acPSMRecValue[64];

                if( FALSE == IsMarkingRecordFound )
                {
                    CcspTraceError(("%s %d - Failed to update since record(%s) not exists!\n",__FUNCTION__,__LINE__,pMarking->Alias));
                    return ANSC_STATUS_FAILURE;
                }

                //Set Alias into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_ALIAS, ulIfInstanceNumber, pMarking->Alias );
                snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%s", pMarking->Alias );
                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                //Set SKBPort into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_SKBPORT, ulIfInstanceNumber, pMarking->Alias );
                snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%u", pMarking->SKBPort );
                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                //Set SKBMark into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_SKBMARK, ulIfInstanceNumber, pMarking->Alias );
                snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%u", pMarking->SKBMark );
                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                //Set Ethernet Priority Mark into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );
                memset( acPSMRecValue, 0, sizeof( acPSMRecValue ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_ETH_PRIORITY_MASK, ulIfInstanceNumber, pMarking->Alias );
                snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%d", pMarking->EthernetPriorityMark );
                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                CcspTraceInfo(("%s Marking table(%s) and records updated successfully\n",__FUNCTION__,pMarking->Alias));
             }
             break; /* * WAN_MARKING_UPDATE */

             case WAN_MARKING_DELETE:
             {
                char    acPSMRecEntry[64],
                        acPSMRecValue[64],
                        acNewMarkingList[64] = { 0 },
                        acNewTmpString[64]   = { 0 },
                        *tmpToken            = NULL;
                INT     iTotalMarking        = 0;


                if( FALSE == IsMarkingRecordFound )
                {
                    CcspTraceError(("%s %d - Failed to delete since record(%s) not exists!\n",__FUNCTION__,__LINE__,pMarking->Alias));
                    return ANSC_STATUS_FAILURE;
                }

                //Set Alias into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_ALIAS, ulIfInstanceNumber, pMarking->Alias );
                DmlWanDeletePSMRecordValue( acPSMRecEntry );

                //Set SKBPort into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_SKBPORT, ulIfInstanceNumber, pMarking->Alias );
                DmlWanDeletePSMRecordValue( acPSMRecEntry );

                //Set SKBMark into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_SKBMARK, ulIfInstanceNumber, pMarking->Alias );
                DmlWanDeletePSMRecordValue( acPSMRecEntry );

                //Set Ethernet Priority Mark into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_ETH_PRIORITY_MASK, ulIfInstanceNumber, pMarking->Alias );
                DmlWanDeletePSMRecordValue( acPSMRecEntry );

                //Remove entry from LIST

                //Parse PSM output
                snprintf( acNewTmpString, sizeof( acNewTmpString ), acPSMValue );

                //split marking table value
                tmpToken = strtok( acNewTmpString, "-" );

                //check and add
                while ( tmpToken != NULL )
                {
                    //Copy all the values except delete alias
                    if( 0 != strcmp( pMarking->Alias, tmpToken ) )
                    {
                        if( 0 == iTotalMarking )
                        {
                            snprintf( acNewMarkingList, sizeof( acNewMarkingList ), "%s", tmpToken );
                        }
                        else
                        {
                            //Append remaining marking strings
                            strncat( acNewMarkingList, "-", strlen("-") + 1 );
                            strncat( acNewMarkingList, tmpToken, strlen( tmpToken ) + 1 );
                        }

                        iTotalMarking++;
                    }

                    tmpToken = strtok( NULL, "-" );
                }

                //Check whether any marking available or not
                if( iTotalMarking == 0 )
                {
                    snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%s", "" ); //Copy empty
                }
                else
                {
                    snprintf( acPSMRecValue, sizeof( acPSMRecValue ), "%s", acNewMarkingList ); //Copy new string
                }

                //Set Marking LIST into PSM
                memset( acPSMRecEntry, 0, sizeof( acPSMRecEntry ) );

                snprintf( acPSMRecEntry, sizeof( acPSMRecEntry ), PSM_MARKING_LIST, ulIfInstanceNumber );

                DmlWanSetPSMRecordValue( acPSMRecEntry, acPSMRecValue );

                CcspTraceInfo(("%s Marking table(%s) and records deleted successfully\n",__FUNCTION__,pMarking->Alias));
             }
             break; /* * WAN_MARKING_DELETE */

             default:
             {
                 CcspTraceError(("%s Invalid case\n",__FUNCTION__));
                 return ANSC_STATUS_FAILURE;
             }
        }
    }

    return ANSC_STATUS_SUCCESS;
}

/* * DmlAddMarking() */
ANSC_STATUS
DmlAddMarking
    (
        ANSC_HANDLE         hContext,
        DML_MARKING*   pMarking
    )
{
    ANSC_STATUS                 returnStatus      = ANSC_STATUS_SUCCESS;

    //Validate param
    if ( NULL == pMarking )
    {
        CcspTraceError(("%s %d Invalid Buffer\n", __FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    returnStatus = DmlCheckAndProceedMarkingOperations( hContext, pMarking, WAN_MARKING_ADD );

    if( ANSC_STATUS_SUCCESS != returnStatus )
    {
        CcspTraceError(("%s %d - Failed to Add Marking Entry\n",__FUNCTION__,__LINE__));
    }

    return ANSC_STATUS_SUCCESS;
}

/* * DmlDeleteMarking() */
ANSC_STATUS
DmlDeleteMarking
    (
        ANSC_HANDLE         hContext,
        DML_MARKING*   pMarking
    )
{
    ANSC_STATUS                 returnStatus      = ANSC_STATUS_SUCCESS;

    //Validate param
    if ( NULL == pMarking )
    {
        CcspTraceError(("%s %d Invalid Buffer\n", __FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    returnStatus = DmlCheckAndProceedMarkingOperations( hContext, pMarking, WAN_MARKING_DELETE );

    if( ANSC_STATUS_SUCCESS != returnStatus )
    {
        CcspTraceError(("%s %d - Failed to Delete Marking Entry\n",__FUNCTION__,__LINE__));
    }

    return returnStatus;
}

/* * DmlSetMarking() */
ANSC_STATUS
DmlSetMarking
    (
        ANSC_HANDLE         hContext,
        DML_MARKING*   pMarking
    )
{
    ANSC_STATUS                 returnStatus      = ANSC_STATUS_SUCCESS;

    //Validate param
    if ( NULL == pMarking )
    {
        CcspTraceError(("%s %d Invalid Buffer\n", __FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    returnStatus = DmlCheckAndProceedMarkingOperations( hContext, pMarking, WAN_MARKING_UPDATE );

    if( ANSC_STATUS_SUCCESS != returnStatus )
    {
        CcspTraceError(("%s %d - Failed to Update Marking Entry\n",__FUNCTION__,__LINE__));
    }

    return returnStatus;
}
#endif /* * FEATURE_802_1P_COS_MARKING */

/* DmlGetTotalNoOfWanInterfaces() */
ANSC_STATUS DmlGetTotalNoOfWanInterfaces(int *wan_if_count)
{
    int ret_val = ANSC_STATUS_SUCCESS;
    int retPsmGet = CCSP_SUCCESS;
    char param_value[64] = {0};

    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(PSM_WANMANAGER_WANIFCOUNT,param_value,sizeof(param_value));
    if (retPsmGet != CCSP_SUCCESS) { \
        AnscTraceFlow(("%s Error %d reading %s\n", __FUNCTION__, retPsmGet, PSM_WANMANAGER_WANIFCOUNT));
        ret_val = ANSC_STATUS_FAILURE;
    }
    else if(param_value[0] != '\0') {
        _ansc_sscanf(param_value, "%d", wan_if_count);
    }

    return ret_val;
}

/* DmlGetWanIfCfg() */
ANSC_STATUS DmlGetWanIfCfg( INT LineIndex, DML_WAN_IFACE* pstLineInfo )
{
    return ANSC_STATUS_SUCCESS;
}


/* DmlSetWanIfCfg() */
ANSC_STATUS DmlSetWanIfCfg( INT LineIndex, DML_WAN_IFACE* pstLineInfo )
{
    int ret_val = ANSC_STATUS_SUCCESS;
    ret_val = write_Wan_Interface_ParametersFromPSM(LineIndex, pstLineInfo);
    if(ret_val != ANSC_STATUS_SUCCESS) {
        AnscTraceFlow(("%s Failed!! Error code: %d", __FUNCTION__, ret_val));
    }

    return ret_val;
}


ANSC_STATUS WanMgr_WanIfaceConfInit(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl)
{
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

        //Memset all memory
        memset( pWanIfaceCtrl->pIface, 0, ( sizeof(WanMgr_Iface_Data_t) * MAX_WAN_INTERFACE_ENTRY ) );

        //Get static interface configuration from PSM data store
        for( idx = 0 ; idx < uiTotalIfaces ; idx++ )
        {
            WanMgr_Iface_Data_t*  pIfaceData  = &(pWanIfaceCtrl->pIface[idx]);
            WanMgr_IfaceData_Init(pIfaceData, idx);
            get_Wan_Interface_ParametersFromPSM((idx+1), &(pIfaceData->data));
        }
        // initialize
        pWanIfaceCtrl->update = 0;
    }

    return ANSC_STATUS_SUCCESS;
}


static ANSC_STATUS WanMgr_WanConfInit (DML_WANMGR_CONFIG* pWanConfig)
{
    unsigned int wan_enable;
    unsigned int wan_policy;
    unsigned int wan_allow_remote_iface = 0;
    unsigned int wan_restoration_delay = 0;
    unsigned int wan_idle_timeout;
    int ret_val = ANSC_STATUS_SUCCESS;
    int retPsmGet = CCSP_SUCCESS;
    char param_name[256] = {0};
    char param_value[256] = {0};

    memset(param_name, 0, sizeof(param_name));
    memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANENABLE);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS && param_value[0] != '\0')
        wan_enable = atoi(param_value);
    else
        ret_val = ANSC_STATUS_FAILURE;

    pWanConfig->Enable = wan_enable;

    memset(param_name, 0, sizeof(param_name));
    memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANPOLICY);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS && param_value[0] != '\0')
        wan_policy = atoi(param_value);
    else
        ret_val = ANSC_STATUS_FAILURE;

    pWanConfig->Policy = wan_policy;

    memset(param_name, 0, sizeof(param_name));
    memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_ALLOW_REMOTE_IFACE);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS && param_value[0] != '\0')
        wan_allow_remote_iface = atoi(param_value);

    pWanConfig->AllowRemoteInterfaces = wan_allow_remote_iface;

    memset(param_name, 0, sizeof(param_name));
    memset(param_value, 0, sizeof(param_value));
    _ansc_sprintf(param_name, PSM_WANMANAGER_RESTORATION_DELAY);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name,param_value,sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS && param_value[0] != '\0')
        wan_restoration_delay = atoi(param_value);

    pWanConfig->RestorationDelay = wan_restoration_delay;

    return ret_val;
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


    //Wan Interface Configuration init
    retStatus = WanMgr_WanDataInit();
    return retStatus;
}


ANSC_STATUS
SListPushEntryByInsNum
    (
        PSLIST_HEADER               pListHead,
        PCONTEXT_LINK_OBJECT   pContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pContextEntry = (PCONTEXT_LINK_OBJECT)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY       )NULL;
    ULONG                           ulIndex           = 0;

    if ( pListHead->Depth == 0 )
    {
        AnscSListPushEntryAtBack(pListHead, &pContext->Linkage);
    }
    else
    {
        pSLinkEntry = AnscSListGetFirstEntry(pListHead);

        for ( ulIndex = 0; ulIndex < pListHead->Depth; ulIndex++ )
        {
            pContextEntry = ACCESS_CONTEXT_LINK_OBJECT(pSLinkEntry);
            pSLinkEntry       = AnscSListGetNextEntry(pSLinkEntry);

            if ( pContext->InstanceNumber < pContextEntry->InstanceNumber )
            {
                AnscSListPushEntryByIndex(pListHead, &pContext->Linkage, ulIndex);

                return ANSC_STATUS_SUCCESS;
            }
        }

        AnscSListPushEntryAtBack(pListHead, &pContext->Linkage);
    }

    return ANSC_STATUS_SUCCESS;
}

PCONTEXT_LINK_OBJECT SListGetEntryByInsNum( PSLIST_HEADER pListHead, ULONG InstanceNumber)
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT            pContextEntry = (PCONTEXT_LINK_OBJECT)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY       )NULL;
    ULONG                           ulIndex           = 0;

    if ( pListHead->Depth == 0 )
    {
        return NULL;
    }
    else
    {
        pSLinkEntry = AnscSListGetFirstEntry(pListHead);

        for ( ulIndex = 0; ulIndex < pListHead->Depth; ulIndex++ )
        {
            pContextEntry = ACCESS_CONTEXT_LINK_OBJECT(pSLinkEntry);
            pSLinkEntry       = AnscSListGetNextEntry(pSLinkEntry);

            if ( pContextEntry->InstanceNumber == InstanceNumber )
            {
                return pContextEntry;
            }
        }
    }

    return NULL;
}

ANSC_STATUS DmlSetWanActiveLinkInPSMDB( UINT uiInterfaceIdx , bool storeValue )
{
    int retPsmSet = CCSP_SUCCESS;
    char param_name[256] = {0};
    char param_value[256] = {0};

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));

    if(storeValue == TRUE)
    {
        _ansc_sprintf(param_value, PSM_ENABLE_STRING_TRUE);
    }
    else
    {
        _ansc_sprintf(param_value, PSM_ENABLE_STRING_FALSE);
    }
    _ansc_sprintf(param_name, PSM_WANMANAGER_IF_ACTIVELINK, (uiInterfaceIdx + 1));
    CcspTraceInfo(("%s %d: setting %s = %s\n", __FUNCTION__, __LINE__, param_name, param_value));
    if (WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value) != CCSP_SUCCESS)
    {
        CcspTraceError(("%s %d: setting %s = %s in PSM failed\n", __FUNCTION__, __LINE__, param_name, param_value));
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS WanController_ClearWanConfigurationsInPSM()
{
    char param_name[256] = {0};
    char param_value[256] = {0};
    UINT        uiTotalIfaces;
    ANSC_STATUS result;

    result = DmlGetTotalNoOfWanInterfaces(&uiTotalIfaces);
    if(result != ANSC_STATUS_SUCCESS) 
    {
        return ANSC_STATUS_FAILURE;
    }

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));

    for(int instancenum = 1; instancenum <=uiTotalIfaces; instancenum++)
    {
        _ansc_sprintf(param_value, PSM_ENABLE_STRING_FALSE);
        _ansc_sprintf(param_name, PSM_WANMANAGER_IF_ACTIVELINK, (instancenum));
        WanMgr_RdkBus_SetParamValuesToDB(param_name,param_value);
    }
    return ANSC_STATUS_SUCCESS;
}

void SortedInsert( struct IFACE_INFO** head_ref,  struct IFACE_INFO *new_node)
{
    struct IFACE_INFO* current;
    if (*head_ref == NULL
            || (*head_ref)->Priority
            >= new_node->Priority) {
        new_node->next = *head_ref;
        *head_ref = new_node;
    }else
    {
        current = *head_ref;
        while (current->next != NULL
                && current->next->Priority < new_node->Priority) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}

ANSC_STATUS Update_Interface_Status()
{
    struct IFACE_INFO *head = NULL;
    DEVICE_NETWORKING_MODE devMode = GATEWAY_MODE;
    CHAR    InterfaceAvailableStatus[BUFLEN_64]  = {0};
    CHAR    InterfaceActiveStatus[BUFLEN_64]     = {0};
    CHAR    CurrentActiveInterface[BUFLEN_64] = {0};
    CHAR    CurrentStandbyInterface[BUFLEN_64] = {0};

    CHAR    prevInterfaceAvailableStatus[BUFLEN_64]  = {0};
    CHAR    prevInterfaceActiveStatus[BUFLEN_64]     = {0};
    CHAR    prevCurrentActiveInterface[BUFLEN_64] = {0};
    CHAR    prevCurrentStandbyInterface[BUFLEN_64] = {0};

#ifdef RBUS_BUILD_FLAG_ENABLE
    bool    publishAvailableStatus  = FALSE;
    bool    publishActiveStatus = FALSE;
    bool    publishCurrentActiveInf  = FALSE;
    bool    publishCurrentStandbyInf = FALSE;
#endif
    int uiLoopCount;

    WanMgr_Config_Data_t*   pWanConfigData = WanMgr_GetConfigData_locked();
    if (pWanConfigData != NULL)
    {
        DML_WANMGR_CONFIG* pWanDmlData = &(pWanConfigData->data);
        devMode = pWanDmlData->DeviceNwMode;
        WanMgrDml_GetConfigData_release(pWanConfigData);
    }

    int TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();
    for (uiLoopCount = 0; uiLoopCount < TotalIfaces; uiLoopCount++)
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            if(pWanIfaceData->Wan.Enable == TRUE)
            {
                struct IFACE_INFO *newIface = calloc(1, sizeof( struct IFACE_INFO));
                newIface->next = NULL;

                newIface->Priority = pWanIfaceData->Wan.Priority;
                if(pWanIfaceData->Phy.Status == WAN_IFACE_PHY_STATUS_UP)
                {
                    snprintf(newIface->AvailableStatus, sizeof(newIface->AvailableStatus), "%s,1", pWanIfaceData->DisplayName);
                }else
                    snprintf(newIface->AvailableStatus, sizeof(newIface->AvailableStatus), "%s,0", pWanIfaceData->DisplayName);

                if((pWanIfaceData->SelectionStatus == WAN_IFACE_ACTIVE) &&
                   ((pWanIfaceData->Wan.IfaceType == REMOTE_IFACE &&
                     pWanIfaceData->Wan.Status == WAN_IFACE_STATUS_UP &&
                     pWanIfaceData->Wan.RemoteStatus == WAN_IFACE_STATUS_UP) ||
                    (pWanIfaceData->Wan.IfaceType == LOCAL_IFACE &&
                     pWanIfaceData->Wan.Status == WAN_IFACE_STATUS_UP)) )
                {
                    snprintf(newIface->ActiveStatus, sizeof(newIface->ActiveStatus), "%s,1", pWanIfaceData->DisplayName);
                }else
                    snprintf(newIface->ActiveStatus, sizeof(newIface->ActiveStatus), "%s,0", pWanIfaceData->DisplayName);

                if(devMode  == GATEWAY_MODE)
                {
                    if(pWanIfaceData->SelectionStatus == WAN_IFACE_ACTIVE)
                    {
                        snprintf(newIface->CurrentActive, sizeof(newIface->CurrentActive), "%s", pWanIfaceData->Wan.Name);
#if defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
                        /* Update Only for Gateway mode. Wan IP Interface entry not added in PAM for MODEM_MODE */
                        WanMgr_RdkBus_setWanIpInterfaceData(pWanIfaceData);
#endif
                    }
                    else if(pWanIfaceData->SelectionStatus == WAN_IFACE_SELECTED)
                    {
                        snprintf(newIface->CurrentStandby, sizeof(newIface->CurrentStandby), "%s", pWanIfaceData->Wan.Name);
                    }
                }
                else // MODEM_MODE
                {
                    if(pWanIfaceData->SelectionStatus == WAN_IFACE_ACTIVE)
                    {
                        strncpy(newIface->CurrentActive, MESH_IFNAME, sizeof(MESH_IFNAME));
                    }
                }
                /* Sort the link list based on priority */
                SortedInsert(&head, newIface);
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }

    struct IFACE_INFO* pHead = head;
    struct IFACE_INFO* tmp = NULL;
    while(pHead!= NULL)
    {
        if(strlen(CurrentActiveInterface)>0 && strlen(pHead->CurrentActive)>0)
        {
            strcat(CurrentActiveInterface,",");
        }
        strcat(CurrentActiveInterface,pHead->CurrentActive);

        if(strlen(CurrentStandbyInterface)>0 && strlen(pHead->CurrentStandby)>0)
        {
            strcat(CurrentStandbyInterface,",");
        }
        strcat(CurrentStandbyInterface,pHead->CurrentStandby);

        if(strlen(InterfaceAvailableStatus)>0 && strlen(pHead->AvailableStatus)>0)
        {
            strcat(InterfaceAvailableStatus,"|");
        }
        strcat(InterfaceAvailableStatus,pHead->AvailableStatus);

        if(strlen(InterfaceActiveStatus)>0 && strlen(pHead->ActiveStatus)>0)
        {
            strcat(InterfaceActiveStatus,"|");
        }
        strcat(InterfaceActiveStatus,pHead->ActiveStatus);

        tmp = pHead->next;
        free(pHead);
        pHead = tmp;
    }

    pWanConfigData = WanMgr_GetConfigData_locked();
    if (pWanConfigData != NULL)
    {
        DML_WANMGR_CONFIG* pWanDmlData = &(pWanConfigData->data);
        if(strcmp(pWanDmlData->InterfaceAvailableStatus,InterfaceAvailableStatus) != 0)
        {
            strcpy(prevInterfaceAvailableStatus,pWanDmlData->InterfaceAvailableStatus);
            memset(pWanDmlData->InterfaceAvailableStatus,0, sizeof(pWanDmlData->InterfaceAvailableStatus));
            strcpy(pWanDmlData->InterfaceAvailableStatus,InterfaceAvailableStatus);
#ifdef RBUS_BUILD_FLAG_ENABLE
            publishAvailableStatus = TRUE;
#endif
        }
        if(strcmp(pWanDmlData->InterfaceActiveStatus,InterfaceActiveStatus) != 0)
        {
            strcpy(prevInterfaceActiveStatus,pWanDmlData->InterfaceActiveStatus);
            memset(pWanDmlData->InterfaceActiveStatus,0, sizeof(pWanDmlData->InterfaceActiveStatus));
            strcpy(pWanDmlData->InterfaceActiveStatus,InterfaceActiveStatus);
#ifdef RBUS_BUILD_FLAG_ENABLE
            publishActiveStatus = TRUE;
#endif
        }

        CcspTraceInfo(("%s %d -CurrentActiveInterface- [%s] [%s]\n",__FUNCTION__,__LINE__,pWanDmlData->CurrentActiveInterface,CurrentActiveInterface));
        if(strlen(CurrentActiveInterface) > 0)
        {
            if(strcmp(pWanDmlData->CurrentActiveInterface,CurrentActiveInterface) != 0 )
            {
                strcpy(prevCurrentActiveInterface,pWanDmlData->CurrentActiveInterface);
                strcpy(pWanDmlData->CurrentActiveInterface,CurrentActiveInterface);
#ifdef RBUS_BUILD_FLAG_ENABLE
                publishCurrentActiveInf = TRUE;
#endif //RBUS_BUILD_FLAG_ENABLE
            }
        }
        else
        {
            CcspTraceInfo(("%s %d -CurrentActiveInterface- No update\n",__FUNCTION__,__LINE__));
        }

        CcspTraceInfo(("%s %d -CurrentStandbyInterface- [%s] [%s]\n",__FUNCTION__,__LINE__,pWanDmlData->CurrentStandbyInterface,CurrentStandbyInterface));
        if(strcmp(pWanDmlData->CurrentStandbyInterface,CurrentStandbyInterface) != 0)
        {
            strcpy(prevCurrentStandbyInterface, pWanDmlData->CurrentStandbyInterface);
            strcpy(pWanDmlData->CurrentStandbyInterface, CurrentStandbyInterface);
#ifdef RBUS_BUILD_FLAG_ENABLE
            publishCurrentStandbyInf = TRUE;
#endif //RBUS_BUILD_FLAG_ENABLE
        }
        WanMgrDml_GetConfigData_release(pWanConfigData);
    }
#ifdef RBUS_BUILD_FLAG_ENABLE
    if(publishCurrentActiveInf == TRUE)
    {
        WanMgr_Rbus_String_EventPublish_OnValueChange(WANMGR_CONFIG_WAN_CURRENTACTIVEINTERFACE, prevCurrentActiveInterface, CurrentActiveInterface);
    }

    if(publishCurrentStandbyInf == TRUE)
    {
        WanMgr_Rbus_String_EventPublish_OnValueChange(WANMGR_CONFIG_WAN_CURRENTSTANDBYINTERFACE, prevCurrentStandbyInterface, CurrentStandbyInterface);
    }
    if(publishAvailableStatus == TRUE)
    {
        WanMgr_Rbus_String_EventPublish_OnValueChange(WANMGR_CONFIG_WAN_INTERFACEAVAILABLESTATUS, prevInterfaceAvailableStatus, InterfaceAvailableStatus);
    }
    if(publishActiveStatus == TRUE)
    {
        WanMgr_Rbus_String_EventPublish_OnValueChange(WANMGR_CONFIG_WAN_INTERFACEACTIVESTATUS, prevInterfaceActiveStatus, InterfaceActiveStatus);
    }

#endif //RBUS_BUILD_FLAG_ENABLE
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS WanMgr_Publish_WanStatus(UINT IfaceIndex)
{
    char param_name[256] = {0};
    static UINT PrevWanStatus = 0;
    WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(IfaceIndex);
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
        if (pWanIfaceData->Wan.Status != PrevWanStatus)
        {
            PrevWanStatus = pWanIfaceData->Wan.Status;
            if (pWanIfaceData->Sub.WanStatusSub)
            {
                memset(param_name, 0, sizeof(param_name));
                _ansc_sprintf(param_name, WANMGR_IFACE_WAN_STATUS, (IfaceIndex+1));
#ifdef RBUS_BUILD_FLAG_ENABLE
                WanMgr_Rbus_String_EventPublish(param_name, &PrevWanStatus);
#endif
            }
        }
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }
    return ANSC_STATUS_SUCCESS;
}
