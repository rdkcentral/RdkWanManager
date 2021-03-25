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

/* ---- Include Files ---------------------------------------- */
#include <arpa/inet.h> /* for inet_pton */
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>
#include "wanmgr_net_utils.h"
#include "wanmgr_rdkbus_utils.h"
#include "wanmgr_dhcpv4_apis.h"
#include "wanmgr_rdkbus_apis.h"
#include "wanmgr_ssp_internal.h"
#include "ansc_platform.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include "platform_hal.h"
#include <sys/sysinfo.h>

#define RESOLV_CONF_FILE "/etc/resolv.conf"
#define LOOPBACK "127.0.0.1"
#define BROADCAST_IP "255.255.255.255"
/* To ignore link local addresses configured as DNS servers,
 * it covers the range 169.254.x.x */
#define LINKLOCAL_RANGE "169.254"
/* To ignore loopback addresses configured as DNS servers,
 * it covers the range 127.x.x.x */
#define LOOPBACK_RANGE "127"

#define BASE_IFNAME_PPPoA "atm0"
#define BASE_IFNAME_PPPoE "vlan101"
#define DEFAULT_IFNAME    "erouter0"

/** Macro to determine if a string parameter is empty or not */
#define IS_EMPTY_STR(s)    ((s == NULL) || (*s == '\0'))

#define DHCPV4C_PID_FILE "/tmp/erouter_dhcp4c.pid"
#define MTU_SIZE (1520)
#define MTU_DEFAULT_SIZE (1500)
#define DHCPV6_OPTION_STR_LEN 288
#define DEFAULT_SER_FIELD_LEN 64
#define DHCPV6_OPTIONS_FILE "/var/skydhcp6_options.txt"
#ifdef FEATURE_IPOE_HEALTH_CHECK
#define DHCP6C_RENEW_PREFIX_FILE    "/tmp/erouter0.dhcpc6c_renew_prefix.conf"
#endif

/** Some defines for selecting which address family (IPv4 or IPv6) we want.
 *  Note: are bits for use in bitmasks (not integers)
 */
#define AF_SELECT_IPV4       0x0001
#define AF_SELECT_IPV6       0x0002

extern ANSC_HANDLE bus_handle;

#define DATAMODEL_PARAM_LENGTH 256

#define SYSCFG_WAN_INTERFACE_NAME "wan_physical_ifname"
#define DUID_CLIENT "/tmp/dibbler/client-duid"
#define DUID_TYPE "00:03:"  /* duid-type duid-ll 3 */
#define HW_TYPE "00:01:"    /* hw type is always 1 */
#define DIBBLER_IPV6_CLIENT_ENABLE      "Device.DHCPv6.Client.%d.Enable"

#define IPMONITOR_WAIT_MAX_TIMEOUT 240
#define IPMONITOR_QUERY_INTERVAL 1
/***************************************************************************
 * @brief API used to check the incoming nameserver is valid
 * @param af indicates ip address family
 * @param nameserver dns namserver name
 * @return RETURN_OK if execution successful else returned error.
 ****************************************************************************/
static int IsValidDnsServer(int32_t af, const char *nameServer);

/***************************************************************************
 * @brief API used to check the incoming ipv4 address is a valid ipv4 address
 * @param input string contains ipv4 address
 * @return TRUE if its a valid IP address else returned false.
 ****************************************************************************/
static BOOL IsValidIpv4Address(const char *input);

/***************************************************************************
 * @brief API used to check the incoming ip address is a valid one
 * @param ipvx ip address family either v4 or v6
 * @param addr string contains ip address
 * @return TRUE if its a valid IP address else returned false.
 ****************************************************************************/
static BOOL IsZeroIpvxAddress(uint32_t ipvx, const char *addr);

/***************************************************************************
 * @brief API used to check the incoming ipv address is a valid.
 * @param af indicates address family
 * @param address string contains ip address
 * @return TRUE if its a valid IP address else returned false.
 ****************************************************************************/
static BOOL IsValidIpAddress(int32_t af, const char *address);

/***************************************************************************
 * @brief API used to parse ipv6 prefix address
 * @param prefixAddr ipv6 prefix address
 * @param address string contains ip address
 * @param plen holds length of the prefix address
 * @return TRUE if its a valid IP address else returned false.
 ****************************************************************************/
static int ParsePrefixAddress(const char *prefixAddr, char *address, uint32_t *plen);


/***************************************************************************
 * @brief API used to enable/disable dibbler client
 * @param enable boolean contains enable flag
 * @return ANSC_STATUS_SUCCESS if the operation is successful
 * @return ANSC_STATUS_FAILURE if the operation is failure
 ****************************************************************************/
static ANSC_STATUS setDibblerClientEnable(BOOL * enable);

#ifdef _HUB4_PRODUCT_REQ_
/***************************************************************************
 * @brief API used to get ADSL username and password
 * @param Username: ADSL username
 * @param Password: ADSL Password
 * @return TRUE if ADSL Username and Password is read from file else returned false.
 ****************************************************************************/
#define SERIALIZATION_DATA "/tmp/serial.txt"
static ANSC_STATUS GetAdslUsernameAndPassword(char *Username, char *Password);
#endif

 /***************************************************************************
 * @brief Thread that sets enable/disable data model of dhcpv6 client
 * @param arg: enable/disable flag to start and stop dhcp6c client
 ****************************************************************************/
static void* Dhcpv6HandlingThread( void *arg );
static int get_index_from_path(const char *path);
static void* DmlHandlePPPCreateRequestThread( void *arg );
static void generate_client_duid_conf();
static void createDummyWanBridge();
static void deleteDummyWanBridgeIfExist();
static INT IsIPObtained(char *pInterfaceName);

