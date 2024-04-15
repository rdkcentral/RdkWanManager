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
#include "wanmgr_plugin_main_apis.h"
#include "wanmgr_dhcpv6_apis.h"
#include "wanmgr_dhcpv6_internal.h"
#include <sysevent/sysevent.h>

extern int sysevent_fd;
extern token_t sysevent_token;
extern WANMGR_BACKEND_OBJ* g_pWanMgrBE;

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

    DHCPv6.

    *  DHCPv6_GetParamBoolValue
    *  DHCPv6_GetParamIntValue
    *  DHCPv6_GetParamUlongValue
    *  DHCPv6_GetParamStringValue

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        DHCPv6_GetParamBoolValue
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
DHCPv6_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pBool);
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        DHCPv6_GetParamIntValue
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
DHCPv6_GetParamIntValue
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
        DHCPv6_GetParamUlongValue
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
DHCPv6_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(puLong);
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        DHCPv6_GetParamStringValue
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
DHCPv6_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pValue);
    UNREFERENCED_PARAMETER(pUlSize);
    return -1;
}

/***********************************************************************

 APIs for Object:

    DHCPv6.Client.{i}.

    *  Client3_GetEntryCount
    *  Client3_GetEntry
    *  Client3_AddEntry
    *  Client3_DelEntry
    *  Client3_GetParamBoolValue
    *  Client3_GetParamIntValue
    *  Client3_GetParamUlongValue
    *  Client3_GetParamStringValue
    *  Client3_SetParamBoolValue
    *  Client3_SetParamIntValue
    *  Client3_SetParamUlongValue
    *  Client3_SetParamStringValue
    *  Client3_Validate
    *  Client3_Commit
    *  Client3_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client3_GetEntryCount
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
Client3_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    ULONG ClientCount = 0;
	PWAN_DHCPV6_DATA   pDhcpv6       =  (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;
    if(pDhcpv6 != NULL)
    {
        ClientCount = AnscSListQueryDepth( &pDhcpv6->ClientList );
    }

    return ClientCount;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        Client3_GetEntry
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
Client3_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
	PWAN_DHCPV6_DATA             pDhcpv6       = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink          = NULL;

    if (pDhcpv6 != NULL)
    {
        pSListEntry = AnscSListGetEntryByIndex(&pDhcpv6->ClientList, nIndex);

        if ( pSListEntry )
        {
            pCxtLink          = ACCESS_DHCPCV6_CONTEXT_LINK_OBJECT(pSListEntry);
            *pInsNumber       = pCxtLink->InstanceNumber;
        }
    }

    return pSListEntry;

}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        Client3_AddEntry
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
Client3_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = NULL;
    PDML_DHCPCV6_FULL            pDhcpc          = NULL;
	PWAN_DHCPV6_DATA             pDhcpv6       = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;


    if (pDhcpv6 != NULL)
    {
        pDhcpc  = (PDML_DHCPCV6_FULL)AnscAllocateMemory( sizeof(DML_DHCPCV6_FULL) );
        if ( !pDhcpc )
        {
            return NULL; /* return the handle */
        }

        /* Set default value */
        DHCPV6_CLIENT_SET_DEFAULTVALUE(pDhcpc);

        /* Add into our link tree*/    
        pCxtLink = (PDHCPCV6_CONTEXT_LINK_OBJECT)AnscAllocateMemory( sizeof(DHCPCV6_CONTEXT_LINK_OBJECT) );
        if ( !pCxtLink )
        {
            AnscFreeMemory(pDhcpc);
            return NULL;
        }

        DHCPV6_CLIENT_INITIATION_CONTEXT(pCxtLink)

            pCxtLink->hContext       = (ANSC_HANDLE)pDhcpc;
        pCxtLink->bNew           = TRUE;

        if ( !++pDhcpv6->maxInstanceOfClient )
        {
            pDhcpv6->maxInstanceOfClient = 1;
        }
        pDhcpc->Cfg.InstanceNumber = pDhcpv6->maxInstanceOfClient;
        pCxtLink->InstanceNumber   = pDhcpc->Cfg.InstanceNumber;
        *pInsNumber                = pDhcpc->Cfg.InstanceNumber;

        _ansc_sprintf( pDhcpc->Cfg.Alias, "Client%d", pDhcpc->Cfg.InstanceNumber);

        /* Put into our list */
        SListPushEntryByInsNum(&pDhcpv6->ClientList, (PCONTEXT_LINK_OBJECT)pCxtLink);

        /* we recreate the configuration because we has new delay_added entry for dhcpv6 */
        WanMgr_Dhcpv6RegSetDhcpv6Info(pDhcpv6);

    }

    return (ANSC_HANDLE)pCxtLink;

}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client3_DelEntry
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
Client3_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(hInstance);
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
	PWAN_DHCPV6_DATA             pDhcpv6       = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInstance;
    PCONTEXT_LINK_OBJECT       pCxtLink2         = NULL;
    PDML_DHCPCV6_FULL          pDhcpc            = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

    /* Normally, two sublinks are empty because our framework will firstly 
       call delEntry for them before coming here. We needn't care them.
       */

    if (pDhcpv6 != NULL)
    {
        if ( !pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpv6cDelEntry(NULL, pDhcpc->Cfg.InstanceNumber);
            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                return returnStatus;
            }
        }

        if (AnscSListPopEntryByLink(&pDhcpv6->ClientList, &pCxtLink->Linkage) )
        {
            /* Because the current NextInstanceNumber information need be deleted */
            WanMgr_Dhcpv6RegSetDhcpv6Info(pDhcpv6);

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
        Client3_GetParamBoolValue
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
Client3_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    ANSC_STATUS                       returnStatus    = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpc          = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* collect value */
#if defined(WAN_MANAGER_UNIFICATION_ENABLED) 
        /* For unification enabled builds Device.DHCPv6.Client.1.Enable is not used to start the DHCP v6 client. Check if the dhcpv6 client running. */
        CcspTraceInfo(("%s %d Calling WanMgr_DmlDhcpv6cGetEnabled to get dhcpv6 status. \n", __FUNCTION__, __LINE__));
        *pBool   =WanMgr_DmlDhcpv6cGetEnabled(NULL);
#else
        *pBool   = pDhcpc->Cfg.bEnabled;
#endif

        return TRUE;
    }

    if (strcmp(ParamName, "RequestAddresses") == 0)
    {
        /* collect value */
        *pBool   = pDhcpc->Cfg.RequestAddresses;

        return TRUE;
    }

    if (strcmp(ParamName, "RequestPrefixes") == 0)
    {
        /* collect value */
        *pBool   = pDhcpc->Cfg.RequestPrefixes;

        return TRUE;
    }

    if (strcmp(ParamName, "RapidCommit") == 0)
    {
        /* collect value */
        *pBool   = pDhcpc->Cfg.RapidCommit;

        return TRUE;
    }

    if (strcmp(ParamName, "Renew") == 0)
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
        Client3_GetParamIntValue
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
Client3_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    ANSC_STATUS                       returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpc            = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "SuggestedT1") == 0)
    {
        /* collect value */        
        *pInt   = pDhcpc->Cfg.SuggestedT1;
        
        return TRUE;
    }

    if (strcmp(ParamName, "SuggestedT2") == 0)
    {
        /* collect value */
        *pInt   = pDhcpc->Cfg.SuggestedT2;

        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client3_GetParamUlongValue
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
Client3_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    ANSC_STATUS                       returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpc            = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Status") == 0)
    {
        /* collect value */
        WanMgr_DmlDhcpv6cGetInfo(pDhcpc, pCxtLink->InstanceNumber, &pDhcpc->Info);

        *puLong   = pDhcpc->Info.Status;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client3_GetParamStringValue
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
Client3_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpc            = (PDML_DHCPCV6_FULL)pCxtLink->hContext;
    CHAR                         tmpBuff[DATAMODEL_PARAM_LENGTH]   = {0};
    CHAR                         interface[DATAMODEL_PARAM_LENGTH] = {0};
    INT                             instanceNumber    = -1;
	ULONG                        i, len            = 0;
    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpc->Cfg.Alias) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpc->Cfg.Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpc->Cfg.Alias)+1;
            return 1;
        }

        return 0;
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

        return 0;
    }
    else if (strcmp(ParamName, "DUID") == 0)
    {
        WanMgr_DmlDhcpv6cGetInfo(pDhcpc, pCxtLink->InstanceNumber, &pDhcpc->Info);

        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpc->Info.DUID) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpc->Info.DUID);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpc->Info.DUID)+1;
            return 1;
        }
    }
    else if (strcmp(ParamName, "SupportedOptions") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpc->Info.SupportedOptions) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpc->Info.SupportedOptions);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpc->Info.SupportedOptions)+1;
            return 1;
        }
    }
    else if (strcmp(ParamName, "RequestedOptions") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpc->Cfg.RequestedOptions) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpc->Cfg.RequestedOptions);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpc->Cfg.RequestedOptions)+1;
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
        Client3_SetParamBoolValue
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
Client3_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    ANSC_STATUS                       returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpc            = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* save update to backup */
        pDhcpc->Cfg.bEnabled = bValue;

        return TRUE;
    }
	else if (strcmp(ParamName, "RequestAddresses") == 0)
    {
        /* save update to backup */
        pDhcpc->Cfg.RequestAddresses = bValue;

        return TRUE;
    }
    else if (strcmp(ParamName, "RequestPrefixes") == 0)
    {
        /* save update to backup */
        pDhcpc->Cfg.RequestPrefixes = bValue;

        return TRUE;
    }
    else if (strcmp(ParamName, "RapidCommit") == 0)
    {
        /* save update to backup */
        pDhcpc->Cfg.RapidCommit = bValue;

        return TRUE;
    }
    else if (strcmp(ParamName, "Renew") == 0)
    {
        /* save update to backup */
        if ( bValue )
        {
            returnStatus = WanMgr_DmlDhcpv6cRenew(NULL, pDhcpc->Cfg.InstanceNumber);
            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                return  FALSE;
            }
        }
        
        return  TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client3_SetParamIntValue
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
Client3_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    ANSC_STATUS                       returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpc            = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "SuggestedT1") == 0)
    {
        /* save update to backup */
        pDhcpc->Cfg.SuggestedT1 = iValue;
        
        return TRUE;
    }
    else if (strcmp(ParamName, "SuggestedT2") == 0)
    {
        /* save update to backup */
        pDhcpc->Cfg.SuggestedT2 = iValue;

        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client3_SetParamUlongValue
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
Client3_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(uValue);
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Client3_SetParamStringValue
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
Client3_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    ANSC_STATUS                   returnStatus  = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT  pCxtLink      = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL             pDhcpc        = (PDML_DHCPCV6_FULL)pCxtLink->hContext;
	PWAN_DHCPV6_DATA              pDhcpv6       = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;
        
    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "RequestedOptions") == 0)
    {
        /* save update to backup */
        AnscCopyString(pDhcpc->Cfg.RequestedOptions, pString);

        return TRUE;
    }
    else if (strcmp(ParamName, "Alias") == 0)
    {
        /* save update to backup */
        AnscCopyString(pDhcpv6->AliasOfClient, pDhcpc->Cfg.Alias);

        AnscCopyString(pDhcpc->Cfg.Alias, pString);

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
        Client3_Validate
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
Client3_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    ANSC_STATUS                    returnStatus   = ANSC_STATUS_SUCCESS;
    PWAN_DHCPV6_DATA               pDhcpv6        = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;
    PDHCPCV6_CONTEXT_LINK_OBJECT   pCxtLink       = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL              pDhcpc         = (PDML_DHCPCV6_FULL)pCxtLink->hContext;
    PSINGLE_LINK_ENTRY             pSListEntry    = NULL;
    PDML_DHCPCV6_FULL              pDhcpc2        = NULL;
    BOOL                           bFound         = FALSE;
    UNREFERENCED_PARAMETER(puLength);
    if (pDhcpv6 != NULL)
    {
        /*  only for Alias */
        if ( pDhcpv6->AliasOfClient[0] )
        {
            pSListEntry           = AnscSListGetFirstEntry(&pDhcpv6->ClientList);
            while( pSListEntry != NULL)
            {
                pCxtLink          = ACCESS_DHCPCV6_CONTEXT_LINK_OBJECT(pSListEntry);
                pSListEntry       = AnscSListGetNextEntry(pSListEntry);

                pDhcpc2 = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

                if( DHCPV6_CLIENT_ENTRY_MATCH2((char*)pDhcpc2->Cfg.Alias, pDhcpc->Cfg.Alias) )
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
#if COSA_DHCPV6_ROLLBACK_TEST        
                Client3_Rollback(hInsContext);
#endif
                return FALSE;
            }
        }

        /* some other checking */
        if (pDhcpc->Cfg.bEnabled)
        {
            if (pDhcpc->Cfg.SuggestedT1 > pDhcpc->Cfg.SuggestedT2)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client3_Commit
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
Client3_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                   returnStatus = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT  pCxtLink     = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL             pDhcpc       = (PDML_DHCPCV6_FULL)pCxtLink->hContext;
    PWAN_DHCPV6_DATA              pDhcpv6      = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;

    if (pDhcpv6 != NULL)
    {
        if ( pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpv6cAddEntry(NULL, pDhcpc );

            if ( returnStatus == ANSC_STATUS_SUCCESS )
            {
                pCxtLink->bNew = FALSE;

                WanMgr_Dhcpv6RegSetDhcpv6Info(pDhcpv6);
            }
            else
            {
                DHCPV6_CLIENT_SET_DEFAULTVALUE(pDhcpc);

                if ( pDhcpv6->AliasOfClient[0] )
                    AnscCopyString( (char*)pDhcpc->Cfg.Alias, pDhcpv6->AliasOfClient );
            }
        }
        else
        {
            returnStatus = WanMgr_DmlDhcpv6cSetCfg(NULL, &pDhcpc->Cfg);

            if ( returnStatus != ANSC_STATUS_SUCCESS)
            {
                WanMgr_DmlDhcpv6cGetCfg(NULL, &pDhcpc->Cfg);
            }
        }

        AnscZeroMemory( pDhcpv6->AliasOfClient, sizeof(pDhcpv6->AliasOfClient) );
    }

    return returnStatus;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Client3_Rollback
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
Client3_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                   returnStatus  = ANSC_STATUS_SUCCESS;
    PWAN_DHCPV6_DATA              pDhcpv6       = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;
    PDHCPCV6_CONTEXT_LINK_OBJECT  pCxtLink      = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL             pDhcpc        = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

    if (pDhcpv6 != NULL)
    {
        if ( pDhcpv6->AliasOfClient[0] )
            AnscCopyString( (char*)pDhcpc->Cfg.Alias, pDhcpv6->AliasOfClient );

        if ( !pCxtLink->bNew )
        {
            WanMgr_DmlDhcpv6cGetCfg( NULL, &pDhcpc->Cfg );
        }
        else
        {
            DHCPV6_CLIENT_SET_DEFAULTVALUE(pDhcpc);
        }

        AnscZeroMemory( pDhcpv6->AliasOfClient, sizeof(pDhcpv6->AliasOfClient) );
    }

    return returnStatus;
}

/***********************************************************************

 APIs for Object:

    DHCPv6.Client.{i}.Server.{i}.

    *  Server2_GetEntryCount
    *  Server2_GetEntry
    *  Server2_IsUpdated
    *  Server2_Synchronize
    *  Server2_GetParamBoolValue
    *  Server2_GetParamIntValue
    *  Server2_GetParamUlongValue
    *  Server2_GetParamStringValue

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Server2_GetEntryCount
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
Server2_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                       returnStatus    = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;

    return (pCxtLink->NumberOfServer);
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        Server2_GetEntry
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
Server2_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    ANSC_STATUS                       returnStatus    = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;

    *pInsNumber  = nIndex + 1; 

    return (ANSC_HANDLE)&pCxtLink->pServerEntry[nIndex]; /* return the handle */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Server2_IsUpdated
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is checking whether the table is updated or not.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     TRUE or FALSE.

**********************************************************************/
BOOL
Server2_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    BOOL                              bIsUpdated   = TRUE;

    /* 
        We can use one rough granularity interval to get whole table in case 
        that the updating is too frequent.
        */
    if ( ( AnscGetTickInSeconds() - pCxtLink->PreviousVisitTimeOfServer ) < COSA_DML_DHCPV6_ACCESS_INTERVAL_CLIENTSERVER )
    {
        bIsUpdated  = FALSE;
    }
    else
    {
        pCxtLink->PreviousVisitTimeOfServer =  AnscGetTickInSeconds();
        bIsUpdated  = TRUE;
    }

    return bIsUpdated;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Server2_Synchronize
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to synchronize the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
Server2_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                       returnStatus    = ANSC_STATUS_SUCCESS;    
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SVR             pDhcpcServer    = NULL;
    ULONG                             count           = 0;
    
    if ( pCxtLink->pServerEntry )
    {
        AnscFreeMemory(pCxtLink->pServerEntry);
        pCxtLink->pServerEntry   = NULL;
        pCxtLink->NumberOfServer = 0;
    }

    returnStatus = WanMgr_DmlDhcpv6cGetServerCfg
                    (
                        NULL,
                        pCxtLink->InstanceNumber,
                        &pDhcpcServer,
                        &count
                    );

    if ( returnStatus == ANSC_STATUS_SUCCESS )
    {
        pCxtLink->pServerEntry    = pDhcpcServer;
        pCxtLink->NumberOfServer  = count;
    }

    return returnStatus;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Server2_GetParamBoolValue
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
Server2_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pBool);
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Server2_GetParamIntValue
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
Server2_GetParamIntValue
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
        Server2_GetParamUlongValue
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
Server2_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(puLong);
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Server2_GetParamStringValue
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
Server2_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    ANSC_STATUS           returnStatus    = ANSC_STATUS_SUCCESS;    
    PDML_DHCPCV6_SVR pDhcpcServer    = (PDML_DHCPCV6_SVR)hInsContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "SourceAddress") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpcServer->SourceAddress) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpcServer->SourceAddress);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpcServer->SourceAddress)+1;
            return 1;
        }

        return 0;
    }
	else if (strcmp(ParamName, "DUID") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpcServer->DUID) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpcServer->DUID);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpcServer->DUID)+1;
            return 1;
        }
        return 0;
    }
	else if (strcmp(ParamName, "InformationRefreshTime") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpcServer->InformationRefreshTime) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpcServer->InformationRefreshTime);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpcServer->InformationRefreshTime)+1;
            return 1;
        }
        return 0;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/***********************************************************************

 APIs for Object:

    DHCPv6.Client.{i}.SentOption.{i}.

    *  SentOption1_GetEntryCount
    *  SentOption1_GetEntry
    *  SentOption1_AddEntry
    *  SentOption1_DelEntry
    *  SentOption1_GetParamBoolValue
    *  SentOption1_GetParamIntValue
    *  SentOption1_GetParamUlongValue
    *  SentOption1_GetParamStringValue
    *  SentOption1_SetParamBoolValue
    *  SentOption1_SetParamIntValue
    *  SentOption1_SetParamUlongValue
    *  SentOption1_SetParamStringValue
    *  SentOption1_Validate
    *  SentOption1_Commit
    *  SentOption1_Rollback

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption1_GetEntryCount
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
SentOption1_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                       returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink          = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpc            = (PDML_DHCPCV6_FULL)pCxtLink->hContext;

    return AnscSListQueryDepth( &pCxtLink->SentOptionList );
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        SentOption1_GetEntry
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
SentOption1_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtDhcpcLink      = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PCONTEXT_LINK_OBJECT       pCxtLink             = NULL;
    PSINGLE_LINK_ENTRY              pSListEntry          = NULL;

    pSListEntry = AnscSListGetEntryByIndex(&pCxtDhcpcLink->SentOptionList, nIndex);

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
        SentOption1_AddEntry
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
SentOption1_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{
    ANSC_STATUS                       returnStatus         = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtDhcpcLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpc               = (PDML_DHCPCV6_FULL)pCxtDhcpcLink->hContext;
    PCONTEXT_LINK_OBJECT         pCxtLink             = NULL;
    PDML_DHCPCV6_SENT            pDhcpSentOption      = NULL;

    pDhcpSentOption  = (PDML_DHCPCV6_SENT)AnscAllocateMemory( sizeof(DML_DHCPCV6_SENT) );
    if ( !pDhcpSentOption )
    {
        goto EXIT2;
    }

    DHCPV6_SENTOPTION_SET_DEFAULTVALUE(pDhcpSentOption);

    pCxtLink = (PCONTEXT_LINK_OBJECT)AnscAllocateMemory( sizeof(CONTEXT_LINK_OBJECT) );
    if ( !pCxtLink )
    {
        goto EXIT1;
    }

    pCxtLink->hContext       = (ANSC_HANDLE)pDhcpSentOption;
    pCxtLink->hParentTable   = (ANSC_HANDLE)pCxtDhcpcLink;
    pCxtLink->bNew           = TRUE;

    if ( !++pCxtDhcpcLink->maxInstanceOfSent )
    {
        pCxtDhcpcLink->maxInstanceOfSent = 1;
    }

    pDhcpSentOption->InstanceNumber = pCxtDhcpcLink->maxInstanceOfSent;
    pCxtLink->InstanceNumber       = pDhcpSentOption->InstanceNumber;
    *pInsNumber                    = pDhcpSentOption->InstanceNumber;

    _ansc_sprintf( (char*)pDhcpSentOption->Alias, "SentOption%d", pDhcpSentOption->InstanceNumber);

    /* Put into our list */
    SListPushEntryByInsNum(&pCxtDhcpcLink->SentOptionList, (PCONTEXT_LINK_OBJECT)pCxtLink);

    /* we recreate the configuration */
	PWAN_DHCPV6_DATA pDhcpv6 = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;
    if (pDhcpv6 != NULL)
    {
        WanMgr_Dhcpv6RegSetDhcpv6Info(pDhcpv6);
    }

    return (ANSC_HANDLE)pCxtLink;

