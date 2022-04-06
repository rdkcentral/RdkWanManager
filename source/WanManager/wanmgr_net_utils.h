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

#ifndef _WANMGR_NET_UTILS_H_
#define _WANMGR_NET_UTILS_H_

/* ---- Include Files ---------------------------------------- */
#include <stdio.h>
#include <stdbool.h>
#include "wanmgr_rdkbus_common.h"
#include "ipc_msg.h"
#include "wanmgr_dml.h"
#include "wanmgr_utils.h"
//
//
/* ---- Global Constants -------------------------- */

#define DHCPV6_CLIENT_NAME "dibbler-client"
//


#define PTM_IFC_STR                 "ptm"
#define PHY_WAN_IF_NAME             "erouter0"
#define ETH_BRIDGE_NAME             "brlan0"
#define LAN_BRIDGE_NAME             "brlan0"
//

#define WAN_STATUS_UP   "up"
#define WAN_STATUS_DOWN "down"

#define WAN_IF_MARKING_MAX_LIMIT       ( 15 )
typedef  struct _CONTEXT_MARKING_LINK_OBJECT
{
    CONTEXT_LINK_CLASS_CONTENT
    BOOL    bFound;
} CONTEXT_MARKING_LINK_OBJECT;



#define  ACCESS_CONTEXT_MARKING_LINK_OBJECT(p)              \
         ACCESS_CONTAINER(p, CONTEXT_MARKING_LINK_OBJECT, Linkage)



