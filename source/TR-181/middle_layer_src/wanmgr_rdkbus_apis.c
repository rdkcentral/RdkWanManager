/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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
#include "wanmgr_data.h"
#include "wanmgr_net_utils.h"
//
#define PSM_ENABLE_STRING_TRUE  "TRUE"
#define PSM_ENABLE_STRING_FALSE  "FALSE"
#define PPP_LINKTYPE_PPPOA "PPPoA"
#define PPP_LINKTYPE_PPPOE "PPPoE"

#define DATA_SKB_MARKING_LOCATION "/tmp/skb_marking.conf"
extern char g_Subsystem[32];
extern ANSC_HANDLE bus_handle;

#define _PSM_READ_PARAM(_PARAM_NAME) { \
    _ansc_memset(param_name, 0, sizeof(param_name)); \
    _ansc_sprintf(param_name, _PARAM_NAME, instancenum); \
    retPsmGet = PSM_Get_Record_Value2(bus_handle,g_Subsystem, param_name, NULL, &param_value); \
    if (retPsmGet != CCSP_SUCCESS) { \
        AnscTraceFlow(("%s Error %d reading %s %s\n", __FUNCTION__, retPsmGet, param_name, param_value));\
    } \
    else { \
        /*AnscTraceFlow(("%s: retPsmGet == CCSP_SUCCESS reading %s = \n%s\n", __FUNCTION__,param_name, param_value)); */\
    } \
}

#define _PSM_WRITE_PARAM(_PARAM_NAME) { \
    _ansc_sprintf(param_name, _PARAM_NAME, instancenum); \
    retPsmSet = PSM_Set_Record_Value2(bus_handle,g_Subsystem, param_name, ccsp_string, param_value); \
    if (retPsmSet != CCSP_SUCCESS) { \
        AnscTraceFlow(("%s Error %d writing %s %s\n", __FUNCTION__, retPsmSet, param_name, param_value));\
    } \
    else \
    { \
        /*AnscTraceFlow(("%s: retPsmSet == CCSP_SUCCESS writing %s = %s \n", __FUNCTION__,param_name,param_value)); */\
    } \
    _ansc_memset(param_name, 0, sizeof(param_name)); \
    _ansc_memset(param_value, 0, sizeof(param_value)); \
}