EXIT1:

    AnscFreeMemory(pDhcpSentOption);

EXIT2:

    return NULL;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption1_DelEntry
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
SentOption1_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )
{
    ANSC_STATUS                  returnStatus      = ANSC_STATUS_SUCCESS;
	PWAN_DHCPV6_DATA             pDhcpv6           = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_FULL            pDhcpClient       = (PDML_DHCPCV6_FULL)pCxtDhcpcLink->hContext;
    PCONTEXT_LINK_OBJECT         pCxtLink          = (PCONTEXT_LINK_OBJECT)hInstance;
    PDML_DHCPCV6_SENT            pDhcpSentOption   = (PDML_DHCPCV6_SENT)pCxtLink->hContext;

    if (pDhcpv6 != NULL)
    {
        if ( !pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpv6cDelSentOption(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpSentOption->InstanceNumber);
            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                return returnStatus;
            }
        }

        if ( AnscSListPopEntryByLink(&pCxtDhcpcLink->SentOptionList, &pCxtLink->Linkage) )
        {
            WanMgr_Dhcpv6RegSetDhcpv6Info(pDhcpv6);
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
        SentOption1_GetParamBoolValue
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
SentOption1_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SENT          pDhcpSentOption      = (PDML_DHCPCV6_SENT)pCxtLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* collect value */
        *pBool  = pDhcpSentOption->bEnabled;

        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption1_GetParamIntValue
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
SentOption1_GetParamIntValue
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
        SentOption1_GetParamUlongValue
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
SentOption1_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SENT          pDhcpSentOption      = (PDML_DHCPCV6_SENT)pCxtLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Tag") == 0)
    {
        /* collect value */
        *puLong = pDhcpSentOption->Tag;

        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption1_GetParamStringValue
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
SentOption1_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SENT          pDhcpSentOption      = (PDML_DHCPCV6_SENT)pCxtLink->hContext;
    ANSC_STATUS                     returnStatus         = ANSC_STATUS_SUCCESS;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpSentOption->Alias) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpSentOption->Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpSentOption->Alias)+1;
            return 1;
        }
    }
    else if (strcmp(ParamName, "Value") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpSentOption->Value) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpSentOption->Value);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpSentOption->Value)+1;
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
        SentOption1_SetParamBoolValue
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
SentOption1_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SENT          pDhcpSentOption      = (PDML_DHCPCV6_SENT)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        /* save update to backup */
        pDhcpSentOption->bEnabled  = bValue;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption1_SetParamIntValue
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
SentOption1_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    /* check the parameter name and set the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(iValue);
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption1_SetParamUlongValue
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
SentOption1_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink             = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SENT          pDhcpSentOption      = (PDML_DHCPCV6_SENT)pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Tag") == 0)
    {
        /* save update to backup */
        pDhcpSentOption->Tag  = (UCHAR)uValue;

        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption1_SetParamStringValue
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
SentOption1_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PCONTEXT_LINK_OBJECT         pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SENT            pDhcpSentOption   = (PDML_DHCPCV6_SENT)pCxtLink->hContext;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPCV6_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        /* save update to backup */
        AnscCopyString(pCxtDhcpcLink->AliasOfSent, (char*)pDhcpSentOption->Alias);

        AnscCopyString((char*)pDhcpSentOption->Alias, pString);

        return TRUE;
    }
    else if (strcmp(ParamName, "Value") == 0)
    {
        /* save update to backup */
        AnscCopyString((char*)pDhcpSentOption->Value, pString);
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        SentOption1_Validate
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
SentOption1_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
        PCONTEXT_LINK_OBJECT         pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
        PDML_DHCPCV6_SENT            pDhcpSentOption   = (PDML_DHCPCV6_SENT)pCxtLink->hContext;
        PDHCPCV6_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPCV6_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
        PCONTEXT_LINK_OBJECT         pCxtLink2         = NULL;
        PDML_DHCPCV6_SENT            pDhcpSentOption2  = NULL;
        PSINGLE_LINK_ENTRY                pSListEntry       = NULL;
        BOOL                              bFound            = FALSE;
    
        /* Parent hasn't set, we don't permit child is set.*/
        if ( pCxtDhcpcLink->bNew )
        {
#if COSA_DHCPV6_ROLLBACK_TEST        
            SentOption1_Rollback(hInsContext);
#endif
            return FALSE;
        }
    
        /* This is for alias */
        if ( TRUE )
        {
            bFound                = FALSE;
            pSListEntry           = AnscSListGetFirstEntry(&pCxtDhcpcLink->SentOptionList);
            while( pSListEntry != NULL)
            {
                pCxtLink2         = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
                pSListEntry       = AnscSListGetNextEntry(pSListEntry);
    
                pDhcpSentOption2  = (PDML_DHCPCV6_SENT)pCxtLink2->hContext;
    
                if( DHCPV6_SENDOPTION_ENTRY_MATCH2((char*)pDhcpSentOption->Alias, pDhcpSentOption2->Alias) )
                {
                    if ( (ANSC_HANDLE)pCxtLink2 == hInsContext )
                    {
                        continue;
                    }
    
                    strncpy(pReturnParamName, "Alias",sizeof("Alias"));
                    *puLength = AnscSizeOfString("Alias");
    
                    bFound = TRUE;
                    
                    break;
                }

                if ( (pDhcpSentOption->bEnabled && pDhcpSentOption2->bEnabled) &&
                     pDhcpSentOption->Tag == pDhcpSentOption2->Tag)
                {
                    if ( (ANSC_HANDLE)pCxtLink2 == hInsContext )
                    {
                        continue;
                    }
    
                    strncpy(pReturnParamName, "Tag",sizeof("Tag"));
                    *puLength = AnscSizeOfString("Tag");
    
                    bFound = TRUE;
                    
                    break;

                }
                
            }
            
            if ( bFound )
            {
#if COSA_DHCPV6_ROLLBACK_TEST        
                SentOption1_Rollback(hInsContext);
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
        SentOption1_Commit
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
SentOption1_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                       returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT         pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SENT            pDhcpSentOption   = (PDML_DHCPCV6_SENT)pCxtLink->hContext;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPCV6_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PDML_DHCPCV6_FULL            pDhcpClient       = (PDML_DHCPCV6_FULL)pCxtDhcpcLink->hContext;
    PWAN_DHCPV6_DATA             pDhcpv6           = (PWAN_DHCPV6_DATA)g_pWanMgrBE->hDhcpv6;

    if (pDhcpv6 != NULL)
    {
        if ( pCxtLink->bNew )
        {
            returnStatus = WanMgr_DmlDhcpv6cAddSentOption(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpSentOption );

            if ( returnStatus == ANSC_STATUS_SUCCESS )
            {
                pCxtLink->bNew = FALSE;

                WanMgr_Dhcpv6RegSetDhcpv6Info(pDhcpv6);
            }
            else
            {
                DHCPV6_SENTOPTION_SET_DEFAULTVALUE(pDhcpSentOption);

                if ( pCxtDhcpcLink->AliasOfSent[0] )
                    AnscCopyString( (char*)pDhcpSentOption->Alias, pCxtDhcpcLink->AliasOfSent );
            }
        }
        else
        {
            returnStatus = WanMgr_DmlDhcpv6cSetSentOption(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpSentOption);

            if ( returnStatus != ANSC_STATUS_SUCCESS)
            {
                WanMgr_DmlDhcpv6cGetSentOptionbyInsNum(NULL, pDhcpClient->Cfg.InstanceNumber, pDhcpSentOption);
            }
        }

        AnscZeroMemory( pCxtDhcpcLink->AliasOfSent, sizeof(pCxtDhcpcLink->AliasOfSent) );
    }

    return returnStatus;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        SentOption1_Rollback
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
SentOption1_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                       returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT         pCxtLink          = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_SENT            pDhcpSentOption   = (PDML_DHCPCV6_SENT)pCxtLink->hContext;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtDhcpcLink     = (PDHCPCV6_CONTEXT_LINK_OBJECT)pCxtLink->hParentTable;
    PDML_DHCPCV6_FULL            pDhcpc            = (PDML_DHCPCV6_FULL)pCxtDhcpcLink->hContext;

    if ( pCxtDhcpcLink->AliasOfSent[0] )
        AnscCopyString( (char*)pDhcpSentOption->Alias, pCxtDhcpcLink->AliasOfSent );

    if ( !pCxtLink->bNew )
    {
        WanMgr_DmlDhcpv6cGetSentOptionbyInsNum(NULL, pDhcpc->Cfg.InstanceNumber, pDhcpSentOption);
    }
    else
    {
        DHCPV6_SENTOPTION_SET_DEFAULTVALUE(pDhcpSentOption);
    }
    
    AnscZeroMemory( pCxtDhcpcLink->AliasOfSent, sizeof(pCxtDhcpcLink->AliasOfSent) );
    
    return returnStatus;
}

/***********************************************************************

 APIs for Object:

    DHCPv6.Client.{i}.ReceivedOption.{i}.

    *  ReceivedOption_GetEntryCount
    *  ReceivedOption_GetEntry
    *  ReceivedOption_IsUpdated
    *  ReceivedOption_Synchronize
    *  ReceivedOption_GetParamBoolValue
    *  ReceivedOption_GetParamIntValue
    *  ReceivedOption_GetParamUlongValue
    *  ReceivedOption_GetParamStringValue

***********************************************************************/
/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        ReceivedOption_GetEntryCount
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
ReceivedOption_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                       returnStatus    = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;

    return pCxtLink->NumberOfRecv;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        ReceivedOption_GetEntry
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
ReceivedOption_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    ANSC_STATUS                       returnStatus    = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;

    *pInsNumber  = nIndex + 1; 

    return (ANSC_HANDLE)&pCxtLink->pRecvEntry[nIndex]; /* return the handle */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReceivedOption_IsUpdated
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is checking whether the table is updated or not.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     TRUE or FALSE.

**********************************************************************/
BOOL
ReceivedOption_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    BOOL                              bIsUpdated   = TRUE;

    /* 
        We can use one rough granularity interval to get whole table in case 
        that the updating is too frequent.
        */
    if ( ( AnscGetTickInSeconds() - pCxtLink->PreviousVisitTimeOfRecv ) < COSA_DML_DHCPV6_ACCESS_INTERVAL_CLIENTRECV )
    {
        bIsUpdated  = FALSE;
    }
    else
    {
        pCxtLink->PreviousVisitTimeOfRecv =  AnscGetTickInSeconds();
        bIsUpdated  = TRUE;
    }

    return bIsUpdated;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        ReceivedOption_Synchronize
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to synchronize the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
ReceivedOption_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                       returnStatus    = ANSC_STATUS_SUCCESS;    
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtLink        = (PDHCPCV6_CONTEXT_LINK_OBJECT)hInsContext;
    PDML_DHCPCV6_RECV            pDhcpcRecv      = NULL;
    ULONG                             count           = 0;
    
    if ( pCxtLink->pRecvEntry )
    {
        AnscFreeMemory(pCxtLink->pRecvEntry);
        pCxtLink->pRecvEntry   = NULL;
        pCxtLink->NumberOfRecv = 0;
    }

    returnStatus = WanMgr_DmlDhcpv6cGetReceivedOptionCfg
                    (
                        NULL,
                        pCxtLink->InstanceNumber,
                        &pDhcpcRecv,
                        &count
                    );

    if ( returnStatus == ANSC_STATUS_SUCCESS )
    {
        pCxtLink->pRecvEntry    = pDhcpcRecv;
        pCxtLink->NumberOfRecv  = count;
    }

    return returnStatus;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReceivedOption_GetParamBoolValue
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
ReceivedOption_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    UNREFERENCED_PARAMETER(hInsContext);
    UNREFERENCED_PARAMETER(ParamName);
    UNREFERENCED_PARAMETER(pBool);
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        ReceivedOption_GetParamIntValue
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
ReceivedOption_GetParamIntValue
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
        ReceivedOption_GetParamUlongValue
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
ReceivedOption_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    ANSC_STATUS            returnStatus    = ANSC_STATUS_SUCCESS;    
    PDML_DHCPCV6_RECV pDhcpcRecv      = (PDML_DHCPCV6_RECV)hInsContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Tag") == 0)
    {
        /* collect value */
        *puLong  = pDhcpcRecv->Tag;
        
        return TRUE;
    }


    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        ReceivedOption_GetParamStringValue
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
ReceivedOption_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    ANSC_STATUS            returnStatus    = ANSC_STATUS_SUCCESS;    
    PDML_DHCPCV6_RECV pDhcpcRecv      = (PDML_DHCPCV6_RECV)hInsContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Value") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpcRecv->Value) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpcRecv->Value);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpcRecv->Value)+1;
            return 1;
        }
    }
	else if (strcmp(ParamName, "Server") == 0)
    {
        /* collect value */
        if ( AnscSizeOfString((const char*)pDhcpcRecv->Server) < *pUlSize)
        {
            AnscCopyString(pValue, (char*)pDhcpcRecv->Server);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString((const char*)pDhcpcRecv->Server)+1;
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
        dhcp6c_mapt_mape_GetParamBoolValue
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
dhcp6c_mapt_mape_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "MapIsFMR") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        char temp[32] = {0};
        sysevent_get(sysevent_fd, sysevent_token,SYSEVENT_MAP_IS_FMR, temp, sizeof(temp));
        if (strcmp(temp, "TRUE") == 0)
            *pBool  = TRUE;
        else
            *pBool  = FALSE;
#else
        *pBool  = FALSE;
#endif
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        dhcp6c_mapt_mape_GetParamUlongValue
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
dhcp6c_mapt_mape_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
    char temp[64] = {0};
#endif
    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "MapEALen") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAP_EA_LEN, temp, sizeof(temp));
        *puLong  = strtoul(temp, NULL, 10);
