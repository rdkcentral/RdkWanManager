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

#ifndef  _DHCPV6_INTERNAL_H
#define  _DHCPV6_INTERNAL_H

#include "wanmgr_apis.h"
#include "wanmgr_dml_dhcpv6.h"
#include "wanmgr_dhcpv6_apis.h"
#include "wanmgr_dhcpv4_apis.h"
#include "wanmgr_rdkbus_common.h"

/*
  *   This is cosa middle layer definition 
  *
  */
#define  DHCPV6_IREP_FOLDER_NAME                      "Dhcpv6"
#define  DHCPV6_IREP_FOLDER_NAME_CLIENT               "Client"
#define  DHCPV6_IREP_FOLDER_NAME_SENTOPTION           "SentOption"
#define  DHCPV6_IREP_FOLDER_NAME_OPTION               "Option"
#define  COSA_DML_RR_NAME_Dhcpv6Alias                      "Alias"
#define  COSA_DML_RR_NAME_Dhcpv6bNew                       "bNew"
#define  COSA_DML_RR_NAME_Dhcpv6NextInsNunmber             "NextInstanceNumber"

#define  COSA_DML_DHCPV6_ALIAS                             64

#define  COSA_NAT_ROLLBACK_TEST                            0       /* This is just for test purpose */

#define   COSA_DML_DHCPV6_ACCESS_INTERVAL_CLIENTSERVER   60 /* seconds*/
#define   COSA_DML_DHCPV6_ACCESS_INTERVAL_CLIENTRECV     60 /* seconds*/
#define   COSA_DML_DHCPV6_ACCESS_INTERVAL_POOLCLIENT     10 /* seconds*/

/*
*  This struct is only for dhcpc because it have two sub tables.
*  For the two table, they just use common link struct because they havenot sub tables.
*/
#define  COSA_CONTEXT_DHCPCV6_LINK_CLASS_CONTENT                                              \
        CONTEXT_LINK_CLASS_CONTENT                                                     \
        SLIST_HEADER                      SentOptionList;                                    \
        ULONG                             maxInstanceOfSent;                                \
        PDML_DHCPCV6_SVR             pServerEntry;                                       \
        ULONG                             NumberOfServer;                                     \
        ULONG                             PreviousVisitTimeOfServer;                                      \
        PDML_DHCPCV6_RECV            pRecvEntry;                                    \
        ULONG                             NumberOfRecv;                                     \
        ULONG                             PreviousVisitTimeOfRecv;                                      \
        CHAR                              AliasOfSent[COSA_DML_DHCPV6_ALIAS];                \

typedef  struct
_DHCPCV6_CONTEXT_LINK_OBJECT
{
    COSA_CONTEXT_DHCPCV6_LINK_CLASS_CONTENT
}
DHCPCV6_CONTEXT_LINK_OBJECT, *PDHCPCV6_CONTEXT_LINK_OBJECT;

#define  ACCESS_DHCPCV6_CONTEXT_LINK_OBJECT(p)                                \
         ACCESS_CONTAINER(p, DHCPCV6_CONTEXT_LINK_OBJECT, Linkage)            \

/*
*  This struct is for dhcp.
*/
#define  WAN_DHCPV6_DATA_CLASS_CONTENT                                              \
    /* duplication of the base object class content */                                              \
    BASE_CONTENT                                                                     \
    /* start of NAT object class content */                                                      \
    SLIST_HEADER                    ClientList;    /* This is for entry added */               \
    SLIST_HEADER                    PoolList;    /* This is for entry added */                 \
    ULONG                           maxInstanceOfClient;                                  \
    ULONG                           maxInstanceOfPool;                                    \
    ANSC_HANDLE                     hIrepFolderDhcpv6;                                    \
    CHAR                            AliasOfClient[COSA_DML_DHCPV6_ALIAS];                 \
    CHAR                            AliasOfPool[COSA_DML_DHCPV6_ALIAS];                   \