static int get_Wan_Interface_ParametersFromPSM(ULONG instancenum, DML_WAN_IFACE* p_Interface)
{
    int retPsmGet = CCSP_SUCCESS;
    char *param_value= NULL;
    char param_name[256]= {0};

    p_Interface->uiInstanceNumber = instancenum;

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_ENABLE);
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
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->Wan.Enable = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_NAME);
    if (retPsmGet == CCSP_SUCCESS)
    {
        AnscCopyString(p_Interface->Name, param_value);
        AnscCopyString(p_Interface->Wan.Name, param_value);
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_DISPLAY_NAME);
    if (retPsmGet == CCSP_SUCCESS)
    {
        AnscCopyString(p_Interface->DisplayName, param_value);
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_TYPE);
    if (retPsmGet == CCSP_SUCCESS)
    {
        _ansc_sscanf(param_value, "%d", &(p_Interface->Wan.Type));
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_PRIORITY);
    if (retPsmGet == CCSP_SUCCESS)
    {
        _ansc_sscanf(param_value, "%d", &(p_Interface->Wan.Priority));
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_SELECTIONTIMEOUT);
    if (retPsmGet == CCSP_SUCCESS)
    {
        _ansc_sscanf(param_value, "%d", &(p_Interface->Wan.SelectionTimeout));
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_ENABLE_MAPT);
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
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->Wan.EnableMAPT = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_ENABLE_DSLITE);
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
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->Wan.EnableDSLite = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_ENABLE_IPOE);
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
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->Wan.EnableIPoE = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_VALIDATION_DISCOVERY_OFFER);
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.Validation.DiscoverOffer = TRUE;
        }
        else
        {
             p_Interface->Wan.Validation.DiscoverOffer = FALSE;
        }
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->Wan.Validation.DiscoverOffer = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_VALIDATION_SOLICIT_ADVERTISE);
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.Validation.SolicitAdvertise = TRUE;
        }
        else
        {
             p_Interface->Wan.Validation.SolicitAdvertise = FALSE;
        }
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->Wan.Validation.SolicitAdvertise = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_VALIDATION_RS_RA);
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.Validation.RS_RA = TRUE;
        }
        else
        {
             p_Interface->Wan.Validation.RS_RA = FALSE;
        }
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->Wan.Validation.RS_RA = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_VALIDATION_PADI_PADO);
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->Wan.Validation.PadiPado = TRUE;
        }
        else
        {
             p_Interface->Wan.Validation.PadiPado = FALSE;
        }
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->Wan.Validation.PadiPado = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_DYNTRIGGERENABLE);
    if (retPsmGet == CCSP_SUCCESS)
    {
        if(strcmp(param_value, PSM_ENABLE_STRING_TRUE) == 0)
        {
             p_Interface->DynamicTrigger.Enable = TRUE;
        }
        else
        {
             p_Interface->DynamicTrigger.Enable = FALSE;
        }
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_PPP_ENABLE);
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
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->PPP.Enable = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_PPP_IPCP_ENABLE);
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
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->PPP.IPCPEnable = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_PPP_IPV6CP_ENABLE);
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
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->PPP.IPV6CPEnable = FALSE;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_WAN_PPP_LINKTYPE);
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
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }
    else
    {
        p_Interface->PPP.LinkType = WAN_IFACE_PPP_LINK_TYPE_PPPoA;
    }

    _PSM_READ_PARAM(PSM_WANMANAGER_IF_DYNTRIGGERDELAY);
    if (retPsmGet == CCSP_SUCCESS)
    {
        _ansc_sscanf(param_value, "%d", &(p_Interface->DynamicTrigger.Delay));
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
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
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_ENABLE);

    _ansc_sprintf(param_value, "%d", p_Interface->Wan.Type );
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_TYPE);

    _ansc_sprintf(param_value, "%d", p_Interface->Wan.Priority );
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_PRIORITY);

    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_SELECTIONTIMEOUT);
    _ansc_sprintf(param_value, "%d", p_Interface->Wan.SelectionTimeout );

    if(p_Interface->DynamicTrigger.Enable) {
        _ansc_sprintf(param_value, "TRUE");
    }
    else {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_DYNTRIGGERENABLE);

    if(p_Interface->Wan.EnableMAPT)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_ENABLE_MAPT);

    if(p_Interface->Wan.EnableDSLite)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_ENABLE_DSLITE);

    if(p_Interface->Wan.EnableIPoE)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_ENABLE_IPOE);

    if(p_Interface->Wan.Validation.DiscoverOffer)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }

    if(p_Interface->PPP.Enable)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_PPP_ENABLE);

    if(p_Interface->PPP.LinkType == WAN_IFACE_PPP_LINK_TYPE_PPPoA)
    {
        _ansc_sprintf(param_value, "PPPoA");
    }
    else if(p_Interface->PPP.LinkType == WAN_IFACE_PPP_LINK_TYPE_PPPoE)
    {
        _ansc_sprintf(param_value, "PPPoE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_PPP_LINKTYPE);

    if(p_Interface->PPP.IPCPEnable)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_PPP_IPCP_ENABLE);

    if(p_Interface->PPP.IPV6CPEnable)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_PPP_IPV6CP_ENABLE);

    _ansc_sprintf(param_value, "%d", p_Interface->Wan.Priority );
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_PRIORITY);

    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_DYNTRIGGERDELAY);
    _ansc_sprintf(param_value, "%d", p_Interface->DynamicTrigger.Delay );

    return ANSC_STATUS_SUCCESS;
}