static ANSC_STATUS SetDataModelParamValues( char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, BOOLEAN bCommit )
{
    CCSP_MESSAGE_BUS_INFO *bus_info              = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t   param_val[1]          = { 0 };
    char                  *faultParam            = NULL;
    int                    ret                   = 0;

    //Copy Name
    param_val[0].parameterName  = pParamName;
    //Copy Value
    param_val[0].parameterValue = pParamVal;

    //Copy Type
    param_val[0].type           = type;
        ret = CcspBaseIf_setParameterValues(
                                        bus_handle,
                                        pComponent,
                                        pBus,
                                        0,
                                        0,
                                        param_val,
                                        1,
                                        bCommit,
                                        &faultParam
                                       );

    if( ( ret != CCSP_SUCCESS ) && ( faultParam != NULL ) )
    {
        CcspTraceError(("%s-%d Failed to set %s\n",__FUNCTION__,__LINE__,pParamName));
        bus_info->freefunc( faultParam );
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}


static ANSC_STATUS setDibblerClientEnable(BOOL *enable)
{
    INT              index               = 0;
    pthread_t        dibblerThreadId;
    INT              iErrorCode          = -1;

    if (NULL == enable)
    {
        CcspTraceError(("%s %d - Invalid memory \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    BOOL *enable_client = NULL;
    enable_client = (BOOL *) malloc (sizeof(BOOL));
    if (NULL == enable_client)
    {
        CcspTraceError(("%s %d - Failed to reserve memory \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    *enable_client = *enable;

    iErrorCode = pthread_create( &dibblerThreadId, NULL, &Dhcpv6HandlingThread, (void*)enable_client );
    if( 0 != iErrorCode )
    {
        CcspTraceInfo(("%s %d - Dhcpv6HandlingThread thread failed. EC:%d\n", __FUNCTION__, __LINE__, iErrorCode ));
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}

static void* Dhcpv6HandlingThread( void *arg )
{
    char ParamName[BUFLEN_256] = {0};
    char ParamValue[BUFLEN_256] = {0};

    //detach thread from caller stack
    pthread_detach(pthread_self());

    if( NULL == arg)
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        pthread_exit(NULL);
    }

    BOOL enable_client = *(BOOL *) arg;
    snprintf( ParamName, BUFLEN_256, DIBBLER_IPV6_CLIENT_ENABLE, 1 );
    if(enable_client)
        snprintf( ParamValue, BUFLEN_256, "%s", "true");
    else
        snprintf( ParamValue, BUFLEN_256, "%s", "false");

    SetDataModelParamValues( WAN_COMPONENT_NAME, COMPONENT_PATH_WANMANAGER, ParamName, ParamValue, ccsp_boolean, TRUE );

    CcspTraceInfo(("%s %d Successfully set %d value to %s data model \n", __FUNCTION__, __LINE__, enable_client, ParamName));

    //free memory.
    if (arg)
    {
        free (arg);
        arg = NULL;
    }

    pthread_exit(NULL);
}


int WanManager_Ipv6AddrUtil(char *ifname, Ipv6OperType opr, int preflft, int vallft)
{
    struct ifaddrs *ifap, *ifa;
    char addr[INET6_ADDRSTRLEN] = {0};
    char cmdLine[128] = {0};

    if (getifaddrs(&ifap) == -1)
    {
        return -1;
    }

    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (strncmp(ifa->ifa_name, ifname, strlen(ifname)))
            continue;
        if (ifa->ifa_addr->sa_family != AF_INET6)
            continue;
        getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), addr,
                    sizeof(addr), NULL, 0, NI_NUMERICHOST);
        if ((strncmp(addr, "fe", 2) != 0) && (strncmp(addr, "fd", 2) != 0))
        {
            switch (opr)
            {
            case DEL_ADDR:
            {
                memset(cmdLine, 0, sizeof(cmdLine));
                snprintf(cmdLine, sizeof(cmdLine), "ip -6 addr del %s/64 dev %s", addr, ifname);
                if (WanManager_DoSystemActionWithStatus("ip -6 addr del ADDR dev xxxx", cmdLine) != 0)
                    CcspTraceError(("failed to run cmd: %s", cmdLine));
                memset(cmdLine, 0, sizeof(cmdLine));
                snprintf(cmdLine, sizeof(cmdLine), "ip -6 route del %s/64 dev %s", addr, ifname);
                if (WanManager_DoSystemActionWithStatus("ip -6 route del ADDR dev xxxx", cmdLine) != 0)
                    CcspTraceError(("failed to run cmd: %s", cmdLine));
                break;
            }
            case SET_LFT:
            {
                memset(cmdLine, 0, sizeof(cmdLine));
                snprintf(cmdLine, sizeof(cmdLine), "ip -6 addr change %s dev brlan0 valid_lft %d preferred_lft %d ", addr, vallft, preflft);
                if (WanManager_DoSystemActionWithStatus("processDhcp6cStateChanged: ip -6 addr change L3IfName", (cmdLine)) != 0)
                    CcspTraceError(("failed to run cmd: %s", cmdLine));
                break;
            }
            }
        }
    }
    freeifaddrs(ifap);
    return 0;
}




ANSC_STATUS WanManager_StartDhcpv6Client(const char *pcInterfaceName, BOOL isPPP)
{
    char cmdLine[BUFLEN_128];
    bool enableClient = TRUE;

    CcspTraceInfo(("Enter WanManager_StartDhcpv6Client for  %s \n", DHCPV6_CLIENT_NAME));
    sprintf(cmdLine, "%s start", DHCPV6_CLIENT_NAME);
    system(cmdLine);
    CcspTraceInfo(("Started %s \n", cmdLine ));
    if(setDibblerClientEnable(&enableClient) == ANSC_STATUS_SUCCESS)
    {
        CcspTraceInfo(("setDibblerClientEnable is successful \n"));
    }
    else
    {
        CcspTraceInfo(("setDibblerClientEnable is failure \n"));
    }

    return ANSC_STATUS_SUCCESS;
}

/**
 * @brief This function will stop the DHCPV6 client(WAN side) in router
 *
 * @param boolDisconnect : This indicates whether this function called from disconnect context or not.
 *              TRUE (disconnect context) / FALSE (Non disconnect context)
 */
ANSC_STATUS WanManager_StopDhcpv6Client(BOOL boolDisconnect)
{
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;
    char cmdLine[BUFLEN_128];

    CcspTraceInfo(("Enter WanManager_StopDhcpv6Client for  %s \n", DHCPV6_CLIENT_NAME));
    sprintf(cmdLine, "killall %s", DHCPV6_CLIENT_NAME);
    system(cmdLine);

    CcspTraceInfo(("Exit WanManager_StopDhcpv6Client\n"));
    return ret;
}

uint32_t WanManager_StartDhcpv4Client(const char *intf, BOOL discover)
{
    uint32_t pid = 0;
    char cmdLine[BUFLEN_128];
    char exeBuff[1024] = {0};

    /**
     * In case of udhcpc, we are passing action handler program which will be invoked
     * and fill the required dhcpv4 configuration. This handler will pass the data back
     * to wanmanager. Get full path of the action handler executable and pass to udhcpc.
    */
    if (ANSC_STATUS_SUCCESS != GetPathToApp(DHCPV4_ACTION_HANDLER, exeBuff, sizeof(exeBuff) - 1))
    {
        CcspTraceError(("Could not find requested app [%s] in CPE , not passing -s option to udhcpc \n", DHCPV4_ACTION_HANDLER));
        snprintf(cmdLine, sizeof(cmdLine), "-f -i %s -p %s", (intf != NULL ? intf : ""),DHCPV4C_PID_FILE);
    }
    else
    {
        snprintf(cmdLine, sizeof(cmdLine), "-f -i %s -p %s -s %s", (intf != NULL ? intf : ""),DHCPV4C_PID_FILE, exeBuff);
    }

    pid = WanManager_DoStartApp(DHCPV4_CLIENT_NAME, cmdLine);

    return pid;
}

ANSC_STATUS WanManager_StopDhcpv4Client(BOOL sendReleaseAndExit)
{
    CcspTraceInfo(("Enter WanManager_StopDhcpv4Client for  %s \n", DHCPV4_CLIENT_NAME));
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;

    if (!sendReleaseAndExit)
    {
        WanManager_DoStopApp(DHCPV4_CLIENT_NAME);
    }
    else
    {
        CcspTraceInfo(("Sending release and exit  msg to %s \n", DHCPV4_CLIENT_NAME));
        if (WanManager_IsApplicationRunning(DHCPV4_CLIENT_NAME) == TRUE)
        {
            int pid = util_getPidByName(DHCPV4_CLIENT_NAME);
            CcspTraceInfo(("sending SIGUSR2 to [%s][pid=%d], this will let the %s to send release packet out and exit \n", DHCPV4_CLIENT_NAME));
            if (util_signalProcess(pid, SIGUSR2) != RETURN_OK)
            {
                WanManager_DoStopApp(DHCPV4_CLIENT_NAME);
            }
            else
            {
                /* Collect if any zombie process. */
                WanManager_DoCollectApp(DHCPV4_CLIENT_NAME);
            }
        }
    }

    return ret;
}

void WanUpdateDhcp6cProcessId(char *currentBaseIfName)
{
    INT           wanIndex = -1;
    int processId = 0;
    char cmdLine[BUFLEN_128];
    char out[BUFLEN_128] = {0};

    sprintf(cmdLine, "pidof %s", DHCPV6_CLIENT_NAME);
    _get_shell_output(cmdLine, out, sizeof(out));
    CcspTraceError(("%s Updating dibbler client pid %s\n", __func__, out));
    processId = atoi(out);

    WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceDataByName_locked(currentBaseIfName);
    if (pWanDmlIfaceData != NULL)
    {
        pWanDmlIfaceData->data.IP.Dhcp6cPid = processId;		
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }
    else
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__, currentBaseIfName));
        return;
    }
}

#ifdef FEATURE_MAPT
const char *nat44PostRoutingTable = "OUTBOUND_POSTROUTING";
char ipv6AddressString[BUFLEN_256] = " ";
#ifdef FEATURE_MAPT_DEBUG
void WanManager_UpdateMaptLogFile(Dhcp6cMAPTParametersMsgBody *dhcp6cMAPTMsgBody);
#endif // FEATURE_MAPT_DEBUG
static int WanManager_CalculateMAPTPsid(char *pdIPv6Prefix, int v6PrefixLen, int iapdPrefixLen, int v4PrefixLen, int *psidValue, int *ipv4IndexValue, int *psidLen);
static int WanManager_ConfigureIpv6Sysevents(char *pdIPv6Prefix, char *ipAddressString, int psidValue);

static unsigned WanManager_GetMAPTbits(unsigned value, int pos, int num)
{
    return (value >> (pos + 1 - num)) & ~(~0 << num);
}
/*
 *   Returns LAN IP Address in the form X.X.X.X/Y, e.g. 192.168.0.1/24
 */
static ANSC_STATUS WanManager_GetLANIPAddress(char *ipAddress, size_t length)
{
    char lan_ip_address[IP_ADDR_LENGTH] = {0};

    if (syscfg_get(NULL, SYSCFG_LAN_IP_ADDRESS, lan_ip_address, sizeof(lan_ip_address)) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("Failed to get LAN IP address \n"));
        return ANSC_STATUS_FAILURE;
    }

    char lan_subnet_mask[IP_ADDR_LENGTH] = {0};

    if (syscfg_get(NULL, SYSCFG_LAN_NET_MASK, lan_subnet_mask, sizeof(lan_subnet_mask)) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("Failed to get LAN subnet mask \n"));
        return ANSC_STATUS_FAILURE;
    }

    // count number of bits set in subnet mask.
    unsigned int lanSubnetMask = inet_network(lan_subnet_mask);
    unsigned int subnetCount = 0;
    while (lanSubnetMask)
    {
        subnetCount += lanSubnetMask & 1;
        lanSubnetMask = lanSubnetMask >> 1;
    }

    CcspTraceInfo(("LAN IP address/subnet mask = %s/%u \n", lan_ip_address, subnetCount));

    snprintf(ipAddress, length, "%s/%u", lan_ip_address, subnetCount);

    return ANSC_STATUS_SUCCESS;
}