#else
        *puLong  = 0;
#endif
        return TRUE;
    }

    if (strcmp(ParamName, "MapPSIDOffset") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAPT_PSID_OFFSET, temp, sizeof(temp));
        *puLong  = strtoul(temp, NULL, 10);
#else
        *puLong  = 0;
#endif
        return TRUE;
    }

    if (strcmp(ParamName, "MapPSIDLen") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAPT_PSID_LENGTH, temp, sizeof(temp));
        *puLong  = strtoul(temp, NULL, 10);
#else
        *puLong  = 0;
#endif
        return TRUE;
    }

    if (strcmp(ParamName, "MapPSID") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAPT_PSID_VALUE, temp, sizeof(temp));
        *puLong  = strtoul(temp, NULL, 10);
#else
        *puLong  = 0;
#endif
        return TRUE;
    }

    if (strcmp(ParamName, "MapRatio") == 0)
    {
#if defined(FEATURE_SUPPORT_MAPT_NAT46) || defined(FEATURE_MAPT)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAPT_RATIO, temp, sizeof(temp));
        *puLong  = strtoul(temp, NULL, 10);
#else
        *puLong  = 0;
#endif
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        dhcp6c_mapt_mape_GetParamStringValue
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
dhcp6c_mapt_mape_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    UNREFERENCED_PARAMETER(hInsContext);
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
    char temp[64] = {0};
#endif
    UNREFERENCED_PARAMETER(pUlSize);
    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "MapTransportMode") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAP_TRANSPORT_MODE, temp, sizeof(temp));
        if ( AnscSizeOfString(temp) < *pUlSize)
        {
            AnscCopyString(pValue, temp);
#if defined (FEATURE_SUPPORT_MAPT_NAT46)
            if ( !(*temp) )
            {
                 AnscCopyString(pValue, "NONE");
            }
#endif
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(temp)+1;
            return 1;
        }
