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

#include "ansc_platform.h"
#include "wanmgr_dml_dhcpv4.h"
#include "wanmgr_dhcpv4_apis.h"
#include "wanmgr_dhcpv4_internal.h"
#include "wanmgr_plugin_main_apis.h"

#include "ccsp_base_api.h"
#include "messagebus_interface_helper.h"

extern WANMGR_BACKEND_OBJ* g_pWanMgrBE;
extern void* g_pDslhDmlAgent;

extern ULONG g_currentBsUpdate;
extern char * getRequestorString();
extern char * getTime();

#define BS_SOURCE_WEBPA_STR "webpa"
#define BS_SOURCE_RFC_STR "rfc"
#define  PARTNER_ID_LEN  64

#define DATAMODEL_PARAM_LENGTH 256


/***********************************************************************
 IMPORTANT NOTE:

 According to TR69 spec:
 On successful receipt of a SetParameterValues RPC, the CPE MUST apply 
 the changes to all of the specified Parameters atomically. That is, either 
 all of the value changes are applied together, or none of the changes are 
 applied at all. In the latter case, the CPE MUST return a fault response 
 indicating the reason for the failure to apply the changes. 
 
 The CPE MUST NOT apply any of the specified changes without applying all 
 of them.

 In order to set parameter values correctly, the back-end is required to
 hold the updated values until "Validate" and "Commit" are called. Only after
 all the "Validate" passed in different objects, the "Commit" will be called.
 Otherwise, "Rollback" will be called instead.

 The sequence in COSA Data Model will be:

 SetParamBoolValue/SetParamIntValue/SetParamUlongValue/SetParamStringValue
 -- Backup the updated values;

 if( Validate_XXX())
 {
     Commit_XXX();    -- Commit the update all together in the same object
 }
 else
 {
     Rollback_XXX();  -- Remove the update at backup;
 }
 
***********************************************************************/
/***********************************************************************

 APIs for Object:

    DHCPv4.

    *  DHCPv4_GetParamBoolValue
    *  DHCPv4_GetParamIntValue
    *  DHCPv4_GetParamUlongValue
    *  DHCPv4_GetParamStringValue

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        DHCPv4_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
DHCPv4_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pBool);
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        DHCPv4_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
DHCPv4_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pInt);
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        DHCPv4_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
DHCPv4_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(puLong);
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        DHCPv4_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
DHCPv4_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pValue);
    UNREFERENCED_PARAMETER(pUlSize);

    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/***********************************************************************

 APIs for Object:

    DHCPv4.Client.{i}.

    *  Client_GetEntryCount
    *  Client_GetEntry
    *  Client_AddEntry
    *  Client_DelEntry
    *  Client_GetParamBoolValue
    *  Client_GetParamIntValue
    *  Client_GetParamUlongValue
    *  Client_GetParamStringValue
    *  Client_SetParamBoolValue
    *  Client_SetParamIntValue
    *  Client_SetParamUlongValue
    *  Client_SetParamStringValue
    *  Client_Validate
    *  Client_Commit
    *  Client_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
ULONG
Client_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    ULONG ClientCount = 0;
	PWAN_DHCPV4_DATA  pDhcpv4 = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;

    if (pDhcpv4 != NULL)
    {
        ClientCount = AnscSListQueryDepth( &pDhcpv4->ClientList );
    }
    return ClientCount;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        Client_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/
ANSC_HANDLE
Client_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    PWAN_DHCPV4_DATA           pDhcpv4     = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = NULL;

    UNREFERENCED_PARAMETER(hInsContext);

    if (pDhcpv4 != NULL)
    {
        pSListEntry = AnscSListGetEntryByIndex(&pDhcpv4->ClientList, nIndex);

        if ( pSListEntry )
        {
            pCxtLink          = ACCESS_CONTEXT_DHCPC_LINK_OBJECT(pSListEntry);
            *pInsNumber       = pCxtLink->InstanceNumber;
        }
    }

    return pSListEntry;

}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        Client_AddEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to add a new entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle of new added entry.

**********************************************************************/
ANSC_HANDLE
Client_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
	return WanMgr_DmlDhcpcAddEntry(hInsContext, *pInsNumber);

}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client_DelEntry
            (
                ANSC_HANDLE                 hInsContext,
                ANSC_HANDLE                 hInstance
            );

    description:

        This function is called to delete an exist entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ANSC_HANDLE                 hInstance
                The exist entry handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Client_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
	PWAN_DHCPV4_DATA     pDhcpv4       = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;
    UNREFERENCED_PARAMETER(hInsContext);

    if (pDhcpv4 != NULL)
    {
        PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInstance;
        PCONTEXT_LINK_OBJECT       pCxtLink2         = NULL;
        PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;

        /* Normally, two sublinks are empty because our framework will firstly 
           call delEntry for them before coming here. We needn't care them.
           */
        if ( !pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpcDelEntry(NULL, pDhcpc->Cfg.InstanceNumber);
            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                return returnStatus;
            }
        }

        if (AnscSListPopEntryByLink(&pDhcpv4->ClientList, &pCxtLink->Linkage) )
        {
            /* Because the current NextInstanceNumber information need be deleted */
            WanMgr_Dhcpv4RegSetDhcpv4Info(pDhcpv4);

            AnscFreeMemory(pCxtLink->hContext);
            AnscFreeMemory(pCxtLink);
        }
    }

    return returnStatus;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Client_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* collect value */
        *pBool   = pDhcpc->Cfg.bEnabled;
        
        return TRUE;
    }

    else if (strcmp(ParamName, "Renew") == 0)
    {
        /* collect value */
        *pBool   = FALSE;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Client_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "LeaseTimeRemaining") == 0)
    {
        /* collect value */
        WanMgr_DmlDhcpcGetInfo(NULL, pCxtLink->InstanceNumber, &pDhcpc->Info);
        
        *pInt   = pDhcpc->Info.LeaseTimeRemaining;
        
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Client_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;


    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Status") == 0)
    {
        /* collect value */
        WanMgr_DmlDhcpcGetInfo(NULL, pCxtLink->InstanceNumber, &pDhcpc->Info);

        *puLong   = pDhcpc->Info.Status;
        
        return TRUE;
    }

    else if (strcmp(ParamName, "DHCPStatus") == 0)
    {
        /* collect value */
        WanMgr_DmlDhcpcGetInfo(NULL, pCxtLink->InstanceNumber, &pDhcpc->Info);

        *puLong   = pDhcpc->Info.DHCPStatus;
        
        return TRUE;
    }

    else if (strcmp(ParamName, "IPAddress") == 0)
    {
        /* collect value */
        WanMgr_DmlDhcpcGetInfo(NULL, pCxtLink->InstanceNumber, &pDhcpc->Info);
        if ( pDhcpc->Info.DHCPStatus == DML_DHCPC_STATUS_Bound )
        {
            *puLong = pDhcpc->Info.IPAddress.Value;
        }
        else
        {
            *puLong = 0;
        }
        
        
        return TRUE;
    }

	else if (strcmp(ParamName, "SubnetMask") == 0)
    {
        /* collect value */
        WanMgr_DmlDhcpcGetInfo(NULL, pCxtLink->InstanceNumber, &pDhcpc->Info);
        if ( pDhcpc->Info.DHCPStatus == DML_DHCPC_STATUS_Bound )
        {
            *puLong = pDhcpc->Info.SubnetMask.Value;
        }
        else
        {
            *puLong = 0;
        }
        
        return TRUE;
    }

	else if (strcmp(ParamName, "DHCPServer") == 0)
    {
        /* collect value */
        if ( !pDhcpc->Info.DHCPServer.Value )
        {
            WanMgr_DmlDhcpcGetInfo(NULL, pCxtLink->InstanceNumber, &pDhcpc->Info);
        }
        
        *puLong = pDhcpc->Info.DHCPServer.Value;
        
        return TRUE;
    }
    
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
Client_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;

    ULONG i, len                            = 0;
	INT   instanceNumber                    = -1;
	CHAR  tmpBuff[DATAMODEL_PARAM_LENGTH]   = {0};
    CHAR                            interface[DATAMODEL_PARAM_LENGTH] = {0};

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString(pDhcpc->Cfg.Alias) < *pUlSize)
        {
            AnscCopyString(pValue, pDhcpc->Cfg.Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDhcpc->Cfg.Alias)+1;
            return 1;
        }
    }

	else if (strcmp(ParamName, "X_CISCO_COM_BootFileName") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString(pDhcpc->Cfg.X_CISCO_COM_BootFileName) < *pUlSize)
        {
            AnscCopyString(pValue, pDhcpc->Cfg.X_CISCO_COM_BootFileName);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDhcpc->Cfg.X_CISCO_COM_BootFileName)+1;
            return 1;
        }
    }

	else if (strcmp(ParamName, "Interface") == 0)
    {
        DmlGetInstanceByKeywordFromPandM(pDhcpc->Cfg.Interface, &instanceNumber);
        snprintf(interface, DATAMODEL_PARAM_LENGTH, PAM_IF_TABLE_OBJECT, instanceNumber);

        if ( interface[0] != '\0' )
        {
            if ( AnscSizeOfString(interface) < *pUlSize)
            {
                AnscCopyString(pValue, interface);
                return 0;
            }
            else
            {
                *pUlSize = AnscSizeOfString(interface)+1;
                
                return 1;
            }
        }
        else
        {
            return 0;
        }

    }

	else if (strcmp(ParamName, "IPRouters") == 0)
    {
        /* collect value */
        WanMgr_DmlDhcpcGetInfo(NULL, pCxtLink->InstanceNumber, &pDhcpc->Info);
        if ( pDhcpc->Info.DHCPStatus != DML_DHCPC_STATUS_Bound && (strlen(_ansc_inet_ntoa(*((struct in_addr *)(&pDhcpc->Info.IPRouters[0])))) == 0))
        {
            *pValue    = '\0';
            return 0;
        }
        
        AnscZeroMemory(tmpBuff, sizeof(tmpBuff));
        for( i=0; i<pDhcpc->Info.NumIPRouters && i<DML_DHCP_MAX_ENTRIES; i++)
        {
            len = AnscSizeOfString(tmpBuff);
            
            if(i > 0)
                tmpBuff[len++] = ',';
          
            AnscCopyString( &tmpBuff[len], _ansc_inet_ntoa(*((struct in_addr *)(&pDhcpc->Info.IPRouters[i]))) );
        }
        
        if ( AnscSizeOfString(tmpBuff) < *pUlSize)
        {
            AnscCopyString(pValue, tmpBuff);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDhcpc->Cfg.Interface)+1;
            return 1;
        }

        return 0;
    }

	else if (strcmp(ParamName, "DNSServers") == 0)
    {
        /* collect value */
        
        if ( pDhcpc->Info.DHCPStatus == DML_DHCPC_STATUS_Bound )
        {
            WanMgr_DmlDhcpcGetInfo(NULL, pCxtLink->InstanceNumber, &pDhcpc->Info);
        }
        else
        {
            *pValue    = '\0';
            return 0;
        }
        
        AnscZeroMemory(tmpBuff, sizeof(tmpBuff));
        for( i=0; i<pDhcpc->Info.NumDnsServers && i<DML_DHCP_MAX_ENTRIES; i++)
        {
            len = AnscSizeOfString(tmpBuff);
            
            if(i > 0)
                tmpBuff[len++] = ',';
          
            AnscCopyString( &tmpBuff[len], _ansc_inet_ntoa(*((struct in_addr *)(&pDhcpc->Info.DNSServers[i]))) );
        }
        
        if ( AnscSizeOfString(tmpBuff) < *pUlSize)
        {
            AnscCopyString(pValue, tmpBuff);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDhcpc->Cfg.Interface)+1;
            return 1;
        }

        return 0;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Client_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* save update to backup */
        pDhcpc->Cfg.bEnabled = bValue;
        
        return  TRUE;
    }

	else if (strcmp(ParamName, "Renew") == 0)
    {
        /* save update to backup */
        if ( bValue )
        {
            returnStatus = WanMgr_DmlDhcpcRenew(NULL, pDhcpc->Cfg.InstanceNumber);
            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                return  FALSE;
            }
        }
        
        return  TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return  FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client_SetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int                         iValue
            );

    description:

        This function is called to set integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int                         iValue
                The updated integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Client_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(iValue);

    /* check the parameter name and set the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG                       uValue
            );

    description:

        This function is called to set ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Client_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(uValue);

    /* check the parameter name and set the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );

    description:

        This function is called to set string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pString
                The updated string value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
Client_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
	PWAN_DHCPV4_DATA           pDhcpv4           = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    PDML_DHCPC_FULL            pDhcpc2           = NULL;
    BOOL                            bFound            = FALSE;

    if (pDhcpv4 == NULL)
    {
        AnscTraceError(("%s:%d:: Pointer is null!!\n", __FUNCTION__, __LINE__));
        return FALSE;
    }
    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        AnscCopyString(pDhcpv4->AliasOfClient, pDhcpc->Cfg.Alias);
        AnscCopyString(pDhcpc->Cfg.Alias, pString);
        return TRUE;
    }
    else if (strcmp(ParamName, "X_CISCO_COM_BootFileName") == 0)
    {
        AnscCopyString(pDhcpc->Cfg.X_CISCO_COM_BootFileName, pString);
        
        return TRUE;
    }

	else if (strcmp(ParamName, "Interface") == 0)
    {
        /* save update to backup */
        AnscCopyString(pDhcpc->Cfg.Interface, pString);

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation. 

                ULONG*                      puLength
                The output length of the param name. 

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
Client_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    PDML_DHCPC_FULL            pDhcpc2           = NULL;
    BOOL                            bFound            = FALSE;

    UNREFERENCED_PARAMETER(puLength);

    /*  only for Alias */
	PWAN_DHCPV4_DATA  pDhcpv4 = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;
    if ( pDhcpv4 != NULL && pDhcpv4->AliasOfClient[0] )
    {
        pSListEntry           = AnscSListGetFirstEntry(&pDhcpv4->ClientList);
        while( pSListEntry != NULL)
        {
            pCxtLink          = ACCESS_CONTEXT_DHCPC_LINK_OBJECT(pSListEntry);
            pSListEntry       = AnscSListGetNextEntry(pSListEntry);

            pDhcpc2 = (PDML_DHCPC_FULL)pCxtLink->hContext;

            if( DHCPV4_CLIENT_ENTRY_MATCH2(pDhcpc2->Cfg.Alias, pDhcpc->Cfg.Alias) )
            {
                if ( (ANSC_HANDLE)pCxtLink == hInsContext )
                {
                    continue;
                }

                strncpy(pReturnParamName, "Alias",sizeof("Alias"));

                bFound = TRUE;

                break;
            }
        }

        if ( bFound )
        {
#if COSA_DHCPV4_ROLLBACK_TEST
            Client_Rollback(hInsContext);
#endif
            return FALSE;
        }
    }

    /* some other checking */

    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Client_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;
    PWAN_DHCPV4_DATA           pDhcpv4           = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;

    if (pDhcpv4 != NULL)
    {
        if ( pCxtLink->bNew )
        {
            pCxtLink->bNew = FALSE;
            WanMgr_Dhcpv4RegSetDhcpv4Info(pDhcpv4);
        }
        else
        {
            returnStatus = WanMgr_DmlDhcpcSetCfg(NULL, &pDhcpc->Cfg);

            if ( returnStatus != ANSC_STATUS_SUCCESS)
            {
                WanMgr_DmlDhcpcGetCfg(NULL, &pDhcpc->Cfg);
            }
        }

        AnscZeroMemory( pDhcpv4->AliasOfClient, sizeof(pDhcpv4->AliasOfClient) );
    }

    return returnStatus;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a 
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Client_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;
	PWAN_DHCPV4_DATA           pDhcpv4           = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;

    if (pDhcpv4 != NULL)
    {
        if ( pDhcpv4->AliasOfClient[0] )
            AnscCopyString( pDhcpc->Cfg.Alias, pDhcpv4->AliasOfClient );

        if ( !pCxtLink->bNew )
        {
            WanMgr_DmlDhcpcGetCfg( NULL, &pDhcpc->Cfg );
        }
        else
        {
            DHCPV4_CLIENT_SET_DEFAULTVALUE(pDhcpc);
        }

        AnscZeroMemory( pDhcpv4->AliasOfClient, sizeof(pDhcpv4->AliasOfClient) );
    }

    return returnStatus;
}

