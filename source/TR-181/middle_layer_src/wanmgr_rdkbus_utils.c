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


/**************************************************************************

    module: wan_controller_utils.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        State machine to manage a Wan Controller

    -------------------------------------------------------------------

    environment:

        Platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        13/02/2020    initial revision.

**************************************************************************/

/* ---- Include Files ---------------------------------------- */
#include "dmsb_tr181_psm_definitions.h"
#include "wanmgr_rdkbus_utils.h"
#include "ansc_platform.h"
#include "ccsp_psm_helper.h"
#include "wanmgr_data.h"
#include "wanmgr_data.h"

extern char g_Subsystem[32];
extern ANSC_HANDLE bus_handle;

ANSC_STATUS WanMgr_RdkBus_getWanPolicy(DML_WAN_POLICY *wan_policy)
{
    int result = ANSC_STATUS_SUCCESS;
    int retPsmGet = CCSP_SUCCESS;
    char *param_value= NULL;
    char param_name[BUFLEN_256]= {0};

    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANPOLICY);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL) {
        *wan_policy = strtol(param_value, NULL, 10);
    }
    else {
        result = ANSC_STATUS_FAILURE;
    }

    if (retPsmGet == CCSP_SUCCESS) {
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }

    return result;
}


ANSC_STATUS WanMgr_RdkBus_setWanPolicy(DML_WAN_POLICY wan_policy)
{
    int result = ANSC_STATUS_SUCCESS;
    int retPsmSet = CCSP_SUCCESS;
    char param_name[BUFLEN_256] = {0};
    char param_value[BUFLEN_256] = {0};

    /* Update the wan policy information in PSM */
    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));

    snprintf(param_value, sizeof(param_value), "%d", wan_policy);
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANPOLICY);

    retPsmSet = PSM_Set_Record_Value2(bus_handle,g_Subsystem, param_name, ccsp_string, param_value);
    if (retPsmSet != CCSP_SUCCESS) {
        AnscTraceError(("%s Error %d writing %s %s\n", __FUNCTION__, retPsmSet, param_name, param_value));
        result = ANSC_STATUS_FAILURE;
    }

    return result;
}