#else
        AnscCopyString(pValue, "");
        return 0;
#endif
    }

    if (strcmp(ParamName, "MapBRPrefix") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAP_BR_IPV6_PREFIX, temp, sizeof(temp));
        if ( AnscSizeOfString(temp) < *pUlSize)
        {
            AnscCopyString(pValue, temp);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(temp)+1;
            return 1;
        }
#else
        AnscCopyString(pValue, "");
        return 0;
#endif
    }

    if (strcmp(ParamName, "MapRuleIPv4Prefix") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
         sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAP_RULE_IPADDRESS, temp, sizeof(temp));
        if ( AnscSizeOfString(temp) < *pUlSize)
        {
            AnscCopyString(pValue, temp);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(temp)+1;
            return 1;
        }
#else
        AnscCopyString(pValue, "");
        return 0;
#endif
    }

    if (strcmp(ParamName, "MapRuleIPv6Prefix") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAP_RULE_IPV6_ADDRESS, temp, sizeof(temp));
        if ( AnscSizeOfString(temp) < *pUlSize)
        {
            AnscCopyString(pValue, temp);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(temp)+1;
            return 1;
        }
#else
        AnscCopyString(pValue, "");
        return 0;
#endif
    }

    if (strcmp(ParamName, "MapIpv4Address") == 0)
    {
#if defined(_HUB4_PRODUCT_REQ_) || defined(FEATURE_SUPPORT_MAPT_NAT46)
        sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAPT_IPADDRESS, temp, sizeof(temp));
        if ( AnscSizeOfString(temp) < *pUlSize)
        {
            AnscCopyString(pValue, temp);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(temp)+1;
            return 1;
        }
#else
        AnscCopyString(pValue, "");
        return 0;
#endif
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}