static int write_Wan_Interface_Validation_ParametersToPSM(ULONG instancenum, DML_WAN_IFACE* p_Interface)
{
    if (NULL == p_Interface)
    {
        AnscTraceFlow(("%s Invalid memory!!!\n", __FUNCTION__));
        return ANSC_STATUS_INTERNAL_ERROR;
    }

    int retPsmSet = CCSP_SUCCESS;
    char param_name[256] = {0};
    char param_value[256] = {0};

    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));

    if(p_Interface->Wan.Validation.DiscoverOffer)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_VALIDATION_DISCOVERY_OFFER);

    if(p_Interface->Wan.Validation.SolicitAdvertise)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_VALIDATION_SOLICIT_ADVERTISE);

    if(p_Interface->Wan.Validation.RS_RA)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_VALIDATION_RS_RA);

    if(p_Interface->Wan.Validation.PadiPado)
    {
        _ansc_sprintf(param_value, "TRUE");
    }
    else
    {
        _ansc_sprintf(param_value, "FALSE");
    }
    _PSM_WRITE_PARAM(PSM_WANMANAGER_IF_WAN_VALIDATION_PADI_PADO);

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
    char *strValue  = NULL;

    //Validate buffer
    if( ( NULL == pPSMEntry ) && ( NULL == pOutputString ) )
    {
        CcspTraceError(("%s %d Invalid buffer\n",__FUNCTION__,__LINE__));
        return retPsmGet;
    }

    retPsmGet = PSM_Get_Record_Value2( bus_handle, g_Subsystem, pPSMEntry, NULL, &strValue );
    if ( retPsmGet == CCSP_SUCCESS )
    {
        //Copy till end of the string
        snprintf( pOutputString, strlen( strValue ) + 1, "%s", strValue );

        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(strValue);
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

    retPsmGet = PSM_Set_Record_Value2( bus_handle, g_Subsystem, pPSMEntry, ccsp_string, pSetString );

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
static ANSC_STATUS WanMgr_WanIfaceMarkingInit (WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl)
{
    INT iLoopCount  = 0;

    //Validate received buffer
    if( NULL == pWanIfaceCtrl )
    {
        CcspTraceError(("%s %d - Invalid buffer\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Initialise Marking Params
    for( iLoopCount = 0; iLoopCount < pWanIfaceCtrl->ulTotalNumbWanInterfaces; iLoopCount++ )
    {
        WanMgr_Iface_Data_t* pWanIfaceData = WanMgr_GetIfaceData_locked(iLoopCount);
        if(pWanIfaceData != NULL)
        {
            DML_WAN_IFACE*      pWanIface           = &(pWanIfaceData->data);
            DATAMODEL_MARKING*  pDataModelMarking   = &(pWanIface->Marking);
            ULONG                    ulIfInstanceNumber = 0;
            char                     acPSMQuery[128]    = { 0 },
                                     acPSMValue[64]     = { 0 };

            /* Initiation all params */
            AnscSListInitializeHeader( &pDataModelMarking->MarkingList );
            pDataModelMarking->ulNextInstanceNumber     = 1;

            //Interface instance number
            ulIfInstanceNumber = pWanIface->uiInstanceNumber;

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
                   CONTEXT_MARKING_LINK_OBJECT*         pMarkingCxtLink   = NULL;
                   ULONG                                ulInstanceNumber  = 0;

                   /* Insert into marking table */
                   if( ( NULL != ( pMarkingCxtLink = WanManager_AddIfaceMarking( pWanIface, &ulInstanceNumber ) ) )  &&
                       ( 0 < ulInstanceNumber ) )
                   {
                       DML_MARKING*    p_Marking = ( DML_MARKING* )pMarkingCxtLink->hContext;

                       //Reset this flag during init so set should happen in next time onwards
                       pMarkingCxtLink->bNew  = FALSE;

                       if( NULL != p_Marking )
                       {
                           char acTmpMarkingData[ 32 ] = { 0 };

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


#ifdef _HUB4_PRODUCT_REQ_
                            /* Adding skb mark to config file if alis is 'DATA', so that udhcpc could use it to mark dhcp packets */
                            if(0 == strncmp(p_Marking->Alias, "DATA", 4))
                            {
                                AddSkbMarkingToConfFile(p_Marking->SKBMark);
                            }
#endif
                       }
                   }

                   token = strtok( NULL, "-" );
                }
            }

            WanMgrDml_GetIfaceData_release(pWanIfaceData);
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
    char* param_value = NULL;

    retPsmGet = PSM_Get_Record_Value2(bus_handle,g_Subsystem, PSM_WANMANAGER_WANIFCOUNT, NULL, &param_value);
    if (retPsmGet != CCSP_SUCCESS) { \
        AnscTraceFlow(("%s Error %d reading %s %s\n", __FUNCTION__, retPsmGet, PSM_WANMANAGER_WANIFCOUNT, param_value));
        ret_val = ANSC_STATUS_FAILURE;
    }
    else if(param_value != NULL) {
        _ansc_sscanf(param_value, "%d", wan_if_count);
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
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

/* DmlSetWanIfValidationCfg() */
ANSC_STATUS DmlSetWanIfValidationCfg( INT WanIfIndex, DML_WAN_IFACE* pWanIfInfo)
{
    int ret_val = ANSC_STATUS_SUCCESS;

    if (NULL == pWanIfInfo)
    {
        AnscTraceFlow(("%s Failed!! Invalid memory \n", __FUNCTION__));
        return ANSC_STATUS_INTERNAL_ERROR;
    }
    ret_val = write_Wan_Interface_Validation_ParametersToPSM(WanIfIndex, pWanIfInfo);
    if(ret_val != ANSC_STATUS_SUCCESS) {
        AnscTraceFlow(("%s Failed!! Error code: %d", __FUNCTION__, ret_val));
    }

    return ret_val;
}

static ANSC_STATUS WanMgr_WanIfaceConfInit(WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl)
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

        pWanIfaceCtrl->pIface = (WanMgr_Iface_Data_t*) AnscAllocateMemory( sizeof(WanMgr_Iface_Data_t) * uiTotalIfaces);
        if( NULL == pWanIfaceCtrl->pIface )
        {
            return ANSC_STATUS_FAILURE;
        }

        pWanIfaceCtrl->ulTotalNumbWanInterfaces = uiTotalIfaces;

        //Memset all memory
        memset( pWanIfaceCtrl->pIface, 0, ( sizeof(WanMgr_Iface_Data_t) * uiTotalIfaces ) );

        //Get static interface configuration from PSM data store
        for( idx = 0 ; idx < uiTotalIfaces ; idx++ )
        {
            WanMgr_Iface_Data_t*  pIfaceData  = &(pWanIfaceCtrl->pIface[idx]);
            WanMgr_IfaceData_Init(pIfaceData, idx);
            get_Wan_Interface_ParametersFromPSM((idx+1), &(pIfaceData->data));
        }
    }

    return ANSC_STATUS_SUCCESS;
}


static ANSC_STATUS WanMgr_WanConfInit (DML_WANMGR_CONFIG* pWanConfig)
{
    unsigned int wan_enable;
    unsigned int wan_policy;
    unsigned int wan_idle_timeout;
    int ret_val = ANSC_STATUS_SUCCESS;
    int retPsmGet = CCSP_SUCCESS;
    char param_name[256] = {0};
    char* param_value = NULL;

    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANENABLE);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
        wan_enable = atoi(param_value);
    else
        ret_val = ANSC_STATUS_FAILURE;

    pWanConfig->Enable = wan_enable;

    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANPOLICY);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
        wan_policy = atoi(param_value);
    else
        ret_val = ANSC_STATUS_FAILURE;

    pWanConfig->Policy = wan_policy;

    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANIDLETIMEOUT);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
        wan_idle_timeout = atoi(param_value);
    else
        ret_val = ANSC_STATUS_FAILURE;

    pWanConfig->IdleTimeout = wan_idle_timeout;

    if(param_value != NULL)
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);

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
    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = WanMgr_GetIfaceCtrl_locked();
    if(pWanIfaceCtrl != NULL)
    {
        retStatus = WanMgr_WanIfaceConfInit(pWanIfaceCtrl);

#ifdef FEATURE_802_1P_COS_MARKING
        /* Initialize middle layer for Device.X_RDK_WanManager.CPEInterface.{i}.Marking.  */
        WanMgr_WanIfaceMarkingInit(pWanIfaceCtrl);
#endif /* * FEATURE_802_1P_COS_MARKING */


        WanMgrDml_GetIfaceCtrl_release(pWanIfaceCtrl);
    }

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