int WanManager_ProcessMAPTConfiguration(Dhcp6cMAPTParametersMsgBody *dhcp6cMAPTMsgBody, const char *baseIf, const char *vlanIf)
{
    int ret = RETURN_OK;
    char cmdDMRConfig[BUFLEN_128 + BUFLEN_64];
    char cmdBMRConfig[BUFLEN_256];
    char cmdStartMAPT[BUFLEN_256];
    char cmdDisableMapFiltering[BUFLEN_64];
    int psidValue = 0;
    int ipv4IndexValue = 0;
    char ipAddressString[BUFLEN_32] = "";
    char ipLANAddressString[BUFLEN_32] = "";
    struct in_addr result;
    unsigned char ipAddressBytes[BUFLEN_4];
    unsigned int ipValue = 0;
    int psidLen = 0;
    char cmdConfigureMTUSize[BUFLEN_64] = "";
    char cmdEnableIpv4Traffic[BUFLEN_64] = "";
    MaptData_t maptInfo;

    /* RM16042: Since erouter0 is vlan interface on top of eth3, we need
       to first set the MTU size of eth3 to 1520 and then change MTU of erouter0.
       Otherwise we can't configure MTU as we are getting `Numerical result out of range` error.
       Configure eth3 MTU size to 1520.
    */
    snprintf(cmdConfigureMTUSize, sizeof(cmdConfigureMTUSize), "ip link set dev %s mtu %d ", baseIf, MTU_SIZE);

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:cmdConfigureMTUSize:%s", cmdConfigureMTUSize);
#endif

    if ((ret = WanManager_DoSystemActionWithStatus("ivictl", cmdConfigureMTUSize)) < RETURN_OK)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdConfigureMTUSize, ret));
        return ret;
    }

    /*  Configure erouter0 MTU size. */
    snprintf(cmdConfigureMTUSize, sizeof(cmdConfigureMTUSize), "ip link set dev %s mtu %d ", vlanIf, MTU_SIZE);

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:cmdConfigureMTUSize:%s", cmdConfigureMTUSize);
#endif

    if ((ret = WanManager_DoSystemActionWithStatus("ivictl", cmdConfigureMTUSize)) < RETURN_OK)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdConfigureMTUSize, ret));
        return ret;
    }

    snprintf(cmdDMRConfig, sizeof(cmdDMRConfig), "ivictl -r -d -P %s -T", dhcp6cMAPTMsgBody->brIPv6Prefix);
#ifdef FEATURE_MAPT_DEBUG
    WanManager_UpdateMaptLogFile(dhcp6cMAPTMsgBody);
    LOG_PRINT_MAPT("### ivictl commands - START ###");
    LOG_PRINT_MAPT("ivictl:DMR config:%s", cmdDMRConfig);
#endif

    if ((ret = WanManager_DoSystemActionWithStatus("ivictl", cmdDMRConfig)) < RETURN_OK)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdDMRConfig, ret));
        return ret;
    }
    snprintf(cmdBMRConfig, sizeof(cmdBMRConfig), "ivictl -r -p %s -P %s -z %d -R %d -T", dhcp6cMAPTMsgBody->ruleIPv4Prefix,
             dhcp6cMAPTMsgBody->ruleIPv6Prefix, dhcp6cMAPTMsgBody->psidOffset, dhcp6cMAPTMsgBody->ratio);

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:BMR config:%s", cmdBMRConfig);
#endif

    if ((ret = WanManager_DoSystemActionWithStatus("ivictl", cmdBMRConfig)) < RETURN_OK)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdBMRConfig, ret));
        return ret;
    }

    ret = WanManager_CalculateMAPTPsid(dhcp6cMAPTMsgBody->pdIPv6Prefix, dhcp6cMAPTMsgBody->v6Len, dhcp6cMAPTMsgBody->iapdPrefixLen,
                                          dhcp6cMAPTMsgBody->v4Len, &psidValue, &ipv4IndexValue, &psidLen);

    if (ret != RETURN_OK)
    {
        CcspTraceInfo(("Error in calculating MAPT PSID value"));
#ifdef FEATURE_MAPT_DEBUG
        LOG_PRINT_MAPT("Exiting MAPT configuration, MAPT will not be configured, as invalid dhcpc6c options found");
#endif
        CcspTraceNotice(("FEATURE_MAPT: MAP-T configuration failed\n"));
        return ret;
    }

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:PSID Value: %d, ipv4IndexValue: %d", psidValue, ipv4IndexValue);
#endif

    inet_pton(AF_INET, dhcp6cMAPTMsgBody->ruleIPv4Prefix, &(result));

    ipValue = htonl(result.s_addr);

    ipAddressBytes[0] = ipValue & 0xFF;
    ipAddressBytes[1] = (ipValue >> 8) & 0xFF;
    ipAddressBytes[2] = (ipValue >> 16) & 0xFF;
    ipAddressBytes[3] = (ipValue >> 24) & 0xFF;

    if (ipAddressBytes[0] + ipv4IndexValue < 255)
        ipAddressBytes[0] += ipv4IndexValue;
    else
        ipAddressBytes[0] = (ipAddressBytes[0] + ipv4IndexValue) - 255;

    //store new ipv4 address
    //dhcp6cMAPTMsgBody->ipv4Address = ( ((ipAddressBytes[3] & 0xFF) << 24) | ((ipAddressBytes[2] & 0xFF) << 16) | ((ipAddressBytes[1] & 0xFF) << 8) | (ipAddressBytes[0] & 0xFF) );
    snprintf(ipAddressString, sizeof(ipAddressString), "%d.%d.%d.%d", ipAddressBytes[3], ipAddressBytes[2], ipAddressBytes[1], ipAddressBytes[0]);

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ipAddressString:%s", ipAddressString);
#endif

    //get LAN IP Address
    if ((ret = WanManager_GetLANIPAddress(ipLANAddressString, sizeof(ipLANAddressString))) != RETURN_OK)
    {
        CcspTraceError(("Could not get LAN IP Address"));
        return ret;
    }
    //The sharing ratio cannot be zero, a value of zero means the sharing ratio is 1
    if (dhcp6cMAPTMsgBody->ratio == 0)
        dhcp6cMAPTMsgBody->ratio = 1;

    if ((ret = WanManager_ConfigureIpv6Sysevents(dhcp6cMAPTMsgBody->pdIPv6Prefix, ipAddressString, psidValue)) != RETURN_OK)
    {
        CcspTraceError(("Failed to configure ipv6Tablerules"));
        return ret;
    }
    // create MAP-T command string
    snprintf(cmdStartMAPT, sizeof(cmdStartMAPT), "ivictl -s -i %s -I %s -H -a %s -A %s/%d -P %s -z %d -R %d -T -o %d", ETH_BRIDGE_NAME, vlanIf,
             ipLANAddressString, ipAddressString, dhcp6cMAPTMsgBody->v4Len, dhcp6cMAPTMsgBody->ruleIPv6Prefix, dhcp6cMAPTMsgBody->psidOffset,
             dhcp6cMAPTMsgBody->ratio, psidValue);

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:startMapt Command:%s", cmdStartMAPT);
#endif
    ret = WanManager_DoSystemActionWithStatus("ivictl", cmdStartMAPT);

    //Added check since the 'system' command is not returning the expected value
    if (ret == IVICTL_COMMAND_ERROR)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdStartMAPT, ret));
        return ret;
    }

    snprintf(cmdDisableMapFiltering, sizeof(cmdDisableMapFiltering), "echo 0 > /proc/sys/net/ipv4/conf/%s/rp_filter", vlanIf);

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:Disable RP Filtering:%s", cmdDisableMapFiltering);
#endif

    if ((ret = WanManager_DoSystemActionWithStatus("ivictl", cmdDisableMapFiltering)) < RETURN_OK)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdDisableMapFiltering, ret));
        return ret;
    }

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:Calling configureDefaultIpTableRules()... ");
#endif

    /**
     * Firewall rules are changed to utopia firewall
     * To update the firewall rules, required the MAPT specific values, so
     * updated sysvents and restart firewall.
     * First set MAPT_CONFIG_FLAG to set, so firewall can add rules for the MAPT configuration. */
    memset(&maptInfo, 0, sizeof(maptInfo));
    strncpy(maptInfo.maptConfigFlag, SET, sizeof(maptInfo.maptConfigFlag));
    strncpy(maptInfo.ruleIpAddressString, dhcp6cMAPTMsgBody->ruleIPv4Prefix, sizeof(maptInfo.ruleIpAddressString));
    strncpy(maptInfo.ruleIpv6AddressString, dhcp6cMAPTMsgBody->ruleIPv6Prefix, sizeof(maptInfo.ruleIpv6AddressString));
    strncpy(maptInfo.brIpv6PrefixString, dhcp6cMAPTMsgBody->brIPv6Prefix, sizeof(maptInfo.brIpv6PrefixString));
    strncpy(maptInfo.ipAddressString, ipAddressString, sizeof(maptInfo.ipAddressString));
    strncpy(maptInfo.ipv6AddressString, ipv6AddressString, sizeof(maptInfo.ipv6AddressString));
    maptInfo.psidValue = psidValue;
    maptInfo.psidLen = psidLen;
    maptInfo.ratio = dhcp6cMAPTMsgBody->ratio;
    maptInfo.psidOffset = dhcp6cMAPTMsgBody->psidOffset;
    maptInfo.eaLen = dhcp6cMAPTMsgBody->eaLen;
    pthread_mutex_lock(&gmWanDataMutex);
    maptInfo.maptAssigned = gWanData.ipv6Data.maptAssigned;
    maptInfo.mapeAssigned = gWanData.ipv6Data.mapeAssigned;
    maptInfo.isFMR = gWanData.ipv6Data.isFMR;
    pthread_mutex_unlock(&gmWanDataMutex);

    if ((ret = maptInfo_set(&maptInfo)) != RETURN_OK)
    {
        CcspTraceError(("Failed to set sysevents for MAPT feature to set firewall rules \n"));
        return ret;
    }

    /* Restart firewall to reflect the changes. Do a sysevent to notify firewall to
     * restart and update rules. */
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIREWALL_RESTART, NULL, 0);
    snprintf(cmdEnableIpv4Traffic, sizeof(cmdEnableIpv4Traffic), "ip ro rep default dev %s", vlanIf);
#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:cmdEnableIpv4Traffic:%s", cmdEnableIpv4Traffic);
#endif
    if ((ret = WanManager_DoSystemActionWithStatus("ivictl", cmdEnableIpv4Traffic)) < RETURN_OK)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdEnableIpv4Traffic, ret));
        return ret;
    }

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("### ivictl commands - ENDS  ###");
#endif
    CcspTraceNotice(("FEATURE_MAPT: MAP-T configuration done\n"));
    return RETURN_OK;
}

