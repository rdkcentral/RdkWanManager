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

#ifndef  _DHCPV4_INTERNAL_H
#define  _DHCPV4_INTERNAL_H

#include "wanmgr_apis.h"

/*
  *   This is cosa middle layer definition 
  *
  */
#define  DHCPV4_IREP_FOLDER_NAME                      "Dhcpv4"
#define  DHCPV4_IREP_FOLDER_NAME_CLIENT               "Client"
#define  DHCPV4_IREP_FOLDER_NAME_REQOPTION            "ReqOption"
#define  DHCPV4_IREP_FOLDER_NAME_SENTOPTION           "SentOption"
#define  COSA_DML_RR_NAME_Dhcpv4Alias                      "Alias"
#define  COSA_DML_RR_NAME_Dhcpv4bNew                       "bNew"
#define  COSA_DML_RR_NAME_Dhcpv4NextInsNunmber             "NextInstanceNumber"

#define  COSA_DML_DHCPV4_ALIAS                             64

/*
*  This struct is only for dhcpc because it have two sub tables.
*  For the two table, they just use common link struct because they havenot sub tables.
*/
#define COSA_CONTEXT_DHCPC_LINK_CLASS_CONTENT                                              \
        CONTEXT_LINK_CLASS_CONTENT                                                     \
        SLIST_HEADER                      SendOptionList;                                   \
        SLIST_HEADER                      ReqOptionList;                                    \
        ULONG                             maxInstanceOfSend;                                \
        ULONG                             maxInstanceOfReq;                                 \
        CHAR                              AliasOfReq[COSA_DML_DHCPV4_ALIAS];                \
        CHAR                              AliasOfSend[COSA_DML_DHCPV4_ALIAS];               \

typedef  struct
_DHCPC_CONTEXT_LINK_OBJECT
{
    COSA_CONTEXT_DHCPC_LINK_CLASS_CONTENT
}
DHCPC_CONTEXT_LINK_OBJECT, *PDHCPC_CONTEXT_LINK_OBJECT;

#define  ACCESS_CONTEXT_DHCPC_LINK_OBJECT(p)                                \
         ACCESS_CONTAINER(p, DHCPC_CONTEXT_LINK_OBJECT, Linkage)            \

/*
*  This struct is for dhcp.
*/
#define  WAN_DHCPV4_DATA_CLASS_CONTENT                                              \
    /* duplication of the base object class content */                                    \
    BASE_CONTENT                                                                     \
    /* start of NAT object class content */                                               \
    SLIST_HEADER                    X_CISCO_COM_StaticAddressList;                        \
    SLIST_HEADER                    ClientList;    /* This is for entry added */          \
    ULONG                           maxInstanceOfClient;                                  \
    CHAR                            AliasOfClient[COSA_DML_DHCPV4_ALIAS];                 \
    ANSC_HANDLE                     hIrepFolderDhcpv4;                                    \

typedef  struct
_WAN_DHCPV4_DATA                                               
{
    WAN_DHCPV4_DATA_CLASS_CONTENT
}
WAN_DHCPV4_DATA,  *PWAN_DHCPV4_DATA;


#define   DHCPV4_CLIENT_ENTRY_MATCH(src,dst)                       \
    (strcmp((src)->Alias, (dst)->Alias) == 0)

#define   DHCPV4_CLIENT_ENTRY_MATCH2(src,dst)                      \
    (strcmp((src), (dst)) == 0)

#define   DHCPV4_SENDOPTION_ENTRY_MATCH(src,dst)                   \
    (strcmp((src)->Alias, (dst)->Alias) == 0)
        
#define   DHCPV4_SENDOPTION_ENTRY_MATCH2(src,dst)                  \
    (strcmp((src), (dst)) == 0)

#define   DHCPV4_REQOPTION_ENTRY_MATCH(src,dst)                    \
    (strcmp((src)->Alias, (dst)->Alias) == 0)

#define   DHCPV4_REQOPTION_ENTRY_MATCH2(src,dst)                   \
    (strcmp((src), (dst)) == 0)

#define   DHCPV4_CLIENT_INITIATION_CONTEXT(pDhcpc)                 \
    CONTEXT_LINK_INITIATION_CONTENT(((PCONTEXT_LINK_OBJECT)(pDhcpc)))  \
    AnscSListInitializeHeader(&(pDhcpc)->SendOptionList);          \
    AnscSListInitializeHeader(&(pDhcpc)->ReqOptionList);           \
    (pDhcpc)->maxInstanceOfSend               = 0;                 \
    (pDhcpc)->maxInstanceOfReq                = 0;                 \
    AnscZeroMemory((pDhcpc)->AliasOfReq,  sizeof((pDhcpc)->AliasOfReq) );  \
    AnscZeroMemory((pDhcpc)->AliasOfSend, sizeof((pDhcpc)->AliasOfSend) ); \

#define DHCPV4_CLIENT_SET_DEFAULTVALUE(pDhcpc)                                         \
    (pDhcpc)->Cfg.bEnabled                    = FALSE;                                 \
    AnscZeroMemory((pDhcpc)->Cfg.Interface, sizeof((pDhcpc)->Cfg.Interface));                     \
    (pDhcpc)->Info.Status                     = DML_DHCP_STATUS_Disabled;         \
    (pDhcpc)->Info.DHCPStatus                 = DML_DHCPC_STATUS_Init;            \

#define DHCPV4_SENDOPTION_SET_DEFAULTVALUE(pSendOption)                                \
    (pSendOption)->bEnabled                   = FALSE;                                 \
     (pSendOption)->Tag                        = 0;                                    \
     AnscZeroMemory( (pSendOption)->Value, sizeof( (pSendOption)->Value ) );           \
    

#define DHCPV4_REQOPTION_SET_DEFAULTVALUE(pReqOption)                                  \
    (pReqOption)->bEnabled                    = FALSE;                                 \
    (pReqOption)->Order                       = 0;                                     \
    (pReqOption)->Tag                         = 0;                                     \
    AnscZeroMemory( (pReqOption)->Value, sizeof( (pReqOption)->Value ) );              \


#define DHCPV4_OPTION_SET_DEFAULTVALUE(pOption)                                        \
    (pOption)->bEnabled                        = FALSE;                                \

/*
    Function declaration 
*/ 

ANSC_HANDLE
WanMgr_Dhcpv4Create
    (
        VOID
    );

ANSC_STATUS
WanMgr_Dhcpv4Initialize
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
WanMgr_Dhcpv4Remove
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
WanMgr_Dhcpv4RegGetDhcpv4Info
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
WanMgr_Dhcpv4RegSetDhcpv4Info
    (
        ANSC_HANDLE                 hThisObject
    );

BOOL
WanMgr_Dhcpv4ClientHasDelayAddedChild
    (
        PDHCPC_CONTEXT_LINK_OBJECT     hContext
    );

ANSC_STATUS
WanMgr_Dhcpv4BackendGetDhcpv4Info
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

ANSC_HANDLE
WanMgr_Dhcpv4GetClientContentbyClient
    (
        ANSC_HANDLE                 hClientContext
    );


#endif