/***********************************************************************

 APIs for Object:

    DHCPv4.Client.{i}.SentOption.{i}.

    *  SentOption_GetEntryCount
    *  SentOption_GetEntry
    *  SentOption_AddEntry
    *  SentOption_DelEntry
    *  SentOption_GetParamBoolValue
    *  SentOption_GetParamIntValue
    *  SentOption_GetParamUlongValue
    *  SentOption_GetParamStringValue
    *  SentOption_SetParamBoolValue
    *  SentOption_SetParamIntValue
    *  SentOption_SetParamUlongValue
    *  SentOption_SetParamStringValue
    *  SentOption_Validate
    *  SentOption_Commit
    *  SentOption_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
ULONG
SentOption_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;

    return AnscSListQueryDepth( &pCxtLink->SendOptionList );
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption_GetEntryStatus
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
BOOL
SentOption_GetEntryStatus
    (
        ANSC_HANDLE                 hInsContext,
        PCHAR                       StatusName
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;

    if (strcmp(StatusName, "Committed") == 0)
    {
        /* collect value */
        if ( pCxtLink->bNew )
            return FALSE;
        
    }

    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        SentOption_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/
ANSC_HANDLE
SentOption_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink        = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PCONTEXT_LINK_OBJECT       pCxtLink             = NULL;
    PSINGLE_LINK_ENTRY              pSListEntry          = NULL;

    pSListEntry = AnscSListGetEntryByIndex(&pCxtDhcpcLink->SendOptionList, nIndex);

    if ( pSListEntry )
    {
        pCxtLink          = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
        *pInsNumber       = pCxtLink->InstanceNumber;
    }

    return pSListEntry;

}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        SentOption_AddEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to add a new entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle of new added entry.

