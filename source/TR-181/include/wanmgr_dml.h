/*
 * If not stated otherwise in this file or this component's LICENSE file the
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
#define REMOTE_INTERFACE_NAME       "brRWAN"
#define REMOTE_INTERFACE_GROUP        2
#define MAX_INTERFACE_GROUP           2
#define WAN_MANAGER_VERSION         "1.5"

typedef enum _WANMGR_IFACE_SELECTION_STATUS
{
    WAN_IFACE_UNKNOWN,
    WAN_IFACE_NOT_SELECTED,
    WAN_IFACE_VALIDATING,
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
   AUTOWAN_MODE,
   PARALLEL_SCAN
} DML_WAN_POLICY;

typedef enum _DEVICE_NETWORKING_MODE_
{
    GATEWAY_MODE = 1,
    MODEM_MODE
} DEVICE_NETWORKING_MODE;

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
    WAN_IFACE_STATUS_VALID,
    WAN_IFACE_STATUS_UP,
    WAN_IFACE_STATUS_INVALID,
    WAN_IFACE_STATUS_STANDBY
} DML_WAN_IFACE_STATUS;


typedef enum _DML_WAN_IFACE_SCAN_STATUS
{
    WAN_IFACE_STATUS_NOT_SCANNED = 1,
    WAN_IFACE_STATUS_SCANNED,
} DML_WAN_IFACE_SCAN_STATUS;

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

typedef enum _DML_WAN_IP_SOURCE
{
    DML_WAN_IP_SOURCE_STATIC = 1,
    DML_WAN_IP_SOURCE_DHCP,
    DML_WAN_IP_SOURCE_PPP,
} DML_WAN_IP_SOURCE;

typedef enum _DML_WAN_IP_MODE
{
    DML_WAN_IP_MODE_IPV4_ONLY = 1,
    DML_WAN_IP_MODE_IPV6_ONLY,
    DML_WAN_IP_MODE_DUAL_STACK,
    DML_WAN_IP_MODE_NO_IP
} DML_WAN_IP_MODE;

typedef enum _DML_WAN_IP_PREFERRED_MODE
{
    DUAL_STACK_MODE = 1,
    MAPT_MODE,
    MAPE_MODE,
    DSLITE_MODE
} DML_WAN_IP_PREFERRED_MODE;

typedef enum _TIMER_STATUS
{
    NOTSTARTED = 1,
    EXPIRED,
    RUNNING,
    COMPLETE
} TIMER_STATUS;

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

typedef enum _DML_WAN_IFACE_PPP_LINK_STATUS
{
    WAN_IFACE_PPP_LINK_STATUS_DOWN = 1,
    WAN_IFACE_PPP_LINK_STATUS_CONFIGURING,
    WAN_IFACE_PPP_LINK_STATUS_UP,
} DML_WAN_IFACE_PPP_LINK_STATUS;

typedef enum _IFACE_TYPE 
{
    LOCAL_IFACE = 1,
    REMOTE_IFACE
} IFACE_TYPE;

typedef enum {
    STATE_GROUP_UNKNOWN = 1,
    STATE_GROUP_RUNNING,
    STATE_GROUP_STOPPED,
    STATE_GROUP_DEACTIVATED,
} WAN_IFACE_GROUP_STATUS;

typedef enum {
    HOTSWAP = 1,
    COLDSWAP
} FAILOVER_TYPE;

typedef enum {
    WAN_CONNECTION_UNKNOWN =0,
    WAN_CONNECTIVITY_UP = 1,
    WAN_CONNECTIVITY_DOWN,
} CONNECTIVITY_STATUS;

typedef enum {
    WAN_CONNECTIVITY_TYPE_NO_CHECK = 1,
    WAN_CONNECTIVITY_TYPE_IHC,
    WAN_CONNECTIVITY_TYPE_TAD,
} CONNECTIVITY_CHECK_TYPE;

typedef enum {
    CONNECTION_MSG_IPV4 = 1,
    CONNECTION_MSG_IPV6
} CONNECTION_TYPE;

/* WCC -  WAN CONNECTIVITY CHECK */
typedef enum {
    WCC_STOP = 0,
    WCC_START,
    WCC_RESTART,
} WCC_EVENT;