typedef  struct
_WAN_DHCPV6_DATA                                               
{
    WAN_DHCPV6_DATA_CLASS_CONTENT
}
WAN_DHCPV6_DATA,  *PWAN_DHCPV6_DATA;


#define   DHCPV6_CLIENT_ENTRY_MATCH(src,dst)                       \
    (strcmp((src)->Alias, (dst)->Alias) == 0)

#define   DHCPV6_CLIENT_ENTRY_MATCH2(src,dst)                      \
    (strcmp((src), (dst)) == 0)

#define   DHCPV6_SENDOPTION_ENTRY_MATCH(src,dst)                   \
    (strcmp((src)->Alias, (dst)->Alias) == 0)
        
#define   DHCPV6_SENDOPTION_ENTRY_MATCH2(src,dst)                  \
    (strcmp((src), (dst)) == 0)

#define   DHCPV6_REQOPTION_ENTRY_MATCH(src,dst)                    \
    (strcmp((src)->Alias, (dst)->Alias) == 0)

#define   DHCPV6_REQOPTION_ENTRY_MATCH2(src,dst)                   \
    (strcmp((src), (dst)) == 0)

#define   DHCPV6_CLIENT_INITIATION_CONTEXT(pDhcpc)                 \
    CONTEXT_LINK_INITIATION_CONTENT(((PCONTEXT_LINK_OBJECT)(pDhcpc)))  \
    AnscSListInitializeHeader(&(pDhcpc)->SentOptionList);          \
    (pDhcpc)->maxInstanceOfSent               = 0;                 \
    (pDhcpc)->pServerEntry                    = NULL;                 \
    (pDhcpc)->NumberOfServer                  = 0;                 \
    (pDhcpc)->PreviousVisitTimeOfServer       = 0;                 \
    (pDhcpc)->pRecvEntry                      = NULL;                 \
    (pDhcpc)->NumberOfRecv                    = 0;                 \
    (pDhcpc)->PreviousVisitTimeOfRecv         = 0;                 \
    AnscZeroMemory((pDhcpc)->AliasOfSent, sizeof((pDhcpc)->AliasOfSent) ); \

#define DHCPV6_CLIENT_SET_DEFAULTVALUE(pDhcpc)                                         \
    (pDhcpc)->Cfg.bEnabled                    = FALSE;                                 \
    AnscZeroMemory((pDhcpc)->Cfg.Interface, sizeof((pDhcpc)->Cfg.Interface));          \
    (pDhcpc)->Info.Status                     = DML_DHCP_STATUS_Disabled;         \

#define DHCPV6_SENTOPTION_SET_DEFAULTVALUE(pSentOption)                                \
    (pSentOption)->bEnabled                   = FALSE;                                 \
    (pSentOption)->Tag                        = 0;                                    \
    AnscZeroMemory( (pSentOption)->Value, sizeof( (pSentOption)->Value ) );           \
    
/*
    Function declaration 
*/ 

ANSC_HANDLE
WanMgr_Dhcpv6Create
    (
        VOID
    );

ANSC_STATUS
WanMgr_Dhcpv6Initialize
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
WanMgr_Dhcpv6Remove
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
WanMgr_Dhcpv6RegGetDhcpv6Info
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
WanMgr_Dhcpv6RegSetDhcpv6Info
    (
        ANSC_HANDLE                 hThisObject
    );

BOOL
WanMgr_Dhcpv6ClientHasDelayAddedChild
    (
        PDHCPCV6_CONTEXT_LINK_OBJECT     hContext
    );

ANSC_STATUS
WanMgr_Dhcpv6BackendGetDhcpv6Info
    (
        ANSC_HANDLE                 hThisObject
    );

BOOL
WanMgr_DmlSetIpaddr
    (
        PULONG pIPAddr, 
        PCHAR pString, 
        ULONG MaxNumber 
    );

BOOL 
WanMgr_DmlGetIpaddrString
    (
        PUCHAR pString, 
        PULONG pulStrLength, 
        PULONG pIPAddr, 
        ULONG  MaxNumber
    );


#endif