**********************************************************************/
ANSC_HANDLE
SentOption_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink        = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc               = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;
    PCONTEXT_LINK_OBJECT       pCxtLink             = NULL;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption      = NULL;
    
    pDhcpSendOption  = (PCOSA_DML_DHCP_OPT)AnscAllocateMemory( sizeof(COSA_DML_DHCP_OPT) );
    if ( !pDhcpSendOption )
    {
        goto EXIT2;
    }

    DHCPV4_SENDOPTION_SET_DEFAULTVALUE(pDhcpSendOption);
    
    pCxtLink = (PCONTEXT_LINK_OBJECT)AnscAllocateMemory( sizeof(CONTEXT_LINK_OBJECT) );
    if ( !pCxtLink )
    {
        goto EXIT1;
    }

    pCxtLink->hContext       = (ANSC_HANDLE)pDhcpSendOption;
    pCxtLink->hParentTable   = (ANSC_HANDLE)pCxtDhcpcLink;
    pCxtLink->bNew           = TRUE;
    
    if ( !++pCxtDhcpcLink->maxInstanceOfSend )
    {
        pCxtDhcpcLink->maxInstanceOfSend = 1;
    }
    
    pDhcpSendOption->InstanceNumber = pCxtDhcpcLink->maxInstanceOfSend; 
    pCxtLink->InstanceNumber       = pDhcpSendOption->InstanceNumber;
    *pInsNumber                    = pDhcpSendOption->InstanceNumber;

    _ansc_sprintf( pDhcpSendOption->Alias, "SentOption%d", pDhcpSendOption->InstanceNumber);

    /* Put into our list */
    SListPushEntryByInsNum(&pCxtDhcpcLink->SendOptionList, (PCONTEXT_LINK_OBJECT)pCxtLink);

    /* we recreate the configuration */
	PWAN_DHCPV4_DATA pDhcpv4 = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;
    if (pDhcpv4 != NULL)
    {
        WanMgr_Dhcpv4RegSetDhcpv4Info(pDhcpv4);
    }

    return (ANSC_HANDLE)pCxtLink;    
       