typedef struct _DATAMODEL_PPP
{
    BOOL                          Enable;
    CHAR                          Interface[BUFLEN_64];
    DML_WAN_IFACE_IPCP_STATUS     IPCPStatus;
    DML_WAN_IFACE_IPV6CP_STATUS   IPV6CPStatus;
    BOOL                          IPCPStatusChanged;
    BOOL                          IPV6CPStatusChanged;
    DML_WAN_IFACE_PPP_LINK_STATUS LinkStatus;
} DATAMODEL_PPP;

typedef struct _WANMGR_IPV4_DATA
{
    char ifname[BUFLEN_64];
    char ip[BUFLEN_32];                /** New IP address, if addressAssigned==TRUE */
    char mask[BUFLEN_32];              /** New netmask, if addressAssigned==TRUE */
    char gateway[BUFLEN_32];           /** New gateway, if addressAssigned==TRUE */
    char dnsServer[BUFLEN_64];         /** New dns Server, if addressAssigned==TRUE */
    char dnsServer1[BUFLEN_64];        /** New dns Server, if addressAssigned==TRUE */
    uint32_t mtuSize;                  /** New MTU size, if mtuAssigned==TRUE */
#if defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
    int32_t timeOffset;                /** New time offset, if addressAssigned==TRUE */
    bool isTimeOffsetAssigned;         /** Is the time offset assigned ? */
    char timeZone[BUFLEN_64];          /** New time zone, if addressAssigned==TRUE */
    uint32_t upstreamCurrRate;         /** Upstream rate */
    uint32_t downstreamCurrRate;       /** Downstream rate */
    char dhcpServerId[BUFLEN_64];      /** Dhcp server id */
    char dhcpState[BUFLEN_64];         /** Dhcp state. */
    uint32_t leaseTime;                /** Lease time, , if addressAssigned==TRUE */
#endif
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
   #if defined(FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE)
   /* Params to store the IPv6 IPC message */
   bool addrAssigned;
   uint32_t addrCmd;
   bool prefixAssigned;  /**< Have we been assigned a site prefix ? */
   uint32_t prefixCmd;
   bool domainNameAssigned;     /**< Have we been assigned dns server addresses ? */
   #endif
} WANMGR_IPV6_DATA;


