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
#include <syscfg/syscfg.h>
#include "dmsb_tr181_psm_definitions.h"
#include "wanmgr_rdkbus_utils.h"
#include "ansc_platform.h"
#include "ccsp_psm_helper.h"
#include "wanmgr_data.h"
#include "wanmgr_dhcpv4_apis.h"
#include "wanmgr_dhcpv6_apis.h"

#define UPSTREAM_SET_MAX_RETRY_COUNT 10 // max. retry count for Upstream set requests

extern char g_Subsystem[32];
extern ANSC_HANDLE bus_handle;
ANSC_STATUS WanMgr_RdkBus_getWanPolicy(DML_WAN_POLICY *wan_policy)
{
    int result = ANSC_STATUS_SUCCESS;
    int retPsmGet = CCSP_SUCCESS;
    char param_value[BUFLEN_256] = {0};
    char param_name[BUFLEN_256]= {0};

    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANPOLICY);
    retPsmGet = WanMgr_RdkBus_GetParamValuesFromDB(param_name, param_value, sizeof(param_value));
    if (retPsmGet == CCSP_SUCCESS && param_value[0] != '\0') {
        *wan_policy = strtol(param_value, NULL, 10);
    }
    else {
        result = ANSC_STATUS_FAILURE;
    }

    return result;
}


ANSC_STATUS WanMgr_RdkBus_setRestorationDelay(UINT delay)
{
    int result = ANSC_STATUS_SUCCESS;
    int retPsmSet = CCSP_SUCCESS;
    char param_name[BUFLEN_256] = {0};
    char param_value[BUFLEN_256] = {0};

    /* Update the wan policy information in PSM */
    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));

    snprintf(param_value, sizeof(param_value), "%d", delay);
    _ansc_sprintf(param_name, PSM_WANMANAGER_RESTORATION_DELAY);

    retPsmSet = WanMgr_RdkBus_SetParamValuesToDB(param_name, param_value);
    if (retPsmSet != CCSP_SUCCESS) {
        AnscTraceError(("%s Error %d writing %s %s\n", __FUNCTION__, retPsmSet, param_name, param_value));
        result = ANSC_STATUS_FAILURE;
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

    retPsmSet = WanMgr_RdkBus_SetParamValuesToDB(param_name, param_value);
    if (retPsmSet != CCSP_SUCCESS) {
        AnscTraceError(("%s Error %d writing %s %s\n", __FUNCTION__, retPsmSet, param_name, param_value));
        result = ANSC_STATUS_FAILURE;
    }

    return result;
}

ANSC_STATUS WanMgr_RdkBus_setWanEnableToPsm(BOOL WanEnable)
{
    int result = ANSC_STATUS_SUCCESS;
    int retPsmSet = CCSP_SUCCESS;
    char param_name[BUFLEN_256] = {0};
    char param_value[BUFLEN_256] = {0};

    /* Update the wan Enable information in PSM */
    memset(param_value, 0, sizeof(param_value));
    memset(param_name, 0, sizeof(param_name));

    snprintf(param_value, sizeof(param_value), "%d", WanEnable);
    _ansc_sprintf(param_name, PSM_WANMANAGER_WANENABLE);

    retPsmSet = WanMgr_RdkBus_SetParamValuesToDB(param_name, param_value);
    if (retPsmSet != CCSP_SUCCESS) {
        AnscTraceError(("%s Error %d writing %s %s\n", __FUNCTION__, retPsmSet, param_name, param_value));
        result = ANSC_STATUS_FAILURE;
    }

    return result;
}

ANSC_STATUS WanMgr_RdkBus_setAllowRemoteIfaceToPsm(BOOL Enable)
{
    int result = ANSC_STATUS_SUCCESS;
    int retPsmSet = CCSP_SUCCESS;
    char param_name[BUFLEN_256] = {0};
    char param_value[BUFLEN_256] = {0};

    /* Update the AllowRemoteIface information in PSM */
    snprintf(param_value, sizeof(param_value), "%d", Enable);
    _ansc_sprintf(param_name, PSM_WANMANAGER_ALLOW_REMOTE_IFACE);

    retPsmSet = WanMgr_RdkBus_SetParamValuesToDB(param_name, param_value);
    if (retPsmSet != CCSP_SUCCESS) {
        AnscTraceError(("%s Error %d writing %s %s\n", __FUNCTION__, retPsmSet, param_name, param_value));
        result = ANSC_STATUS_FAILURE;
    }

    return result;
}