EXIT1:
        
    AnscFreeMemory(pDhcpSendOption);
    
EXIT2:   
        
    return NULL;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption_DelEntry
            (
                ANSC_HANDLE                 hInsContext,
                ANSC_HANDLE                 hInstance
            );

    description:

        This function is called to delete an exist entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ANSC_HANDLE                 hInstance
                The exist entry handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
SentOption_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink        = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpClient          = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInstance;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption      = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;
    PWAN_DHCPV4_DATA           pDhcpv4              = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;

    if (pDhcpv4 != NULL)
    {
        if ( !pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpcDelSentOption(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpSendOption->InstanceNumber);
            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                return returnStatus;
            }
        }

        if ( AnscSListPopEntryByLink(&pCxtDhcpcLink->SendOptionList, &pCxtLink->Linkage) )
        {
            WanMgr_Dhcpv4RegSetDhcpv4Info(pDhcpv4);

            AnscFreeMemory(pCxtLink->hContext);
            AnscFreeMemory(pCxtLink);
        }
    }

    return returnStatus;

}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SentOption_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption      = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* collect value */
        *pBool  = pDhcpSendOption->bEnabled;
        
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SentOption_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pInt);

    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SentOption_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption      = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Tag") == 0)
    {
        /* collect value */
        *puLong = pDhcpSendOption->Tag;
        
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
SentOption_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption      = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString(pDhcpSendOption->Alias) < *pUlSize)
        {
            AnscCopyString(pValue, pDhcpSendOption->Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDhcpSendOption->Alias)+1;
            return 1;
        }
    }

	else if (strcmp(ParamName, "Value") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpSendOption->Value) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpSendOption->Value);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpSendOption->Value)+1;
            return 1;
        }
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SentOption_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption      = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* save update to backup */
        pDhcpSendOption->bEnabled  = bValue;
        
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption_SetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int                         iValue
            );

    description:

        This function is called to set integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int                         iValue
                The updated integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SentOption_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(iValue);

    /* check the parameter name and set the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG                       uValue
            );

    description:

        This function is called to set ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SentOption_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption      = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Tag") == 0)
    {
        /* save update to backup */
        pDhcpSendOption->Tag  = (UCHAR)uValue;
        
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );

    description:

        This function is called to set string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pString
                The updated string value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