#ifdef FEATURE_MAPT_DEBUG
#define MaptError(fmt, arg...) \
        RDK_LOG(RDK_LOG_ERROR, "LOG.RDK.MAPTLOG", fmt "\n", ##arg);
#define MaptNotice(fmt, arg...) \
        RDK_LOG(RDK_LOG_NOTICE, "LOG.RDK.MAPTLOG", fmt "\n", ##arg);
#define MaptInfo(fmt, arg...) \
        RDK_LOG(RDK_LOG_INFO, "LOG.RDK.MAPTLOG", fmt "\n", ##arg);
#endif /*FEATURE_MAPT_DEBUG*/

typedef struct
{
   char dns_ipv4_1[BUFLEN_64];
   char dns_ipv4_2[BUFLEN_64];
   char dns_ipv6_1[BUFLEN_128];
   char dns_ipv6_2[BUFLEN_128];
} DnsData_t;


typedef enum
{
    DEL_ADDR = 0,
    SET_LFT = 1
} Ipv6OperType;


/* ---- Global Variables -------------------------- */
//extern int sysevent_fd;
//extern token_t sysevent_token;
//extern eRouterRegion_t region;
/* Global structure to hold wan interface data.
 * This structure object defined in ipc module and this
 * can be also may required in other area. */
//extern WanData_t gWanData;
//extern pthread_mutex_t gmWanDataMutex;

/* ---- Global Prototypes -------------------------- */
/***************************************************************************
 * @brief API used to start Dhcpv6 client application.
 * @param pcInterfaceName Interface name on which the dhcpv6 needs to start
 * @param isPPP indicates PPP enabled or nor
 * @return ANSC_STATUS_SUCCESS upon success else returned error code.
 ***************************************************************************/
uint32_t WanManager_StartDhcpv6Client(char *pcInterfaceName);

/***************************************************************************
 * @brief API used to stop Dhcpv6 client application.
 * @param intf Interface name on which the dhcpv4 needs to start
 * @return ANSC_STATUS_SUCCESS upon success else returned error code.
 ***************************************************************************/
ANSC_STATUS WanManager_StopDhcpv6Client(char * iface_name);

/***************************************************************************
 * @brief API used to start Dhcpv4 client application.
 * @param intf Interface name on which the dhcpv4 needs to start
 * @return ANSC_STATUS_SUCCESS upon success else returned error code.
 ***************************************************************************/
uint32_t WanManager_StartDhcpv4Client(char* intf);

/***************************************************************************
 * @brief API used to stop Dhcpv4 client application.
 * @param pcInterfaceName Interface name for which dhcp v4 app needs to be stopped
 * @return ANSC_STATUS_SUCCESS upon success else returned error code.
 ***************************************************************************/
ANSC_STATUS WanManager_StopDhcpv4Client(char * iface_name);

/***************************************************************************
 * @brief API used to restart Dhcpv6 client application.
 * @param ifName_info interface name
 * @param dynamicIpEnabled indicates dynamicip needs to be enable
 * @param pdEnabled indicates packket delegation enabled
 * @return ANSC_STATUS_SUCCESS upon success else returned error code.
 ***************************************************************************/
uint32_t WanManager_RestartDhcp6c(const char *ifName_info, BOOL dynamicIpEnabled,
                            BOOL pdEnabled, BOOL aftrName);




/***************************************************************************
 * @brief Utility function used to check Application is running.
 * @param appName string indicates application name
 * @return status of system() call.
 ***************************************************************************/
BOOL WanManager_IsApplicationRunning(const char* appName);

/***************************************************************************
 * @brief Utility function used to perform operation on IPV6 addresses
 * for a particular interface
 * @param ifname string indicates interface name
 * @param opr indicates operation type (Delete/Set)
 * @param preflft indicates preferred lifetime
 * @param vallft indicates valid lifetime
 * @return 0 upon success else -1 returned
 ***************************************************************************/
int WanManager_Ipv6AddrUtil(char *ifname,Ipv6OperType opr,int preflft,int vallft);

#ifdef FEATURE_MAPT
/*************************************************************************************
 * @brief checks kernel module loaded.
 * This API calls the proc entry.
 * @param moduleName is the kernal modulename
 * @return RETURN_OK upon success else RETURN_ERR returned
 **************************************************************************************/
int isModuleLoaded(char *moduleName); // checks kernel module loaded.

/***********************************************************************************
 * @brief This API used to process mapt configuration data.
 * @param dhcp6cMAPTMsgBody Hold the mapt data received from dhcp6 server.
 * @param baseIf Base interface name
 * @param vlanIf Vlan interface name
 * @return RETURN_OK in case of success else error code returned.
 ************************************************************************************/
int WanManager_ProcessMAPTConfiguration(ipc_mapt_data_t *dhcp6cMAPTMsgBody, const char *baseIf, const char *vlanIf);
#endif

/***********************************************************************************
 * @brief This API used to reset mapt configuration data.
 * @param baseIf Base interface name
 * @param vlanIf Vlan interface name
 * @return RETURN_OK in case of success else error code returned.
 ************************************************************************************/
int WanManager_ResetMAPTConfiguration(const char *baseIf, const char *vlanIf);

/***************************************************************************
 * @brief API used to update /etc/resolv.conf file with dns configuration
 * @param dnsInfo pointer to DnsData_t contains the dns info
 * @return RETURN_OK upon success else returned error code.
 ****************************************************************************/
int WanManager_CreateResolvCfg(const DnsData_t *dnsInfo);

/***************************************************************************
 * @brief API used to update default ipv4 gateway
 * @param ipv4Info pointer to ipc_dhcpv4_data_t holds the IPv4 configuration
 * @return RETURN_OK upon success else returned error code.
 ****************************************************************************/
int WanManager_AddDefaultGatewayRoute(const WANMGR_IPV4_DATA* ipv4Info);

/***************************************************************************
 * @brief API used to get broadcast IP from IP and subnet mask
 * @param inIpStr IP address
 * @param inSubnetMaskStr Subnet mask
 * @param outBcastStr Stores the broadcast address
 * @return RETURN_OK upon success else returned error code.
 ****************************************************************************/
int WanManager_GetBCastFromIpSubnetMask(const char *inIpStr, const char *inSubnetMaskStr, char *outBcastStr);

/***************************************************************************
 * @brief API used to update ipv4 gateway
 * @param ipv4Info pointer to dhcpv4_data_t holds the IPv4 configuration
 * @return RETURN_OK upon success else returned error code.
 ****************************************************************************/
int WanManager_AddGatewayRoute(const WANMGR_IPV4_DATA* ipv4Info);

/***************************************************************************
 * @brief API used to start PPP client application.
 * @param pcInterfaceName Interface name on which the dhcpv6 needs to start
 * @param isPPP indicates PPP enabled or nor
 * @return ANSC_STATUS_SUCCESS upon success else returned error code.
 ***************************************************************************/
ANSC_STATUS WanManager_CreatePPPSession(DML_WAN_IFACE* pInterface);

/***************************************************************************
 * @brief API used to start PPP client application.
 * @param pcInterfaceName Interface name on which the dhcpv6 needs to start
 * @param isPPP indicates PPP enabled or nor
 * @return ANSC_STATUS_SUCCESS upon success else returned error code.
 ***************************************************************************/
ANSC_STATUS WanManager_DeletePPPSession(DML_WAN_IFACE* pInterface);


ANSC_STATUS WanManager_getGloballyUniqueIfAddr6(const char *ifname, char *ipAddr, uint32_t *prefixLen);

#ifdef FEATURE_802_1P_COS_MARKING
ANSC_HANDLE WanManager_AddIfaceMarking(DML_WAN_IFACE* pWanDmlIface, ULONG* pInsNumber);
#endif /* * FEATURE_802_1P_COS_MARKING */
ANSC_STATUS WanManager_CheckGivenTypeExists(INT IfIndex, UINT uiTotalIfaces, DML_WAN_IFACE_TYPE priorityType, INT priority, BOOL *Status);
ANSC_STATUS WanManager_CheckGivenPriorityExists(INT IfIndex, UINT uiTotalIfaces, INT priority, BOOL *Status);
INT WanMgr_StartIpMonitor(UINT iface_index);
#endif // _WANMGR_NET_UTILS_H_