static int WanManager_CalculateMAPTPsid(char *pdIPv6Prefix, int v6PrefixLen, int iapdPrefixLen, int v4PrefixLen, int *psidValue, int *ipv4IndexValue, int *psidLen)
{
    int ret = RETURN_OK;
    int len, startPdOffset, endPdOffset;
    int psidByte;
    struct in6_addr in6Addr;
    int ipv4BitIndexLen;
    int psidBitIndexLen;

    startPdOffset = v6PrefixLen;
    endPdOffset = iapdPrefixLen;
    if (endPdOffset <= startPdOffset)
    {
#ifdef FEATURE_MAPT_DEBUG
        LOG_PRINT_MAPT("Error: Invalid dhcpc6 option,endPdOffset(%d) has a value which <= startPdOffset(%d)", endPdOffset, startPdOffset);
#endif
        CcspTraceError(("Error: Invalid dhcpc6 option,endPdOffset(%d) has a value which <= startPdOffset(%d)", endPdOffset, startPdOffset));
        ret = RETURN_ERR;
        return ret;
    }

    len = endPdOffset - startPdOffset;
    ipv4BitIndexLen = 32 - v4PrefixLen;
    psidBitIndexLen = len - ipv4BitIndexLen;

    *psidLen = psidBitIndexLen;

    if (inet_pton(AF_INET6, pdIPv6Prefix, &in6Addr) <= 0)
    {
        CcspTraceError(("Invalid ipv6 address=%s", pdIPv6Prefix));
    }

    if (len > 8)
    {
        psidByte = (in6Addr.s6_addr[v6PrefixLen / 8] << 8) | (in6Addr.s6_addr[v6PrefixLen / 8 + 1]);
        *ipv4IndexValue = WanManager_GetMAPTbits(psidByte, 15, ipv4BitIndexLen);
        *psidValue = WanManager_GetMAPTbits(psidByte, 15 - ipv4BitIndexLen, psidBitIndexLen);
    }
    else
    {
        psidByte = in6Addr.s6_addr[v6PrefixLen / 8];
        *ipv4IndexValue = WanManager_GetMAPTbits(psidByte, 7, ipv4BitIndexLen);
        *psidValue = WanManager_GetMAPTbits(psidByte, 7 - ipv4BitIndexLen, psidBitIndexLen);
    }
#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:psidByte:%d", psidByte);
#endif
    return ret;
}

static int WanManager_ConfigureIpv6Sysevents(char *pdIPv6Prefix, char *ipAddressString, int psidValue)
{
    int ret = RETURN_OK;
    struct in6_addr in6Addr;
    unsigned char ipAddressBytes[BUFLEN_4];
    struct in_addr result;
    unsigned int ipValue = 0;

    inet_pton(AF_INET, ipAddressString, &(result));

    ipValue = htonl(result.s_addr);

    ipAddressBytes[0] = ipValue & 0xFF;
    ipAddressBytes[1] = (ipValue >> 8) & 0xFF;
    ipAddressBytes[2] = (ipValue >> 16) & 0xFF;
    ipAddressBytes[3] = (ipValue >> 24) & 0xFF;

    if (inet_pton(AF_INET6, pdIPv6Prefix, &in6Addr) <= 0)
    {
        CcspTraceError(("Invalid ipv6 address=%s", pdIPv6Prefix));
        ret = RETURN_ERR;
        return ret;
    }

    snprintf(ipv6AddressString, sizeof(ipv6AddressString), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", in6Addr.s6_addr[0], in6Addr.s6_addr[1], in6Addr.s6_addr[2], in6Addr.s6_addr[3], in6Addr.s6_addr[4], in6Addr.s6_addr[5], in6Addr.s6_addr[6], in6Addr.s6_addr[7], 0x0, 0x0, ipAddressBytes[3], ipAddressBytes[2], ipAddressBytes[1], ipAddressBytes[0], 0x0, psidValue);

    return ret;
}

int WanManager_ResetMAPTConfiguration(const char *baseIf, const char *vlanIf)
{
    char cmdConfigureMTUSize[BUFLEN_64] = "";
    int ret = RETURN_OK;

    /* RM16042: Since we have configures MTU size to 1520 for the MAPT functionality,
     * we need to reconfigure it back to 1500 when we reset from MAPT configuration.
     * Reconfigure eth3 mtu size to 1500. */
    snprintf(cmdConfigureMTUSize, sizeof(cmdConfigureMTUSize), "ip link set dev %s mtu %d ", baseIf, MTU_DEFAULT_SIZE);

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:cmdConfigureMTUSize:%s", cmdConfigureMTUSize);
#endif

    if ((ret = WanManager_DoSystemActionWithStatus("ivictl", cmdConfigureMTUSize)) < RETURN_OK)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdConfigureMTUSize, ret));
        return ret;
    }

    /*  ReConfigure erouter0 MTU size to 1500 */
    snprintf(cmdConfigureMTUSize, sizeof(cmdConfigureMTUSize), "ip link set dev %s mtu %d ", vlanIf, MTU_DEFAULT_SIZE);

#ifdef FEATURE_MAPT_DEBUG
    LOG_PRINT_MAPT("ivictl:cmdConfigureMTUSize:%s", cmdConfigureMTUSize);
#endif

    if ((ret = WanManager_DoSystemActionWithStatus("ivictl", cmdConfigureMTUSize)) < RETURN_OK)
    {
        CcspTraceError(("Failed to run: %s:%d", cmdConfigureMTUSize, ret));
        return ret;
    }

    /* Reset MAPT configuration.
     * All the firewall rules should remove once this called.
     * To do this we have reset the value for sysevent field
     * `mapt_configure_flag` and restart the firewall. */
    //Reset MAP sysevent parameters
    maptInfo_reset();
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIREWALL_RESTART, NULL, 0);
    CcspTraceNotice(("FEATURE_MAPT: MAP-T configuration cleared\n"));
    return RETURN_OK;
}

#ifdef FEATURE_MAPT_DEBUG
void logPrintMapt(char *fmt,...)
{
    static FILE *fpMaptLogFile;
    static char strMaptLogFileName[BUFLEN_32] = "/tmp/log_mapt.txt";
    va_list list;
    char *p, *r;
    int e;
    time_t ctime;
    struct tm *info;

    fpMaptLogFile = fopen(strMaptLogFileName, "a");

    time(&ctime); /* Get current time */
    info = localtime(&ctime);

    fprintf(fpMaptLogFile,"[%02d:%02d:%02d] ",
        info->tm_hour,info->tm_min,info->tm_sec);

    va_start(list, fmt);

    for (p = fmt; *p; ++p)
    {
        if (*p != '%')
        {
            fputc(*p, fpMaptLogFile);
        }
        else
        {
            switch (*++p)
            {

            case 's':
            {
                r = va_arg(list, char *);

                fprintf(fpMaptLogFile, "%s", r);
                continue;
            }

            case 'd':
            {
                e = va_arg(list, int);

                fprintf(fpMaptLogFile, "%d", e);
                continue;
            }

            default:
                fputc(*p, fpMaptLogFile);
            }
        }
    }
    va_end(list);
    fputc('\n', fpMaptLogFile);
    fclose(fpMaptLogFile);
}

void WanManager_UpdateMaptLogFile(Dhcp6cMAPTParametersMsgBody *dhcp6cMAPTMsgBody)
{

    LOG_PRINT_MAPT("############MAP-T Options DHCP - START#######################");
    LOG_PRINT_MAPT("ruleIPv6Prefix:%s", dhcp6cMAPTMsgBody->ruleIPv6Prefix);
    LOG_PRINT_MAPT("ruleIPv4Prefix:%s", dhcp6cMAPTMsgBody->ruleIPv4Prefix);
    LOG_PRINT_MAPT("brIPv6Prefix:%s", dhcp6cMAPTMsgBody->brIPv6Prefix);
    LOG_PRINT_MAPT("pdIPv6Prefix:%s", dhcp6cMAPTMsgBody->pdIPv6Prefix);
    LOG_PRINT_MAPT("iapdPrefixLen:%d", dhcp6cMAPTMsgBody->iapdPrefixLen);
    LOG_PRINT_MAPT("psidOffset:%d", dhcp6cMAPTMsgBody->psidOffset);
    LOG_PRINT_MAPT("psidLen:%d", dhcp6cMAPTMsgBody->psidLen);
    LOG_PRINT_MAPT("psid:%d", dhcp6cMAPTMsgBody->psid);
    LOG_PRINT_MAPT("ratio:%d", dhcp6cMAPTMsgBody->ratio);
    LOG_PRINT_MAPT("v4Len:%d", dhcp6cMAPTMsgBody->v4Len);
    LOG_PRINT_MAPT("v6Len:%d", dhcp6cMAPTMsgBody->v6Len);
    LOG_PRINT_MAPT("eaLen:%d", dhcp6cMAPTMsgBody->eaLen);
    LOG_PRINT_MAPT("############MAP-T Options DHCP - END#########################");

    CcspTraceInfo(("SKYWANMGR:MAP-T_OPTIONS:%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d",
                 dhcp6cMAPTMsgBody->ruleIPv6Prefix,
                 dhcp6cMAPTMsgBody->ruleIPv4Prefix,
                 dhcp6cMAPTMsgBody->brIPv6Prefix,
                 dhcp6cMAPTMsgBody->pdIPv6Prefix,
                 dhcp6cMAPTMsgBody->iapdPrefixLen,
                 dhcp6cMAPTMsgBody->psidOffset,
                 dhcp6cMAPTMsgBody->psidLen,
                 dhcp6cMAPTMsgBody->psid,
                 dhcp6cMAPTMsgBody->ratio,
                 dhcp6cMAPTMsgBody->v4Len,
                 dhcp6cMAPTMsgBody->v6Len,
                 dhcp6cMAPTMsgBody->eaLen));
}
#endif /* FEATURE_MAPT_DEBUG */

