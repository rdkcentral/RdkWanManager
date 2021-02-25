/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2015 RDK Management
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


#ifndef  _WANMGR_SYSEVENTS_H_
#define  _WANMGR_SYSEVENTS_H_

#include "ansc_platform.h"
#include "wanmgr_dml.h"

/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/
// /* sysevent/syscfg configurations used/managed by wanmanager.

//Led sysevent
#define SYSEVENT_WAN_LED_STATE "wan_led_state"

#define LED_OFF_STR             "Off"
#define LED_FLASHING_AMBER_STR  "Flashing Amber"
#define LED_FLASHING_GREEN_STR  "Flashing Green"
#define LED_SOLID_AMBER_STR     "Solid Amber"
#define LED_SOLID_GREEN_STR     "Solid Green"

// * The following defines are used for sysevent retrieval. */
#define SYSEVENT_IPV4_CONNECTION_STATE "ipv4_connection_state"
#define SYSEVENT_CURRENT_IPV4_LINK_STATE  "current_ipv4_link_state"
#define SYSEVENT_IPV6_CONNECTION_STATE "ipv6_connection_state"
#define SYSEVENT_CURRENT_WAN_STATE "current_wan_state"
#define SYSEVENT_WAN_START_TIME "wan_start_time"
#define SYSEVENT_IPV4_IP_ADDRESS "ipv4_%s_ipaddr"
#define SYSEVENT_IPV4_WAN_ADDRESS "ipv4_wan_ipaddr"
#define SYSEVENT_IPV4_SUBNET "ipv4_%s_subnet"
#define SYSEVENT_IPV4_WAN_SUBNET "ipv4_wan_subnet"
#define SYSEVENT_IPV4_DNS_NUMBER "ipv4_%s_dns_number"
#define SYSEVENT_IPV4_DNS_PRIMARY "ipv4_%s_dns_0"
#define SYSEVENT_IPV4_DNS_SECONDARY "ipv4_%s_dns_1"
#define SYSEVENT_IPV4_GW_NUMBER "ipv4_%s_gw_number"
#define SYSEVENT_IPV4_GW_ADDRESS "ipv4_%s_gw_0"
#define SYSEVENT_VENDOR_CLASS "vendor_class"
#define SYSEVENT_IPV4_DEFAULT_ROUTER "default_router"
#define SYSEVENT_IPV4_TIME_OFFSET "ipv4-timeoffset"
#define SYSEVENT_IPV4_TIME_ZONE "ipv4_timezone"
#define SYSEVENT_DHCPV4_TIME_OFFSET "dhcpv4_time_offset"
#define SYSEVENT_IPV4_US_CURRENT_RATE "ipv4_%s_us_current_rate_0"
#define SYSEVENT_IPV4_DS_CURRENT_RATE "ipv4_%s_ds_current_rate_0"
#define SYSEVENT_FIELD_SERVICE_ROUTED_STATUS "routed-status"

/*dhcp server restart*/
#define SYSEVENT_DHCP_SERVER_RESTART "dhcp_server-restart"

#define SYSCFG_FIELD_IPV6_ADDRESS                   "wan_ipv6addr"
#define SYSCFG_FIELD_IPV6_PREFIX                    "ipv6_prefix"
#define SYSCFG_FIELD_IPV6_PREFIX_ADDRESS            "ipv6_prefix_address"
#define SYSCFG_FIELD_IPV6_GW_ADDRESS                "wan_ipv6_default_gateway"
#define SYSCFG_FIELD_PREVIOUS_IPV6_PREFIX           "previousipv6_prefix"
#define SYSEVENT_FIELD_IPV6_PREFIX                  "ipv6_prefix"
#define SYSEVENT_FIELD_PREVIOUS_IPV6_PREFIX         "previous_ipv6_prefix"
#define SYSEVENT_FIELD_LAN_IPV6_START               "lan_ipv6-start"
#define SYSEVENT_FIELD_IPV6_DNS_PRIMARY             "ipv6_dns_0"
#define SYSEVENT_FIELD_IPV6_DNS_SECONDARY           "ipv6_dns_1"
#define SYSEVENT_FIELD_IPV6_DOMAIN                  "ipv6_domain"
#define SYSEVENT_FIELD_IPV6_PREFIXVLTIME            "ipv6_prefix_vldtime"
#define SYSEVENT_FIELD_IPV6_PREFIXPLTIME            "ipv6_prefix_prdtime"
#define SYSEVENT_FIELD_PREVIOUS_IPV6_PREFIXVLTIME   "previous_ipv6_prefix_vldtime"
#define SYSEVENT_FIELD_PREVIOUS_IPV6_PREFIXPLTIME   "previous_ipv6_prefix_prdtime"
#define SYSEVENT_RADVD_RESTART                      "radvd_restart"
#define SYSEVENT_GLOBAL_IPV6_PREFIX_SET             "lan_prefix_set"
#define SYSEVENT_ULA_ADDRESS                        "ula_address"
#define SYSEVENT_ULA_ENABLE                         "lan_ula_enable"
#define SYSEVENT_IPV6_ENABLE                        "lan_ipv6_enable"

#define SYSEVENT_FIELD_TR_BRLAN0_DHCPV6_SERVER_ADDRESS        "tr_brlan0_dhcpv6_server_v6addr"
#define SYSEVENT_FIELD_TR_EROUTER_DHCPV6_CLIENT_PREFIX        "tr_erouter0_dhcpv6_client_v6pref"