typedef struct _DML_WANIFACE_IP
{
    CHAR                        Interface[BUFLEN_64];
    DML_WAN_IFACE_IPV4_STATUS   Ipv4Status;
    DML_WAN_IFACE_IPV6_STATUS   Ipv6Status;
    CONNECTIVITY_CHECK_TYPE     ConnectivityCheckType;
    CONNECTIVITY_STATUS         Ipv4ConnectivityStatus;
    CONNECTIVITY_STATUS         Ipv6ConnectivityStatus;
    DML_WAN_IP_SOURCE           IPv4Source;
    DML_WAN_IP_SOURCE           IPv6Source;
    BOOL                        ConnectivityCheckRunning;
    BOOL                        RestartConnectivityCheck;
    BOOL                        RefreshDHCP;
    BOOL                        RestartV6Client; //This is a workaround to restart dhcpv6 client for the platform where PAM configures IPv6. !FEATURE_RDKB_CONFIGURABLE_WAN_INTERFACE
    DML_WAN_IP_MODE             Mode;
    DML_WAN_IP_PREFERRED_MODE   PreferredMode;
    DML_WAN_IP_PREFERRED_MODE   SelectedMode;
    struct timespec             SelectedModeTimerStart;
    TIMER_STATUS                SelectedModeTimerStatus;
    BOOL                        ModeForceEnable;
    BOOL                        Ipv4Changed;
    BOOL                        Ipv6Changed;
    BOOL                        Ipv4Renewed;
    BOOL                        Ipv6Renewed;
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

typedef struct _WANMGR_MAPT_CONFIG_DATA_
{
    int psidValue;
    char ipAddressString[BUFLEN_32];
    char ipLANAddressString[BUFLEN_32];
    int psidLen;
}WANMGR_MAPT_CONFIG_DATA;

#endif

typedef struct _DML_WANIFACE_MAP
{
    DML_WAN_IFACE_MAPT_STATUS   MaptStatus;
    CHAR                        Path[BUFLEN_64];
    BOOL                        MaptChanged;
#ifdef FEATURE_MAPT
    ipc_mapt_data_t dhcp6cMAPTparameters;
    WANMGR_MAPT_CONFIG_DATA     MaptConfig;
#endif
} DML_WANIFACE_MAP;

typedef struct _DML_WANIFACE_DSLITE
{
    CHAR                        Path[BUFLEN_64];
    DML_WAN_IFACE_DSLITE_STATUS Status;
    BOOL                        Changed;
} DML_WANIFACE_DSLITE;


typedef struct _DML_WANIFACE_SUBSCRIBE
{
    UINT BaseInterfaceStatusSub;
    UINT WanStatusSub;
    UINT WanLinkStatusSub;
    UINT WanEnableSub;
} DML_WANIFACE_SUBSCRIBE;

typedef enum
{
    WAN_STATE_EXIT = 0,
    WAN_STATE_VLAN_CONFIGURING,
    WAN_STATE_PPP_CONFIGURING,
    WAN_STATE_VALIDATING_WAN,
    WAN_STATE_OBTAINING_IP_ADDRESSES,
    WAN_STATE_IPV4_LEASED,
    WAN_STATE_IPV6_LEASED,
    WAN_STATE_DUAL_STACK_ACTIVE,
    WAN_STATE_MAPT_ACTIVE,
    WAN_STATE_REFRESHING_WAN,
    WAN_STATE_DECONFIGURING_WAN,
    WAN_STATE_STANDBY
} eWanState_t;

typedef struct _WANMGR_IFACE_GROUP_DATA_
{
    pthread_t          ThreadId;
    UINT               State;
    UINT               groupIdx;
    UINT               InterfaceAvailable;
    UINT               SelectedInterface;
    UINT               SelectionTimeOut;
    UINT               ActivationCount;
    BOOLEAN            PersistSelectedIface;
    DML_WAN_POLICY     Policy;
    BOOLEAN            ConfigChanged;
    BOOLEAN            ResetSelectedInterface;
    BOOLEAN            InitialScanComplete;
}WANMGR_IFACE_GROUP;

typedef struct _WANMGR_IFACE_GROUP_
{
    UINT                       ulTotalNumbWanIfaceGroup;
    WANMGR_IFACE_GROUP*        Group;
}WanMgr_IfaceGroup_t;

typedef struct _DML_VIRTIF_MARKING
{
    struct _DML_VIRTIF_MARKING* next;
    ULONG                       VirtMarkingInstanceNumber;
    UINT                        VirIfIdx;
    UINT                        baseIfIdx;
    UINT                        Entry;
}DML_VIRTIF_MARKING;

typedef struct _DML_VLAN_IFACE_TABLE
{
    struct _DML_VLAN_IFACE_TABLE* next;
    ULONG                       Index;
    UINT                        VirIfIdx;
    UINT                        baseIfIdx;
    CHAR                        Interface[BUFLEN_128];;
}DML_VLAN_IFACE_TABLE;

typedef struct _DML_VIRTUALIF_VLAN
{
    BOOL                        Enable;
    CHAR                        VLANInUse[BUFLEN_128];
    UINT                        ActiveIndex;
    UINT                        Timeout;
    UINT                        NoOfInterfaceEntries;
    DML_VLAN_IFACE_TABLE*       InterfaceList;
    UINT                        NoOfMarkingEntries;
    DML_VIRTIF_MARKING*         VirtMarking;
    DML_WAN_IFACE_LINKSTATUS    Status;
    BOOL                        Expired;
    BOOL                        Reset;
    struct timespec             TimerStart;
} DML_VIRTUALIF_VLAN;

typedef struct _DML_VIRTUAL_IFACE
{
    struct _DML_VIRTUAL_IFACE*  next;
    UINT                        VirIfIdx;
    UINT                        baseIfIdx;
    CHAR                        Name[BUFLEN_64];
    CHAR                        Alias[BUFLEN_64];
    BOOL                        Enable; 
    BOOL                        EnableMAPT;
    BOOL                        EnableDSLite;
    BOOL                        EnableIPoE;
    DML_WAN_IFACE_STATUS        Status;
    DML_WAN_IFACE_STATUS        RemoteStatus;
    DML_VIRTUALIF_VLAN          VLAN;
    BOOL                        Reset;
    DML_WAN_IFACE_OPER_STATUS   OperationalStatus;
    DML_WANIFACE_IP             IP;
    DATAMODEL_PPP               PPP;
    DML_WANIFACE_MAP            MAP;
    DML_WANIFACE_DSLITE         DSLite;
    eWanState_t                 eCurrentState; 
    BOOLEAN                     Interface_SM_Running;  // flag to check whether Interface State machine is running
} DML_VIRTUAL_IFACE;

typedef struct _DML_IFACE_SELECTION
{
    BOOL                        Enable; 
    INT                         Priority;
    UINT                        Timeout;
    WANMGR_IFACE_SELECTION      Status;
    BOOL                        ActiveLink;
    UINT                        Group;
    BOOL                        RequiresReboot;
    BOOL                        RebootTriggerStatus;
} DML_IFACE_SELECTION;

typedef struct _DML_WAN_INTERFACE
{
    UINT                        uiIfaceIdx;
    UINT                        uiInstanceNumber;
    CHAR                        Name[BUFLEN_64];
    CHAR                        DisplayName[BUFLEN_64];
    CHAR                        AliasName[BUFLEN_64];
    BOOL                        MonitorOperStatus;
    BOOL                        WanConfigEnabled;
    BOOL                        VirtIfChanged;
    BOOL                        CustomConfigEnable;
    CHAR                        CustomConfigPath[BUFLEN_128];
    DML_WAN_IFACE_SCAN_STATUS   InterfaceScanStatus;
    CHAR                        RemoteCPEMac[BUFLEN_128];
    CHAR                        BaseInterface[BUFLEN_128];
    DML_WAN_IFACE_TYPE          Type; //TODO: Comcast use
    DML_WAN_IFACE_PHY_STATUS    BaseInterfaceStatus;
    DML_IFACE_SELECTION         Selection;
    IFACE_TYPE                  IfaceType;
    DML_WANIFACE_SUBSCRIBE      Sub; // TODO: NEW_DESIGN : remove
    DATAMODEL_MARKING           Marking;
    UINT                        NoOfVirtIfs;
    DML_VIRTUAL_IFACE*          VirtIfList;
} DML_WAN_IFACE;


/*** RDK WAN Manager ***/
typedef struct _DML_WANMGR_CONFIG_
{
    BOOLEAN Enable;
    DEVICE_NETWORKING_MODE DeviceNwMode;
    BOOLEAN DeviceNwModeChanged;    // Set if DeviceNwMode is changed and config needs to be applied
    BOOLEAN ResetFailOverScan;
    BOOLEAN AllowRemoteInterfaces;
    BOOLEAN BootToWanUp;            // Set if Wan was UP after boot
    CHAR    InterfaceAvailableStatus[BUFLEN_64];
    CHAR    InterfaceActiveStatus[BUFLEN_64];
    CHAR    CurrentActiveInterface[BUFLEN_64];
    CHAR    CurrentStatus[BUFLEN_16];
    CHAR    CurrentStandbyInterface[BUFLEN_64];
    UINT    RestorationDelay;
} DML_WANMGR_CONFIG;

//WAN CONFIG
typedef struct _WANMGR_CONFIG_DATA_
{
    DML_WANMGR_CONFIG       data;
} WanMgr_Config_Data_t;


//WAN IFACE
typedef struct _WANMGR_IFACE_DATA_
{
    DML_WAN_IFACE           data;
}WanMgr_Iface_Data_t;

typedef struct _WANMGR_IFACECTRL_DATA_
{
    UINT                        ulTotalNumbWanInterfaces;
    WanMgr_Iface_Data_t*        pIface;
    UINT                        update;
}WanMgr_IfaceCtrl_Data_t;

typedef struct _WANMGR_DATA_ST_
{
    //Mutex
    pthread_mutex_t             gDataMutex;

    //WAN CONFIG
    WanMgr_Config_Data_t        Config;

    //WAN IFACE
    WanMgr_IfaceCtrl_Data_t     IfaceCtrl;

    //Iface Group
    WanMgr_IfaceGroup_t         IfaceGroup;
} WANMGR_DATA_ST;
#endif //_WANMGR_DML_H_
