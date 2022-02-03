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

#ifndef  _WANMGR_DML_H_
#define  _WANMGR_DML_H_

#include "ipc_msg.h"

#define PAM_COMPONENT_NAME          "eRT.com.cisco.spvtg.ccsp.pam"
#define PAM_DBUS_PATH               "/com/cisco/spvtg/ccsp/pam"
#define PAM_NOE_PARAM_NAME          "Device.IP.InterfaceNumberOfEntries"
#define PAM_IF_TABLE_OBJECT         "Device.IP.Interface.%d."
#define PAM_IF_PARAM_NAME           "Device.IP.Interface.%d.Name"
#define DML_WAN_IFACE_PRIORITY_MAX  255

typedef enum _WANMGR_IFACE_SELECTION_STATUS
{
    WAN_IFACE_NOT_SELECTED = 1,
    WAN_IFACE_SELECTED,
    WAN_IFACE_ACTIVE
} WANMGR_IFACE_SELECTION;

typedef enum _DML_WAN_POLICY
{
   FIXED_MODE_ON_BOOTUP = 1,
   FIXED_MODE,
   PRIMARY_PRIORITY_ON_BOOTUP,
   PRIMARY_PRIORITY,
   MULTIWAN_MODE,
   AUTOWAN_MODE
} DML_WAN_POLICY;

typedef enum _DML_WAN_IFACE_OPER_STATUS
{
    WAN_OPERSTATUS_UNKNOWN = 1,
    WAN_OPERSTATUS_OPERATIONAL,
    WAN_OPERSTATUS_NOT_OPERATIONAL
} DML_WAN_IFACE_OPER_STATUS;

typedef enum _DML_WAN_IFACE_STATUS
{
    WAN_IFACE_STATUS_DISABLED = 1,
    WAN_IFACE_STATUS_INITIALISING,
    WAN_IFACE_STATUS_VALIDATING,
    WAN_IFACE_STATUS_UP,
    WAN_IFACE_STATUS_INVALID
} DML_WAN_IFACE_STATUS;

typedef enum _DML_WAN_IFACE_LINKSTATUS
{
    WAN_IFACE_LINKSTATUS_DOWN = 1,
    WAN_IFACE_LINKSTATUS_CONFIGURING,
    WAN_IFACE_LINKSTATUS_UP
} DML_WAN_IFACE_LINKSTATUS;
typedef enum _WAN_MANAGER_STATUS
{
    WAN_MANAGER_DOWN = 1,
    WAN_MANAGER_UP
} WAN_MANAGER_STATUS;

/** enum wan iface phy status */
typedef enum _DML_WAN_IFACE_PHY_STATUS
{
    WAN_IFACE_PHY_STATUS_DOWN = 1,
    WAN_IFACE_PHY_STATUS_INITIALIZING,
    WAN_IFACE_PHY_STATUS_UP,
    WAN_IFACE_PHY_STATUS_UNKNOWN
} DML_WAN_IFACE_PHY_STATUS;

/** enum wan status */
typedef enum _DML_WAN_IFACE_TYPE
{
    WAN_IFACE_TYPE_UNCONFIGURED = 1,
    WAN_IFACE_TYPE_PRIMARY,
    WAN_IFACE_TYPE_SECONDARY
} DML_WAN_IFACE_TYPE;

/** enum wan status */
typedef enum _DML_WAN_IFACE_IPV4_STATUS
{
    WAN_IFACE_IPV4_STATE_UP = 1,
    WAN_IFACE_IPV4_STATE_DOWN,
    WAN_IFACE_IPV4_STATE_UNKNOWN
} DML_WAN_IFACE_IPV4_STATUS;

/** enum wan status */
typedef enum _DML_WAN_IFACE_IPV6_STATUS
{
    WAN_IFACE_IPV6_STATE_UP = 1,
    WAN_IFACE_IPV6_STATE_DOWN,
    WAN_IFACE_IPV6_STATE_UNKNOWN
} DML_WAN_IFACE_IPV6_STATUS;

/** enum wan status */
typedef enum _DML_WAN_IFACE_MAPT_STATUS
{
    WAN_IFACE_MAPT_STATE_UP = 1,
    WAN_IFACE_MAPT_STATE_DOWN
} DML_WAN_IFACE_MAPT_STATUS;

/** enum dslite status */
typedef enum _DML_WAN_IFACE_DSLITE_STATUS
{
    WAN_IFACE_DSLITE_STATE_UP = 1,
    WAN_IFACE_DSLITE_STATE_DOWN
} DML_WAN_IFACE_DSLITE_STATUS;

/** enum wan status */
typedef enum _WAN_NOTIFY_ENUM
{
    NOTIFY_TO_VLAN_AGENT        = 1
} WAN_NOTIFY_ENUM;

