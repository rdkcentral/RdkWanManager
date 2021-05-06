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

#include <sysevent/sysevent.h>
#include <pthread.h>
#include <syscfg.h>
#include <syscfg/syscfg.h>

#include "wanmgr_sysevents.h"
#include "wanmgr_rdkbus_utils.h"

int sysevent_fd = -1;
token_t sysevent_token;
static int sysevent_msg_fd = -1;
static token_t sysevent_msg_token;

#define PAM_COMPONENT_NAME          "eRT.com.cisco.spvtg.ccsp.pam"
#define PAM_DBUS_PATH               "/com/cisco/spvtg/ccsp/pam"
#define WANMGR_SYSNAME_RCV          "wanmanager"
#define WANMGR_SYSNAME_SND          "wanmgr"
#define SYS_IP_ADDR                 "127.0.0.1"
#define BUFLEN_42                   42
#define SYSEVENT_VALUE_READY        "ready"
#define SYSEVENT_VALUE_STARTED      "started"
#define SYSEVENT_OPEN_MAX_RETRIES   6
#define BRG_INST_SIZE 5
#define BUF_SIZE 256
#define SKY_DHCPV6_OPTIONS "/var/skydhcp6_options.txt"
#define ENTERPRISE_ID 3561 //Broadband Forum.
#define OPTION_16 16
#define WAN_PHY_ADDRESS "/sys/class/net/erouter0/address"
#define LAN_BRIDGE_NAME "brlan0"

static int pnm_inited = 0;
static int lan_wan_started = 0;
static int ipv4_connection_up = 0;
static int ipv6_connection_up = 0;
static void check_lan_wan_ready();
static int CheckV6DefaultRule();
static void do_toggle_v6_status();
static void lan_start();
static void set_vendor_spec_conf();
static int getVendorClassInfo(char *buffer, int length);
static int set_default_conf_entry();

static ANSC_STATUS WanMgr_SyseventInit()
{
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;
    bool send_fd_status = FALSE;
    bool recv_fd_status = FALSE;
    int try = 0;

    /* Open sysevent descriptor to send messages */
    while(try < SYSEVENT_OPEN_MAX_RETRIES)
    {
       sysevent_fd =  sysevent_open(SYS_IP_ADDR, SE_SERVER_WELL_KNOWN_PORT, SE_VERSION, WANMGR_SYSNAME_SND, &sysevent_token);
       if(sysevent_fd >= 0)
       {
          send_fd_status = TRUE;
          break;
       }
       try++;
       usleep(50000);
    }

    try = 0;
    /* Open sysevent descriptor to receive messages */
    while(try < SYSEVENT_OPEN_MAX_RETRIES)
    {
        sysevent_msg_fd =  sysevent_open(SYS_IP_ADDR, SE_SERVER_WELL_KNOWN_PORT, SE_VERSION, WANMGR_SYSNAME_RCV, &sysevent_msg_token);
        if(sysevent_msg_fd >= 0)
        {
            recv_fd_status = TRUE;
            break;
        }
        try++;
        usleep(50000);
    }

    if (send_fd_status == FALSE || recv_fd_status == FALSE)
    {
        ret = ANSC_STATUS_FAILURE;
    }

    return ret;
}

static void WanMgr_SyseventClose()
{
    if (0 <= sysevent_fd)
    {
        sysevent_close(sysevent_fd, sysevent_token);
    }

    if (0 <= sysevent_msg_fd)
    {
        sysevent_close(sysevent_msg_fd, sysevent_msg_token);
    }
}


ANSC_STATUS syscfg_set_string(const char* name, const char* value)
{
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;
    if (syscfg_set(NULL, name, value) != 0)
    {
        CcspTraceError(("syscfg_set failed: %s %s\n", name, value));
        ret = ANSC_STATUS_FAILURE;
    }
    else
    {
        if (syscfg_commit() != 0)
        {
            CcspTraceError(("syscfg_commit failed: %s %s\n", name, value));
            ret = ANSC_STATUS_FAILURE;
        }
    }

    return ret;
}