static void checkComponentHealthStatus(char * compName, char * dbusPath, char *status, int *retStatus)
{
    int ret = 0, val_size = 0;
    parameterValStruct_t **parameterval = NULL;
    char *parameterNames[1] = {};
    char tmp[256];
    char str[256];
    char l_Subsystem[128] = { 0 };

    sprintf(tmp,"%s.%s",compName, "Health");
    parameterNames[0] = tmp;

    strncpy(l_Subsystem, "eRT.",sizeof(l_Subsystem));
    snprintf(str, sizeof(str), "%s%s", l_Subsystem, compName);
    CcspTraceDebug(("str is:%s\n", str));

    ret = CcspBaseIf_getParameterValues(bus_handle, str, dbusPath,  parameterNames, 1, &val_size, &parameterval);
    CcspTraceDebug(("ret = %d val_size = %d\n",ret,val_size));
    if(ret == CCSP_SUCCESS)
    {
        CcspTraceDebug(("parameterval[0]->parameterName : %s parameterval[0]->parameterValue : %s\n",parameterval[0]->parameterName,parameterval[0]->parameterValue));
        strncpy(status, parameterval[0]->parameterValue, strlen(parameterval[0]->parameterValue) + 1 );
        CcspTraceDebug(("status of component:%s\n", status));
    }
    free_parameterValStruct_t (bus_handle, val_size, parameterval);

    *retStatus = ret;
}