/** Enum IP (IPV4/IPV6/MAPT) state type. **/
typedef enum _DML_WAN_IFACE_IP_STATE_TYPE
{
    WAN_IFACE_IPV4_STATE = 0,
    WAN_IFACE_IPV6_STATE,
    WAN_IFACE_MAPT_STATE,
    WAN_IFACE_DSLITE_STATE
} DML_WAN_IFACE_IP_STATE_TYPE;

/** Enum IP state. UP/DOWN */
typedef enum _DML_WAN_IFACE_IP_STATE
{
    WAN_IFACE_IP_STATE_UP = 1,
    WAN_IFACE_IP_STATE_DOWN,
} DML_WAN_IFACE_IP_STATE;
/*
 *  Wan Marking object
 */
typedef enum _DML_WAN_MARKING_DML_OPERATIONS
{
    WAN_MARKING_ADD = 1,
    WAN_MARKING_DELETE,
    WAN_MARKING_UPDATE
} DML_WAN_MARKING_DML_OPERATIONS;

typedef struct _DML_MARKING
{
    ULONG      InstanceNumber;
    ULONG      ulWANIfInstanceNumber;
    CHAR       Alias[BUFLEN_64];
    UINT       SKBPort;
    UINT       SKBMark;
    INT        EthernetPriorityMark;
} DML_MARKING;

typedef struct _DATAMODEL_MARKING
{
    SLIST_HEADER      MarkingList;
    ULONG             ulNextInstanceNumber;
} DATAMODEL_MARKING;

/*** RDK WAN Interface ***/
typedef struct _DML_WANIFACE_PHY
{
    CHAR                         Path[BUFLEN_64];
    DML_WAN_IFACE_PHY_STATUS     Status;
} DML_WANIFACE_PHY;

typedef enum _DML_WAN_IFACE_IPCP_STATUS
{
    WAN_IFACE_IPCP_STATUS_DOWN = 1,
    WAN_IFACE_IPCP_STATUS_UP
} DML_WAN_IFACE_IPCP_STATUS;

typedef enum _DML_WAN_IFACE_IPV6CP_STATUS
{
    WAN_IFACE_IPV6CP_STATUS_DOWN = 1,
    WAN_IFACE_IPV6CP_STATUS_UP,
} DML_WAN_IFACE_IPV6CP_STATUS;

typedef enum _DML_WAN_IFACE_LCP_STATUS
{
    WAN_IFACE_LCP_STATUS_DOWN = 1,
    WAN_IFACE_LCP_STATUS_UP,
} DML_WAN_IFACE_LCP_STATUS;

typedef enum _DML_WAN_IFACE_PPP_LINK_STATUS
{
    WAN_IFACE_PPP_LINK_STATUS_DOWN = 1,
    WAN_IFACE_PPP_LINK_STATUS_UP,
} DML_WAN_IFACE_PPP_LINK_STATUS;

typedef enum _DML_WAN_IFACE_LINK_TYPE
{
    WAN_IFACE_PPP_LINK_TYPE_PPPoA = 1,
    WAN_IFACE_PPP_LINK_TYPE_PPPoE,
} DML_WAN_IFACE_LINK_TYPE;

typedef enum _PPP_CONNECTION_EVENTS
{
    PPP_LINK_STATE_CHANGED = 1,
    PPP_LCP_STATE_CHANGED,
    PPP_IPCP_STATE_CHANGED,
    PPP_IPV6CP_STATE_CHANGED
} DML_PPP_STATE_CHANGED_EVENTS;

typedef struct _DATAMODEL_PPP
{
    BOOL                          Enable;
    CHAR                          Path[BUFLEN_64];
    BOOL                          IPCPEnable;
    BOOL                          IPV6CPEnable;
    DML_WAN_IFACE_IPCP_STATUS     IPCPStatus;
    DML_WAN_IFACE_IPV6CP_STATUS   IPV6CPStatus;
    DML_WAN_IFACE_LCP_STATUS      LCPStatus;
    DML_WAN_IFACE_PPP_LINK_STATUS LinkStatus;
    DML_WAN_IFACE_LINK_TYPE       LinkType;
} DATAMODEL_PPP;

typedef struct _DML_WANIFACE_INFO
{
    CHAR                        Name[BUFLEN_64];
    BOOL                        Enable;
    INT                         Priority;
    DML_WAN_IFACE_TYPE          Type;
    UINT                        SelectionTimeout;
    BOOL                        EnableMAPT;
    BOOL                        EnableDSLite;
    BOOL                        EnableIPoE;
    BOOL                        ActiveLink;
    DML_WAN_IFACE_STATUS        Status;
    DML_WAN_IFACE_LINKSTATUS    LinkStatus;
    BOOL                        Refresh;
    DML_WAN_IFACE_OPER_STATUS   OperationalStatus;
    BOOL                        RebootOnConfiguration;
} DML_WANIFACE_INFO;


