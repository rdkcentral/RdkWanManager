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

/**************************************************************************

    module: wan_dhcpv6_internal.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  WanMgr_Dhcpv6Create
        *  WanMgr_Dhcpv6Initialize
        *  WanMgr_Dhcpv6Remove
        *  WanMgr_Dhcpv6RegGetDhcpv6Info
        *  WanMgr_Dhcpv6RegSetDhcpv6Info
        *  WanMgr_Dhcpv6ClientHasDelayAddedChild

**************************************************************************/
#include "wanmgr_apis.h"
#include "wanmgr_dhcpv6_apis.h"
#include "wanmgr_dhcpv6_internal.h"
#include "wanmgr_plugin_main_apis.h"
#include "poam_irepfo_interface.h"
#include "sys_definitions.h"

extern void * g_pDslhDmlAgent;


/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE
        WanMgr_Dhcpv6Create
            (
            );

    description:

        This function constructs cosa nat object and return handle.

    argument:  

    return:     newly created nat object.

**********************************************************************/

ANSC_HANDLE
WanMgr_Dhcpv6Create
    (
        VOID
    )
{
    ANSC_STATUS       returnStatus =  ANSC_STATUS_SUCCESS;
    PWAN_DHCPV6_DATA  pMyObject    =  (PWAN_DHCPV6_DATA) NULL;

	/*
     * We create object by first allocating memory for holding the variables and member functions.
    */
    pMyObject = (PWAN_DHCPV6_DATA)AnscAllocateMemory(sizeof(WAN_DHCPV6_DATA));

    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    pMyObject->Oid               = WAN_DHCPV6_DATA_OID;
    pMyObject->Create            = WanMgr_Dhcpv6Create;
    pMyObject->Remove            = WanMgr_Dhcpv6Remove;
    pMyObject->Initialize        = WanMgr_Dhcpv6Initialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        WanMgr_Dhcpv6Initialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa nat object and return handle.

    argument:    ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
WanMgr_Dhcpv6Initialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS               returnStatus      = ANSC_STATUS_SUCCESS;
    PPOAM_IREP_FOLDER_OBJECT  pPoamIrepFoCOSA   = NULL;
    PPOAM_IREP_FOLDER_OBJECT  pPoamIrepFoDhcpv6 = NULL;
	PWAN_DHCPV6_DATA          pMyObject         = (PWAN_DHCPV6_DATA) hThisObject;

    /* We need call the initiation function of backend firstly .
       When backend return failure, we don't return because if return, all middle layer function will be not complete*/
    if (pMyObject == NULL)
    {
        AnscTraceError(("%s:%d:: Pointer is null!!\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

#if defined(_DT_WAN_Manager_Enable_)
    WanMgr_DmlDhcpv6Init( NULL, NULL );
#endif
    /* Initiation all functions */
    AnscSListInitializeHeader( &pMyObject->ClientList );
    pMyObject->maxInstanceOfClient  = 0;
    AnscZeroMemory(pMyObject->AliasOfClient, sizeof(pMyObject->AliasOfClient));

    /*We need to get Instance Info from cosa configuration*/
    pPoamIrepFoCOSA = (PPOAM_IREP_FOLDER_OBJECT)g_GetRegistryRootFolder(g_pDslhDmlAgent);
    if ( !pPoamIrepFoCOSA )
    {
        returnStatus = ANSC_STATUS_FAILURE;
        return returnStatus;
    }

    pPoamIrepFoDhcpv6 = 
       (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFoCOSA->GetFolder
            (
             (ANSC_HANDLE)pPoamIrepFoCOSA,
             DHCPV6_IREP_FOLDER_NAME
       );

    if ( !pPoamIrepFoDhcpv6 )
    {
        pPoamIrepFoCOSA->EnableFileSync((ANSC_HANDLE)pPoamIrepFoCOSA, FALSE);
        pPoamIrepFoDhcpv6 =
             pPoamIrepFoCOSA->AddFolder
             (
                 (ANSC_HANDLE)pPoamIrepFoCOSA,
                 DHCPV6_IREP_FOLDER_NAME,
                 0
             );
        pPoamIrepFoCOSA->EnableFileSync((ANSC_HANDLE)pPoamIrepFoCOSA, TRUE);
    }

    if ( !pPoamIrepFoDhcpv6 )
    {
        returnStatus = ANSC_STATUS_FAILURE;
        return returnStatus;
    }
    else
    {
        pMyObject->hIrepFolderDhcpv6 = (ANSC_HANDLE)pPoamIrepFoDhcpv6;
    }

    /* We need get NextInstanceNumber from backend. By the way, the whole tree 
    was created. Moreover, we also need get delay-added entry and put them
    into our tree. */
    WanMgr_Dhcpv6RegGetDhcpv6Info((ANSC_HANDLE)pMyObject);

    /* Firstly we create the whole system from backend */
    WanMgr_Dhcpv6BackendGetDhcpv6Info((ANSC_HANDLE)pMyObject);

    return returnStatus;
}



/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        WanMgr_Dhcpv6Remove
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa nat object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
WanMgr_Dhcpv6Remove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                  returnStatus        = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pCxtDhcpcLink       = NULL;
    PCONTEXT_LINK_OBJECT         pCxtLink            = NULL;
    PSINGLE_LINK_ENTRY           pSListEntry         = NULL;
    PSINGLE_LINK_ENTRY           pSListEntry2        = NULL;
    BOOL                         bFound              = FALSE;
    ULONG                        Index               = 0;
    PWAN_DHCPV6_DATA             pMyObject           = (PWAN_DHCPV6_DATA)hThisObject;

    if (pMyObject != NULL)
    {
        AnscTraceError(("%s:%d:: Pointer is null!!\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoDhcpv6   = (PPOAM_IREP_FOLDER_OBJECT)pMyObject->hIrepFolderDhcpv6;

    pSListEntry         = AnscSListPopEntry(&pMyObject->ClientList);
    while( pSListEntry != NULL)
    {
        pCxtDhcpcLink     = ACCESS_DHCPCV6_CONTEXT_LINK_OBJECT(pSListEntry);
        pSListEntry       = AnscSListGetNextEntry(pSListEntry);

        pSListEntry2         = AnscSListPopEntry(&pCxtDhcpcLink->SentOptionList);
        while( pSListEntry2 != NULL)
        {
            pCxtLink         = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry2);
            pSListEntry2       = AnscSListGetNextEntry(pSListEntry2);
            AnscFreeMemory(pCxtLink->hContext);
            AnscFreeMemory(pCxtLink);  
        }

        AnscFreeMemory( pCxtDhcpcLink->pServerEntry );
        AnscFreeMemory( pCxtDhcpcLink->pRecvEntry );
        AnscFreeMemory(pCxtDhcpcLink->hContext);
        AnscFreeMemory(pCxtDhcpcLink);
    }

    if ( pPoamIrepFoDhcpv6 )
    {
        pPoamIrepFoDhcpv6->Remove( (ANSC_HANDLE)pPoamIrepFoDhcpv6);
    }

    WanMgr_DmlDhcpv6Remove((ANSC_HANDLE)pMyObject);
    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_STATUS
        WanMgr_Dhcpv6RegGetDhcpv6Info
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function is called to retrieve the NextInstanceNumber for every table, Create
        the link tree. For delay_added entry, we also need create them.

    argument:   ANSC_HANDLE                 hThisObject
                This handle is actually the pointer of DHCPv6
                itself.

    return:     status of operation.

**********************************************************************/

ANSC_STATUS
WanMgr_Dhcpv6BackendGetDhcpv6Info
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;    
    PWAN_DHCPV6_DATA          pDhcpv6           = (PWAN_DHCPV6_DATA)hThisObject;
    
    PDML_DHCPCV6_FULL          pDhcpc            = NULL;    
    PDML_DHCPCV6_SVR           pDhcpcServer      = NULL;
    PDML_DHCPCV6_RECV          pDhcpcRecv        = NULL;    
    PDML_DHCPCV6_SENT          pSentOption       = NULL;
    
    ULONG                           clientCount       = 0;
    ULONG                           count             = 0;
    ULONG                           count1            = 0;
    ULONG                           ulIndex           = 0;
    ULONG                           ulIndex2          = 0;

    PDHCPCV6_CONTEXT_LINK_OBJECT pClientCxtLink    = NULL;    
    PDHCPCV6_CONTEXT_LINK_OBJECT pClientCxtLink2   = NULL;       
    PCONTEXT_LINK_OBJECT         pCxtLink          = NULL;
    PCONTEXT_LINK_OBJECT         pCxtLink2         = NULL;

    BOOL                            bNeedSave         = FALSE;

    /* Get DHCPv6.Client.{i} */
    clientCount = WanMgr_DmlDhcpv6cGetNumberOfEntries(NULL);
    for ( ulIndex = 0; ulIndex < clientCount; ulIndex++ )
    {
        pDhcpc  = (PDML_DHCPCV6_FULL)AnscAllocateMemory( sizeof(DML_DHCPCV6_FULL) );
        if ( !pDhcpc )
        {
            break;
        }

        DHCPV6_CLIENT_SET_DEFAULTVALUE(pDhcpc);
        returnStatus = WanMgr_DmlDhcpv6cGetEntry(NULL, ulIndex, pDhcpc);
        if ( returnStatus != ANSC_STATUS_SUCCESS )
        {
            AnscFreeMemory(pDhcpc);
            break;
        }

        pClientCxtLink = (PDHCPCV6_CONTEXT_LINK_OBJECT)AnscAllocateMemory( sizeof(DHCPCV6_CONTEXT_LINK_OBJECT) );
        if ( !pClientCxtLink )
        {
            AnscFreeMemory(pDhcpc);
            break;
        }

        DHCPV6_CLIENT_INITIATION_CONTEXT(pClientCxtLink)
        pClientCxtLink->hContext       = (ANSC_HANDLE)pDhcpc;
        pClientCxtLink->bNew           = FALSE;
        pClientCxtLink->hParentTable   = (ANSC_HANDLE)pDhcpv6;

        if ( !pDhcpc->Cfg.InstanceNumber )
        {
            if ( !++pDhcpv6->maxInstanceOfClient )
            {
                pDhcpv6->maxInstanceOfClient = 1;
            }
            bNeedSave                        = TRUE;

            pDhcpc->Cfg.InstanceNumber     = pDhcpv6->maxInstanceOfClient;
            pClientCxtLink->InstanceNumber = pDhcpc->Cfg.InstanceNumber;
            
            _ansc_sprintf((char *)pDhcpc->Cfg.Alias, "DHCPv6%lu", pDhcpc->Cfg.InstanceNumber);

            returnStatus = WanMgr_DmlDhcpv6cSetValues
                            (
                                NULL, 
                                ulIndex,
                                pDhcpc->Cfg.InstanceNumber, 
                                (char *)pDhcpc->Cfg.Alias
                            );

            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                AnscFreeMemory(pDhcpc);
                AnscFreeMemory(pClientCxtLink);
                break;
            }

            /* Put into our list */
            SListPushEntryByInsNum(&pDhcpv6->ClientList, (PCONTEXT_LINK_OBJECT)pClientCxtLink);
        }
        else
        {
            pClientCxtLink->InstanceNumber = pDhcpc->Cfg.InstanceNumber;

            /* This case never happen. Add it just for simulation code run well */
            if ( pDhcpv6->maxInstanceOfClient < pClientCxtLink->InstanceNumber )
            {
                pDhcpv6->maxInstanceOfClient = pClientCxtLink->InstanceNumber;
                bNeedSave                    = TRUE;
            }

            /* if this entry is in link tree already because it's the parent of delay_added table */
            pClientCxtLink2 = (PDHCPCV6_CONTEXT_LINK_OBJECT)SListGetEntryByInsNum(&pDhcpv6->ClientList, pClientCxtLink->InstanceNumber);
            if ( !pClientCxtLink2 )
            {
                SListPushEntryByInsNum(&pDhcpv6->ClientList, (PCONTEXT_LINK_OBJECT)pClientCxtLink);
            }
            else
            {
                /* When this case happens, somethings happens to be error. We harmonize it here.*/
                AnscFreeMemory( pClientCxtLink2->hContext );
                pClientCxtLink2->hContext       = (ANSC_HANDLE)pDhcpc;
                if ( pClientCxtLink2->bNew )
                {
                    pClientCxtLink2->bNew       = FALSE;
                    bNeedSave                   = TRUE;
                }
                
                AnscFreeMemory(pClientCxtLink);
                pClientCxtLink                  = pClientCxtLink2;
                pClientCxtLink2                 = NULL;
            }            
        }

        /* We begin treat DHCPv6.Client.{i}.Server.{i} 
                    This is one dynamic table. We get all once */
        returnStatus = WanMgr_DmlDhcpv6cGetServerCfg
                        (
                            NULL,
                            pDhcpc->Cfg.InstanceNumber,
                            &pDhcpcServer,
                            &count
                        );
        if ( returnStatus == ANSC_STATUS_SUCCESS )
        {
            pClientCxtLink->pServerEntry    = pDhcpcServer;
            pClientCxtLink->NumberOfServer  = count;
        }
        else
        {
            CcspTraceWarning(("WanMgr_Dhcpv6BackendGetDhcpv6Info -- WanMgr_DmlDhcpv6cGetServerCfg() return error:%d.\n", returnStatus));
        }
 
        /* We begin treat DHCPv6.Client.{i}.SentOption.{i} */
        count = WanMgr_DmlDhcpv6cGetNumberOfSentOption
                    (
                        NULL,
                        pDhcpc->Cfg.InstanceNumber
                    );
        
        for ( ulIndex2 = 0; ulIndex2 < count; ulIndex2++ )
        {
            pSentOption  = (PDML_DHCPCV6_SENT)AnscAllocateMemory( sizeof(DML_DHCPCV6_SENT) );
            if ( !pSentOption )
            {
                break;
            }
        
            DHCPV6_SENTOPTION_SET_DEFAULTVALUE(pSentOption);
            returnStatus = WanMgr_DmlDhcpv6cGetSentOption
                            (
                                NULL, 
                                pDhcpc->Cfg.InstanceNumber, 
                                ulIndex2, 
                                pSentOption
                            );
            if ( returnStatus != ANSC_STATUS_SUCCESS )
            {
                AnscFreeMemory(pSentOption);
                break;
            }
        
            pCxtLink = (PCONTEXT_LINK_OBJECT)AnscAllocateMemory( sizeof(CONTEXT_LINK_OBJECT) );
            if ( !pCxtLink )
            {
                AnscFreeMemory(pSentOption);
                break;
            }
        
            CONTEXT_LINK_INITIATION_CONTENT(pCxtLink);
            pCxtLink->hContext       = (ANSC_HANDLE)pSentOption;
            pCxtLink->hParentTable   = (ANSC_HANDLE)pClientCxtLink;
            pCxtLink->bNew           = FALSE;
        
            if ( !pSentOption->InstanceNumber )
            {
                if ( !++pClientCxtLink->maxInstanceOfSent )
                {
                    pClientCxtLink->maxInstanceOfSent = 1;
                    bNeedSave                  = TRUE;
                }
                bNeedSave                        = TRUE;
                pSentOption->InstanceNumber = pClientCxtLink->maxInstanceOfSent;

                _ansc_sprintf( (char *)pSentOption->Alias, "SentOption%lu", pSentOption->InstanceNumber );
        
                returnStatus = WanMgr_DmlDhcpv6cSetSentOptionValues
                                (
                                    NULL,
                                    pDhcpc->Cfg.InstanceNumber,
                                    ulIndex, 
                                    pSentOption->InstanceNumber, 
                                    (char *)pSentOption->Alias
                                );
        
                if ( returnStatus != ANSC_STATUS_SUCCESS )
                {
                    AnscFreeMemory(pSentOption);
                    AnscFreeMemory(pCxtLink);
                    break;                    
                }

                pCxtLink->InstanceNumber = pSentOption->InstanceNumber; 
            
                /* Put into our list */
                SListPushEntryByInsNum(&pClientCxtLink->SentOptionList, (PCONTEXT_LINK_OBJECT)pCxtLink);
                
            } 
            else
            {
                pCxtLink->InstanceNumber = pSentOption->InstanceNumber; 
                
                /* This case never happen. Add it just for simulation code run well */
                if ( pClientCxtLink->maxInstanceOfSent < pSentOption->InstanceNumber )
                {
                    pClientCxtLink->maxInstanceOfSent = pSentOption->InstanceNumber;
                }

                /* if this entry is in link tree already because it's  delay_added table */
                pCxtLink2 = (PCONTEXT_LINK_OBJECT)SListGetEntryByInsNum(&pClientCxtLink->SentOptionList, pSentOption->InstanceNumber);
                if ( !pCxtLink2 )
                {
                    SListPushEntryByInsNum(&pClientCxtLink->SentOptionList, (PCONTEXT_LINK_OBJECT)pCxtLink);
                }
                else
                {
                    AnscFreeMemory( pCxtLink2->hContext );                
                    pCxtLink2->hContext       = (ANSC_HANDLE)pSentOption;
                    if ( pCxtLink2->bNew )
                    {
                        pCxtLink2->bNew       = FALSE;
                        bNeedSave             = TRUE;
                    }

                    AnscFreeMemory(pCxtLink);
                    pCxtLink                  = pCxtLink2;
                    pCxtLink2                 = NULL;
                }            

            }
        }
        // WanMgr_DmlStartDHCP6Client();
        /* We begin treat DHCPv6.Client.{i}.ReceivedOption.{i} 
                    This is one dynamic table. We get all once */
        returnStatus = WanMgr_DmlDhcpv6cGetReceivedOptionCfg
                        (
                            NULL,
                            pDhcpc->Cfg.InstanceNumber,
                            &pDhcpcRecv,
                            &count
                        );
        if ( returnStatus == ANSC_STATUS_SUCCESS )
        {
            pClientCxtLink->pRecvEntry    = pDhcpcRecv;
            pClientCxtLink->NumberOfRecv  = count;
        }
        else
        {
            CcspTraceWarning(("WanMgr_Dhcpv6BackendGetDhcpv6Info -- WanMgr_DmlDhcpv6cGetReceivedOptionCfg() return error:%d.\n", returnStatus));
        }
        
    }

    return returnStatus;    
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_STATUS
        WanMgr_Dhcpv6RegGetDhcpv6Info
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function is called to retrieve backend inform and put them into our trees.

    argument:   ANSC_HANDLE                 hThisObject
                This handle is actually the pointer of DHCPv6
                itself.

    return:     status of operation.

**********************************************************************/

ANSC_STATUS
WanMgr_Dhcpv6RegGetDhcpv6Info
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PWAN_DHCPV6_DATA          pMyObject         = (PWAN_DHCPV6_DATA   )hThisObject;
    
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoDhcpv6 = (PPOAM_IREP_FOLDER_OBJECT )pMyObject->hIrepFolderDhcpv6;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoClient = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoSntOpt = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoEnumClient       = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoEnumSntOpt    = (PPOAM_IREP_FOLDER_OBJECT )NULL;

    PDHCPCV6_CONTEXT_LINK_OBJECT pDhcpcContext = NULL;
    PCONTEXT_LINK_OBJECT         pDhcpv6SentOptionContext     = NULL;
    
    PSLAP_VARIABLE                  pSlapVariable     = NULL;
    ULONG                           ulEntryCount      = 0;
    ULONG                           ulIndex           = 0;
    ULONG                           ulEntryCount2     = 0;
    ULONG                           ulIndex2          = 0;
    ULONG                           uInstanceNumber   = 0;
    BOOL                            bNew              = FALSE;
    char*                           pAliasClient      = NULL;
    char*                           pAliasSentOption  = NULL;
    char*                           pFolderName       = NULL;
    
    PDML_DHCPCV6_FULL          pDhcpv6Client     = NULL;
    PDML_DHCPCV6_SENT          pDhcpv6SntOpt     = NULL;

    if ( !pPoamIrepFoDhcpv6 )
    {
        return ANSC_STATUS_FAILURE;
    }

    /* This is saved structure for DHCPv6
        *****************************************
              <Dhcpv6>
                  <client>
                      <NextInstanceNumber> xxx </>
                      <1>
                           <alias>xxx</>
                           <bNew>false</>
                          <SendOption>
                              <NextInstanceNumber> xxx </>
                              <1>
                                  <alias>xxx</>
                                  <bNew>true</>
                              </1>
                          </SendOption>
                     </1>
               </client>
              <pool>
                    <NextInstanceNumber> xxx </>
                    <1>
                        <alias>xxx</>
                        <bNew>true</>
                        <Option>
                            <NextInstanceNumber> xxx </>
                            <1>
                                <alias>xxx</>
                                <bNew>true</>
                            </1>
                        </Option>
                    </1>
              </pool>
            </Dhcpv6>
      ****************************************************
      */

    /* Get Folder Client */ 
    pPoamIrepFoClient  = 
        (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFoDhcpv6->GetFolder
            (
                (ANSC_HANDLE)pPoamIrepFoDhcpv6,
                DHCPV6_IREP_FOLDER_NAME_CLIENT
            );

    if ( !pPoamIrepFoClient )
    {
        returnStatus      = ANSC_STATUS_FAILURE;
        goto EXIT1;
    }

    /* Get Client.NextInstanceNumber */
    if ( TRUE )
    {
        pSlapVariable =
            (PSLAP_VARIABLE)pPoamIrepFoClient->GetRecord
                (
                    (ANSC_HANDLE)pPoamIrepFoClient,
                    COSA_DML_RR_NAME_Dhcpv6NextInsNunmber,
                    NULL
                );

        if ( pSlapVariable )
        {
            pMyObject->maxInstanceOfClient = pSlapVariable->Variant.varUint32;

            SlapFreeVariable(pSlapVariable);
        }
    }

    /* enumerate client.{i} */
    ulEntryCount = pPoamIrepFoClient->GetFolderCount((ANSC_HANDLE)pPoamIrepFoClient);
    for ( ulIndex = 0; ulIndex < ulEntryCount; ulIndex++ )
    {
        /* Get i in client.{i} */
        pFolderName =
            pPoamIrepFoClient->EnumFolder
                (
                    (ANSC_HANDLE)pPoamIrepFoClient,
                    ulIndex
                );

        if ( !pFolderName )
        {
            continue;
        }

        uInstanceNumber = _ansc_atol(pFolderName);

        if ( uInstanceNumber == 0 )
        {
            AnscFreeMemory(pFolderName);
            continue;
        }

        /*get folder client.{i} */
        pPoamIrepFoEnumClient = pPoamIrepFoClient->GetFolder((ANSC_HANDLE)pPoamIrepFoClient, pFolderName);

        AnscFreeMemory(pFolderName);

        if ( !pPoamIrepFoEnumClient )
        {
            continue;
        }

        /* Get client.{i}.Alias value*/
        if ( TRUE )
        {
            pSlapVariable =
                (PSLAP_VARIABLE)pPoamIrepFoEnumClient->GetRecord
                    (
                        (ANSC_HANDLE)pPoamIrepFoEnumClient,
                        COSA_DML_RR_NAME_Dhcpv6Alias,
                        NULL
                    );

            if ( pSlapVariable )
            {
                pAliasClient = AnscCloneString(pSlapVariable->Variant.varString);

                SlapFreeVariable(pSlapVariable);
            }
        }

        /* Get client.{i}.bNew value*/
        if ( TRUE )
        {
            pSlapVariable =
                (PSLAP_VARIABLE)pPoamIrepFoEnumClient->GetRecord
                    (
                        (ANSC_HANDLE)pPoamIrepFoEnumClient,
                        COSA_DML_RR_NAME_Dhcpv6bNew,
                        NULL
                    );

            if ( pSlapVariable )
            {
                bNew = pSlapVariable->Variant.varBool;

                SlapFreeVariable(pSlapVariable);
            }
            else
            {
                bNew = TRUE;
            }
        }

        /* Create one entry and keep this delay_added entry */
        /* Firstly create dhcpc content struct */
        pDhcpv6Client = (PDML_DHCPCV6_FULL)AnscAllocateMemory(sizeof(DML_DHCPCV6_FULL));
        if ( !pDhcpv6Client )
        {
            returnStatus = ANSC_STATUS_FAILURE;            
            goto EXIT2;
        }

        /* set some default value firstly */
        DHCPV6_CLIENT_SET_DEFAULTVALUE(pDhcpv6Client);

        /* save alias and instanceNumber */
        pDhcpv6Client->Cfg.InstanceNumber = uInstanceNumber;
        AnscCopyString( (char *)pDhcpv6Client->Cfg.Alias, pAliasClient );
        if (pAliasClient)
        {
            AnscFreeMemory(pAliasClient);
            pAliasClient = NULL;
        }
        
        /* Create one link point */
        pDhcpcContext = (PDHCPCV6_CONTEXT_LINK_OBJECT)AnscAllocateMemory(sizeof(DHCPCV6_CONTEXT_LINK_OBJECT));
        if ( !pDhcpcContext )
        {
            returnStatus = ANSC_STATUS_FAILURE;
            goto EXIT3;
        }

        DHCPV6_CLIENT_INITIATION_CONTEXT(pDhcpcContext)

        pDhcpcContext->InstanceNumber = uInstanceNumber;
        pDhcpcContext->hContext       = (ANSC_HANDLE)pDhcpv6Client;
        pDhcpv6Client                     = NULL; /* reset to NULL */
        pDhcpcContext->bNew           = bNew; /* set to true */

        SListPushEntryByInsNum(&pMyObject->ClientList, (PCONTEXT_LINK_OBJECT)pDhcpcContext);

        /* 
                   Begin treat client.{i}.sentOption. 
                */
        pPoamIrepFoSntOpt = 
            (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFoEnumClient->GetFolder
                (
                    (ANSC_HANDLE)pPoamIrepFoEnumClient,
                    DHCPV6_IREP_FOLDER_NAME_SENTOPTION
                );
        
        if ( !pPoamIrepFoSntOpt )
        {
            goto ClientEnd;
        }
        
        /* Get Maximum number */
        if ( TRUE )
        {
            pSlapVariable =
                (PSLAP_VARIABLE)pPoamIrepFoSntOpt->GetRecord
                    (
                        (ANSC_HANDLE)pPoamIrepFoSntOpt,
                        COSA_DML_RR_NAME_Dhcpv6NextInsNunmber,
                        NULL
                    );
        
            if ( pSlapVariable )
            {
                pDhcpcContext->maxInstanceOfSent = pSlapVariable->Variant.varUint32;
        
                SlapFreeVariable(pSlapVariable);
            }
        }
        
        /* enumerate client.{i}.sentOption.{i} */
        ulEntryCount2 = pPoamIrepFoSntOpt->GetFolderCount((ANSC_HANDLE)pPoamIrepFoSntOpt);

        for ( ulIndex2 = 0; ulIndex2 < ulEntryCount2; ulIndex2++ )
        {
            /* Get i in client.{i}.sentOption.{i} */
            pFolderName =
                pPoamIrepFoSntOpt->EnumFolder
                    (
                        (ANSC_HANDLE)pPoamIrepFoSntOpt,
                        ulIndex2
                    );
            
            if ( !pFolderName )
            {
                continue;
            }
            
            uInstanceNumber = _ansc_atol(pFolderName);
            
            if ( uInstanceNumber == 0 )
            {
                AnscFreeMemory(pFolderName); /* tom*/
                continue;
            }
        
            pPoamIrepFoEnumSntOpt = pPoamIrepFoSntOpt->GetFolder((ANSC_HANDLE)pPoamIrepFoSntOpt, pFolderName);
        
            AnscFreeMemory(pFolderName);
            
            if ( !pPoamIrepFoEnumSntOpt )
            {
                continue;
            }
            
            /* Get client.{i}.sentOption.{i}.Alias value*/
            if ( TRUE )
            {
                pSlapVariable =
                    (PSLAP_VARIABLE)pPoamIrepFoEnumSntOpt->GetRecord
                        (
                            (ANSC_HANDLE)pPoamIrepFoEnumSntOpt,
                            COSA_DML_RR_NAME_Dhcpv6Alias,
                            NULL
                        );
            
                if ( pSlapVariable )
                {
                    pAliasSentOption = AnscCloneString(pSlapVariable->Variant.varString);
            
                    SlapFreeVariable(pSlapVariable);
                }
            }
            
            /* Get client.{i}.sentOption.{i}.bNew value*/
            if ( TRUE )
            {
                pSlapVariable =
                    (PSLAP_VARIABLE)pPoamIrepFoEnumSntOpt->GetRecord
                        (
                            (ANSC_HANDLE)pPoamIrepFoEnumSntOpt,
                            COSA_DML_RR_NAME_Dhcpv6bNew,
                            NULL
                        );
            
                if ( pSlapVariable )
                {
                    bNew = pSlapVariable->Variant.varBool;
            
                    SlapFreeVariable(pSlapVariable);
                }
                else
                {
                    bNew = TRUE;
                }
            }

            /* Create one link and ask backend to get content */
            pDhcpv6SntOpt = (PDML_DHCPCV6_SENT)AnscAllocateMemory(sizeof(DML_DHCPCV6_SENT));
            if ( !pDhcpv6SntOpt )
            {
                returnStatus = ANSC_STATUS_FAILURE;
                AnscFreeMemory(pDhcpv6SntOpt); /*RDKB-6737, CID-32983, free unused resource before exit*/
                pDhcpv6SntOpt = NULL;
                goto EXIT3;
            }

            /* set some default value firstly */
            DHCPV6_SENTOPTION_SET_DEFAULTVALUE(pDhcpv6SntOpt);

            /* save alias and instanceNumber */
            pDhcpv6SntOpt->InstanceNumber = uInstanceNumber;
            AnscCopyString((char *) pDhcpv6SntOpt->Alias, pAliasSentOption );
            if (pAliasSentOption)
            {
                AnscFreeMemory(pAliasSentOption);
                pAliasSentOption = NULL;
            }

            pDhcpv6SentOptionContext = (PCONTEXT_LINK_OBJECT)AnscAllocateMemory(sizeof(CONTEXT_LINK_OBJECT));
            if ( !pDhcpv6SentOptionContext )
            {
                returnStatus = ANSC_STATUS_FAILURE;            
                goto EXIT2;
            }

            CONTEXT_LINK_INITIATION_CONTENT(pDhcpv6SentOptionContext);

            pDhcpv6SentOptionContext->InstanceNumber = uInstanceNumber;
            pDhcpv6SentOptionContext->hContext       = (ANSC_HANDLE)pDhcpv6SntOpt;
            pDhcpv6SntOpt = NULL;
            pDhcpv6SentOptionContext->hParentTable   = pDhcpcContext;
            pDhcpv6SentOptionContext->bNew           = bNew;

            SListPushEntryByInsNum(&pDhcpcContext->SentOptionList, (PCONTEXT_LINK_OBJECT)pDhcpv6SentOptionContext);
        
            /* release some memory */  
            pPoamIrepFoEnumSntOpt->Remove((ANSC_HANDLE)pPoamIrepFoEnumSntOpt);
            pPoamIrepFoEnumSntOpt = NULL;            
        }

        pPoamIrepFoSntOpt->Remove((ANSC_HANDLE)pPoamIrepFoSntOpt);
        pPoamIrepFoSntOpt = NULL;

ClientEnd:
        /* release some memory */
        pPoamIrepFoEnumClient->Remove((ANSC_HANDLE)pPoamIrepFoEnumClient);
        pPoamIrepFoEnumClient = NULL;
        
    }

    pPoamIrepFoClient->Remove((ANSC_HANDLE)pPoamIrepFoClient);
    pPoamIrepFoClient = NULL;
      
EXIT3:
    if(pDhcpv6Client)
        AnscFreeMemory(pDhcpv6Client);
        
EXIT2:
    
    if(pAliasSentOption)
        AnscFreeMemory(pAliasSentOption);
    
    if(pAliasClient)
        AnscFreeMemory(pAliasClient);

    if(pDhcpv6SntOpt )
        AnscFreeMemory(pDhcpv6SntOpt);
    
EXIT1:

    if ( pPoamIrepFoClient )
        pPoamIrepFoClient->Remove((ANSC_HANDLE)pPoamIrepFoClient);

    if ( pPoamIrepFoEnumClient )
        pPoamIrepFoEnumClient->Remove((ANSC_HANDLE)pPoamIrepFoEnumClient);

    if ( pPoamIrepFoSntOpt)
        pPoamIrepFoSntOpt->Remove((ANSC_HANDLE)pPoamIrepFoSntOpt);

    if ( pPoamIrepFoEnumSntOpt)
        pPoamIrepFoEnumSntOpt->Remove((ANSC_HANDLE)pPoamIrepFoEnumSntOpt);

    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_STATUS
        WanMgr_Dhcpv6RegSetDhcpv6Info
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function is called to save current NextInstanceNumber and Delay_added
        entry into sysregistry.

    argument:   ANSC_HANDLE                 hThisObject
                This handle is actually the pointer of DHCPv6
                itself.

    return:     status of operation.

**********************************************************************/

ANSC_STATUS
WanMgr_Dhcpv6RegSetDhcpv6Info
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PWAN_DHCPV6_DATA          pMyObject         = (PWAN_DHCPV6_DATA   )hThisObject;
    
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoDhcpv6 = (PPOAM_IREP_FOLDER_OBJECT )pMyObject->hIrepFolderDhcpv6;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoClient = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoSntOpt = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoPool   = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoPoolOption      = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoEnumClient      = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoEnumSntOpt      = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoEnumPool        = (PPOAM_IREP_FOLDER_OBJECT )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoEnumPoolOption  = (PPOAM_IREP_FOLDER_OBJECT )NULL;

    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY       )NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry2      = (PSINGLE_LINK_ENTRY       )NULL;
    
    PDHCPCV6_CONTEXT_LINK_OBJECT pDhcpcContext = NULL;
    PCONTEXT_LINK_OBJECT         pSentOptionContext   = NULL;
    
    PSLAP_VARIABLE                  pSlapVariable     = NULL;
    ULONG                           ulEntryCount      = 0;
    ULONG                           ulIndex           = 0;
    ULONG                           ulEntryCount2     = 0;
    ULONG                           ulIndex2          = 0;
    ULONG                           uInstanceNumber   = 0;
    char*                           pAliasClient      = NULL;
    char*                           pAliasSentOption  = NULL;
    char*                           pAliasPool        = NULL;
    char*                           pAliasPoolOption  = NULL;
    char*                           pFolderName       = NULL;
    char                            FolderName[16]    = {0};
    
    PDML_DHCPCV6_FULL          pDhcpv6Client     = NULL;
    PDML_DHCPCV6_SENT          pDhcpv6SntOpt     = NULL;

    if ( !pPoamIrepFoDhcpv6 )
    {
        return ANSC_STATUS_FAILURE;
    }
    else
    {
        pPoamIrepFoDhcpv6->EnableFileSync((ANSC_HANDLE)pPoamIrepFoDhcpv6, FALSE);
    }

    if ( TRUE )
    {
        pPoamIrepFoDhcpv6->Clear((ANSC_HANDLE)pPoamIrepFoDhcpv6);

        SlapAllocVariable(pSlapVariable);

        if ( !pSlapVariable )
        {
            returnStatus = ANSC_STATUS_RESOURCES;

            goto  EXIT1;
        }
    }

    /* This is saved structure for DHCPv6
          *****************************************
                <Dhcpv6>
                    <client>
                        <NextInstanceNumber> xxx </>
                        <1>
                             <alias>xxx</>
                             <bNew>false</>
                            <SendOption>
                                <NextInstanceNumber> xxx </>
                                <1>
                                    <alias>xxx</>
                                    <bNew>true</>
                                </1>
                            </SendOption>
                       </1>
                 </client>
                <pool>
                      <NextInstanceNumber> xxx </>
                      <1>
                          <alias>xxx</>
                          <bNew>true</>
                          <Option>
                              <NextInstanceNumber> xxx </>
                              <1>
                                  <alias>xxx</>
                                  <bNew>true</>
                              </1>
                          </Option>
                      </1>
                </pool>
              </Dhcpv6>
        ****************************************************
      */

    /* Add DHCPv6.client.*/
    pPoamIrepFoClient =
        pPoamIrepFoDhcpv6->AddFolder
            (
                (ANSC_HANDLE)pPoamIrepFoDhcpv6,
                DHCPV6_IREP_FOLDER_NAME_CLIENT,
                0
            );

    if ( !pPoamIrepFoClient )
    {
        goto EXIT1;
    }

    /* add client.{i}.NextInstanceNumber  */
    if ( TRUE )
    {
        pSlapVariable->Syntax            = SLAP_VAR_SYNTAX_uint32;
        pSlapVariable->Variant.varUint32 = pMyObject->maxInstanceOfClient;
    
        returnStatus =
            pPoamIrepFoClient->AddRecord
                (
                    (ANSC_HANDLE)pPoamIrepFoClient,
                    COSA_DML_RR_NAME_Dhcpv6NextInsNunmber,
                    SYS_REP_RECORD_TYPE_UINT,
                    SYS_RECORD_CONTENT_DEFAULT,
                    pSlapVariable,
                    0
                );
    
        SlapCleanVariable(pSlapVariable);
        SlapInitVariable (pSlapVariable);
    }
    
    pSLinkEntry = AnscSListGetFirstEntry(&pMyObject->ClientList);

    while ( pSLinkEntry )
    {
        /* create DHCPv6.client.{i} */
        
        pDhcpcContext = ACCESS_DHCPCV6_CONTEXT_LINK_OBJECT(pSLinkEntry);
        pSLinkEntry       = AnscSListGetNextEntry(pSLinkEntry);

        pDhcpv6Client = (PDML_DHCPCV6_FULL)pDhcpcContext->hContext;

        /* When this entry has been added to backend, has not any child and maxInstanceNumber is 0
                  We need not save this entry  */
        if ( !pDhcpcContext->bNew && 
             !AnscSListQueryDepth(&pDhcpcContext->SentOptionList ) &&
             ( pDhcpcContext->maxInstanceOfSent == 0 )
           )
        {
            continue;
        }

        _ansc_sprintf(FolderName, "%lu", pDhcpcContext->InstanceNumber);

        pPoamIrepFoEnumClient =
            pPoamIrepFoClient->AddFolder
                (
                    (ANSC_HANDLE)pPoamIrepFoClient,
                    FolderName,
                    0
                );

        if ( !pPoamIrepFoEnumClient )
        {
            continue;
        }

        /* add  DHCPv6.client.{i}.alias */
        if ( TRUE )
        {
            pSlapVariable->Syntax            = SLAP_VAR_SYNTAX_string;
            pSlapVariable->Variant.varString = AnscCloneString((char *)pDhcpv6Client->Cfg.Alias);

            returnStatus =
                pPoamIrepFoEnumClient->AddRecord
                    (
                        (ANSC_HANDLE)pPoamIrepFoEnumClient,
                        COSA_DML_RR_NAME_Dhcpv6Alias,
                        SYS_REP_RECORD_TYPE_ASTR,
                        SYS_RECORD_CONTENT_DEFAULT,
                        pSlapVariable,
                        0
                    );

            SlapCleanVariable(pSlapVariable);
            SlapInitVariable (pSlapVariable);
        }

        /* add  DHCPv6.client.{i}.bNew */
        if ( TRUE )
        {
            pSlapVariable->Syntax            = SLAP_VAR_SYNTAX_bool;
            pSlapVariable->Variant.varBool   = pDhcpcContext->bNew;

            returnStatus =
                pPoamIrepFoEnumClient->AddRecord
                    (
                        (ANSC_HANDLE)pPoamIrepFoEnumClient,
                        COSA_DML_RR_NAME_Dhcpv6bNew,
                        SYS_REP_RECORD_TYPE_BOOL,
                        SYS_RECORD_CONTENT_DEFAULT,
                        pSlapVariable,
                        0
                    );

            SlapCleanVariable(pSlapVariable);
            SlapInitVariable (pSlapVariable);
        }

        /*
                    begin add sentOption
                */
        if ( !AnscSListQueryDepth(&pDhcpcContext->SentOptionList) )
        {
            goto ClientEnd;
        }

        /* Add DHCPv6.client.{i}.sentOption */
        pPoamIrepFoSntOpt =
            pPoamIrepFoEnumClient->AddFolder
                (
                    (ANSC_HANDLE)pPoamIrepFoEnumClient,
                    DHCPV6_IREP_FOLDER_NAME_SENTOPTION,
                    0
                );
        
        if ( !pPoamIrepFoSntOpt )
        {
            goto EXIT1;
        }
        
        /* add client.{i}.sendOption.maxInstanceNumber  */
        if ( TRUE )
        {
            pSlapVariable->Syntax            = SLAP_VAR_SYNTAX_uint32;
            pSlapVariable->Variant.varUint32 = pDhcpcContext->maxInstanceOfSent;
        
            returnStatus =
                pPoamIrepFoSntOpt->AddRecord
                    (
                        (ANSC_HANDLE)pPoamIrepFoSntOpt,
                        COSA_DML_RR_NAME_Dhcpv6NextInsNunmber,
                        SYS_REP_RECORD_TYPE_UINT,
                        SYS_RECORD_CONTENT_DEFAULT,
                        pSlapVariable,
                        0
                    );
        
            SlapCleanVariable(pSlapVariable);
            SlapInitVariable (pSlapVariable);
        }
        
        pSLinkEntry2 = AnscSListGetFirstEntry(&pDhcpcContext->SentOptionList);
        
        while ( pSLinkEntry2 )
        {
            /* create DHCPv6.client.{i}.sentOption.{i} */
        
            pSentOptionContext = ACCESS_CONTEXT_LINK_OBJECT(pSLinkEntry2);
            pSLinkEntry2          = AnscSListGetNextEntry(pSLinkEntry2);
        
            pDhcpv6SntOpt= (PDML_DHCPCV6_SENT)pSentOptionContext->hContext;

            if ( !pSentOptionContext->bNew )
            {
                continue;
            }
        
            _ansc_sprintf(FolderName, "%lu", pSentOptionContext->InstanceNumber);
        
            pPoamIrepFoEnumSntOpt =
                pPoamIrepFoSntOpt->AddFolder
                    (
                        (ANSC_HANDLE)pPoamIrepFoSntOpt,
                        FolderName,
                        0
                    );
        
            if ( !pPoamIrepFoEnumSntOpt )
            {
                continue;
            }

            /* create DHCPv6.client.{i}.sendOption.{i}.alias */
            if ( TRUE )
            {
                pSlapVariable->Syntax            = SLAP_VAR_SYNTAX_string;
                pSlapVariable->Variant.varString = AnscCloneString((char *)pDhcpv6SntOpt->Alias);
        
                returnStatus =
                    pPoamIrepFoEnumSntOpt->AddRecord
                        (
                            (ANSC_HANDLE)pPoamIrepFoEnumSntOpt,
                            COSA_DML_RR_NAME_Dhcpv6Alias,
                            SYS_REP_RECORD_TYPE_ASTR,
                            SYS_RECORD_CONTENT_DEFAULT,
                            pSlapVariable,
                            0
                        );
                
                SlapCleanVariable(pSlapVariable);
                SlapInitVariable (pSlapVariable);
            }

            /* create DHCPv6.client.{i}.sendOption.{i}.bNew */
            if ( TRUE )
            {
                pSlapVariable->Syntax            = SLAP_VAR_SYNTAX_bool;
                pSlapVariable->Variant.varBool   = pSentOptionContext->bNew;
        
                returnStatus =
                    pPoamIrepFoEnumSntOpt->AddRecord
                        (
                            (ANSC_HANDLE)pPoamIrepFoEnumSntOpt,
                            COSA_DML_RR_NAME_Dhcpv6bNew,
                            SYS_REP_RECORD_TYPE_BOOL,
                            SYS_RECORD_CONTENT_DEFAULT,
                            pSlapVariable,
                            0
                        );
                
                SlapCleanVariable(pSlapVariable);
                SlapInitVariable (pSlapVariable);
            }

            pPoamIrepFoEnumSntOpt->Remove((ANSC_HANDLE)pPoamIrepFoEnumSntOpt);
            pPoamIrepFoEnumSntOpt = NULL;
        }
        
        pPoamIrepFoSntOpt->Remove((ANSC_HANDLE)pPoamIrepFoSntOpt);
        pPoamIrepFoSntOpt = NULL;
        
ClientEnd:

        /*release some resource */        
        pPoamIrepFoEnumClient->Remove((ANSC_HANDLE)pPoamIrepFoEnumClient);
        pPoamIrepFoEnumClient = NULL;
    }
    
    pPoamIrepFoClient->Remove((ANSC_HANDLE)pPoamIrepFoClient);
    pPoamIrepFoClient = NULL;


EXIT1:
    if ( pSlapVariable )
    {
        SlapFreeVariable(pSlapVariable);
        pSlapVariable = NULL;
    }

    if ( pPoamIrepFoClient )
        pPoamIrepFoClient->Remove((ANSC_HANDLE)pPoamIrepFoClient);

    if ( pPoamIrepFoEnumClient )
        pPoamIrepFoEnumClient->Remove((ANSC_HANDLE)pPoamIrepFoEnumClient);

    if ( pPoamIrepFoSntOpt)
        pPoamIrepFoSntOpt->Remove((ANSC_HANDLE)pPoamIrepFoSntOpt);

    if ( pPoamIrepFoEnumSntOpt)
        pPoamIrepFoEnumSntOpt->Remove((ANSC_HANDLE)pPoamIrepFoEnumSntOpt);
    
    pPoamIrepFoDhcpv6->EnableFileSync((ANSC_HANDLE)pPoamIrepFoDhcpv6, TRUE);

    return returnStatus;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        WanMgr_Dhcpv6ClientHasDelayAddedChild
            (
                PDHCPC_CONTEXT_LINK_OBJECT                       hContext
            );

    description:

        This function is called to check whether this is child is pending added. If yes,
        return TRUE. Or else return FALSE.

    argument:   PDHCPC_CONTEXT_LINK_OBJECT                hThisObject
                This handle is actually the pointer of one context link point.

    return:     TRUE or FALSE.

**********************************************************************/
BOOL
WanMgr_Dhcpv6ClientHasDelayAddedChild
    (
        PDHCPCV6_CONTEXT_LINK_OBJECT                 hContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDHCPCV6_CONTEXT_LINK_OBJECT pDhcpcContext = hContext;

    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY )NULL;
    PCONTEXT_LINK_OBJECT       pCxtLink      = NULL;

    pSLinkEntry = AnscSListGetFirstEntry(&pDhcpcContext->SentOptionList);
    while ( pSLinkEntry )
    {
        pCxtLink          = ACCESS_CONTEXT_LINK_OBJECT(pSLinkEntry);
        pSLinkEntry           = AnscSListGetNextEntry(pSLinkEntry);

        if ( pCxtLink->bNew )
        {
            return TRUE;
        }
    }
    return FALSE;
}