#endif /* FEATURE_MAPT */

static int IsValidDnsServer(int32_t af, const char *nameServer)
{
    if (nameServer == NULL)
    {
        CcspTraceError(("%s %d - invalid argument \n",__FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    if(af == AF_INET)
    {
        if(!(IsValidIpv4Address(nameServer)))
        {
            CcspTraceError(("%s %d - invalid nameserver: %s \n",__FUNCTION__,__LINE__, nameServer));
            return RETURN_ERR;
        }
        if(IsZeroIpvxAddress(AF_SELECT_IPV4,nameServer))
        {
            CcspTraceError(("%s %d - invalid nameserver: %s \n",__FUNCTION__,__LINE__, nameServer));
            return RETURN_ERR;
        }
        if(!(strncmp(BROADCAST_IP, nameServer, strlen(BROADCAST_IP))))
        {
            CcspTraceError(("%s %d - invalid nameserver: %s \n",__FUNCTION__,__LINE__, nameServer));
            return RETURN_ERR;
        }
        if(!(strncmp(LINKLOCAL_RANGE, nameServer, strlen(LINKLOCAL_RANGE))))
        {
            CcspTraceError(("%s %d - invalid nameserver: %s \n",__FUNCTION__,__LINE__, nameServer));
            return RETURN_ERR;
        }
        if(!(strncmp(LOOPBACK_RANGE, nameServer, strlen(LOOPBACK_RANGE))))
        {
            CcspTraceError(("%s %d - invalid nameserver: %s \n",__FUNCTION__,__LINE__, nameServer));
            return RETURN_ERR;
        }
    }
    else
    {
        if(!(IsValidIpAddress(af, nameServer)))
        {
            CcspTraceError(("%s %d - invalid nameserver: %s \n",__FUNCTION__,__LINE__, nameServer));
            return RETURN_ERR;
        }
        if(IsZeroIpvxAddress(AF_SELECT_IPV6, nameServer))
        {
            CcspTraceError(("%s %d - invalid nameserver: %s \n",__FUNCTION__,__LINE__, nameServer));
            return RETURN_ERR;
        }
    }
    return RETURN_OK;
}

int WanManager_CreateResolvCfg(const DnsData_t *dnsInfo)
{
   FILE *fp = NULL;
   char cmd[BUFLEN_128]={0};
   int ret = RETURN_OK;
   bool valid_dns = FALSE;
   if(NULL == dnsInfo)
   {
        return ANSC_STATUS_FAILURE;
   }
   CcspTraceInfo(("%s %d -adding nameservers: %s %s %s %s\n", __FUNCTION__,__LINE__,dnsInfo->dns_ipv4_1, dnsInfo->dns_ipv4_2, dnsInfo->dns_ipv6_1, dnsInfo->dns_ipv6_2));
   /* sets nameserver entries to resolv.conf file */
   if((fp = fopen(RESOLV_CONF_FILE, "w+")) == NULL)
   {
        CcspTraceInfo(("%s %d - Open %s error!\n", __FUNCTION__, __LINE__, RESOLV_CONF_FILE));
        ret = RETURN_ERR;
   }
   else
   {
        if(RETURN_OK == IsValidDnsServer(AF_INET, dnsInfo->dns_ipv4_1))
        {
            fprintf(fp, "nameserver %s\n", dnsInfo->dns_ipv4_1);
            valid_dns = TRUE;
        }
        if(RETURN_OK == IsValidDnsServer(AF_INET, dnsInfo->dns_ipv4_2))
        {
            fprintf(fp, "nameserver %s\n", dnsInfo->dns_ipv4_2);
            valid_dns = TRUE;
        }
        if(RETURN_OK == IsValidDnsServer(AF_INET6, dnsInfo->dns_ipv6_1))
        {
            fprintf(fp, "nameserver %s\n", dnsInfo->dns_ipv6_1);
            valid_dns = TRUE;
        }
        if(RETURN_OK == IsValidDnsServer(AF_INET6, dnsInfo->dns_ipv6_2))
        {
            fprintf(fp, "nameserver %s\n", dnsInfo->dns_ipv6_2);
            valid_dns = TRUE;
        }
        if (valid_dns == FALSE)
        {
            CcspTraceInfo(("%s %d - No valid nameserver is available, adding loopback address for nameserver\n", __FUNCTION__,__LINE__));
            fprintf(fp, "nameserver %s \n", LOOPBACK);
        }
        fclose(fp);
        CcspTraceInfo(("%s %d - Active domainname servers set!\n", __FUNCTION__,__LINE__));
   }
   return ret;
}

int WanManager_GetBCastFromIpSubnetMask(const char* inIpStr, const char* inSubnetMaskStr, char *outBcastStr)
{
   struct in_addr ip;
   struct in_addr subnetMask;
   struct in_addr bCast;
   int ret = RETURN_OK;

   if (inIpStr == NULL || inSubnetMaskStr == NULL || outBcastStr == NULL)
   {
      return RETURN_ERR;
   }

   ip.s_addr = inet_addr(inIpStr);
   subnetMask.s_addr = inet_addr(inSubnetMaskStr);
   bCast.s_addr = ip.s_addr | ~subnetMask.s_addr;
   strcpy(outBcastStr, inet_ntoa(bCast));

   return ret;
}

int WanManager_AddDefaultGatewayRoute(const WANMGR_IPV4_DATA* pIpv4Info)
{
   char cmd[BUFLEN_128]={0};
   int ret = RETURN_OK;
   FILE *fp = NULL;

   /* delete default gateway first before add  */
   snprintf(cmd, sizeof(cmd), "route del default 2>/dev/null");
   WanManager_DoSystemAction("SetUpDefaultSystemGateway:", cmd);

   /* Sets default gateway route entry */
   /* For IPoE, always use gw IP address. */
   if (IsValidIpv4Address(pIpv4Info->gateway) && !(IsZeroIpvxAddress(AF_SELECT_IPV4, pIpv4Info->gateway)))
   {
       snprintf(cmd, sizeof(cmd), "route add default gw %s dev %s", pIpv4Info->gateway, pIpv4Info->ifname);
       WanManager_DoSystemAction("SetUpDefaultSystemGateway:", cmd);
       CcspTraceInfo(("%s %d - The default gateway route entries set!\n",__FUNCTION__,__LINE__));
   }

   return ret;
}

static BOOL IsValidIpv4Address(const char* input)
{
   BOOL ret = TRUE;
   char *pToken = NULL;
   char *pLast = NULL;
   char buf[BUFLEN_16];
   uint32_t i, num;

   if (input == NULL || strlen(input) < 7 || strlen(input) > 15)
   {
      return FALSE;
   }

   /* need to copy since strtok_r updates string */
   strcpy(buf, input);

   /* IP address has the following format
      xxx.xxx.xxx.xxx where x is decimal number */
   pToken = strtok_r(buf, ".", &pLast);
   num = strtoul(pToken, NULL, 10);
   if (num > 255)
   {
      ret = FALSE;
   }
   else
   {
      for ( i = 0; i < 3; i++ )
      {
         pToken = strtok_r(NULL, ".", &pLast);

         num = strtoul(pToken, NULL, 10);
         if (num > 255)
         {
            ret = FALSE;
            break;
         }
      }
   }

   return ret;
}

static BOOL IsZeroIpvxAddress(uint32_t ipvx, const char *addr)
{
   if (IS_EMPTY_STR(addr))
   {
      return TRUE;
   }

   /*
    * Technically, the ::/0 is not an all zero address, but it is used by our
    * routing code to specify the default route.  See Wikipedia IPv6_address
    */
   if (((ipvx & AF_SELECT_IPV4) && !strcmp(addr, "0.0.0.0")) ||
       ((ipvx & AF_SELECT_IPV6) &&
           (!strcmp(addr, "0:0:0:0:0:0:0:0") ||
            !strcmp(addr, "::") ||
            !strcmp(addr, "::/128") ||
            !strcmp(addr, "::/0")))  )
   {
      return TRUE;
   }

   return FALSE;
}

static BOOL IsValidIpAddress(int32_t af, const char* address)
{
   if ( IS_EMPTY_STRING(address) ) return FALSE;
   if (af == AF_INET6)
   {
      struct in6_addr in6Addr;
      uint32_t plen;
      char   addr[IP_ADDR_LENGTH];

      if (ParsePrefixAddress(address, addr, &plen) != ANSC_STATUS_SUCCESS)
      {
         CcspTraceInfo(("Invalid ipv6 address=%s", address));
         return FALSE;
      }

      if (inet_pton(AF_INET6, addr, &in6Addr) <= 0)
      {
         CcspTraceInfo(("Invalid ipv6 address=%s", address));
         return FALSE;
      }

      return TRUE;
   }
   else
   {
      if (af == AF_INET)
      {
         return IsValidIpv4Address(address);
      }
      else
      {
         return FALSE;
      }
   }
}

static int ParsePrefixAddress(const char *prefixAddr, char *address, uint32_t *plen)
{
   int ret = RETURN_OK;
   char *tmpBuf;
   char *separator;
   uint32_t len;

   if (prefixAddr == NULL || address == NULL || plen == NULL)
   {
      return RETURN_ERR;
   }

   *address = '\0';
   *plen    = 128;

   len = strlen(prefixAddr);

   if ((tmpBuf = malloc(len+1)) == NULL)
   {
      CcspTraceError(("%s %d - alloc of %d bytes failed",__FUNCTION__,__LINE__, len));
      ret = RETURN_ERR;
   }
   else
   {
      sprintf(tmpBuf, "%s", prefixAddr);
      separator = strchr(tmpBuf, '/');
      if (separator != NULL)
      {
         /* break the string into two strings */
         *separator = 0;
         separator++;
         while ((isspace(*separator)) && (*separator != 0))
         {
            /* skip white space after comma */
            separator++;
         }

         *plen = atoi(separator);
      }

      if (strlen(tmpBuf) < BUFLEN_40 && *plen <= 128)
      {
         strcpy(address, tmpBuf);
      }
      else
      {
         ret = RETURN_ERR;
      }
      free(tmpBuf);
   }

   return ret;

}

static ANSC_STATUS getIfAddr6(const char *ifname , uint32_t addrIdx,
                      char *ipAddr, uint32_t *ifIndex, uint32_t *prefixLen, uint32_t *scope, uint32_t *ifaFlags)
{
    int   ret = ANSC_STATUS_FAILURE;
    FILE     *fp;
    uint32_t   count = 0;
    char     line[BUFLEN_64];

    *ipAddr = '\0';

    if ((fp = fopen("/proc/net/if_inet6", "r")) == NULL)
    {
        CcspTraceInfo(("failed to open /proc/net/if_inet6"));
        return ANSC_STATUS_FAILURE;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        /* remove the carriage return char */
        line[strlen(line)-1] = '\0';

        if (strstr(line, ifname) != NULL)
        {
            char *addr, *ifidx, *plen, *scp, *flags, *devname;
            char *nextToken = NULL;

            /* the first token in the line is the ip address */
            addr = strtok_r(line, " ", &nextToken);

            /* the second token is the Netlink device number (interface index) in hexadecimal */
            ifidx = strtok_r(NULL, " ", &nextToken);
            if (ifidx == NULL)
            {
                CcspTraceInfo(("Invalid /proc/net/if_inet6 line"));
                ret = ANSC_STATUS_FAILURE;
                break;
            }

            /* the third token is the Prefix length in hexadecimal */
            plen = strtok_r(NULL, " ", &nextToken);
            if (plen == NULL)
            {
                CcspTraceInfo(("Invalid /proc/net/if_inet6 line"));
                ret = ANSC_STATUS_FAILURE;
                break;
            }

            /* the forth token is the Scope value */
            scp = strtok_r(NULL, " ", &nextToken);
            if (scp == NULL)
            {
                CcspTraceInfo(("Invalid /proc/net/if_inet6 line"));
                ret = ANSC_STATUS_FAILURE;
                break;
            }

            /* the fifth token is the ifa flags */
            flags = strtok_r(NULL, " ", &nextToken);
            if (flags == NULL)
            {
                CcspTraceInfo(("Invalid /proc/net/if_inet6 line"));
                ret = ANSC_STATUS_FAILURE;
                break;
            }

            /* the sixth token is the device name */
            devname = strtok_r(NULL, " ", &nextToken);
            if (devname == NULL)
            {
                CcspTraceInfo(("Invalid /proc/net/if_inet6 line"));
                ret = ANSC_STATUS_FAILURE;
                break;
            }
            else
            {
                if (strcmp(devname, ifname) != 0)
                {
                    continue;
                }
                else if (count == addrIdx)
                {
                    int32_t   i;
                    char     *p1, *p2;

                    *ifIndex   = strtoul(ifidx, NULL, 16);
                    *prefixLen = strtoul(plen, NULL, 16);
                    *scope     = strtoul(scp, NULL, 16);
                    *ifaFlags  = strtoul(flags, NULL, 16);

                    /* insert a colon every 4 digits in the address string */
                    p2 = ipAddr;
                    for (i = 0, p1 = addr; *p1 != '\0'; i++)
                    {
                        if (i == 4)
                        {
                            i = 0;
                            *p2++ = ':';
                        }
                        *p2++ = *p1++;
                    }
                    *p2 = '\0';

                    ret = ANSC_STATUS_SUCCESS;
                    break;   /* done */
                }
                else
                {
                    count++;
                }
            }
        }
    }  /* while */

    fclose(fp);

    return ret;

}  /* End of getIfAddr6() */

ANSC_STATUS WanManager_getGloballyUniqueIfAddr6(const char *ifname, char *ipAddr, uint32_t *prefixLen)
{
   uint32_t addrIdx=0;
   uint32_t netlinkIndex=0;
   uint32_t scope=0;
   uint32_t ifaflags=0;
   int ret= ANSC_STATUS_SUCCESS;

   while (ANSC_STATUS_SUCCESS == ret)
   {
      ret = getIfAddr6(ifname, addrIdx, ipAddr, &netlinkIndex,
                              prefixLen, &scope, &ifaflags);
      if ((ANSC_STATUS_SUCCESS == ret) && (0 == scope))  // found it
         return ANSC_STATUS_SUCCESS;

      addrIdx++;
   }

   return ANSC_STATUS_FAILURE;
}


#ifdef FEATURE_802_1P_COS_MARKING

ANSC_HANDLE WanManager_AddIfaceMarking(DML_WAN_IFACE* pWanDmlIface, ULONG* pInsNumber)
{
    DATAMODEL_MARKING*              pDmlMarking     = (DATAMODEL_MARKING*) &(pWanDmlIface->Marking);
    DML_MARKING*                    p_Marking       = NULL;
    CONTEXT_MARKING_LINK_OBJECT*    pMarkingCxtLink = NULL;

    //Verify limit of the marking table
    if( WAN_IF_MARKING_MAX_LIMIT < pDmlMarking->ulNextInstanceNumber )
    {
        CcspTraceError(("%s %d - Failed to add Marking entry due to maximum limit(%d)\n",__FUNCTION__,__LINE__,WAN_IF_MARKING_MAX_LIMIT));
        return NULL;
    }

    p_Marking       = (DML_MARKING*)AnscAllocateMemory(sizeof(DML_MARKING));
    pMarkingCxtLink = (CONTEXT_MARKING_LINK_OBJECT*)AnscAllocateMemory(sizeof(CONTEXT_MARKING_LINK_OBJECT));
    if((p_Marking == NULL) || (pMarkingCxtLink == NULL))
    {
        if( NULL != pMarkingCxtLink )
        {
          AnscFreeMemory(pMarkingCxtLink);
          pMarkingCxtLink = NULL;
        }

        if( NULL != p_Marking )
        {
          AnscFreeMemory(p_Marking);
          p_Marking = NULL;
        }
        return NULL;
    }


    /* now we have this link content */
    pMarkingCxtLink->hContext = (ANSC_HANDLE)p_Marking;
    pMarkingCxtLink->bNew     = TRUE;

    pMarkingCxtLink->InstanceNumber  = pDmlMarking->ulNextInstanceNumber;
    *pInsNumber                      = pDmlMarking->ulNextInstanceNumber;

    //Assign actual instance number
    p_Marking->InstanceNumber = pDmlMarking->ulNextInstanceNumber;

    pDmlMarking->ulNextInstanceNumber++;

    //Assign WAN interface instance for reference
    p_Marking->ulWANIfInstanceNumber = pWanDmlIface->uiInstanceNumber;

    //Initialise all marking members
    memset( p_Marking->Alias, 0, sizeof( p_Marking->Alias ) );
    p_Marking->SKBPort = 0;
    p_Marking->SKBMark = 0;
    p_Marking->EthernetPriorityMark = -1;

    SListPushMarkingEntryByInsNum(&pDmlMarking->MarkingList, (PCONTEXT_LINK_OBJECT)pMarkingCxtLink);

    return (ANSC_HANDLE)pMarkingCxtLink;
}


#endif /* * FEATURE_802_1P_COS_MARKING */

ANSC_STATUS WanManager_CreatePPPSession(DML_WAN_IFACE* pInterface)
{
    pthread_t pppThreadId;
    INT iErrorCode;
    char wan_iface_name[10] = {0};

    syscfg_init();
    /* Generate client-duid file for dibbler */
    generate_client_duid_conf();

    /* Remove erouter0 dummy wan bridge if exists */
    deleteDummyWanBridgeIfExist();
    if (pInterface->PPP.LinkType == WAN_IFACE_PPP_LINK_TYPE_PPPoA)
    {
        strncpy(wan_iface_name, BASE_IFNAME_PPPoA, strlen(BASE_IFNAME_PPPoA));
    }
    else if (pInterface->PPP.LinkType == WAN_IFACE_PPP_LINK_TYPE_PPPoE)
    {
        strncpy(wan_iface_name, BASE_IFNAME_PPPoE, strlen(BASE_IFNAME_PPPoE));
    }
    else
    {
        strncpy(wan_iface_name, DEFAULT_IFNAME, strlen(DEFAULT_IFNAME));
    }    
    if (syscfg_set(NULL, SYSCFG_WAN_INTERFACE_NAME, wan_iface_name) != 0)
    {
        CcspTraceError(("%s %d - syscfg_set failed to set Interafce=%s \n", __FUNCTION__, __LINE__, wan_iface_name ));
    }else{
        CcspTraceInfo(("%s %d - syscfg_set successfully to set Interafce=%s \n", __FUNCTION__, __LINE__, wan_iface_name ));
    }
    syscfg_commit();
    iErrorCode = pthread_create( &pppThreadId, NULL, &DmlHandlePPPCreateRequestThread, (void*)pInterface );
    if( 0 != iErrorCode )
    {
        CcspTraceInfo(("%s %d - Failed to start VLAN refresh thread EC:%d\n", __FUNCTION__, __LINE__, iErrorCode ));
        return ANSC_STATUS_FAILURE;
    }
    return ANSC_STATUS_SUCCESS;
}

static void* DmlHandlePPPCreateRequestThread( void *arg )
{
    char acSetParamName[DATAMODEL_PARAM_LENGTH] = {0};
    char acSetParamValue[DATAMODEL_PARAM_LENGTH] = {0};
    char adslPassword[DATAMODEL_PARAM_LENGTH] = {0};
    char adslUserName[DATAMODEL_PARAM_LENGTH] = {0};
    INT  iPPPInstance = -1;

    DML_WAN_IFACE* pInterface = (char *) arg;

    if( NULL == pInterface )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    pthread_detach(pthread_self());

    //Create PPP Interface
    if( -1 == iPPPInstance )
    {
        char  acTableName[ 128 ] = { 0 };
        INT   iNewTableInstance  = -1;

        sprintf( acTableName, "%s", PPP_INTERFACE_TABLE );
        if ( CCSP_SUCCESS != CcspBaseIf_AddTblRow (
             bus_handle,
             PPPMGR_COMPONENT_NAME,
             PPPMGR_DBUS_PATH,
             0,  /* session id */
             acTableName,
             &iNewTableInstance
           ) )
       {
            CcspTraceError(("%s Failed to add table %s\n", __FUNCTION__,acTableName));
            return ANSC_STATUS_FAILURE;
       }

       //Assign new instance
       iPPPInstance = iNewTableInstance;
    }

    CcspTraceInfo(("%s %d PPP Interface Instance:%d\n",__FUNCTION__, __LINE__, iPPPInstance));

    //Set Lower Layer
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_LOWERLAYERS, iPPPInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", pInterface->Phy.Path );
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, FALSE );

    //Set Alias
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_ALIAS, iPPPInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", pInterface->Wan.Name );
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, FALSE );

    CcspTraceError(("%s Going to call GetAdslUsernameAndPassword \n", __FUNCTION__ ));

#ifdef _HUB4_PRODUCT_REQ_
    if (GetAdslUsernameAndPassword(adslUserName, adslPassword) != ANSC_STATUS_SUCCESS )
    {
        CcspTraceError(("%s Failed to get ADSL username and password \n", __FUNCTION__ ));
    }
#endif
    CcspTraceError(("%s adslusername = %s adslpassword = %s \n", __FUNCTION__, adslUserName, adslPassword));
    //Set Username
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_USERNAME, iPPPInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", adslUserName );
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, TRUE );

    //Set Password
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_PASSWORD, iPPPInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", adslPassword );
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, TRUE );

    //Set IPCPEnable
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_IPCP_ENABLE, iPPPInstance );
    if (pInterface->PPP.IPCPEnable == TRUE)
    {
        snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "true" );
    }
    else
    {
        snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "false" );
    }
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_boolean, TRUE );

    //Set IPv6CPEnable
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_IPV6CP_ENABLE, iPPPInstance );
    if (pInterface->PPP.IPV6CPEnable == TRUE)
    {
        snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "true" );
    }
    else
    {
        snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "false" );
    }
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_boolean, TRUE );

    //Set LinkType
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_LINKTYPE, iPPPInstance );
    if (pInterface->PPP.LinkType == WAN_IFACE_PPP_LINK_TYPE_PPPoA)
    {
        snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "PPPoA" );
    }
    else if (pInterface->PPP.LinkType == WAN_IFACE_PPP_LINK_TYPE_PPPoE)
    {
        snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "PPPoE" );
    }
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, TRUE );

    //Set PPP Enable
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_ENABLE, iPPPInstance );
    if (pInterface->PPP.Enable == TRUE)
    {
        snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "true" );
    }
    else
    {
        snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "false" );
    }
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_boolean, TRUE );

    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s%d.", PPP_INTERFACE_TABLE, iPPPInstance);
    CcspTraceInfo(("%s %d Set ppp path to %s \n", __FUNCTION__,__LINE__ ,acSetParamValue ));
    strcpy(pInterface->PPP.Path, acSetParamValue);

    CcspTraceInfo(("%s %d Successfully created PPP %s interface \n", __FUNCTION__,__LINE__, pInterface->Wan.Name ));

    //Clean exit
    pthread_exit(NULL);

    return NULL;
}