typedef struct _WANMGR_IPV4_DATA
{
    char ifname[BUFLEN_64];
    char ip[BUFLEN_32];                /** New IP address, if addressAssigned==TRUE */
    char mask[BUFLEN_32];              /** New netmask, if addressAssigned==TRUE */
    char gateway[BUFLEN_32];           /** New gateway, if addressAssigned==TRUE */
    char dnsServer[BUFLEN_64];         /** New dns Server, if addressAssigned==TRUE */
    char dnsServer1[BUFLEN_64];        /** New dns Server, if addressAssigned==TRUE */
} WANMGR_IPV4_DATA;


typedef struct _WANMGR_IPV6_DATA
{
   char ifname[BUFLEN_32];
   char address[BUFLEN_48];      /**< New IPv6 address, if addrAssigned==TRUE */
   char pdIfAddress[BUFLEN_48];      /**< New IPv6 address of PD interface */
   char nameserver[BUFLEN_128];  /**< New nameserver, if addressAssigned==TRUE */
   char nameserver1[BUFLEN_128];  /**< New nameserver, if addressAssigned==TRUE */
   char domainName[BUFLEN_64];  /**< New domain Name, if addressAssigned==TRUE */
   char sitePrefix[BUFLEN_48];   /**< New site prefix, if prefixAssigned==TRUE */
   uint32_t prefixPltime;
   uint32_t prefixVltime;
   char sitePrefixOld[BUFLEN_48]; /**< add support for RFC7084 requirement L-13 */
} WANMGR_IPV6_DATA;


typedef struct _DML_WANIFACE_IP
{
    CHAR                        Path[BUFLEN_64];
    DML_WAN_IFACE_IPV4_STATUS   Ipv4Status;
    DML_WAN_IFACE_IPV6_STATUS   Ipv6Status;
    BOOL                        Ipv4Changed;
    BOOL                        Ipv6Changed;
#ifdef FEATURE_IPOE_HEALTH_CHECK
    BOOL                        Ipv4Renewed;
    BOOL                        Ipv6Renewed;
#endif
    WANMGR_IPV4_DATA            Ipv4Data;
    WANMGR_IPV6_DATA            Ipv6Data;
    ipc_dhcpv4_data_t*          pIpcIpv4Data;
    ipc_dhcpv6_data_t*          pIpcIpv6Data;
    UINT                        Dhcp4cPid;
    UINT                        Dhcp6cPid;
} DML_WANIFACE_IP;

#ifdef FEATURE_MAPT
/* Data body for MAPT information to set sysevents*/
typedef struct
{
    char maptConfigFlag[BUFLEN_8]; //Flag to indicates to set/reset firewall rules. [SET/RESET]
    UINT ratio;
    char baseIfName[BUFLEN_64];
    char ipAddressString[BUFLEN_32];
    char ruleIpAddressString[BUFLEN_32];
    char ipv6AddressString[BUFLEN_128];
    char brIpv6PrefixString[BUFLEN_128];
    char ruleIpv6AddressString[BUFLEN_128];
    UINT psidOffset;
    UINT psidValue;
    UINT psidLen;
    UINT eaLen;
    UINT v4Len;
    BOOL mapeAssigned;     /**< Have we been assigned mape config ? */
    BOOL maptAssigned;     /**< Have we been assigned mapt config ? */
    BOOL isFMR;
}MaptData_t;
#endif

typedef struct _DML_WANIFACE_MAP
{
    DML_WAN_IFACE_MAPT_STATUS   MaptStatus;
    CHAR                        Path[BUFLEN_64];
    BOOL                        MaptChanged;
#ifdef FEATURE_MAPT
    ipc_mapt_data_t dhcp6cMAPTparameters;
#endif
} DML_WANIFACE_MAP;

typedef struct _DML_WANIFACE_DSLITE
{
    CHAR                        Path[BUFLEN_64];
    DML_WAN_IFACE_DSLITE_STATUS Status;
    BOOL                        Changed;
} DML_WANIFACE_DSLITE;

typedef struct _DML_WAN_INTERFACE
{
    UINT                        uiIfaceIdx;
    UINT                        uiInstanceNumber;
    CHAR                        Name[BUFLEN_64];
    CHAR                        DisplayName[BUFLEN_64];
    WANMGR_IFACE_SELECTION      SelectionStatus;
    BOOL                        MonitorOperStatus;
    BOOL                        WanConfigEnabled;
    BOOL                        CustomConfigEnable;
    CHAR                        CustomConfigPath[BUFLEN_128];
    DML_WANIFACE_PHY            Phy;
    DML_WANIFACE_INFO           Wan;
    DML_WANIFACE_IP             IP;
    DATAMODEL_PPP               PPP;
    DML_WANIFACE_MAP            MAP;
    DML_WANIFACE_DSLITE         DSLite;
    DATAMODEL_MARKING           Marking;
} DML_WAN_IFACE;


/*** RDK WAN Manager ***/
typedef struct _DML_WANMGR_CONFIG_
{
    BOOLEAN Enable;
    DML_WAN_POLICY Policy;
    BOOLEAN ResetActiveInterface;
    BOOLEAN PolicyChanged;
} DML_WANMGR_CONFIG;

#endif //_WANMGR_DML_H_