SentOption_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption   = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PCONTEXT_LINK_OBJECT       pCxtLink2         = NULL;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption2  = NULL;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    BOOL                            bFound            = FALSE;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        AnscCopyString(pCxtDhcpcLink->AliasOfSend, pDhcpSendOption->Alias);

        AnscCopyString(pDhcpSendOption->Alias, pString);
        
        return TRUE;
    }

	else if (strcmp(ParamName, "Value") == 0)
    {
        /* save update to backup */
        AnscCopyString((char*)pDhcpSendOption->Value, pString);

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation. 

                ULONG*                      puLength
                The output length of the param name. 

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
SentOption_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    UNREFERENCED_PARAMETER(puLength);
    PCONTEXT_LINK_OBJECT       pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption   = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PCONTEXT_LINK_OBJECT       pCxtLink2         = NULL;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption2  = NULL;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    BOOL                            bFound            = FALSE;

    /* Parent hasn't set, we don't permit child is set.*/
    if ( pCxtDhcpcLink->bNew )
    {
#if COSA_DHCPV4_ROLLBACK_TEST        
        SentOption_Rollback(hInsContext);
#endif
        return FALSE;
    }

    /* This is for alias */
    if ( pCxtDhcpcLink->AliasOfSend[0] )
    {
        bFound                = FALSE;
        pSListEntry           = AnscSListGetFirstEntry(&pCxtDhcpcLink->SendOptionList);
        while( pSListEntry != NULL)
        {
            pCxtLink2         = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
            pSListEntry       = AnscSListGetNextEntry(pSListEntry);

            pDhcpSendOption2  = (PCOSA_DML_DHCP_OPT)pCxtLink2->hContext;

            if( DHCPV4_SENDOPTION_ENTRY_MATCH2(pDhcpSendOption->Alias, pDhcpSendOption2->Alias) )
            {
                if ( (ANSC_HANDLE)pCxtLink2 == hInsContext )
                {
                    continue;
                }

                strncpy(pReturnParamName, "Alias",sizeof("Alias"));

                bFound = TRUE;
                
                break;
            }
        }
        
        if ( bFound )
        {
#if COSA_DHCPV4_ROLLBACK_TEST        
            SentOption_Rollback(hInsContext);
#endif
            return FALSE;
        }
    }

    /* For other check */

    
    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
SentOption_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption   = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PDML_DHCPC_FULL            pDhcpClient       = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;
	PWAN_DHCPV4_DATA           pDhcpv4           = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;

    if (pDhcpv4 != NULL)
    {
        if ( pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpcAddSentOption(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpSendOption );

            if ( returnStatus == ANSC_STATUS_SUCCESS )
            {
                pCxtLink->bNew = FALSE;

                WanMgr_Dhcpv4RegSetDhcpv4Info(pDhcpv4);
            }
            else
            {
                DHCPV4_SENDOPTION_SET_DEFAULTVALUE(pDhcpSendOption);

                if ( pCxtDhcpcLink->AliasOfSend[0] )
                    AnscCopyString( pDhcpSendOption->Alias, pCxtDhcpcLink->AliasOfSend );
            }
        }
        else
        {
            returnStatus = WanMgr_DmlDhcpcSetSentOption(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpSendOption);

            if ( returnStatus != ANSC_STATUS_SUCCESS)
            {
                WanMgr_DmlDhcpcGetSentOptionbyInsNum(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpSendOption);
            }
        }

        AnscZeroMemory( pCxtDhcpcLink->AliasOfSend, sizeof(pCxtDhcpcLink->AliasOfSend) );
    }

    return returnStatus;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a 
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
SentOption_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PCOSA_DML_DHCP_OPT              pDhcpSendOption   = (PCOSA_DML_DHCP_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;

    if ( pCxtDhcpcLink->AliasOfSend[0] )
        AnscCopyString( pDhcpSendOption->Alias, pCxtDhcpcLink->AliasOfSend );

    if ( !pCxtLink->bNew )
    {
        WanMgr_DmlDhcpcGetSentOptionbyInsNum(NULL, pDhcpc->Cfg.InstanceNumber, pDhcpSendOption);
    }
    else
    {
        DHCPV4_SENDOPTION_SET_DEFAULTVALUE(pDhcpSendOption);
    }
    
    AnscZeroMemory( pCxtDhcpcLink->AliasOfSend, sizeof(pCxtDhcpcLink->AliasOfSend) );
    
    return returnStatus;
}

/***********************************************************************

 APIs for Object:

    DHCPv4.Client.{i}.ReqOption.{i}.

    *  ReqOption_GetEntryCount
    *  ReqOption_GetEntry
    *  ReqOption_AddEntry
    *  ReqOption_DelEntry
    *  ReqOption_GetParamBoolValue
    *  ReqOption_GetParamIntValue
    *  ReqOption_GetParamUlongValue
    *  ReqOption_GetParamStringValue
    *  ReqOption_SetParamBoolValue
    *  ReqOption_SetParamIntValue
    *  ReqOption_SetParamUlongValue
    *  ReqOption_SetParamStringValue
    *  ReqOption_Validate
    *  ReqOption_Commit
    *  ReqOption_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        ReqOption_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/
ULONG
ReqOption_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtLink->hContext;

    return AnscSListQueryDepth( &pCxtLink->ReqOptionList );
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        ReqOption_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/
ANSC_HANDLE
ReqOption_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink        = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PCONTEXT_LINK_OBJECT       pCxtLink             = NULL;
    PSINGLE_LINK_ENTRY              pSListEntry          = NULL;
    
    pSListEntry = AnscSListGetEntryByIndex(&pCxtDhcpcLink->ReqOptionList, nIndex);

    if ( pSListEntry )
    {
        pCxtLink          = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
        *pInsNumber       = pCxtLink->InstanceNumber;
    }

    return pSListEntry;
}


/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        ReqOption_AddEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to add a new entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle of new added entry.

**********************************************************************/
ANSC_HANDLE
ReqOption_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink        = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpc               = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;
    PCONTEXT_LINK_OBJECT       pCxtLink             = NULL;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption       = NULL;
    CHAR                            tmpBuff[64]          = {0};

    /* We need ask from backend */
    pDhcpReqOption  = (PDML_DHCPC_REQ_OPT)AnscAllocateMemory( sizeof(DML_DHCPC_REQ_OPT) );
    if ( !pDhcpReqOption )
    {
        goto EXIT2;
    }

    DHCPV4_REQOPTION_SET_DEFAULTVALUE(pDhcpReqOption);

    pCxtLink = (PCONTEXT_LINK_OBJECT)AnscAllocateMemory( sizeof(CONTEXT_LINK_OBJECT) );
    if ( !pCxtLink )
    {
        goto EXIT1;
    }

    pCxtLink->hContext       = (ANSC_HANDLE)pDhcpReqOption;
    pCxtLink->hParentTable   = (ANSC_HANDLE)pCxtDhcpcLink;
    pCxtLink->bNew           = TRUE;

    if ( !++pCxtDhcpcLink->maxInstanceOfReq )
    {
        pCxtDhcpcLink->maxInstanceOfReq = 1;
    }
    pDhcpReqOption->InstanceNumber = pCxtDhcpcLink->maxInstanceOfReq;
    pCxtLink->InstanceNumber       = pDhcpReqOption->InstanceNumber; 
    *pInsNumber                    = pDhcpReqOption->InstanceNumber;

    _ansc_sprintf( pDhcpReqOption->Alias, "ReqOption%lu", pDhcpReqOption->InstanceNumber);

    /* Put into our list */
    SListPushEntryByInsNum(&pCxtDhcpcLink->ReqOptionList, (PCONTEXT_LINK_OBJECT)pCxtLink);

    /* we recreate the configuration */
	PWAN_DHCPV4_DATA pDhcpv4 = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;
    if (pDhcpv4 != NULL)
    {
        WanMgr_Dhcpv4RegSetDhcpv4Info(pDhcpv4);
    }

    return (ANSC_HANDLE)pCxtLink;    

EXIT1:

    AnscFreeMemory(pDhcpReqOption);

EXIT2:   

    return NULL;
}