ANSC_STATUS syscfg_set_bool(const char* name, int value)
{
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;
    char buf[10];
    memset(buf,0,sizeof(buf));

    sprintf(buf, "%d", value);
    if (syscfg_set(NULL, name, buf) != 0)
    {
        CcspTraceError(("syscfg_set failed: %s %d\n", name, value));
        ret = ANSC_STATUS_FAILURE;
    }
    else
    {
        if (syscfg_commit() != 0)
            CcspTraceError(("syscfg_commit failed: %s %d\n", name, value));
        ret = ANSC_STATUS_FAILURE;
    }

    return ret;
}

ANSC_STATUS wanmgr_sysevents_ipv6Info_init()
{
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIELD_IPV6_DNS_PRIMARY, "", 0);
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIELD_IPV6_DNS_SECONDARY, "", 0);
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_FIELD_IPV6_DOMAIN, "", 0);
    syscfg_set_string(SYSCFG_FIELD_IPV6_PREFIX_ADDRESS, "");
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS wanmgr_sysevents_ipv4Info_set(const ipc_dhcpv4_data_t* dhcp4Info, const char *wanIfName)
{
    char name[BUFLEN_64] = {0};
    char value[BUFLEN_64] = {0};

    snprintf(name, sizeof(name), SYSEVENT_CURRENT_WAN_IFNAME);
    sysevent_set(sysevent_fd, sysevent_token,name, dhcp4Info->dhcpcInterface, 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_IP_ADDRESS, wanIfName);
    sysevent_set(sysevent_fd, sysevent_token,name, dhcp4Info->ip, 0);

    //same as SYSEVENT_IPV4_IP_ADDRESS. But this is required in other components
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_IPV4_WAN_ADDRESS, dhcp4Info->ip, 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_SUBNET, wanIfName);
    sysevent_set(sysevent_fd, sysevent_token,name, dhcp4Info->mask, 0);

    //same as SYSEVENT_IPV4_SUBNET. But this is required in other components
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_IPV4_WAN_SUBNET, dhcp4Info->mask, 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_GW_NUMBER, wanIfName);
    sysevent_set(sysevent_fd, sysevent_token, name, "1", 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_GW_ADDRESS, wanIfName);
    sysevent_set(sysevent_fd, sysevent_token,name, dhcp4Info->gateway, 0);
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_IPV4_DEFAULT_ROUTER, dhcp4Info->gateway, 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_DNS_NUMBER, wanIfName);
    sysevent_set(sysevent_fd, sysevent_token,name, "2", 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_DNS_PRIMARY, wanIfName);
    sysevent_set(sysevent_fd, sysevent_token,name, dhcp4Info->dnsServer, 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_DNS_SECONDARY, wanIfName);
    sysevent_set(sysevent_fd, sysevent_token,name, dhcp4Info->dnsServer1, 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_DS_CURRENT_RATE, wanIfName);
    snprintf(value, sizeof(value), "%d", dhcp4Info->downstreamCurrRate);
    sysevent_set(sysevent_fd, sysevent_token,name, value, 0);

    snprintf(name, sizeof(name), SYSEVENT_IPV4_US_CURRENT_RATE, wanIfName);
    snprintf(value, sizeof(value), "%d", dhcp4Info->upstreamCurrRate);
    sysevent_set(sysevent_fd, sysevent_token,name, value, 0);

    if (dhcp4Info->isTimeOffsetAssigned)
    {
        snprintf(value, sizeof(value), "@%d", dhcp4Info->timeOffset);
        sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_IPV4_TIME_OFFSET, value, 0);
        sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_DHCPV4_TIME_OFFSET, SET, 0);
    }
    else
    {
        sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_DHCPV4_TIME_OFFSET, UNSET, 0);
    }

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_IPV4_TIME_ZONE, dhcp4Info->timeZone, 0);

    snprintf(name,sizeof(name),SYSEVENT_IPV4_DHCP_SERVER,dhcp4Info->dhcpcInterface);
    sysevent_set(sysevent_fd, sysevent_token,name, dhcp4Info->dhcpServerId,0);

    snprintf(name,sizeof(name),SYSEVENT_IPV4_DHCP_STATE ,dhcp4Info->dhcpcInterface);
    sysevent_set(sysevent_fd, sysevent_token,name, dhcp4Info->dhcpState,0);

    snprintf(name,sizeof(name), SYSEVENT_IPV4_LEASE_TIME, dhcp4Info->dhcpcInterface);
    snprintf(value, sizeof(value), "%u",dhcp4Info->leaseTime);
    sysevent_set(sysevent_fd, sysevent_token,name, value, 0);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS wanmgr_sysevents_ipv4Info_init(const char *wanIfName)
{
    char name[BUFLEN_64] = {0};
    char value[BUFLEN_64] = {0};
    ipc_dhcpv4_data_t ipv4Data;

    memset(&ipv4Data, 0, sizeof(ipc_dhcpv4_data_t));
    strncpy(ipv4Data.ip, "0.0.0.0", strlen("0.0.0.0"));
    strncpy(ipv4Data.mask, "0.0.0.0", strlen("0.0.0.0"));
    strncpy(ipv4Data.gateway, "0.0.0.0", strlen("0.0.0.0"));
    strncpy(ipv4Data.dnsServer, "0.0.0.0", strlen("0.0.0.0"));
    strncpy(ipv4Data.dnsServer1, "0.0.0.0", strlen("0.0.0.0"));
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_CURRENT_WAN_IPADDR, "0.0.0.0", 0);
    snprintf(name, sizeof(name), SYSEVENT_IPV4_START_TIME, wanIfName);
    sysevent_set(sysevent_fd, sysevent_token,name, "0", 0);
    return wanmgr_sysevents_ipv4Info_set(&ipv4Data, wanIfName);
}