ANSC_STATUS WaitForInterfaceComponentReady(char *pPhyPath)
{
    char status[32] = {'\0'};
    int count = 0;
    int ret = -1;
    char  pCompName[BUFLEN_64] = {0};
    char  pCompPath[BUFLEN_64] = {0};

    if (pPhyPath == NULL)
    {
        CcspTraceInfo(("%s %d Error: phyPath is NULL \n", __FUNCTION__, __LINE__ ));
        return ANSC_STATUS_FAILURE;
    }
        
    if(strstr(pPhyPath, "CableModem") != NULL) { // CM wan interface
        strncpy(pCompName, CMAGENT_COMP_NAME_WITHOUTSUBSYSTEM, sizeof(pCompName));
        strncpy(pCompPath, CMAGENT_COMPONENT_PATH, sizeof(pCompPath));
    }
    else if(strstr(pPhyPath, "Ethernet") != NULL) { // ethernet wan interface
        strncpy(pCompName, ETH_COMP_NAME_WITHOUTSUBSYSTEM, sizeof(pCompName));
        strncpy(pCompPath, ETH_COMPONENT_PATH, sizeof(pCompPath));
    }
    else if(strstr(pPhyPath, "Cellular") != NULL) { // cellular wan interface
        strncpy(pCompName, CELLULAR_COMP_NAME_WITHOUTSUBSYSTEM, sizeof(pCompName));
        strncpy(pCompPath, CELLULAR_COMPONENT_PATH, sizeof(pCompPath));
    }

    while(1)
    {
        checkComponentHealthStatus(pCompName, pCompPath, status,&ret);
        if(ret == CCSP_SUCCESS && (strcmp(status, "Green") == 0))
        {
            CcspTraceInfo(("%s component health is %s, continue\n", pCompName, status));
            return ANSC_STATUS_SUCCESS;
        }
        else
        {
            count++;
            if(count%5== 0)
            {
                CcspTraceError(("%s component Health, ret:%d, waiting\n", pCompName, ret));
            }
            sleep(5);
        }
    }
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS WanMgr_RdkBus_SetRequestIfComponent(char *pPhyPath, char *pInputparamName, char *pInputParamValue, enum dataType_e type)
{
    char param_name[BUFLEN_256];
    char  pCompName[BUFLEN_64] = {0};
    char  pCompPath[BUFLEN_64] = {0};
    char *faultParam = NULL;
    int ret = 0;

    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t param_info[1] = {0};

    if((pPhyPath == NULL) || (pInputparamName == NULL) || (pInputParamValue == NULL)) {
        CcspTraceInfo(("%s %d Error: phyPath/inputParamName is NULL \n", __FUNCTION__, __LINE__ ));
        return ANSC_STATUS_FAILURE;
    }

    snprintf(param_name, sizeof(param_name), "%s%s", pPhyPath, pInputparamName);

    if(strstr(param_name, "CableModem") != NULL) { // CM wan interface
        strncpy(pCompName, CMAGENT_COMPONENT_NAME, sizeof(pCompName));
        strncpy(pCompPath, CMAGENT_COMPONENT_PATH, sizeof(pCompPath));
    }
    else if(strstr(param_name, "Ethernet") != NULL) { // ethernet wan interface
        strncpy(pCompName, ETH_COMPONENT_NAME, sizeof(pCompName));
        strncpy(pCompPath, ETH_COMPONENT_PATH, sizeof(pCompPath));
    }
    else if(strstr(param_name, "Cellular") != NULL) { // cellular wan interface
        strncpy(pCompName, CELLULAR_COMPONENT_NAME, sizeof(pCompName));
        strncpy(pCompPath, CELLULAR_COMPONENT_PATH, sizeof(pCompPath));
    }

     CcspTraceInfo(("%s: Param Name %s value %s\n", __FUNCTION__,param_name,pInputParamValue));
    param_info[0].parameterName = param_name;
    param_info[0].parameterValue = pInputParamValue;
    param_info[0].type = type;

    ret = CcspBaseIf_setParameterValues(bus_handle, pCompName, pCompPath,
                                        0, 0x0,   /* session id and write id */
                                        param_info, 1, TRUE,   /* Commit  */
                                        &faultParam);

    if ( ( ret != CCSP_SUCCESS ) && ( faultParam )) {
        CcspTraceInfo(("%s CcspBaseIf_setParameterValues failed with error %d\n",__FUNCTION__, ret ));
        bus_info->freefunc( faultParam );
        return ANSC_STATUS_FAILURE;
    }
    
    return ANSC_STATUS_SUCCESS;
}

#ifdef FEATURE_RDKB_AUTO_PORT_SWITCH
void WanMgr_SetPortCapabilityForEthIntf (DML_WAN_POLICY eWanPolicy)
{
    ANSC_STATUS result = ANSC_STATUS_FAILURE;
    char dmQuery[BUFLEN_256] = {0};
    char dmValue[BUFLEN_32]  = {0};
    int foundPhyPath = 0;
    int uiLoopCount;
    int TotalIfaces = WanMgr_IfaceData_GetTotalWanIface();

    if(eWanPolicy == PRIMARY_PRIORITY)
        strcpy(dmValue,"WAN");
    else
        strcpy(dmValue,"WAN_LAN");

    CcspTraceInfo(("%s %d: TotalIfaces:%d \n", __FUNCTION__, __LINE__, TotalIfaces));
    for (uiLoopCount = 0; uiLoopCount < TotalIfaces; uiLoopCount++)
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            if (strstr(pWanIfaceData->Phy.Path,"Ethernet") != NULL)
            {
                foundPhyPath = 1;
                snprintf(dmQuery, sizeof(dmQuery)-1, "%s%s", pWanIfaceData->Phy.Path, ETH_INTERFACE_PORTCAPABILITY);
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }
    if(foundPhyPath)
    {
        CcspTraceInfo(("%s: dmQuery[%s] dmValue[%s]\n", __FUNCTION__, dmQuery,dmValue));
        result = WanMgr_RdkBus_SetParamValues(ETH_COMPONENT_NAME, ETH_COMPONENT_PATH, dmQuery, dmValue, ccsp_string, TRUE );
    }
    CcspTraceInfo(("%s: result[%s]\n", __FUNCTION__, (result != ANSC_STATUS_SUCCESS)?"FAILED":"SUCCESS"));
}
#endif  //FEATURE_RDKB_AUTO_PORT_SWITCH

ANSC_STATUS WanMgr_RdkBus_Get_InterfaceRebootRequired(UINT IfaceIndex, BOOL *RebootRequired)
{
    char acTmpReturnValue[BUFLEN_256] = { 0 };
    char acTmpQueryParam[BUFLEN_256] = { 0 };

    if (IfaceIndex == -1)
    {
        return ANSC_STATUS_FAILURE;
    }

    memset( acTmpReturnValue, 0, BUFLEN_256);
    memset( acTmpQueryParam, 0, BUFLEN_256);

    snprintf( acTmpQueryParam, sizeof(acTmpQueryParam ), ETH_X_RDK_REBOOTREQUIRED_PARAM_NAME, (IfaceIndex+1));
    if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValues( ETH_COMPONENT_NAME, ETH_COMPONENT_PATH, acTmpQueryParam, acTmpReturnValue ) )
    {
        CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, acTmpQueryParam));
        return ANSC_STATUS_FAILURE;
    }
    //UpStream
    *RebootRequired = atoi( acTmpReturnValue );
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS WanMgr_RdkBus_updateInterfaceUpstreamFlag(char *phyPath, BOOL flag)
{
    char param_name[BUFLEN_256] = {0};
    char param_value[BUFLEN_256] = {0};
    char  pCompName[BUFLEN_64] = {0};
    char  pCompPath[BUFLEN_64] = {0};
    char *faultParam = NULL;
    int ret = 0;
    int retry_count = UPSTREAM_SET_MAX_RETRY_COUNT;

    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t upstream_param[1] = {0};
    CcspTraceInfo(("%s %d Upstream[%s]\n", __FUNCTION__, __LINE__, (flag)?"UP":"Down" ));
    if(phyPath == NULL) {
        CcspTraceInfo(("%s %d Error: phyPath is NULL \n", __FUNCTION__, __LINE__ ));
        return ANSC_STATUS_FAILURE;
    }

    strncpy(param_name, phyPath, sizeof(param_name)-1);

    if(strstr(param_name, "DSL") != NULL) { // dsl wan interface
        strncat(param_name, DSL_UPSTREAM_NAME, sizeof(param_name) - strlen(param_name));
        strncpy(pCompName, DSL_COMPONENT_NAME, sizeof(pCompName));
        strncpy(pCompPath, DSL_COMPONENT_PATH, sizeof(pCompPath));
    }
    else if(strstr(param_name, "Ethernet") != NULL) { // ethernet wan interface
        strncat(param_name, ETH_UPSTREAM_NAME, sizeof(param_name) - strlen(param_name));
        strncpy(pCompName, ETH_COMPONENT_NAME, sizeof(pCompName));
        strncpy(pCompPath, ETH_COMPONENT_PATH, sizeof(pCompPath));
    }
    else if(strstr(param_name, "Cellular") != NULL) { // cellular wan interface
        strncat(param_name, CELLULAR_UPSTREAM_NAME, sizeof(param_name) - strlen(param_name));
        strncpy(pCompName, CELLULAR_COMPONENT_NAME, sizeof(pCompName));
        strncpy(pCompPath, CELLULAR_COMPONENT_PATH, sizeof(pCompPath));
    }

    if(flag)
        strncpy(param_value, "true", sizeof(param_value));
    else
        strncpy(param_value, "false", sizeof(param_value));

    upstream_param[0].parameterName = param_name;
    upstream_param[0].parameterValue = param_value;
    upstream_param[0].type = ccsp_boolean;

    while (retry_count--)
    {
        ret = CcspBaseIf_setParameterValues(bus_handle, pCompName, pCompPath,
                                        0, 0x0,   /* session id and write id */
                                        upstream_param, 1, TRUE,   /* Commit  */
                                        &faultParam);

        if ( ( ret != CCSP_SUCCESS ) && ( faultParam )) {
            CcspTraceInfo(("%s CcspBaseIf_setParameterValues failed with error %d\n",__FUNCTION__, ret ));
            bus_info->freefunc( faultParam );
        }
        else {
            break;
        }
        usleep(500000);
    }
    return (ret == CCSP_SUCCESS) ? ANSC_STATUS_SUCCESS : ANSC_STATUS_FAILURE;
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


ANSC_STATUS WanMgr_RdkBus_GetParamValueFromAnyComp( char * pQuery, char *pValue)
{
    if ((pQuery == NULL) || (pValue == NULL))
    {
        CcspTraceError (("%s %d: invalid args..\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    int ret ;
    int size = 0;
    char dst_pathname_cr[BUFLEN_256] = {0};
    componentStruct_t ** ppComponents = NULL;

    snprintf(dst_pathname_cr, sizeof(dst_pathname_cr) - 1, "eRT.%s", CCSP_DBUS_INTERFACE_CR);

    // Get the component name and dbus path which has the data model 
    ret = CcspBaseIf_discComponentSupportingNamespace
        (
         bus_handle,
         dst_pathname_cr,
         pQuery,
         "",
         &ppComponents,
         &size
        );

    if ((ret != CCSP_SUCCESS) || (size <= 0))
    {
        CcspTraceError (("%s %d: CcspBaseIf_discComponentSupportingNamespace() call failed\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    // query the data model from the component
    CcspTraceInfo (("%s %d: quering dm:%s from component:%s of dbuspath:%s\n", 
                __FUNCTION__, __LINE__, pQuery, ppComponents[0]->componentName, ppComponents[0]->dbusPath));
    if (WanMgr_RdkBus_GetParamValues (ppComponents[0]->componentName, ppComponents[0]->dbusPath, pQuery, pValue) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError (("%s %d: CcspBaseIf_discComponentSupportingNamespace() call failed\n", __FUNCTION__, __LINE__));
        free_componentStruct_t(bus_handle, size, ppComponents);
        return ANSC_STATUS_FAILURE;
    }

    free_componentStruct_t(bus_handle, size, ppComponents);
    CcspTraceInfo (("%s %d: dm:%s got value %s\n", __FUNCTION__, __LINE__, pQuery, pValue));

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS WanMgr_RdkBus_GetParamValues( char *pComponent, char *pBus, char *pParamName, char *pReturnVal )
{
    CCSP_MESSAGE_BUS_INFO  *bus_info         = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t   **retVal = NULL;
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
    snprintf( acParameterName,sizeof(acParameterName)-1, "%s", pParamName );
    param_val[0].parameterName  = acParameterName;

    //Copy Value
    snprintf( acParameterValue,sizeof(acParameterValue)-1, "%s", pParamVal );
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
                snprintf( acTmpQueryParam, sizeof(acTmpQueryParam ), "%sX_RDK_BaseInterface", a2cTmpTableParams[ iLoopCount ] );

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
                    sscanf(tmpTableParam, VLAN_ETHLINK_TABLE_FORMAT , piInstanceNumber);
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

int WanMgr_RdkBus_GetParamValuesFromDB( char *pParamName, char *pReturnVal, int returnValLength )
{
    int     retPsmGet     = CCSP_SUCCESS;
    CHAR   *param_value   = NULL, tmpOutput[BUFLEN_256] = {0};

    /* Input Validation */
    if( ( NULL == pParamName) || ( NULL == pReturnVal ) || ( 0 >= returnValLength ) )
    {
        CcspTraceError(("%s Invalid Input Parameters\n",__FUNCTION__));
        return CCSP_FAILURE;
    }

#ifndef WAN_DB_USAGE_BY_SYSCFG
    retPsmGet = PSM_Get_Record_Value2(bus_handle,g_Subsystem, pParamName, NULL, &param_value); 
    if (retPsmGet != CCSP_SUCCESS) { 
        CcspTraceError(("%s Error %d reading %s\n", __FUNCTION__, retPsmGet, pParamName));
    } 
    else { 
        /* Copy DB Value */
        snprintf(pReturnVal, returnValLength, "%s", param_value);
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    } 
#else
    if( 0 == syscfg_get( NULL, pParamName, tmpOutput, sizeof(tmpOutput)) )
    {
        /* Copy DB Value */
        snprintf(pReturnVal, returnValLength, "%s", tmpOutput);
        retPsmGet = CCSP_SUCCESS;
    }
    else
    {
        retPsmGet = CCSP_FAILURE;
    }
#endif

   return retPsmGet;
}

int WanMgr_RdkBus_SetParamValuesToDB( char *pParamName, char *pParamVal )
{
    int     retPsmSet  = CCSP_SUCCESS;

    /* Input Validation */
    if( ( NULL == pParamName) || ( NULL == pParamVal ) )
    {
        CcspTraceError(("%s Invalid Input Parameters\n",__FUNCTION__));
        return CCSP_FAILURE;
    }

#ifndef WAN_DB_USAGE_BY_SYSCFG
    retPsmSet = PSM_Set_Record_Value2(bus_handle,g_Subsystem, pParamName, ccsp_string, pParamVal); 
    if (retPsmSet != CCSP_SUCCESS) { 
        CcspTraceError(("%s Error %d writing %s\n", __FUNCTION__, retPsmSet, pParamName));
    } 
#else
    if ( syscfg_set_commit(NULL, pParamName, pParamVal) != 0 )
    {
        CcspTraceError(("%s Error writing %s\n", __FUNCTION__,pParamName));
        retPsmSet  = CCSP_FAILURE;
    }
#endif

    return retPsmSet;
}

ANSC_STATUS WanMgr_RestartGetPhyStatus (DML_WAN_IFACE *pWanIfaceData)
{
    //get PHY status
    char dmQuery[BUFLEN_256] = {0};
    char dmValue[BUFLEN_256] = {0};

    if(pWanIfaceData->Phy.Path == NULL)
    {
        CcspTraceError(("%s %d: Invalid args\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    if(strstr(pWanIfaceData->Phy.Path, "Cellular") != NULL)
    {
        snprintf(dmQuery, sizeof(dmQuery)-1, "%s%s", pWanIfaceData->Phy.Path, CELLULARMGR_PHY_STATUS_DM_SUFFIX);
        if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
        {
            CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
            return ANSC_STATUS_FAILURE;
        }

        if(strcmp(dmValue,"true") == 0)
        {
            pWanIfaceData->Phy.Status = WAN_IFACE_PHY_STATUS_UP;
        }

    }
    else
    {
        snprintf(dmQuery, sizeof(dmQuery)-1, "%s%s", pWanIfaceData->Phy.Path, STATUS_DM_SUFFIX);
        if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
        {
            CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
            return ANSC_STATUS_FAILURE;
        }

        if(strcmp(dmValue,"Up") == 0)
        {
            pWanIfaceData->Phy.Status = WAN_IFACE_PHY_STATUS_UP;
        }
    }
    return ANSC_STATUS_SUCCESS;
}

/**************************************
  WanMgr_RestartUpdatePPPinfo 
  If already PPP session running.Updates PPP session details and returns TRUE.
  else return FALSE.
 ***************************************/
bool WanMgr_RestartUpdatePPPinfo(DML_WAN_IFACE *pWanIfaceData)
{
    char dmQuery[BUFLEN_256] = {0};
    char ppp_Status[BUFLEN_256]             = {0};
    char ppp_ConnectionStatus[BUFLEN_256]   = {0};

    if(pWanIfaceData->PPP.Enable == TRUE && (strlen(pWanIfaceData->PPP.Path) > 0))
    {
        snprintf(dmQuery, sizeof(dmQuery)-1, "%sStatus", pWanIfaceData->PPP.Path);
        if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, ppp_Status))
        {
            CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
            return FALSE;
        }

        snprintf(dmQuery, sizeof(dmQuery)-1, "%sConnectionStatus", pWanIfaceData->PPP.Path);
        if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, ppp_ConnectionStatus))
        {
            CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
            return FALSE;
        }

        CcspTraceInfo(("%s %d PPP entry Status: %s, ConnectionStatus: %s\n",__FUNCTION__, __LINE__, ppp_Status, ppp_ConnectionStatus));

        if(!strcmp(ppp_Status, "Up") && !strcmp(ppp_ConnectionStatus, "Connected"))
        {
            pWanIfaceData->PPP.LinkStatus = WAN_IFACE_PPP_LINK_STATUS_UP;
            pWanIfaceData->PPP.LCPStatus = WAN_IFACE_LCP_STATUS_UP;
            pWanIfaceData->PPP.IPCPStatus = WAN_IFACE_IPCP_STATUS_UP;
            pWanIfaceData->PPP.IPV6CPStatus = WAN_IFACE_IPV6CP_STATUS_UP;

            //Start IPCP Status handler threads
            {
                pthread_t IPCPHandlerThread;
                char *pInterface = NULL;
                int iErrorCode = 0;
                pInterface = (char *) malloc(64);

                if (pInterface != NULL)
                {
                    strncpy(pInterface, pWanIfaceData->Wan.Name, 64);
                    iErrorCode = pthread_create( &IPCPHandlerThread, NULL, &IPCPStateChangeHandler, (void*) pInterface );
                    if( 0 != iErrorCode )
                    {
                        CcspTraceInfo(("%s %d - Failed to handle IPCP event change   %d\n", __FUNCTION__, __LINE__, iErrorCode ));
                    }

                }
            }

            //Start IPCPv6 Status handler threads
            {
                pthread_t IPV6CPHandlerThread;
                char *pInterface = NULL;
                int iErrorCode = 0;
                pInterface = (char *) malloc(64);

                if (pInterface != NULL)
                {
                    strncpy(pInterface, pWanIfaceData->Wan.Name, 64);
                    iErrorCode = pthread_create( &IPV6CPHandlerThread, NULL, &IPV6CPStateChangeHandler, (void*) pInterface );
                    if( 0 != iErrorCode )
                    {
                        CcspTraceInfo(("%s %d - Failed to handle IPCP event change   %d\n", __FUNCTION__, __LINE__, iErrorCode ));
                    }

                }
            }
            return TRUE;
        }
    }
    return FALSE;

}

ANSC_STATUS WanMgr_RestartGetLinkStatus (DML_WAN_IFACE *pWanIfaceData)
{
    //get PHY status
    char dmQuery[BUFLEN_256] = {0};
    char dmValue[BUFLEN_256] = {0};

    if(pWanIfaceData->PPP.Enable == TRUE)
    {
        if((strlen(pWanIfaceData->PPP.Path) <= 0))
        {
            return ANSC_STATUS_FAILURE;
        }

        snprintf(dmQuery, sizeof(dmQuery)-1, "%sEnable", pWanIfaceData->PPP.Path);
        if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
        {
            CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
            return ANSC_STATUS_FAILURE;
        }

        if(!strcmp(dmValue, "true"))
        {
            pWanIfaceData->Wan.LinkStatus = WAN_IFACE_LINKSTATUS_UP;
        }

        //Get Wan.name
        memset(dmQuery, 0, sizeof(dmQuery));
        memset(dmValue, 0, sizeof(dmValue));

        snprintf(dmQuery, sizeof(dmQuery)-1,"%sAlias", pWanIfaceData->PPP.Path);

        if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
        {
            CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
            return ANSC_STATUS_FAILURE;
        }

        if(dmValue != NULL)
        {
            strncpy(pWanIfaceData->Wan.Name, dmValue, sizeof(pWanIfaceData->Wan.Name));
        }
    }
    else if(strstr(pWanIfaceData->Phy.Path, "Cellular") != NULL)
    {
        snprintf(dmQuery, sizeof(dmQuery)-1, "%s%s", pWanIfaceData->Phy.Path, CELLULARMGR_LINK_STATUS_DM_SUFFIX);
        if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
        {
            CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
            return ANSC_STATUS_FAILURE;
        }

        if(strcmp(dmValue,"true") == 0)
        {
            pWanIfaceData->Wan.LinkStatus = WAN_IFACE_LINKSTATUS_UP;

            strncpy(pWanIfaceData->Wan.Name, CELLULARMGR_WAN_NAME, sizeof(pWanIfaceData->Wan.Name));
        }
    }
    else
    {
        //get Vlan status
        INT     iVLANInstance   = -1;
        WanMgr_RdkBus_GetInterfaceInstanceInOtherAgent( NOTIFY_TO_VLAN_AGENT, pWanIfaceData->Name, &iVLANInstance );

        //Index is not present. so no need to do anything any VLAN instance
        if( -1 != iVLANInstance )
        {
            CcspTraceInfo(("%s %d VLAN Instance:%d\n",__FUNCTION__, __LINE__,iVLANInstance));

            snprintf(dmQuery, sizeof(dmQuery)-1,VLAN_ETHLINK_STATUS_PARAM_NAME, iVLANInstance);

            if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
            {
                CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
                return ANSC_STATUS_FAILURE;
            }

            if(strcmp(dmValue,"Up") == 0)
            {
                pWanIfaceData->Wan.LinkStatus = WAN_IFACE_LINKSTATUS_UP;
            }

            //Get Vlan interface name
            memset(dmQuery, 0, sizeof(dmQuery));
            memset(dmValue, 0, sizeof(dmValue));

            snprintf(dmQuery, sizeof(dmQuery)-1,VLAN_ETHLINK_NAME_PARAM_NAME, iVLANInstance);

            if ( ANSC_STATUS_FAILURE == WanMgr_RdkBus_GetParamValueFromAnyComp (dmQuery, dmValue))
            {
                CcspTraceError(("%s-%d: %s, Failed to get param value\n", __FUNCTION__, __LINE__, dmQuery));
                return ANSC_STATUS_FAILURE;
            }

            if(dmValue != NULL)
            {
                strncpy(pWanIfaceData->Wan.Name, dmValue, sizeof(pWanIfaceData->Wan.Name));  
            }
        }
    }
    return ANSC_STATUS_SUCCESS;
}
ANSC_STATUS WanMgr_RdkBus_setDhcpv6DnsServerInfo(void)
{
    ANSC_STATUS retStatus = ANSC_STATUS_FAILURE;
    char sysevent_buf[BUFLEN_128] = {'\0'};

    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_ULA_ADDRESS, sysevent_buf, BUFLEN_128);
    if(sysevent_buf[0] != '\0')
    {
        retStatus = WanMgr_RdkBus_SetParamValues( PAM_COMPONENT_NAME, PAM_DBUS_PATH, "Device.DHCPv6.Server.Pool.1.X_RDKCENTRAL-COM_DNSServersEnabled", "true", ccsp_boolean, TRUE );
        if(retStatus == ANSC_STATUS_SUCCESS)
        {
            retStatus = WanMgr_RdkBus_SetParamValues( PAM_COMPONENT_NAME, PAM_DBUS_PATH, "Device.DHCPv6.Server.Pool.1.X_RDKCENTRAL-COM_DNSServers", sysevent_buf, ccsp_string, TRUE );
            if(retStatus != ANSC_STATUS_SUCCESS)
            {
                CcspTraceError(("%s %d - SetDataModelParameter() failed for X_RDKCENTRAL-COM_DNSServers parameter \n", __FUNCTION__, __LINE__));
            }
        }
        else
        {
            CcspTraceError(("%s %d - SetDataModelParameter() failed for X_RDKCENTRAL-COM_DNSServersEnabled parameter \n", __FUNCTION__, __LINE__));
        }
    } 
    else
    {
        CcspTraceInfo(("%s %d - sysevent ula_address is empty \n", __FUNCTION__, __LINE__));
    }
    return retStatus;
}
#if defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
ANSC_STATUS WanMgr_RdkBus_setWanIpInterfaceData(DML_WAN_IFACE* pWanIfaceData)
{
    ANSC_STATUS retStatus = ANSC_STATUS_FAILURE;

    if(pWanIfaceData->PPP.Enable == TRUE)
    {
        retStatus = WanMgr_RdkBus_SetParamValues( PAM_COMPONENT_NAME, PAM_DBUS_PATH, "Device.IP.Interface.1.LowerLayers", pWanIfaceData->PPP.Path, ccsp_string, TRUE );
        CcspTraceInfo(("%s %d - Updating Device.IP.Interface.1.LowerLayers  => %s\n", __FUNCTION__, __LINE__, pWanIfaceData->PPP.Path));
    }else
    {
        INT     iVLANInstance   = -1;
        CHAR    VlanPath[BUFLEN_264]  = {0};
        WanMgr_RdkBus_GetInterfaceInstanceInOtherAgent( NOTIFY_TO_VLAN_AGENT, pWanIfaceData->Name, &iVLANInstance );
        snprintf( VlanPath, sizeof(VlanPath), VLAN_ETHLINK_TABLE_FORMAT, iVLANInstance);

        retStatus = WanMgr_RdkBus_SetParamValues( PAM_COMPONENT_NAME, PAM_DBUS_PATH, "Device.IP.Interface.1.LowerLayers", VlanPath, ccsp_string, TRUE );
        CcspTraceInfo(("%s %d - Updating Device.IP.Interface.1.LowerLayers  => %s\n", __FUNCTION__, __LINE__,VlanPath));
    }
    return retStatus;
}
#endif