/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        ReqOption_DelEntry
            (
                ANSC_HANDLE                 hInsContext,
                ANSC_HANDLE                 hInstance
            );

    description:

        This function is called to delete an exist entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ANSC_HANDLE                 hInstance
                The exist entry handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
ReqOption_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )
{
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink        = (PDHCPC_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_FULL            pDhcpClient          = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInstance;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption       = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;
    PWAN_DHCPV4_DATA           pDhcpv4              = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;

    if (pDhcpv4 != NULL)
    {
        if ( !pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpcDelReqOption( NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpReqOption->InstanceNumber );
            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                return returnStatus;
            }
        }

        if ( AnscSListPopEntryByLink(&pCxtDhcpcLink->ReqOptionList, &pCxtLink->Linkage) )
        {
            WanMgr_Dhcpv4RegSetDhcpv4Info(pDhcpv4);

            AnscFreeMemory(pCxtLink->hContext);
            AnscFreeMemory(pCxtLink);
        }
    }

    return returnStatus;

}


/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReqOption_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
ReqOption_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption       = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* collect value */
        *pBool    =  pDhcpReqOption->bEnabled;
        
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReqOption_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
ReqOption_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pInt);

    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReqOption_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
ReqOption_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption       = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Order") == 0)
    {
        /* collect value */
        *puLong  =  pDhcpReqOption->Order;
        
        return TRUE;
    }

	else if (strcmp(ParamName, "Tag") == 0)
    {
        /* collect value */
        *puLong  =  pDhcpReqOption->Tag;
        
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        ReqOption_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/
ULONG
ReqOption_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption       = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString(pDhcpReqOption->Alias) < *pUlSize)
        {
            AnscCopyString(pValue, pDhcpReqOption->Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pDhcpReqOption->Alias)+1;
            return 1;
        }
    }

	else if (strcmp(ParamName, "Value") == 0)
    {
        /* collect value */
        WanMgr_DmlDhcpcGetReqOptionbyInsNum(NULL, pDhcpc->Cfg.InstanceNumber, pDhcpReqOption);
        
        if ( AnscSizeOfString((const char*)pDhcpReqOption->Value) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpReqOption->Value);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpReqOption->Value)+1;
            return 1;
        }
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReqOption_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
ReqOption_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption       = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* save update to backup */
        pDhcpReqOption->bEnabled   =  bValue;
        
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReqOption_SetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int                         iValue
            );

    description:

        This function is called to set integer parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int                         iValue
                The updated integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