void wanmgr_sysevents_setWanLedState(const char * LedState)
{
    if (sysevent_fd == -1)      return;
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_WAN_LED_STATE, LedState, 0);
}


#ifdef FEATURE_MAPT
ANSC_STATUS maptInfo_set(const MaptData_t *maptInfo)
{
    if (NULL == maptInfo)
    {
        CcspTraceError(("Invalid arguments \n"));
        return ANSC_STATUS_FAILURE;
    }

    char name[BUFLEN_64] = {0};
    char value[BUFLEN_64] = {0};

    snprintf(name, sizeof(name), SYSEVENT_MAPT_CONFIG_FLAG);
    sysevent_set(sysevent_fd, sysevent_token,name, maptInfo->maptConfigFlag, 0);

    snprintf(name, sizeof(name),SYSEVENT_MAPT_RATIO);
    snprintf(value, sizeof(value), "%d", maptInfo->ratio);
    sysevent_set(sysevent_fd, sysevent_token,name, value, 0);

    snprintf(name, sizeof(name), SYSEVENT_MAPT_IPADDRESS);
    sysevent_set(sysevent_fd, sysevent_token,name, maptInfo->ipAddressString, 0);

    snprintf(name, sizeof(name), SYSEVENT_MAP_BR_IPV6_PREFIX);
    sysevent_set(sysevent_fd, sysevent_token,name, maptInfo->brIpv6PrefixString, 0);

    snprintf(name, sizeof(name), SYSEVENT_MAP_RULE_IPADDRESS);
    sysevent_set(sysevent_fd, sysevent_token,name, maptInfo->ruleIpAddressString, 0);

    snprintf(name, sizeof(name), SYSEVENT_MAPT_IPV6_ADDRESS);
    sysevent_set(sysevent_fd, sysevent_token,name, maptInfo->ipv6AddressString, 0);

    snprintf(name, sizeof(name), SYSEVENT_MAP_RULE_IPV6_ADDRESS);
    sysevent_set(sysevent_fd, sysevent_token,name, maptInfo->ruleIpv6AddressString, 0);

    snprintf(name, sizeof(name), SYSEVENT_MAPT_PSID_OFFSET);
    snprintf(value, sizeof(value), "%d", maptInfo->psidOffset);
    sysevent_set(sysevent_fd, sysevent_token,name, value, 0);

    snprintf(name, sizeof(name), SYSEVENT_MAPT_PSID_VALUE);
    snprintf(value, sizeof(value), "%d", maptInfo->psidValue);
    sysevent_set(sysevent_fd, sysevent_token,name, value, 0);

    snprintf(name, sizeof(name), SYSEVENT_MAPT_PSID_LENGTH);
    snprintf(value, sizeof(value), "%d", maptInfo->psidLen);
    sysevent_set(sysevent_fd, sysevent_token,name, value, 0);

    if(maptInfo->maptAssigned)
    {
        strncpy(value, "MAPT", 4);
    }
    else if(maptInfo->mapeAssigned)
    {
        strncpy(value, "MAPE", 4);
    }
    else
    {
        strncpy(value, "NON-MAP", 7);
    }
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAP_TRANSPORT_MODE, value, 0);

    if(maptInfo->isFMR ==1)
    {
        strncpy(value, "TRUE", 4);
    }
    else
    {
        strncpy(value, "FALSE", 5);
    }
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAP_IS_FMR, value, 0);

    snprintf(value, sizeof(value), "%d", maptInfo->eaLen);
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAP_EA_LENGTH, value, 0);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS maptInfo_reset()
{
    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAPT_CONFIG_FLAG, RESET, 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAPT_RATIO, "", 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAPT_IPADDRESS, "", 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAPT_IPV6_ADDRESS, "", 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAPT_PSID_OFFSET, "", 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAPT_PSID_VALUE, "", 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAPT_PSID_LENGTH, "", 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAP_TRANSPORT_MODE, "", 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAP_IS_FMR, "", 0);

    sysevent_set(sysevent_fd, sysevent_token,SYSEVENT_MAP_EA_LENGTH, "", 0);

    return ANSC_STATUS_SUCCESS;
}
#endif // FEATURE_MAPT