ANSC_STATUS WanManager_DeletePPPSession(DML_WAN_IFACE* pInterface)
{
    char acSetParamName[256] = {0};
    char acSetParamValue[256] = {0};
    INT  iPPPInstance = -1;

    if( NULL == pInterface )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    syscfg_init();
    memset( acSetParamName, 0, DATAMODEL_PARAM_LENGTH );
    memset( acSetParamValue, 0, DATAMODEL_PARAM_LENGTH );

    iPPPInstance = get_index_from_path(pInterface->PPP.Path);
    if (iPPPInstance == -1)
    {
        CcspTraceInfo(("%s %d PPP path is invalid \n",__FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    //Set PPP Enable
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_ENABLE, iPPPInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "false" );
    WanMgr_RdkBus_SetParamValues( PPPMGR_COMPONENT_NAME, PPPMGR_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_boolean, TRUE );

    //Delete PPP Instance
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, PPP_INTERFACE_INSTANCE, iPPPInstance );
    if ( CCSP_SUCCESS != CcspBaseIf_DeleteTblRow (
                            bus_handle,
                            PPPMGR_COMPONENT_NAME,
                            PPPMGR_DBUS_PATH,
                            0, /* session id */
                            acSetParamName))
     {
         CcspTraceError(("%s Failed to delete table %s\n", __FUNCTION__, acSetParamName));
         return ANSC_STATUS_FAILURE;
     }

    CcspTraceInfo(("%s %d Successfully deleted PPP interface \n", __FUNCTION__,__LINE__ ));

    sleep(2);

    /* Create a dummy wan bridge */
    if (syscfg_set(NULL, SYSCFG_WAN_INTERFACE_NAME, DEFAULT_IFNAME) != 0)
    {
        CcspTraceError(("%s %d - syscfg_set failed to set Interafce=%s \n", __FUNCTION__, __LINE__, DEFAULT_IFNAME ));
    }else{
        CcspTraceInfo(("%s %d - syscfg_set successfully to set Interafce=%s \n", __FUNCTION__, __LINE__, DEFAULT_IFNAME ));
    }
    syscfg_commit();    
    createDummyWanBridge();

    return ANSC_STATUS_SUCCESS;
}

int WanManager_AddGatewayRoute(const WANMGR_IPV4_DATA* pIpv4Info)
{
    char cmd[BUFLEN_128]={0};
    int ret = RETURN_OK;
    FILE *fp = NULL;
    
    /* delete gateway first before add  */
    snprintf(cmd, sizeof(cmd), "route del %s dev %s", pIpv4Info->gateway, pIpv4Info->ifname);
    WanManager_DoSystemAction("SetUpSystemGateway:", cmd);

    /* Sets gateway route entry */
    if (IsValidIpv4Address(pIpv4Info->gateway) && !(IsZeroIpvxAddress(AF_SELECT_IPV4, pIpv4Info->gateway)))
    {
        snprintf(cmd, sizeof(cmd), "ip route add %s dev %s", pIpv4Info->gateway, pIpv4Info->ifname);
        WanManager_DoSystemAction("SetUpSystemGateway:", cmd);
        CcspTraceInfo(("%s %d - The gateway route entries set!\n",__FUNCTION__,__LINE__));
    }

    return ret;
}

static int get_index_from_path(const char *path)
{
    int index = -1;

    if(path == NULL)
    {
        return -1;
    }

    sscanf(path, "%*[^0-9]%d", &index);
    return index;
}

#ifdef _HUB4_PRODUCT_REQ_
static ANSC_STATUS GetAdslUsernameAndPassword(char *Username, char *Password)
{
    int ret= ANSC_STATUS_SUCCESS;
    FILE *fp = NULL;
    char line[BUFLEN_128] = {0};

    if (Username == NULL || Password == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }
    if (fp = fopen(SERIALIZATION_DATA, "r"))
    {
        while(fgets(line, sizeof(line), fp)!=NULL)
        {
            if(strstr(line,"ADSLUSER"))
            {
                char * ch = strrchr(line,'=');
                strncpy(Username, ch+1, BUFLEN_64);
                Username[strcspn(Username, "\n\r")] = 0;
            }
            if(strstr(line,"ADSLPASS"))
            {
                char * ch = strrchr(line,'=');
                strncpy(Password, ch+1, BUFLEN_64);
                Password[strcspn(Password, "\n\r")] = 0;
            }
        }
        if(fp != NULL)
        {
            fclose(fp);
            fp = NULL;
        }
    }
    else
    {
        ret = ANSC_STATUS_FAILURE;
    }

    return ret;
}
#endif

static void generate_client_duid_conf()
{
    char duid[256] = {0};
    char file_path[64] = {0};
    char wan_mac[64] = {0};

    FILE *fp_duid = NULL;
    FILE *fp_mac_addr_table = NULL;

    fp_duid = fopen(DUID_CLIENT, "r");

    if( fp_duid == NULL )
    {
        sprintf(file_path, "/sys/class/net/%s/address", PHY_WAN_IF_NAME);
        fp_mac_addr_table = fopen(file_path, "r");
        if(fp_mac_addr_table == NULL)
        {
             CcspTraceError(("Failed to open mac address table"));
        }
        else
        {
            fread(wan_mac, sizeof(wan_mac),1, fp_mac_addr_table);
            fclose(fp_mac_addr_table);
        }

        fp_duid = fopen(DUID_CLIENT, "w");

        if(fp_duid)
        {
            sprintf(duid, DUID_TYPE);
            sprintf(duid+6, HW_TYPE);
            sprintf(duid+12, wan_mac);

            fprintf(fp_duid, "%s", duid);
            fclose(fp_duid);
        }
    }
    else
    {
        fclose(fp_duid);
        CcspTraceError(("dibbler client-duid file exist"));
    }

    return;
}

static void createDummyWanBridge()
{
    char syscmd[256] = {'\0'};
    char wan_mac[64] = {'\0'};
    char file_path[64] = {0};
    FILE *fp_mac_addr_table = NULL;

    sprintf(file_path, "/sys/class/net/%s/address", PHY_WAN_IF_NAME);
    fp_mac_addr_table = fopen(file_path, "r");
    if(fp_mac_addr_table == NULL)
    {
        CcspTraceError(("Failed to open mac address table"));
    }
    else
    {
        fread(wan_mac, sizeof(wan_mac),1, fp_mac_addr_table);
        fclose(fp_mac_addr_table);
    }

    memset(syscmd, '\0', sizeof(syscmd));
    snprintf(syscmd, sizeof(syscmd), "brctl addbr %s", PHY_WAN_IF_NAME);
    system(syscmd);

    memset(syscmd, '\0', sizeof(syscmd));
    snprintf(syscmd, sizeof(syscmd), "ip link set dev %s address %s", PHY_WAN_IF_NAME, wan_mac);
    system(syscmd);

    return;
}

static void deleteDummyWanBridgeIfExist()
{
    char syscmd[256] = {'\0'};
    char resultBuff[256] = {'\0'};
    char cmd[256] = {'\0'};
    FILE *fp = NULL;

    memset(resultBuff, '\0', sizeof(resultBuff));
    memset(cmd, '\0', sizeof(cmd));
    memset(syscmd, '\0', sizeof(syscmd));
    snprintf(cmd, sizeof(cmd), "ip -d link show %s | tail -n +2 | grep bridge", PHY_WAN_IF_NAME);
    fp = popen(cmd, "r");
    if (fp != NULL)
    {
        fgets(resultBuff, 1024, fp);
        if (resultBuff[0] == '\0')
        {
            // Empty result. No bridge found.
            CcspTraceInfo(("%s bridge interface is not exists in the system \n", PHY_WAN_IF_NAME));
        }
        else
        {
            CcspTraceInfo(("%s bridge interface is found in the system, so delete it \n", PHY_WAN_IF_NAME));
            /* Down the interface before we delete it. */
            snprintf(syscmd, sizeof(syscmd), "ifconfig %s down", PHY_WAN_IF_NAME);
            system(syscmd);
            memset(syscmd, '\0', sizeof(syscmd));
            snprintf(syscmd, sizeof(syscmd), "brctl delbr %s", PHY_WAN_IF_NAME);
            system(syscmd);
        }
        pclose(fp);
    }

    return;
}

ANSC_STATUS WanManager_CheckGivenPriorityExists(INT IfIndex, UINT uiTotalIfaces, INT priority, DML_WAN_IFACE_TYPE priorityType, BOOL *Status)
{
    ANSC_STATUS             retStatus    = ANSC_STATUS_SUCCESS;
    INT                     uiLoopCount   = 0;
    INT                     wan_if_count = 0;

    if ( uiTotalIfaces <= 0 )
    {
        CcspTraceError(("%s Invalid Parameter\n", __FUNCTION__ ));
        return ANSC_STATUS_FAILURE;
    }

    for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            if( uiLoopCount == IfIndex )
            {
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                continue;
            }
            if(pWanIfaceData->Wan.Priority == priority)
            {
                if ( pWanIfaceData->Wan.Type == priorityType)
                {
                    *Status = TRUE;
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    return retStatus;
                }
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }

    return retStatus;
}

ANSC_STATUS WanManager_CheckGivenTypeExists(INT IfIndex, UINT uiTotalIfaces, DML_WAN_IFACE_TYPE priorityType, INT priority, BOOL *Status)
{
    ANSC_STATUS             retStatus    = ANSC_STATUS_SUCCESS;
    INT                     uiLoopCount   = 0;
    INT                     wan_if_count = 0;

    if ( uiTotalIfaces <= 0 )
    {
        CcspTraceError(("%s Invalid Parameter\n", __FUNCTION__ ));
        return ANSC_STATUS_FAILURE;
    }

    for( uiLoopCount = 0; uiLoopCount < uiTotalIfaces; uiLoopCount++ )
    {
        WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(uiLoopCount);
        if(pWanDmlIfaceData != NULL)
        {
            DML_WAN_IFACE* pWanIfaceData = &(pWanDmlIfaceData->data);
            if( uiLoopCount == IfIndex )
            {
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                continue;
            }
            if(pWanIfaceData->Wan.Type == priorityType)
            {
                if ( pWanIfaceData->Wan.Priority == priority)
                {
                    *Status = TRUE;
                    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                    return retStatus;
                }
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
    }

    return retStatus;
}

static INT IsIPObtained(char *pInterfaceName)
{
    char command[256] = {0};
    char buff[256] = {0};
    FILE *fp = NULL;
    if (!pInterfaceName)
    {
        return 0;
    }

    /* Validate IPv4 Connection on WAN interface */
    memset(command,0,sizeof(command));
    snprintf(command, sizeof(command), "ip addr show %s |grep -i 'inet ' |awk '{print $2}' |cut -f2 -d:", pInterfaceName);
    memset(buff,0,sizeof(buff));

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL)
    {
        printf("<%s>:<%d> Error popen\n", __FUNCTION__, __LINE__);

    }
    else
    {
        /* Read the output a line at a time - output it. */
        if (fgets(buff, 50, fp) != NULL)
        {
            printf("IP :%s", buff);
        }
        /* close */
        pclose(fp);
        if(buff[0] != 0)
        {
            CcspTraceInfo(("%s %d - IP obtained %s\n", __FUNCTION__, __LINE__,pInterfaceName));
            return 1;
        }
    }

    /* Validate IPv6 Connection on WAN interface */
    memset(command,0,sizeof(command));
    snprintf(command,sizeof(command), "ip addr show %s |grep -i 'inet6 ' |grep -i 'Global' |awk '{print $2}'", pInterfaceName);
    memset(buff,0,sizeof(buff));

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL)
    {
       printf("<%s>:<%d> Error popen\n", __FUNCTION__, __LINE__);
    }
    else
    {
        /* Read the output a line at a time - output it. */
        if (fgets(buff, 50, fp) != NULL)
        {
            printf("IP :%s", buff);
        }
        /* close */
        pclose(fp);
        if(buff[0] != 0)
        {
            CcspTraceInfo(("%s %d - IP obtained %s \n", __FUNCTION__, __LINE__,pInterfaceName));
            return 2;
        }
    }
    return 0;
}


void* ThreadWanMgr_MonitorAndUpdateIpStatus( void *arg )
{
    UINT counter = 0;
    UINT  *puIndex = NULL;
    INT ipstatus = 0;
    DML_WAN_IFACE* pFixedInterface = NULL;

    pthread_detach(pthread_self());

    if ( NULL == arg )
    {
       CcspTraceError(("%s %d Invalid buffer\n", __FUNCTION__,__LINE__));
       //Cleanup current thread when exit
       pthread_exit(NULL);
    }
    puIndex = (UINT*)arg;
    CcspTraceInfo(("%s %d index %d\n", __FUNCTION__,__LINE__,*puIndex));
    while(1)
    {
       WanMgr_Iface_Data_t*   pWanDmlIfaceData = WanMgr_GetIfaceData_locked(*puIndex);
        if (!pWanDmlIfaceData)
        {
            break;
        }
        pFixedInterface = &pWanDmlIfaceData->data;
        if (!pFixedInterface)
        {
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            break;
        }
        ipstatus = IsIPObtained(pFixedInterface->Wan.Name);
        if (ipstatus != 0)
        {
            if (ipstatus == 1)
            {
                pFixedInterface->IP.Ipv4Status = WAN_IFACE_IPV4_STATE_UP;
            }
            if (ipstatus == 2)
            {
                pFixedInterface->IP.Ipv6Status = WAN_IFACE_IPV6_STATE_UP;
            }
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            break;
        }
        if (counter >= IPMONITOR_WAIT_MAX_TIMEOUT)
        {
            pFixedInterface->IP.Ipv4Status = WAN_IFACE_IPV4_STATE_DOWN;
            pFixedInterface->IP.Ipv6Status = WAN_IFACE_IPV6_STATE_DOWN;
            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
            break;
        }
        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        sleep(IPMONITOR_QUERY_INTERVAL);
        counter += IPMONITOR_QUERY_INTERVAL;
    }
    free(puIndex);
    return arg;
}

INT WanMgr_StartIpMonitor(UINT iface_index)
{
    INT  iErrorCode     = 0;
    pthread_t ipMonitorThreadId;

    UINT *pIndex = malloc(sizeof(UINT));
    if (pIndex)
    {
        *pIndex = iface_index;
        iErrorCode = pthread_create( &ipMonitorThreadId, NULL, &ThreadWanMgr_MonitorAndUpdateIpStatus, (void*)pIndex );
    }
    return iErrorCode;
}