/*WAN specific sysevent fieldnames*/
#define SYSEVENT_CURRENT_WAN_IFNAME "current_wan_ifname"
#define SYSEVENT_WAN_STATUS         "wan-status"
#define SYSEVENT_LAN_STATUS         "lan-status"
#define SYSEVENT_CURRENT_WAN_IPADDR "current_wan_ipaddr"
#define SYSEVENT_CURRENT_WAN_SUBNET "current_wan_subnet"
#define SYSEVENT_WAN_SERVICE_STATUS "wan_service-status"
#define SYSEVENT_PNM_STATUS         "pnm-status"
#define SYSEVENT_LAN_START          "lan-start"
#define SYSEVENT_DHCP_SERVER_RESYNC "dhcp_server-resync"
#define SYSEVENT_START_MISC         "start-misc"
#define SYSEVENT_PRIMARY_LAN_L3NET  "primary_lan_l3net"
#define SYSEVENT_L3NET_INSTANCES    "l3net_instances"
#define SYSEVENT_IPV4_UP            "ipv4-up"
#define SYSEVENT_LAN_ULA_ADDRESS    "lan_ula_address"
#define SYSEVENT_VENDOR_SPEC        "vendor_spec"
#define SYSEVENT_PRIMARY_LAN_L3NET  "primary_lan_l3net"
#define SYSEVENT_IPV6_PREFIX        "ipv6_prefix"
#define SYSEVENT_RADVD_RESTART      "radvd_restart"
#define SYSEVENT_GLOBAL_IPV6_PREFIX_CLEAR   "lan_prefix_clear"
#define SYSEVENT_ETHWAN_INITIALIZED   "ethwan-initialized"
#define SYSEVENT_ETH_WAN_MAC          "eth_wan_mac"
#define SYSEVENT_BRIDGE_MODE          "bridge_mode"
#define SYSEVENT_WAN_START            "wan-start"
#define SYSCFG_ETH_WAN_ENABLED        "eth_wan_enabled"
#define SYSCFG_NTP_ENABLED            "ntp_enabled"
#define SYSEVENT_LAN_PD_INTERFACES    "lan_pd_interfaces"
#define SYSEVENT_MULTINET_NAME        "multinet_1-name"
#define SYSEVENT_MULTINET_INSTANCES   "multinet-instances"
#define SYSEVENT_TOPOLOGY_MODE        "erouter_topology-mode"

//firewall restart
#define SYSEVENT_FIREWALL_RESTART "firewall-restart"
#define SYSEVENT_FIREWALL_STATUS "firewall-status"

#define SYSEVENT_IPV4_LEASE_TIME  "ipv4_%s_lease_time"
#define SYSEVENT_IPV4_DHCP_SERVER "ipv4_%s_dhcp_server"
#define SYSEVENT_IPV4_DHCP_STATE  "ipv4_%s_dhcp_state"
#define SYSEVENT_IPV4_START_TIME  "ipv4_%s_start_time"


#define WAN_STATUS_STARTED      "started"
#define WAN_STATUS_STOPPED      "stopped"
#define FIREWALL_STATUS_STARTED "started"
#define STATUS_UP_STRING        "up"
#define STATUS_DOWN_STRING      "down"
#define SET "set"
#define UNSET "unset"
#define RESET "reset"

/**********************************************************************
                FUNCTION PROTOTYPES
**********************************************************************/
ANSC_STATUS WanMgr_SysEvents_Init(void);
ANSC_STATUS WanMgr_SysEvents_Finalise(void);

/*
 * @brief Utility function used to set string values to syscfg.
 * @param[in] const char* name - Indicates string name represent the value
 * @param[in] const char* value- Indicates the value to pass to the curresponding mem
 * @return Returns ANSC_STATUS.
*/
ANSC_STATUS syscfg_set_string(const char* name, const char* value);

/*
 * @brief Utility function used to set bool values to syscfg.
 * @param[in] const char* name - Indicates string name represent the value
 * @param[in] int value- Indicates the value to pass to the curresponding mem
 * @return Returns ANSC_STATUS.
*/
ANSC_STATUS syscfg_set_bool(const char* name, int value);

/*
 * @brief Utility function used to init all IPv6 values in sysevent
 * @param[in] None
 * @return Returns ANSC_STATUS.
*/
ANSC_STATUS wanmgr_sysevents_ipv6Info_init();

/*
 * @brief Utility function used to init all IPv4 values in sysevent
 * @param[in]
 * @return Returns ANSC_STATUS.
*/
ANSC_STATUS wanmgr_sysevents_ipv4Info_init(const char *wanIfName);

/*
 * @brief Utility function used to store all dhcpv4_data_t values in sysevent
 * @param[in] dhcpv4_data_t *dhcp4Info
 * @return Returns ANSC_STATUS.
*/
ANSC_STATUS wanmgr_sysevents_ipv4Info_set(const ipc_dhcpv4_data_t* dhcp4Info, const char *wanIfName);


/*
 * @brief Utility function used to set led state.
 * @param[in] const char* LedState - Indicates the led state value
 * @return Returns NONE.
*/
void wanmgr_sysevents_setWanLedState(const char * LedState);



//#ifdef FEATURE_MAPT
///*
// * @brief Utility function used to store MAPT specific values in sysevent/
// * @param[in] MaptData_t *maptInfo
// * @return Returns ANSC_STATUS_SUCCESS if data properly update in sysevent.
//*/
//ANSC_STATUS maptInfo_set(const MaptData_t *maptInfo);
//ANSC_STATUS maptInfo_reset();
//#endif // FEATURE_MAPT

#endif //_WANMGR_SYSEVENTS_H_