static int set_default_conf_entry()
{
    char command[BUFLEN_64];
    char result[BUFLEN_64];
    FILE *fp;

    syscfg_init();
    memset(command, 0, sizeof(command));
    memset(result, 0, sizeof(result));

    snprintf(command, sizeof(command), "cat %s", WAN_PHY_ADDRESS);
    fp = popen(command, "r");
    fgets(result, sizeof(result), fp);
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_ETH_WAN_MAC, result, 0);
    pclose(fp);
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_BRIDGE_MODE, "0", 0); // to boot in router mode
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_TOPOLOGY_MODE, "1", 0);
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_MULTINET_INSTANCES, LAN_BRIDGE_NAME, 0);
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_MULTINET_NAME, LAN_BRIDGE_NAME, 0);
    syscfg_set(NULL, SYSEVENT_LAN_PD_INTERFACES, LAN_BRIDGE_NAME); // sets the lan interface for prefix deligation
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_WAN_START, "", 0);
    syscfg_set(NULL, SYSCFG_ETH_WAN_ENABLED, "true"); // to handle Factory reset case
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_ETHWAN_INITIALIZED, "0", 0);
    syscfg_set(NULL, SYSCFG_NTP_ENABLED, "1"); // Enable NTP in case of ETHWAN

    syscfg_commit();

    return 0;
}