ReqOption_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(iValue);

    /* check the parameter name and set the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReqOption_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG                       uValue
            );

    description:

        This function is called to set ULONG parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
ReqOption_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption       = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Order") == 0)
    {
        /* save update to backup */
        pDhcpReqOption->Order   = uValue;
        
        return TRUE;
    }

	else if (strcmp(ParamName, "Tag") == 0)
    {
        /* save update to backup */
        pDhcpReqOption->Tag   = (UCHAR)uValue;
        
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReqOption_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );

    description:

        This function is called to set string parameter value; 

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pString
                The updated string value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
ReqOption_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption    = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PCONTEXT_LINK_OBJECT       pCxtLink2         = NULL;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption2   = NULL;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    BOOL                            bFound            = FALSE;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* Backup old alias firstly */
        AnscCopyString(pCxtDhcpcLink->AliasOfReq, pDhcpReqOption->Alias);

        AnscCopyString(pDhcpReqOption->Alias, pString);

        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReqOption_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation. 

                ULONG*                      puLength
                The output length of the param name. 

    return:     TRUE if there's no validation.

**********************************************************************/
BOOL
ReqOption_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption    = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PCONTEXT_LINK_OBJECT       pCxtLink2         = NULL;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption2   = NULL;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    BOOL                            bFound            = FALSE;

    UNREFERENCED_PARAMETER(puLength);

    /* Parent hasn't set, we don't permit child is set.*/
    if ( pCxtDhcpcLink->bNew )
    {
#if COSA_DHCPV4_ROLLBACK_TEST        
        ReqOption_Rollback(hInsContext);
#endif

        return FALSE;
    }

    /* For other check */
    if ( pCxtDhcpcLink->AliasOfReq[0] )
    {
        /* save update to backup */
        bFound                = FALSE;
        pSListEntry           = AnscSListGetFirstEntry(&pCxtDhcpcLink->ReqOptionList);
        while( pSListEntry != NULL)
        {
            pCxtLink2         = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
            pSListEntry       = AnscSListGetNextEntry(pSListEntry);

            pDhcpReqOption2  = (PDML_DHCPC_REQ_OPT)pCxtLink2->hContext;

            if( DHCPV4_REQOPTION_ENTRY_MATCH2(pDhcpReqOption->Alias, pDhcpReqOption2->Alias ) )
            {
                if ( (ANSC_HANDLE)pCxtLink2 == hInsContext )
                {
                    continue;
                }
                
                strncpy(pReturnParamName, "Alias",sizeof("Alias"));

                bFound = TRUE;
                
                break;
            }
        }

        if ( bFound )
        {
#if COSA_DHCPV4_ROLLBACK_TEST        
            ReqOption_Rollback(hInsContext);
#endif
            return FALSE;
        }
    }

    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        ReqOption_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