ANSC_STATUS WanMgr_RdkBus_updateInterfaceUpstreamFlag(char *phyPath, BOOL flag)
{
    char param_name[BUFLEN_256] = {0};
    char param_value[BUFLEN_256] = {0};
    char  pComponentName[BUFLEN_64] = {0};
    char  pComponentPath[BUFLEN_64] = {0};
    char *faultParam = NULL;
    int ret = 0;

    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t upstream_param[1] = {0};

    if(phyPath == NULL) {
        CcspTraceInfo(("%s %d Error: phyPath is NULL \n", __FUNCTION__, __LINE__ ));
        return ANSC_STATUS_FAILURE;
    }

    strncpy(param_name, phyPath, sizeof(param_name));

    if(strstr(param_name, "DSL") != NULL) { // dsl wan interface
        strncat(param_name, DSL_UPSTREAM_NAME, sizeof(param_name));
        strncpy(pComponentName, DSL_COMPONENT_NAME, sizeof(pComponentName));
        strncpy(pComponentPath, DSL_COMPONENT_PATH, sizeof(pComponentPath));
    }
    else if(strstr(param_name, "Ethernet") != NULL) { // ethernet wan interface
        strncat(param_name, ETH_UPSTREAM_NAME, sizeof(param_name));
        strncpy(pComponentName, ETH_COMPONENT_NAME, sizeof(pComponentName));
        strncpy(pComponentPath, ETH_COMPONENT_PATH, sizeof(pComponentPath));
    }
    if(flag)
        strncpy(param_value, "true", sizeof(param_value));
    else
        strncpy(param_value, "false", sizeof(param_value));

    upstream_param[0].parameterName = param_name;
    upstream_param[0].parameterValue = param_value;
    upstream_param[0].type = ccsp_boolean;

    ret = CcspBaseIf_setParameterValues(bus_handle, pComponentName, pComponentPath,
                                        0, 0x0,   /* session id and write id */
                                        upstream_param, 1, TRUE,   /* Commit  */
                                        &faultParam);

    if ( ( ret != CCSP_SUCCESS ) && ( faultParam )) {
        CcspTraceInfo(("%s CcspBaseIf_setParameterValues failed with error %d\n",__FUNCTION__, ret ));
        bus_info->freefunc( faultParam );
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}


static ANSC_STATUS WanMgr_RdkBus_GetParamNames( char *pComponent, char *pBus, char *pParamName, char a2cReturnVal[][BUFLEN_256], int *pReturnSize )
{
    CCSP_MESSAGE_BUS_INFO  *bus_info         = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterInfoStruct_t  **retInfo;
    char                    *ParamName[ 1 ];
    int                    ret               = 0,
                           nval;

    ret = CcspBaseIf_getParameterNames(
                                    bus_handle,
                                    pComponent,
                                    pBus,
                                    pParamName,
                                    1,
                                    &nval,
                                    &retInfo);

    //Copy the value
    if( CCSP_SUCCESS == ret )
    {
        int iLoopCount;

        *pReturnSize = nval;

        for( iLoopCount = 0; iLoopCount < nval; iLoopCount++ )
        {
           if( NULL != retInfo[iLoopCount]->parameterName )
           {
              //CcspTraceInfo(("%s parameterName[%d,%s]\n",__FUNCTION__,iLoopCount,retInfo[iLoopCount]->parameterName));
              snprintf( a2cReturnVal[iLoopCount], strlen( retInfo[iLoopCount]->parameterName ) + 1, "%s", retInfo[iLoopCount]->parameterName );
           }
        }

        if( retInfo )
        {
          free_parameterInfoStruct_t(bus_handle, nval, retInfo);
        }

        return ANSC_STATUS_SUCCESS;
    }

    if( retInfo )
    {
      free_parameterInfoStruct_t(bus_handle, nval, retInfo);
    }

    return ANSC_STATUS_FAILURE;
}

ANSC_STATUS WanMgr_RdkBus_GetParamValues( char *pComponent, char *pBus, char *pParamName, char *pReturnVal )
{
    CCSP_MESSAGE_BUS_INFO  *bus_info         = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t   **retVal;
    char                    *ParamName[ 1 ];
    int                    ret               = 0,
                           nval;

    //Assign address for get parameter name
    ParamName[0] = pParamName;

    ret = CcspBaseIf_getParameterValues(
                                    bus_handle,
                                    pComponent,
                                    pBus,
                                    ParamName,
                                    1,
                                    &nval,
                                    &retVal);

    //Copy the value
    if( CCSP_SUCCESS == ret )
    {
        //CcspTraceInfo(("%s parameterValue[%s]\n",__FUNCTION__,retVal[0]->parameterValue));

        if( NULL != retVal[0]->parameterValue )
        {
            memcpy( pReturnVal, retVal[0]->parameterValue, strlen( retVal[0]->parameterValue ) + 1 );
        }

        if( retVal )
        {
            free_parameterValStruct_t (bus_handle, nval, retVal);
        }

        return ANSC_STATUS_SUCCESS;
    }

    if( retVal )
    {
       free_parameterValStruct_t (bus_handle, nval, retVal);
    }

    return ANSC_STATUS_FAILURE;
}



ANSC_STATUS WanMgr_RdkBus_SetParamValues( char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, BOOLEAN bCommit )
{
    CCSP_MESSAGE_BUS_INFO *bus_info              = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t   param_val[1]          = { 0 };
    char                  *faultParam            = NULL;
    char                   acParameterName[BUFLEN_256]  = { 0 },
                           acParameterValue[BUFLEN_128] = { 0 };
    int                    ret                   = 0;

    //Copy Name
    sprintf( acParameterName, "%s", pParamName );
    param_val[0].parameterName  = acParameterName;

    //Copy Value
    sprintf( acParameterValue, "%s", pParamVal );
    param_val[0].parameterValue = acParameterValue;

    //Copy Type
    param_val[0].type           = type;

    ret = CcspBaseIf_setParameterValues(
                                        bus_handle,
                                        pComponent,
                                        pBus,
                                        0,
                                        0,
                                        param_val,
                                        1,
                                        bCommit,
                                        &faultParam
                                       );

    if( ( ret != CCSP_SUCCESS ) && ( faultParam != NULL ) )
    {
        CcspTraceError(("%s-%d Failed to set %s\n",__FUNCTION__,__LINE__,pParamName));
        bus_info->freefunc( faultParam );
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}

/* * WanMgr_RdkBus_GetInterfaceInstanceInOtherAgent() */
static ANSC_STATUS WanMgr_RdkBus_GetInterfaceInstanceInOtherAgent( WAN_NOTIFY_ENUM enNotifyAgent, char *pIfName, INT *piInstanceNumber )
{
    //Validate buffer
    if( ( NULL == pIfName ) || ( NULL == piInstanceNumber ) )
    {
        CcspTraceError(("%s Invalid Buffer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Initialise default value
    *piInstanceNumber   = -1;

    switch( enNotifyAgent )
    {
        case NOTIFY_TO_VLAN_AGENT:
        {
            char acTmpReturnValue[BUFLEN_256] = { 0 },
                 a2cTmpTableParams[10][BUFLEN_256] = { 0 };
            INT  iLoopCount,
                 iTotalNoofEntries;

            if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValues( VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, VLAN_ETHLINK_NOE_PARAM_NAME, acTmpReturnValue ) )
            {
                CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                return ANSC_STATUS_FAILURE;
            }

            //Total count
            iTotalNoofEntries = atoi( acTmpReturnValue );
            CcspTraceInfo(("%s %d - TotalNoofEntries:%d\n", __FUNCTION__, __LINE__, iTotalNoofEntries));

            if( 0 >= iTotalNoofEntries )
            {
               return ANSC_STATUS_SUCCESS;
            }

            //Get table names
            iTotalNoofEntries = 0;
            if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamNames( VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, VLAN_ETHLINK_TABLE_NAME, a2cTmpTableParams , &iTotalNoofEntries ))
            {
                CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                return ANSC_STATUS_FAILURE;
            }

            //Traverse from loop
            for ( iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++ )
            {
                char acTmpQueryParam[BUFLEN_256] = { 0 };

                //Query
                snprintf( acTmpQueryParam, sizeof(acTmpQueryParam ), "%sAlias", a2cTmpTableParams[ iLoopCount ] );

                memset( acTmpReturnValue, 0, sizeof( acTmpReturnValue ) );
                if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValues( VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acTmpQueryParam, acTmpReturnValue ) )
                {
                    CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                    continue;
                }

                //Compare ifname
                if( 0 == strncmp(acTmpReturnValue, pIfName, BUFLEN_256) )
                {
                    char  tmpTableParam[BUFLEN_256] = { 0 };
                    const char *last_two;

                    //Copy table param
                    snprintf( tmpTableParam, sizeof(tmpTableParam), "%s", a2cTmpTableParams[ iLoopCount ] );

                    //Get last two chareters from return value and cut the instance
                    last_two = &tmpTableParam[strlen(tmpTableParam) - 2];

                    *piInstanceNumber   = atoi(last_two);
                    break;
                }
            }
        }
        break; /* * NOTIFY_TO_VLAN_AGENT */

        default:
        {
            CcspTraceError(("%s Invalid Case\n", __FUNCTION__));
        }
        break; /* * default */
    }

    return ANSC_STATUS_SUCCESS;
}

/* * WanMgr_RdkBus_WanIfRefreshThread() */
void* WanMgr_RdkBus_WanIfRefreshThread( void *arg )
{
    DML_WAN_IFACE*    pstWanIface = (DML_WAN_IFACE*)arg;
    char    acSetParamName[BUFLEN_256];
    INT     iVLANInstance   = -1;

    //Validate buffer
    if( NULL == pstWanIface )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        pthread_exit(NULL);
    }

    //detach thread from caller stack
    pthread_detach(pthread_self());

    //Need to sync with the state machine thread.
    sleep(5);

    //Get Instance for corresponding name
    WanMgr_RdkBus_GetInterfaceInstanceInOtherAgent( NOTIFY_TO_VLAN_AGENT, pstWanIface->Name, &iVLANInstance );

    //Index is not present. so no need to do anything any VLAN instance
    if( -1 != iVLANInstance )
    {
       CcspTraceInfo(("%s %d VLAN Instance:%d\n",__FUNCTION__, __LINE__,iVLANInstance));

        //Set VLAN EthLink Refresh
        memset( acSetParamName, 0, sizeof(acSetParamName) );
        snprintf( acSetParamName, sizeof(acSetParamName), VLAN_ETHLINK_REFRESH_PARAM_NAME, iVLANInstance );
        WanMgr_RdkBus_SetParamValues( VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, "true", ccsp_boolean, TRUE );

        CcspTraceInfo(("%s %d Successfully notified refresh event to VLAN Agent for %s interface[%s]\n", __FUNCTION__, __LINE__, pstWanIface->Name,acSetParamName));
    }

    //Free allocated resource
    if( NULL != pstWanIface )
    {
        free(pstWanIface);
        pstWanIface = NULL;
    }

    pthread_exit(NULL);

    return NULL;
}

ANSC_STATUS DmlGetInstanceByKeywordFromPandM(char *ifname, int *piInstanceNumber)
{
    char acTmpReturnValue[256] = {0};
    int iLoopCount,
        iTotalNoofEntries;
    if (ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValues(PAM_COMPONENT_NAME, PAM_DBUS_PATH, PAM_NOE_PARAM_NAME, acTmpReturnValue))
    {
        CcspTraceError(("[%s][%d]Failed to get param value\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Total count
    iTotalNoofEntries = atoi(acTmpReturnValue);

    if ( 0 >= iTotalNoofEntries )
    {
        return ANSC_STATUS_SUCCESS;
    }

    //Traverse from loop
    for (iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++)
    {
        char acTmpQueryParam[256] = {0};

        //Query
        snprintf(acTmpQueryParam, sizeof(acTmpQueryParam), PAM_IF_PARAM_NAME, iLoopCount + 1);
        memset(acTmpReturnValue, 0, sizeof(acTmpReturnValue));
        if (ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValues(PAM_COMPONENT_NAME, PAM_DBUS_PATH, acTmpQueryParam, acTmpReturnValue))
        {
            CcspTraceError(("[%s][%d] Failed to get param value\n", __FUNCTION__, __LINE__));
            continue;
        }

        //Compare name
        if (0 == strcmp(acTmpReturnValue, ifname))
        {
            *piInstanceNumber = iLoopCount + 1;
             break;
        }
    }

    return ANSC_STATUS_SUCCESS;
}