static void *WanManagerSyseventHandler(void *args)
{
    CcspTraceInfo(("%s %d \n", __FUNCTION__, __LINE__));

    //detach thread from caller stack
    pthread_detach(pthread_self());

    async_id_t wanamangr_status_asyncid;
    async_id_t lan_ula_address_event_asyncid;
    async_id_t lan_ula_enable_asyncid;
    async_id_t lan_ipv6_enable_asyncid;
    async_id_t wan_status_asyncid;
    async_id_t wan_service_status_asyncid;
    async_id_t pnm_status_asyncid;
    async_id_t lan_status_asyncid;
    async_id_t primary_lan_l3net_asyncid;
    async_id_t radvd_restart_asyncid;
    async_id_t ipv6_down_asyncid;

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_ULA_ADDRESS, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_ULA_ADDRESS, &lan_ula_address_event_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_ULA_ENABLE, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_ULA_ENABLE, &lan_ula_enable_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_IPV6_ENABLE, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_IPV6_ENABLE, &lan_ipv6_enable_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_WAN_STATUS, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_WAN_STATUS,  &wan_status_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_WAN_SERVICE_STATUS, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_WAN_SERVICE_STATUS, &wan_service_status_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_PNM_STATUS, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_PNM_STATUS, &pnm_status_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_LAN_STATUS, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_LAN_STATUS,  &lan_status_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_PRIMARY_LAN_L3NET, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_PRIMARY_LAN_L3NET,  &primary_lan_l3net_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_RADVD_RESTART, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_RADVD_RESTART, &radvd_restart_asyncid);

    sysevent_set_options(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_GLOBAL_IPV6_PREFIX_CLEAR, TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_msg_fd, sysevent_msg_token, SYSEVENT_GLOBAL_IPV6_PREFIX_CLEAR, &ipv6_down_asyncid);
    for(;;)
    {
        char name[BUFLEN_42] = {0};
        char val[BUFLEN_42] = {0};
        char buf[BUF_SIZE] = {0};
        int namelen = sizeof(name);
        int vallen  = sizeof(val);
        async_id_t getnotification_asyncid;
        int err = 0;
        ANSC_STATUS result = 0;
        char *datamodel_value = NULL;
        char brlan0_inst[BRG_INST_SIZE];
        char* l3net_inst = NULL;
        int l2net_inst_up = FALSE;

        err = sysevent_getnotification(sysevent_msg_fd, sysevent_msg_token, name, &namelen,  val, &vallen, &getnotification_asyncid);

        if(err)
        {
            CcspTraceError(("%s %d sysevent_getnotification failed with error: %d \n", __FUNCTION__, __LINE__, err ));
            sleep(2);
        }
        else
        {
            CcspTraceInfo(("%s %d - received notification event %s:%s\n", __FUNCTION__, __LINE__, name, val ));
            if ( strcmp(name, SYSEVENT_ULA_ADDRESS) == 0 )
            {
                datamodel_value = (char *) malloc(sizeof(char) * 256);
                if(datamodel_value != NULL)
                {
                    memset(datamodel_value, 0, 256);
                    strncpy(datamodel_value, val, sizeof(val));
                    result = WanMgr_RdkBus_SetParamValues( PAM_COMPONENT_NAME, PAM_DBUS_PATH, "Device.DHCPv6.Server.Pool.1.X_RDKCENTRAL-COM_DNSServersEnabled", "true", ccsp_boolean, TRUE );
                    if(result == ANSC_STATUS_SUCCESS)
                    {
                        result = WanMgr_RdkBus_SetParamValues( PAM_COMPONENT_NAME, PAM_DBUS_PATH, "Device.DHCPv6.Server.Pool.1.X_RDKCENTRAL-COM_DNSServers", datamodel_value, ccsp_string, TRUE );
                        if(result != ANSC_STATUS_SUCCESS) {
                            CcspTraceError(("%s %d - SetDataModelParameter() failed for X_RDKCENTRAL-COM_DNSServers parameter \n", __FUNCTION__, __LINE__));
                        }
                    }
                    else {
                        CcspTraceError(("%s %d - SetDataModelParameter() failed for X_RDKCENTRAL-COM_DNSServersEnabled parameter \n", __FUNCTION__, __LINE__));
                    }
                    free(datamodel_value);
                }
            }
            else if ( strcmp(name, SYSEVENT_ULA_ENABLE) == 0 )
            {
                datamodel_value = (char *) malloc(sizeof(char) * 256);
                if(datamodel_value != NULL)
                {
                    memset(datamodel_value, 0, 256);
                    strncpy(datamodel_value, val, sizeof(val));
                    result = WanMgr_RdkBus_SetParamValues( PAM_COMPONENT_NAME, PAM_DBUS_PATH, "Device.DHCPv6.Server.Pool.1.X_RDKCENTRAL-COM_DNSServersEnabled", datamodel_value, ccsp_boolean, TRUE );
                    if(result != ANSC_STATUS_SUCCESS)
                    {
                        CcspTraceError(("%s %d - SetDataModelParameter failed on dns_enable request \n", __FUNCTION__, __LINE__));
                    }
                }
                free(datamodel_value);
            }
            else if ( strcmp(name, SYSEVENT_IPV6_ENABLE) == 0 )
            {
                datamodel_value = (char *) malloc(sizeof(char) * 256);
                if(datamodel_value != NULL)
                {
                    memset(datamodel_value, 0, 256);
                    strncpy(datamodel_value, val, sizeof(val));
                    result = WanMgr_RdkBus_SetParamValues( PAM_COMPONENT_NAME, PAM_DBUS_PATH, "Device.DHCPv6.Server.Pool.1.Enable", datamodel_value, ccsp_boolean, TRUE );
                    if(result != ANSC_STATUS_SUCCESS)
                    {
                        CcspTraceError(("%s %d - SetDataModelParameter failed on ipv6_enable request \n", __FUNCTION__, __LINE__ ));
                    }
                    free(datamodel_value);
                    system("sysevent set zebra-restart");
                }
            }
            else if ((strcmp(name, SYSEVENT_WAN_STATUS) == 0) && (strcmp(val, SYSEVENT_VALUE_STARTED) == 0))
            {
                if (!lan_wan_started)
                {
                    check_lan_wan_ready();
                }
                system("touch /tmp/phylink_wan_state_up");
            }
            else if (strcmp(name, SYSEVENT_WAN_SERVICE_STATUS) == 0)
            {
                CcspTraceError(("%s %d - received notification event %s:%s\n", __FUNCTION__, __LINE__, name, val ));
                if (strcmp(val, SYSEVENT_VALUE_STARTED) == 0) {
                    do_toggle_v6_status();
                }
            }
            else if (strcmp(name, SYSEVENT_PNM_STATUS) == 0)
            {
                if (strcmp(val, STATUS_UP_STRING)==0)
                {
                    pnm_inited = 1;
                    lan_start();
                    system("firewall && execute_dir /etc/utopia/post.d/ restart");
                }
            }
            else if (strcmp(name, SYSEVENT_PRIMARY_LAN_L3NET) == 0)
            {
                if (pnm_inited)
                {
                    lan_start();
                }
            }
            else if (strcmp(name, SYSEVENT_LAN_STATUS) == 0 )
            {
                char wanStatus[BUFLEN_16] = {0};
                if (strcmp(val, SYSEVENT_VALUE_STARTED) == 0)
                {
                    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_PRIMARY_LAN_L3NET, buf, sizeof(buf));
                    strncpy(brlan0_inst, buf, BRG_INST_SIZE-1);
                    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_L3NET_INSTANCES, buf, sizeof(buf));
                    l3net_inst = strtok(buf, " ");
                    while(l3net_inst != NULL)
                    {
                        if(!(strcmp(l3net_inst, brlan0_inst)==0))
                        {
                            sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_IPV4_UP, l3net_inst, 0);
                            l2net_inst_up = TRUE;
                        }
                        l3net_inst = strtok(NULL, " ");
                    }
                    if(l2net_inst_up == FALSE)
                    {
                        sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_IPV4_UP, brlan0_inst, 0);
                    }
                    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_WAN_SERVICE_STATUS, wanStatus, sizeof(wanStatus));
                    if(strcmp(wanStatus, SYSEVENT_VALUE_STARTED) == 0)
                    {
                        do_toggle_v6_status();
                    }
                    memset(buf, '\0', sizeof(buf));
                    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_ULA_ADDRESS, buf, sizeof(buf));
                    if(buf[0] != '\0') {
                        sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_LAN_ULA_ADDRESS, buf, 0);
                    }
                    set_vendor_spec_conf();
                    system("gw_lan_refresh &");