ReqOption_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption    = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PDML_DHCPC_FULL            pDhcpClient       = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;
	PWAN_DHCPV4_DATA           pDhcpv4           = (PWAN_DHCPV4_DATA)g_pWanMgrBE->hDhcpv4;

    if (pDhcpv4 != NULL)
    {
        if ( pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpcAddReqOption(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpReqOption );

            if ( returnStatus == ANSC_STATUS_SUCCESS )
            {
                pCxtLink->bNew = FALSE;

                WanMgr_Dhcpv4RegSetDhcpv4Info(pDhcpv4);
            }
            else
            {
                DHCPV4_REQOPTION_SET_DEFAULTVALUE(pDhcpReqOption);

                if ( pCxtDhcpcLink->AliasOfReq[0] )
                    AnscCopyString( pDhcpReqOption->Alias, pCxtDhcpcLink->AliasOfReq );
            }
        }
        else
        {
            returnStatus = WanMgr_DmlDhcpcSetReqOption(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpReqOption);

            if ( returnStatus != ANSC_STATUS_SUCCESS)
            {
                WanMgr_DmlDhcpcGetReqOptionbyInsNum(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpReqOption);
            }
        }

        AnscZeroMemory( pCxtDhcpcLink->AliasOfReq, sizeof(pCxtDhcpcLink->AliasOfReq) );
    }

    return returnStatus;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        ReqOption_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a 
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
ReqOption_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPC_REQ_OPT         pDhcpReqOption    = (PDML_DHCPC_REQ_OPT)pCxtLink->hContext;
    PDHCPC_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPC_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PDML_DHCPC_FULL            pDhcpc            = (PDML_DHCPC_FULL)pCxtDhcpcLink->hContext;

    if ( pCxtDhcpcLink->AliasOfReq[0] )
        AnscCopyString( pDhcpReqOption->Alias, pCxtDhcpcLink->AliasOfReq );

    if ( !pCxtLink->bNew )
    {
        WanMgr_DmlDhcpcGetReqOptionbyInsNum(NULL, pDhcpc->Cfg.InstanceNumber, pDhcpReqOption);
    }
    else
    {
        DHCPV4_REQOPTION_SET_DEFAULTVALUE(pDhcpReqOption);
    }
    
    AnscZeroMemory( pCxtDhcpcLink->AliasOfReq, sizeof(pCxtDhcpcLink->AliasOfReq) );
    
    return returnStatus;
}