#ifdef FEATURE_MAPT
                    memset(buf, '\0', sizeof(buf));
                    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_MAP_TRANSPORT_MODE, buf, sizeof(buf));
                    if( !strcmp(buf, "MAPT") || !strcmp(buf, "MAPE") )  {
                        set_mapt_rule();
                    }
#endif
                }
            }
            else if (strcmp(name, SYSEVENT_RADVD_RESTART) == 0)
            {
                sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIELD_SERVICE_ROUTED_STATUS, "", 0);
                system("service_routed radv-restart");
            }
            else if (strcmp(name, SYSEVENT_GLOBAL_IPV6_PREFIX_CLEAR) == 0)
            {
                sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_IPV6_CONNECTION_STATE, STATUS_DOWN_STRING, 0);
                sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIREWALL_RESTART, NULL, 0);
            }
            else
            {
                CcspTraceError(("%s %d undefined event %s:%s \n", __FUNCTION__, __LINE__, name, val));
            }
        }
    }

    CcspTraceInfo(("%s %d - WanManagerSyseventHandler Exit \n", __FUNCTION__, __LINE__));
    return 0;
}

static void lan_start()
{
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_LAN_START, "", 0);
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_DHCP_SERVER_RESYNC, "", 0);
    return;
}
static void check_lan_wan_ready()
{
    char lan_st[BUFLEN_16];
    char wan_st[BUFLEN_16];
    memset(lan_st,0,sizeof(lan_st));
    memset(wan_st,0,sizeof(wan_st));
    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_LAN_STATUS, lan_st, sizeof(lan_st));
    sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_WAN_STATUS, wan_st, sizeof(wan_st));

    CcspTraceInfo(("****************************************************\n"));
    CcspTraceInfo(("    Lan Status = %s   Wan Status = %s\n", lan_st, wan_st));
    CcspTraceInfo(("****************************************************\n"));
    if (!strcmp(lan_st, SYSEVENT_VALUE_STARTED) && !strcmp(wan_st, SYSEVENT_VALUE_STARTED))
    {
        sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_START_MISC, SYSEVENT_VALUE_READY, 0);
        lan_wan_started = 1;
    }
    return;
}
static int CheckV6DefaultRule()
{
    int ret = FALSE;
    FILE *fp = NULL;
    char buf[256] = {0};
    char output[256] = {0};

    snprintf(buf, sizeof(buf), " ip -6 ro | grep default | grep via | grep erouter0");
    if(!(fp = popen(buf, "r")))
    {
        return -1;
    }
    while(fgets(output, sizeof(output), fp)!=NULL)
    {
        ret = TRUE; // Default rout entry exist
    }

    pclose(fp);
    return ret;
}

static void do_toggle_v6_status()
{
    char cmdLine[BUFLEN_128] = {0};
    bool isV6DefaultRoutePresent = FALSE;
    isV6DefaultRoutePresent = CheckV6DefaultRule();
    if ( isV6DefaultRoutePresent != TRUE)
    {
        snprintf(cmdLine, sizeof(cmdLine), "echo 1 > /proc/sys/net/ipv6/conf/erouter0/disable_ipv6");
        system(cmdLine);
        snprintf(cmdLine, sizeof(cmdLine), "echo 0 > /proc/sys/net/ipv6/conf/erouter0/disable_ipv6");
        system(cmdLine);
    }
    return;
}

static void set_vendor_spec_conf()
{
    char vendor_class[BUF_SIZE] = {0};
    if(getVendorClassInfo(vendor_class, BUF_SIZE) == 0)
    {
        char vendor_spec_info[BUFLEN_512] = {0};
        sprintf(vendor_spec_info, "%d-%d-\"%s\"", ENTERPRISE_ID, OPTION_16, vendor_class);
        CcspTraceInfo(("vendor_spec information = %s \n", vendor_spec_info));
        sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_VENDOR_SPEC, vendor_spec_info, 0);
    }
    else
    {
        CcspTraceError(("getVendorClassInfo failed"));
    }
}

static int getVendorClassInfo(char *buffer, int length)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t characters;
    fp = fopen(SKY_DHCPV6_OPTIONS, "r");
    if (fp == NULL) {
        //AnscTraceFlow(("%s file not found", SKY_DHCPV6_OPTIONS));
        return -1;
    }
    if ((characters = getline(&line, &len, fp)) != -1) {
        if ((characters = getline(&line, &len, fp)) != -1) {
            if (line != NULL) {
                if (line[characters - 1] == '\n')
                    line[characters - 1] = '\0';
                strncpy(buffer, line, length);
                free(line);
            }
        }
    }
    fclose(fp);
    return 0;
}


ANSC_STATUS WanMgr_SysEvents_Init(void)
{
    ANSC_STATUS retStatus = ANSC_STATUS_SUCCESS;
    pthread_t sysevent_tid;

    // Initialize sysevent daemon
    retStatus = WanMgr_SyseventInit();
    if (retStatus != ANSC_STATUS_SUCCESS)
    {
        WanMgr_SyseventClose();
        CcspTraceError(("%s %d - WanMgr_SyseventInit failed \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    set_default_conf_entry();
    //Init msg status handler
    if(pthread_create(&sysevent_tid, NULL, WanManagerSyseventHandler, NULL) == 0) {
        CcspTraceError(("%s %d - DmlWanMsgHandler -- pthread_create successfully \n", __FUNCTION__, __LINE__));    
    }
    else {
        CcspTraceError(("%s %d - DmlWanMsgHandler -- pthread_create failed \n", __FUNCTION__, __LINE__));
    }

    //Initialize syscfg value of ipv6 address to release previous value
    syscfg_set_string(SYSCFG_FIELD_IPV6_PREFIX_ADDRESS, "");

    return retStatus;
}

ANSC_STATUS WanMgr_SysEvents_Finalise(void)
{
    ANSC_STATUS retStatus = ANSC_STATUS_SUCCESS;

    WanMgr_SyseventClose();

    return retStatus;
}
