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
#include "wanmgr_data.h"
#include "wanmgr_dhcpv6_apis.h"
#include "wanmgr_interface_sm.h"
#include "wanmgr_sysevents.h"
#include "wanmgr_ipc.h"
#include "wanmgr_utils.h"
#include "wanmgr_net_utils.h"


#include <sysevent/sysevent.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <syscfg/syscfg.h>

extern int sysevent_fd;
extern token_t sysevent_token;
extern ANSC_HANDLE bus_handle;
extern char g_Subsystem[32];

#define IFADDRCONF_ADD 0
#define IFADDRCONF_REMOVE 1

#ifdef _HUB4_PRODUCT_REQ_
#include "wanmgr_ipc.h"
#if defined SUCCESS
#undef SUCCESS
#endif
#define SYSEVENT_FIELD_IPV6_DNS_SERVER    "wan6_ns"
#define SYSEVENT_FIELD_IPV6_ULA_ADDRESS   "ula_address"
#endif

#if defined(CISCO_CONFIG_DHCPV6_PREFIX_DELEGATION) && defined(_COSA_BCM_MIPS_)
#include <netinet/in.h>
#endif
#define SYSCFG_FORMAT_DHCP6C "tr_dhcpv6c"
#define CLIENT_DUID_FILE "/var/lib/dibbler/client-duid"
#define DHCPS6V_SERVER_RESTART_FIFO "/tmp/ccsp-dhcpv6-server-restart-fifo.txt"

#define CLIENT_BIN     "dibbler-client"

#if defined (INTEL_PUMA7)
#define NO_OF_RETRY 90
#if defined(MULTILAN_FEATURE)
#define IPV6_PREF_MAXLEN 128
#endif
#endif

static struct {
    pthread_t          dhcpv6c_thread;
}gDhcpv6c_ctx;

extern WANMGR_BACKEND_OBJ* g_pWanMgrBE;
static void * dhcpv6c_dbg_thrd(void * in);

/*erouter topology mode*/
enum tp_mod {
    TPMOD_UNKNOWN,
    FAVOR_DEPTH,
    FAVOR_WIDTH,
};

typedef struct pd_pool {
    char start[INET6_ADDRSTRLEN];
    char end[INET6_ADDRSTRLEN];
    int  prefix_length;
    int  pd_length;
} pd_pool_t;

typedef struct ia_info {
    union {
        char v6addr[INET6_ADDRSTRLEN];
        char v6pref[INET6_ADDRSTRLEN];
    } value;

    char t1[32], t2[32], iaid[32], pretm[32], vldtm[32];
    int len;
} ia_pd_t;

typedef struct ipv6_prefix {
    char value[INET6_ADDRSTRLEN];
    int  len;
    //int  b_used;
} ipv6_prefix_t;

#if defined(MULTILAN_FEATURE)
#define IPV6_PREF_MAXLEN 128
static char v6addr_prev[IPV6_PREF_MAXLEN] = {0};
#endif

void _get_shell_output(char * cmd, char * out, int len);
int _get_shell_output2(char * cmd, char * dststr);
static int _prepare_client_conf(PDML_DHCPCV6_CFG       pCfg);
static int _dibbler_client_operation(char * arg);

static DML_DHCPCV6_FULL  g_dhcpv6_client;

static int _dibbler_server_operation(char * arg);
void _cosa_dhcpsv6_refresh_config();

static int DHCPv6sDmlTriggerRestart(BOOL OnlyTrigger);

void _get_shell_output(char * cmd, char * out, int len)
{
    FILE * fp;
    char   buf[256];
    char * p;

    fp = popen(cmd, "r");

    if (fp)
    {
        fgets(buf, sizeof(buf), fp);

        /*we need to remove the \n char in buf*/
        if ((p = strchr(buf, '\n'))) *p = 0;

        strncpy(out, buf, len-1);

        pclose(fp);
    }

}

int _get_shell_output2(char * cmd, char * dststr)
{
    FILE * fp;
    char   buf[256];
    char * p;
    int   bFound = 0;

    fp = popen(cmd, "r");

    if (fp)
    {
        while( fgets(buf, sizeof(buf), fp) )
        {
            if (strstr(buf, dststr))
            {
                bFound = 1;;
                break;
            }
        }

        pclose(fp);
    }

    return bFound;
}

/*
    Description:
        The API retrieves the number of DHCP clients in the system.
 */
ULONG
WanMgr_DmlDhcpv6cGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    )
{
    UNREFERENCED_PARAMETER(hContext);
    return 1;
}

/*
    Description:
        The API retrieves the complete info of the DHCP clients designated by index. The usual process is the caller gets the total number of entries, then iterate through those by calling this API.
    Arguments:
        ulIndex        Indicates the index number of the entry.
        pEntry        To receive the complete info of the entry.
*/
static int _get_client_duid(UCHAR * out, int len)
{
    char buf[256] = {0};
    FILE * fp = fopen(CLIENT_DUID_FILE, "r+");
    int  i = 0;
    int  j = 0;

    if(fp)
    {
        fgets(buf, sizeof(buf), fp);

        if(buf[0])
        {
            while(j<len && buf[i])
            {
                if (buf[i] != ':')
                    out[j++] = buf[i];

                i++;
            }
        }
        fclose(fp);
    }

    return 0;
}


ANSC_STATUS
WanMgr_DmlDhcpv6cGetEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        PDML_DHCPCV6_FULL      pEntry
    )
{
    UNREFERENCED_PARAMETER(hContext);
    char out[16];

    if (ulIndex)
        return ANSC_STATUS_FAILURE;

    pEntry->Cfg.InstanceNumber = 1;

    /*Cfg members*/
    syscfg_get(NULL, SYSCFG_FORMAT_DHCP6C "_alias", pEntry->Cfg.Alias, sizeof(pEntry->Cfg.Alias));

    pEntry->Cfg.SuggestedT1 = pEntry->Cfg.SuggestedT2 = 0;

    if (syscfg_get(NULL, SYSCFG_FORMAT_DHCP6C "_t1", out, sizeof(out)) == 0)
    {
        sscanf(out, "%lu", &pEntry->Cfg.SuggestedT1);
    }

    if (syscfg_get(NULL, SYSCFG_FORMAT_DHCP6C "_t2", out, sizeof(out)) == 0)
    {
        sscanf(out, "%lu", &pEntry->Cfg.SuggestedT2);
    }

    /*pEntry->Cfg.Interface stores interface name, dml will calculate the full path name*/
    snprintf(pEntry->Cfg.Interface, sizeof(pEntry->Cfg.Interface), "%s", DML_DHCP_CLIENT_IFNAME);

    syscfg_get(NULL, SYSCFG_FORMAT_DHCP6C "_requested_options", pEntry->Cfg.RequestedOptions, sizeof(pEntry->Cfg.RequestedOptions));

    syscfg_get(NULL, SYSCFG_FORMAT_DHCP6C "_enabled", out, sizeof(out));
    pEntry->Cfg.bEnabled = (out[0] == '1') ? TRUE : FALSE;

    syscfg_get(NULL, SYSCFG_FORMAT_DHCP6C "_iana_enabled", out, sizeof(out));
    pEntry->Cfg.RequestAddresses = (out[0] == '1') ? TRUE : FALSE;

    syscfg_get(NULL, SYSCFG_FORMAT_DHCP6C "_iapd_enabled", out, sizeof(out));
    pEntry->Cfg.RequestPrefixes = (out[0] == '1') ? TRUE : FALSE;

    syscfg_get(NULL, SYSCFG_FORMAT_DHCP6C "_rapidcommit_enabled", out, sizeof(out));
    pEntry->Cfg.RapidCommit = (out[0] == '1') ? TRUE : FALSE;

    /*Info members*/
    if (pEntry->Cfg.bEnabled)
        pEntry->Info.Status = DML_DHCP_STATUS_Enabled;
    else
        pEntry->Info.Status = DML_DHCP_STATUS_Disabled;

    /*TODO: supported options*/

    _get_client_duid(pEntry->Info.DUID, sizeof(pEntry->Info.DUID));


    /*if we don't have alias, set a default one*/


    AnscCopyMemory(&g_dhcpv6_client, pEntry, sizeof(g_dhcpv6_client));

    if (pEntry->Cfg.bEnabled)
        _prepare_client_conf(&pEntry->Cfg);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
WanMgr_DmlDhcpv6cSetValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulInstanceNumber);

    if (ulIndex)
    {
        return ANSC_STATUS_FAILURE;
    }

    strncpy(g_dhcpv6_client.Cfg.Alias, pAlias,sizeof(g_dhcpv6_client.Cfg.Alias)-1);


    return ANSC_STATUS_SUCCESS;
}

/*
    Description:
        The API adds DHCP client.
    Arguments:
        pEntry        Caller fills in pEntry->Cfg, except Alias field. Upon return, callee fills pEntry->Cfg.Alias field and as many as possible fields in pEntry->Info.
*/
ANSC_STATUS
WanMgr_DmlDhcpv6cAddEntry
    (
        ANSC_HANDLE                 hContext,
        PDML_DHCPCV6_FULL        pEntry
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(pEntry);
    return ANSC_STATUS_SUCCESS;
}

/*
    Description:
        The API removes the designated DHCP client entry.
    Arguments:
        pAlias        The entry is identified through Alias.
*/
ANSC_STATUS
WanMgr_DmlDhcpv6cDelEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulInstanceNumber);
    return ANSC_STATUS_SUCCESS;
}

/*
A sample content of client.conf
iface wan0 {
    rapid-commit yes
    ia {
    t1 1000
    t2 2000
    }
    pd {
    t1 1000
    t2 2000
    }
}
*/
#define TMP_CLIENT_CONF "/tmp/.dibbler_client_conf"
#define CLIENT_CONF_LOCATION  "/etc/dibbler/client.conf"
#define TMP_SERVER_CONF "/tmp/.dibbler_server_conf"
#define SERVER_CONF_LOCATION  "/etc/dibbler/server.conf"

static int _prepare_client_conf(PDML_DHCPCV6_CFG       pCfg)
{
    FILE * fp = fopen(TMP_CLIENT_CONF, "w+");
    char line[256] = {0};

    if (fp)
    {
        /*we need this to get IANA IAPD info from dibbler*/
        fprintf(fp, "notify-scripts\n");

        fprintf(fp, "iface %s {\n", pCfg->Interface);

        if (pCfg->RapidCommit)
            fprintf(fp, "    rapid-commit yes\n");

        if (pCfg->RequestAddresses)
        {
            fprintf(fp, "    ia {\n");

            if (pCfg->SuggestedT1)
            {
                memset(line, 0, sizeof(line));
                snprintf(line, sizeof(line)-1, "    t1 %lu\n", pCfg->SuggestedT1);
                fprintf(fp, "%s", line);
            }

            if (pCfg->SuggestedT2)
            {
                memset(line, 0, sizeof(line));
                snprintf(line, sizeof(line)-1, "    t2 %lu\n", pCfg->SuggestedT2);
                fprintf(fp, "%s", line);
            }

            fprintf(fp, "    }\n");
        }

        if (pCfg->RequestPrefixes)
        {
            fprintf(fp, "    pd {\n");

            if (pCfg->SuggestedT1)
            {
                memset(line, 0, sizeof(line));
                snprintf(line, sizeof(line)-1, "    t1 %lu\n", pCfg->SuggestedT1);
                fprintf(fp, "%s", line);
            }

            if (pCfg->SuggestedT2)
            {
                memset(line, 0, sizeof(line));
                snprintf(line, sizeof(line)-1, "    t2 %lu\n", pCfg->SuggestedT2);
                fprintf(fp, "%s", line);
            }

            fprintf(fp, "    }\n");
        }

        fprintf(fp, "}\n");

        fclose(fp);
    }

    /*we will copy the updated conf file at once*/
    if (rename(TMP_CLIENT_CONF, CLIENT_CONF_LOCATION))
        CcspTraceWarning(("%s rename failed %s\n", __FUNCTION__, strerror(errno)));

    return 0;
}

static int _dibbler_client_operation(char * arg)
{
    char cmd[256] = {0};
    char out[256] = {0};
#if defined (INTEL_PUMA7)
    int watchdog = NO_OF_RETRY;
#endif

    if (!strncmp(arg, "stop", 4))
    {
        CcspTraceInfo(("%s stop\n", __func__));
        /*TCXB6 is also calling service_dhcpv6_client.sh but the actuall script is installed from meta-rdk-oem layer as the intel specific code
                   had to be removed */
#if defined(CISCO_CONFIG_DHCPV6_PREFIX_DELEGATION) && ! defined(DHCPV6_PREFIX_FIX)
        sysevent_set(sysevent_fd, sysevent_token, "dhcpv6_client-stop", "", 0);
#elif defined(CORE_NET_LIB)
        system("/usr/bin/service_dhcpv6_client dhcpv6_client_service_disable");
        CcspTraceInfo(("%s  Calling service_dhcpv6_client.c with dhcpv6_client_service_disable from wanmgr_dhcpv6_apis.c\n", __func__));
#else
        system("/etc/utopia/service.d/service_dhcpv6_client.sh disable");
#endif

#ifdef _COSA_BCM_ARM_
        if (TRUE == WanManager_IsApplicationRunning (CLIENT_BIN))
        {
            CcspTraceInfo(("%s-%d [%s] is already running, killing it \n", __FUNCTION__,__LINE__,CLIENT_BIN));
            snprintf(cmd, sizeof(cmd)-1, "killall %s", CLIENT_BIN);
            system(cmd);
            sleep(2);
#ifdef _HUB4_PRODUCT_REQ_
            snprintf(cmd, sizeof(cmd)-1, "ps | grep %s | grep -v grep", CLIENT_BIN);
#else
            sprintf(cmd, "ps -A|grep %s", CLIENT_BIN);
#endif // _HUB4_PRODUCT_REQ_
            _get_shell_output(cmd, out, sizeof(out));
            if (strstr(out, CLIENT_BIN))
            {
                snprintf(cmd,sizeof(cmd)-1, "killall -9 %s", CLIENT_BIN);
                system(cmd);
            }
        }
#endif
    }
    else if (!strncmp(arg, "start", 5))
    {
        CcspTraceInfo(("%s start\n", __func__));

#if defined(INTEL_PUMA7) || defined(_COSA_BCM_ARM_)

#if defined(INTEL_PUMA7)
        //Intel Proposed RDKB Generic Bug Fix from XB6 SDK
        /* Waiting for the TLV file to be parsed correctly so that the right erouter mode can be used in the code below.
        For ANYWAN please extend the below code to support the case of TLV config file not being there. */
      do{
         sprintf(cmd, "sysevent get TLV202-status");
         _get_shell_output(cmd, out, sizeof(out));
         fprintf( stderr, "\n%s:%s(): Waiting for CcspGwProvApp to parse TLV config file\n", __FILE__, __FUNCTION__);
         sleep(1);//sleep(1) is to avoid lots of trace msgs when there is latency
         watchdog--;
        }while((!strstr(out,"success")) && (watchdog != 0));

        if(watchdog == 0)
        {
            //When 60s have passed and the file is still not configured by CcspGwprov module
            fprintf(stderr, "\n%s()%s(): TLV data has not been initialized by CcspGwProvApp.Continuing with the previous configuration\n",__FILE__, __FUNCTION__);
        }
#endif
#ifndef _HUB4_PRODUCT_REQ_
        /* This wait loop is not required as we are not configuring IPv6 address on erouter0 interface */
        sprintf(cmd, "syscfg get last_erouter_mode");
        _get_shell_output(cmd, out, sizeof(out));
    /* TODO: To be fixed by Comcast
             IPv6 address assigned to erouter0 gets deleted when erouter_mode=3(IPV4 and IPV6 both)
             Don't start v6 service in parallel. Wait for wan-status to be set to 'started' by IPv4 DHCP client.
    */
        if (strstr(out, "3"))// If last_erouter_mode is both IPV4/IPV6
    {
             do{
                    sprintf(cmd, "sysevent get wan-status");
                _get_shell_output(cmd, out, sizeof(out));
                CcspTraceInfo(("%s waiting for wan-status to started\n", __func__));
            sleep(1);//sleep(1) is to avoid lots of trace msgs when there is latency
        }while(!strstr(out,"started"));
    }
#endif
#endif
        /*This is for ArrisXB6 */
        /*TCXB6 is also calling service_dhcpv6_client.sh but the actuall script is installed from meta-rdk-oem layer as the intel specific code
         had to be removed */
        CcspTraceInfo(("%s  Callin service_dhcpv6_client.sh enable \n", __func__));
#if defined(CISCO_CONFIG_DHCPV6_PREFIX_DELEGATION) && ! defined(DHCPV6_PREFIX_FIX)
        sysevent_set(sysevent_fd, sysevent_token, "dhcpv6_client-start", "", 0);
#elif defined(CORE_NET_LIB)
        system("/usr/bin/service_dhcpv6_client dhcpv6_client_service_enable");
        CcspTraceInfo(("%s  Calling service_dhcpv6_client.c with dhcpv6_client_service_enable from wanmgr_dhcpv6_apis.c\n", __func__));
#else
        system("/etc/utopia/service.d/service_dhcpv6_client.sh enable");
#endif

#ifdef _COSA_BCM_ARM_
        /* Dibbler-init is called to set the pre-configuration for dibbler */
        CcspTraceInfo(("%s dibbler-init.sh Called \n", __func__));
        system("/lib/rdk/dibbler-init.sh");
        /*Start Dibber client for tchxb6*/
        CcspTraceInfo(("%s Dibbler Client Started \n", __func__));
        snprintf(cmd, sizeof(cmd)-1, "%s start", CLIENT_BIN);
        system(cmd);
#endif
    }
    else if (!strncmp(arg, "restart", 7))
    {
        _dibbler_client_operation("stop");
        _dibbler_client_operation("start");
    }

    return 0;
}
/*internal.c will call this when obtained Client and SentOptions. so we don't need to start service in getEntry for Client and SentOptions.*/
int  WanMgr_DmlStartDHCP6Client()
{
    pthread_detach(pthread_self());
#if defined(_COSA_INTEL_XB3_ARM_)
    CcspTraceInfo(("Not restarting ti_dhcp6c for XB3 case\n"));
#else
   // _dibbler_client_operation("restart");
#endif
    return 0;
}
/*
Description:
    The API re-configures the designated DHCP client entry.
Arguments:
    pAlias        The entry is identified through Alias.
    pEntry        The new configuration is passed through this argument, even Alias field can be changed.
*/
ANSC_STATUS
WanMgr_DmlDhcpv6cSetCfg
    (
        ANSC_HANDLE                 hContext,
        PDML_DHCPCV6_CFG       pCfg
    )
{
    UNREFERENCED_PARAMETER(hContext);
    char buf[256] = {0};
    char out[256] = {0};
    int  need_to_restart_service = 0;

    if (pCfg->InstanceNumber != 1)
        return ANSC_STATUS_FAILURE;


    if (!AnscEqualString((char*)pCfg->Alias, (char*)g_dhcpv6_client.Cfg.Alias, TRUE))
    {
        strncpy(buf, SYSCFG_FORMAT_DHCP6C"_alias",sizeof(buf)-1);
        syscfg_set_string(buf,(char*)pCfg->Alias);
    }

    if (pCfg->SuggestedT1 != g_dhcpv6_client.Cfg.SuggestedT1)
    {
        strncpy(buf, SYSCFG_FORMAT_DHCP6C"_t1",sizeof(buf)-1);
        sprintf(out, "%lu", pCfg->SuggestedT1);
        syscfg_set_string(buf, out);
        need_to_restart_service = 1;
    }

    if (pCfg->SuggestedT2 != g_dhcpv6_client.Cfg.SuggestedT2)
    {
        strncpy(buf, SYSCFG_FORMAT_DHCP6C"_t2",sizeof(buf)-1);
        sprintf(out, "%lu", pCfg->SuggestedT2);
        syscfg_set_string(buf, out);
        need_to_restart_service = 1;
    }

    if (!AnscEqualString((char*)pCfg->RequestedOptions, (char*)g_dhcpv6_client.Cfg.RequestedOptions, TRUE))
    {
        strncpy(buf, SYSCFG_FORMAT_DHCP6C"_requested_options",sizeof(buf)-1);
        syscfg_set_string(buf, (char*)pCfg->RequestedOptions);
        need_to_restart_service = 1;
    }

    if (pCfg->bEnabled != g_dhcpv6_client.Cfg.bEnabled)
    {
        strncpy(buf, SYSCFG_FORMAT_DHCP6C"_enabled",sizeof(buf)-1);
        out[0] = pCfg->bEnabled ? '1':'0';
        out[1] = 0;
        syscfg_set_string(buf, out);
        need_to_restart_service = 1;
    }

    if (pCfg->RequestAddresses != g_dhcpv6_client.Cfg.RequestAddresses)
    {
        strncpy(buf, SYSCFG_FORMAT_DHCP6C"_iana_enabled",sizeof(buf)-1);
        out[0] = pCfg->RequestAddresses ? '1':'0';
        out[1] = 0;
        syscfg_set_string(buf, out);
        need_to_restart_service = 1;
    }

    if (pCfg->RequestPrefixes != g_dhcpv6_client.Cfg.RequestPrefixes)
    {
        strncpy(buf, SYSCFG_FORMAT_DHCP6C"_iapd_enabled",sizeof(buf)-1);
        out[0] = pCfg->RequestPrefixes ? '1':'0';
        out[1] = 0;
        syscfg_set_string(buf, out);
        need_to_restart_service = 1;
    }

    if (pCfg->RapidCommit != g_dhcpv6_client.Cfg.RapidCommit)
    {
        strncpy(buf, SYSCFG_FORMAT_DHCP6C"_rapidcommit_enabled",sizeof(buf)-1);
        out[0] = pCfg->RapidCommit ? '1':'0';
        out[1] = 0;
        syscfg_set_string(buf, out);
        need_to_restart_service = 1;
    }

    /*update dibbler-client service if necessary*/
    if (need_to_restart_service)
    {
        if ( pCfg->bEnabled != g_dhcpv6_client.Cfg.bEnabled &&  !pCfg->bEnabled )
            _dibbler_client_operation("stop");
        else if (pCfg->bEnabled != g_dhcpv6_client.Cfg.bEnabled &&  pCfg->bEnabled )
        {
            _prepare_client_conf(pCfg);
            _dibbler_client_operation("restart");
        }
        else if (pCfg->bEnabled == g_dhcpv6_client.Cfg.bEnabled && !pCfg->bEnabled)
        {
            /*do nothing*/
        }
        else if (pCfg->bEnabled == g_dhcpv6_client.Cfg.bEnabled && pCfg->bEnabled)
        {
            _prepare_client_conf(pCfg);
            _dibbler_client_operation("restart");
        }
    }

    AnscCopyMemory(&g_dhcpv6_client.Cfg, pCfg, sizeof(DML_DHCPCV6_CFG));

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
WanMgr_DmlDhcpv6cGetCfg
    (
        ANSC_HANDLE                 hContext,
        PDML_DHCPCV6_CFG       pCfg
    )
{
    UNREFERENCED_PARAMETER(hContext);
    /*for rollback*/
    if (pCfg->InstanceNumber != 1)
        return ANSC_STATUS_FAILURE;

    AnscCopyMemory(pCfg,  &g_dhcpv6_client.Cfg, sizeof(DML_DHCPCV6_CFG));

    return ANSC_STATUS_SUCCESS;
}

BOOL
WanMgr_DmlDhcpv6cGetEnabled
    (
        ANSC_HANDLE                 hContext
    )
{
    UNREFERENCED_PARAMETER(hContext);
    BOOL bEnabled = FALSE;
    char out[256] = {0};
    BOOL dibblerEnabled = FALSE;

// For XB3, AXB6 if dibbler flag enabled, check dibbler-client process status
#if defined(_COSA_INTEL_XB3_ARM_) || defined(INTEL_PUMA7)
        char buf[8];
        if(( syscfg_get( NULL, "dibbler_client_enable", buf, sizeof(buf))==0) && (strcmp(buf, "true") == 0))
    {
        dibblerEnabled = TRUE;
    }
#endif

#if defined (_COSA_BCM_ARM_) || defined (_HUB4_PRODUCT_REQ_)
    FILE *fp = popen("ps |grep -i dibbler-client | grep -v grep", "r");

#elif defined (_XF3_PRODUCT_REQ_)
   FILE *fp = popen("/usr/sbin/dibbler-client status |grep  client", "r");

#else
    FILE *fp;
    // For XB3, AXB6 if dibbler flag enabled, check dibbler-client process status
    if(dibblerEnabled)
        fp = popen("ps |grep -i dibbler-client | grep -v grep", "r");
    else
            fp = popen("ps |grep -i ti_dhcp6c | grep erouter0 | grep -v grep", "r");
#endif

    if ( fp != NULL){
        if ( fgets(out, sizeof(out), fp) != NULL ){
#if defined (_COSA_BCM_ARM_) || defined (_HUB4_PRODUCT_REQ_)
            if ( _ansc_strstr(out, "dibbler-client") )
                bEnabled = TRUE;
#elif defined (_XF3_PRODUCT_REQ_)
            if ( strstr(out, "RUNNING,") )
                bEnabled = TRUE;
#else
    // For XB3, AXB6 if dibbler flag enabled, check dibbler-client process status
    if ( dibblerEnabled && _ansc_strstr(out, "dibbler-client") )
                bEnabled = TRUE;
        if ( _ansc_strstr(out, "erouter_dhcp6c") )
                bEnabled = TRUE;
#endif
       }
        pclose(fp);
    }

    return bEnabled;
}

ANSC_STATUS
WanMgr_DmlDhcpv6cGetInfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PDML_DHCPCV6_INFO      pInfo
    )
{
    UNREFERENCED_PARAMETER(ulInstanceNumber);
    PDML_DHCPCV6_FULL            pDhcpc            = (PDML_DHCPCV6_FULL)hContext;

    _get_client_duid(g_dhcpv6_client.Info.DUID, sizeof(pInfo->DUID));

    if (pDhcpc)
    {
        if (pDhcpc->Cfg.bEnabled)
            g_dhcpv6_client.Info.Status = pDhcpc->Info.Status = DML_DHCP_STATUS_Enabled;
        else
            g_dhcpv6_client.Info.Status = pDhcpc->Info.Status = DML_DHCP_STATUS_Disabled;
    }

    AnscCopyMemory(pInfo,  &g_dhcpv6_client.Info, sizeof(DML_DHCPCV6_INFO));

    return ANSC_STATUS_SUCCESS;
}

#define CLIENT_SERVER_INFO_FILE "/tmp/.dibbler-info/client_server"
/*this file's format:
 line 1: addr server_ipv6_addr
 line 2: duid server_duid*/
/* this memory need to be freed by caller */
ANSC_STATUS
WanMgr_DmlDhcpv6cGetServerCfg
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PDML_DHCPCV6_SVR      *ppCfg,
        PULONG                      pSize
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulClientInstanceNumber);
    FILE * fp = fopen(CLIENT_SERVER_INFO_FILE, "r+");

    *pSize = 0;
    if (fp)
    {
        char buf[1024] = {0};
        char val[1024] = {0};
        int  entry_count = 0;

        /*we only support one entry of ServerCfg*/
        *ppCfg = (PDML_DHCPCV6_SVR)AnscAllocateMemory( sizeof(DML_DHCPCV6_SVR) );
        memset(*ppCfg, 0, sizeof(DML_DHCPCV6_SVR));

        /*InformationRefreshTime not supported*/
        strncpy((char*)(*ppCfg)->InformationRefreshTime, "0001-01-01T00:00:00Z",sizeof((*ppCfg)->InformationRefreshTime)-1);

        while(fgets(buf, sizeof(buf)-1, fp))
        {
            memset(val, 0, sizeof(val));
            if (sscanf(buf, "addr %1023s", val))
            {
                strncpy((char*)(*ppCfg)->SourceAddress, val,sizeof((*ppCfg)->SourceAddress)-1);
                entry_count |= 1;
            }
            else if (sscanf(buf, "duid %1023s", val))
            {
                unsigned int i = 0, j = 0;
                /*the file stores duid in this format 00:01:..., we need to transfer it to continuous hex*/

                for (i=0; i<strlen(val) && j < sizeof((*ppCfg)->DUID)-1; i++)
                {
                    if (val[i] != ':')
                        ((*ppCfg)->DUID)[j++] = val[i];
                }

                entry_count |= 2;
            }

            /*only we have both addr and  duid we think we have a whole server-info*/
            if (entry_count == 3)
                *pSize = 1;

            memset(buf, 0, sizeof(buf));
        }
        fclose(fp);
    }

    return ANSC_STATUS_SUCCESS;
}

/*
    Description:
        The API initiates a DHCP client renewal.
    Arguments:
        pAlias        The entry is identified through Alias.
*/
ANSC_STATUS
WanMgr_DmlDhcpv6cRenew
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulInstanceNumber);
    char cmd[256] = {0};

    snprintf(cmd, sizeof(cmd) - 1, "killall -SIGUSR2 %s", CLIENT_BIN);
    system(cmd);

    return ANSC_STATUS_SUCCESS;
}

/*
 *  DHCP Client Send/Req Option
 *
 *  The options are managed on top of a DHCP client,
 *  which is identified through pClientAlias
 */
#define SYSCFG_DHCP6C_SENT_OPTION_FORMAT "tr_dhcp6c_sent_option_%lu"
static int g_sent_option_num;
static DML_DHCPCV6_SENT * g_sent_options;
static int g_recv_option_num   = 0;
static DML_DHCPCV6_RECV * g_recv_options = NULL;

ULONG
WanMgr_DmlDhcpv6cGetNumberOfSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulClientInstanceNumber);
    char buf[256] = {0};
    char out[256] = {0};


    strncpy(buf, "tr_dhcp6c_sent_option_num", sizeof(out));
    if( syscfg_get( NULL, buf, out, sizeof(out)) == 0 )
    {
    g_sent_option_num = atoi(out);
    }

    if (g_sent_option_num)
    {
        g_sent_options = (DML_DHCPCV6_SENT *)AnscAllocateMemory(sizeof(DML_DHCPCV6_SENT)* g_sent_option_num);
    }

    return g_sent_option_num;

}

#define CLIENT_SENT_OPTIONS_FILE "/tmp/.dibbler-info/client_sent_options"
/*this function will generate sent_option info file to dibbler-client,
 the format of CLIENT_SENT_OPTIONS_FILE:
    option-type:option-len:option-data*/
static int _write_dibbler_sent_option_file(void)
{
    FILE * fp = fopen(CLIENT_SENT_OPTIONS_FILE, "w+");
    int  i = 0;

    if (fp)
    {
        for (i=0; i<g_sent_option_num; i++)
        {
            if (g_sent_options[i].bEnabled)
                fprintf(fp, "%lu:%d:%s\n",
                        g_sent_options[i].Tag,
                        strlen((const char*)g_sent_options[i].Value)/2,
                        g_sent_options[i].Value);
        }

        fclose(fp);
    }

    return 0;
}

ANSC_STATUS
WanMgr_DmlDhcpv6cGetSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulIndex,
        PDML_DHCPCV6_SENT      pEntry
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulClientInstanceNumber);
    char out[256] = {0};
    char buf[256] = {0};
    char namespace[256] = {0};

    if ( (int)ulIndex > g_sent_option_num - 1  || !g_sent_options)
    {
        return ANSC_STATUS_FAILURE;
    }

    /*note in syscfg, sent_options start from 1*/
    sprintf(namespace, SYSCFG_DHCP6C_SENT_OPTION_FORMAT, ulIndex+1);
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    if( syscfg_get( NULL, namespace, out, sizeof(out)) == 0 )
    {
    sscanf(out, "%lu", &pEntry->InstanceNumber);
    }

    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    snprintf(buf, sizeof(buf), "%s_alias", namespace);
    if( syscfg_get( NULL, buf, out, sizeof(out)) == 0 )
    {
        strncpy(pEntry->Alias, out, sizeof(pEntry->Alias));
    }

    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    snprintf(buf, sizeof(buf), "%s_enabled", namespace);
    if( syscfg_get( NULL, buf, out, sizeof(out)) == 0 )
    {
        pEntry->bEnabled = (out[0] == '1') ? TRUE:FALSE;
    }

    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    snprintf(buf, sizeof(buf), "%s_tag", namespace);
    if( syscfg_get( NULL, namespace, out, sizeof(out)) == 0 )
    {
        sscanf(out, "%lu", &pEntry->Tag);
    }

    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    snprintf(buf, sizeof(buf), "%s_value", namespace);
    if( syscfg_get( NULL, buf, out, sizeof(out)) == 0 )
    {
        strncpy(pEntry->Value, out, sizeof(pEntry->Value));
    }

    AnscCopyMemory( &g_sent_options[ulIndex], pEntry, sizeof(DML_DHCPCV6_SENT));

    /*we only wirte to file when obtaining the last entry*/
    if (ulIndex == g_sent_option_num-1)
        _write_dibbler_sent_option_file();


    return ANSC_STATUS_SUCCESS;
}


ANSC_STATUS
WanMgr_DmlDhcpv6cGetSentOptionbyInsNum
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PDML_DHCPCV6_SENT      pEntry
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulClientInstanceNumber);
    ULONG                           index  = 0;

    for( index=0;  index<g_sent_option_num; index++)
    {
        if ( pEntry->InstanceNumber == g_sent_options[index].InstanceNumber )
        {
            AnscCopyMemory( pEntry, &g_sent_options[index], sizeof(DML_DHCPCV6_SENT));
             return ANSC_STATUS_SUCCESS;
        }
    }

    return ANSC_STATUS_FAILURE;
}


ANSC_STATUS
WanMgr_DmlDhcpv6cSetSentOptionValues
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulIndex,
        ULONG                       ulInstanceNumber,
        char*                       pAlias
    )
{
    UNREFERENCED_PARAMETER(hContext);
    char out[256] = {0};
    char namespace[256] = {0};

    if (ulClientInstanceNumber != g_dhcpv6_client.Cfg.InstanceNumber)
        return ANSC_STATUS_FAILURE;

    if ( (int)ulIndex+1 > g_sent_option_num )
        return ANSC_STATUS_SUCCESS;

    g_sent_options[ulIndex].InstanceNumber  = ulInstanceNumber;
    AnscCopyString( (char*)g_sent_options[ulIndex].Alias, pAlias);


    sprintf(namespace, SYSCFG_DHCP6C_SENT_OPTION_FORMAT, ulIndex+1);
    snprintf(out, sizeof(out), "%lu", ulInstanceNumber);
    syscfg_set_string(namespace, out);

    snprintf(out, sizeof(out), "%s_alias", namespace);
    syscfg_set_string(out, pAlias);

    return ANSC_STATUS_SUCCESS;
}

static int _syscfg_add_sent_option(PDML_DHCPCV6_SENT      pEntry, int index)
{
    char out[256] = {0};
    char buf[256] = {0};
    char namespace[256] = {0};

    if (!pEntry)
        return -1;

    sprintf(namespace, SYSCFG_DHCP6C_SENT_OPTION_FORMAT, index);
    snprintf(out, sizeof(out), "%lu", pEntry->InstanceNumber);
    syscfg_set_string(namespace, out);

    snprintf(out, sizeof(out), "%s_alias", namespace);
    syscfg_set_string(out, pEntry->Alias);

    snprintf(buf, sizeof(buf), "%s_enabled", namespace);
    if (pEntry->bEnabled)
        sprintf(out, "1");
    else
        sprintf(out, "0");
    syscfg_set_string(buf, out);

    snprintf(buf, sizeof(buf), "%s_tag", namespace);
    snprintf(out, sizeof(out)-1, "%lu", pEntry->Tag);
    syscfg_set_string(buf, out);

    snprintf(buf, sizeof(buf), "%s_value", namespace);
    syscfg_set_string(buf, pEntry->Value);

    return 0;
}

ANSC_STATUS
WanMgr_DmlDhcpv6cAddSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PDML_DHCPCV6_SENT      pEntry
    )
{
    UNREFERENCED_PARAMETER(hContext);
    char out[256] = {0};
    char namespace[256] = {0};

    /*we only have one client*/
    if (ulClientInstanceNumber != g_dhcpv6_client.Cfg.InstanceNumber)
        return ANSC_STATUS_FAILURE;

    g_sent_options = realloc(g_sent_options, (++g_sent_option_num)* sizeof(DML_DHCPCV6_SENT));
    if (!g_sent_options)
        return ANSC_STATUS_FAILURE;

    _syscfg_add_sent_option(pEntry, g_sent_option_num);

    snprintf(out, sizeof(out)-1, "%d", g_sent_option_num);
    syscfg_set_string("tr_dhcp6c_sent_option_num", out);

    g_sent_options[g_sent_option_num-1] = *pEntry;

    /*if the added entry is disabled, don't update dibbler-info file*/
    if (pEntry->bEnabled)
    {
        _write_dibbler_sent_option_file();
        _dibbler_client_operation("restart");
    }

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
WanMgr_DmlDhcpv6cDelSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        ULONG                       ulInstanceNumber
    )
{
    UNREFERENCED_PARAMETER(hContext);
    char out[256] = {0};
    char namespace[256] = {0};
    int  i = 0;
    int  j = 0;
    int  saved_enable = 0;

    /*we only have one client*/
    if (ulClientInstanceNumber != g_dhcpv6_client.Cfg.InstanceNumber)
        return ANSC_STATUS_FAILURE;


    for (i=0; i<g_sent_option_num; i++)
    {
        if (g_sent_options[i].InstanceNumber == ulInstanceNumber)
            break;
    }

    if (i == g_sent_option_num)
        return ANSC_STATUS_CANT_FIND;

    saved_enable = g_sent_options[i].bEnabled;

    for (j=i; j<g_sent_option_num-1; j++)
    {
        g_sent_options[j] = g_sent_options[j+1];

        /*move syscfg ahead to fill in the deleted one*/
        _syscfg_add_sent_option(g_sent_options+j+1, j+1);
    }

    g_sent_option_num--;


    /*syscfg unset the last one, since we move the syscfg table ahead*/
    sprintf(namespace, SYSCFG_DHCP6C_SENT_OPTION_FORMAT, (ULONG)(g_sent_option_num+1));
    syscfg_unset(namespace, "inst_num");
    syscfg_unset(namespace, "alias");
    syscfg_unset(namespace, "enabled");
    syscfg_unset(namespace, "tag");
    syscfg_unset(namespace, "value");

    snprintf(out, sizeof(out)-1, "%d", g_sent_option_num);
    syscfg_set_string("tr_dhcp6c_sent_option_num", out);


    /*only take effect when the deleted entry was enabled*/
    if (saved_enable)
    {
        _write_dibbler_sent_option_file();
        _dibbler_client_operation("restart");
    }

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS
WanMgr_DmlDhcpv6cSetSentOption
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PDML_DHCPCV6_SENT      pEntry
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulClientInstanceNumber);
    ULONG                           index  = 0;
    char                            buf[256] = {0};
    char                            out[256] = {0};
    char                            namespace[256] = {0};
    int                             i = 0;
    int                             j = 0;
    int                             need_restart_service = 0;
    PDML_DHCPCV6_SENT          p_old_entry = NULL;

    for( index=0;  index<g_sent_option_num; index++)
    {
        if ( pEntry->InstanceNumber == g_sent_options[index].InstanceNumber )
        {
            p_old_entry = &g_sent_options[index];


            /*handle syscfg*/
            sprintf(namespace, SYSCFG_DHCP6C_SENT_OPTION_FORMAT, index+1);

            if (!AnscEqualString(pEntry->Alias, p_old_entry->Alias, TRUE))
            {
                snprintf(buf,sizeof(buf)-1, "%s_alias", namespace);
                syscfg_set_string(buf, pEntry->Alias);
            }

            if (pEntry->bEnabled != p_old_entry->bEnabled)
            {
                if (pEntry->bEnabled)
                    snprintf(out,sizeof(out)-1,"1");
                else
                    snprintf(out, sizeof(out)-1,"0");
                snprintf(buf,sizeof(buf)-1, "%s_enabled", namespace);
                syscfg_set_string(buf, out);
                need_restart_service = 1;
            }


            if (pEntry->Tag != p_old_entry->Tag)
            {
                snprintf(buf, sizeof(buf)-1, "%s_tag", namespace);
                snprintf(out, sizeof(out)-1, "%lu", pEntry->Tag);
                syscfg_set_string(buf, out);
                need_restart_service = 1;
            }

            if (!AnscEqualString(pEntry->Value, p_old_entry->Value, TRUE))
            {
                snprintf(buf, sizeof(buf)-1, "%s_value", namespace);
                syscfg_set_string(buf, pEntry->Value);
                need_restart_service = 1;
            }

            AnscCopyMemory( &g_sent_options[index], pEntry, sizeof(DML_DHCPCV6_SENT));

            if (need_restart_service)
            {
                _write_dibbler_sent_option_file();
                _dibbler_client_operation("restart");
            }

            return ANSC_STATUS_SUCCESS;
        }
    }

    return ANSC_STATUS_SUCCESS;
}

/* This is to get all
 */
#define CLIENT_RCVED_OPTIONS_FILE "/tmp/.dibbler-info/client_received_options"
/*this file has the following format:
 ......
 line n : opt-len opt-type opt-data(HexBinary)
 ......
*/
ANSC_STATUS
WanMgr_DmlDhcpv6cGetReceivedOptionCfg
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulClientInstanceNumber,
        PDML_DHCPCV6_RECV     *ppEntry,
        PULONG                      pSize
    )
{
    UNREFERENCED_PARAMETER(hContext);
    UNREFERENCED_PARAMETER(ulClientInstanceNumber);
    FILE *                          fp = fopen(CLIENT_RCVED_OPTIONS_FILE, "r+");
    SLIST_HEADER                    option_list;
    DML_DHCPCV6_RECV *         p_rcv = NULL;
    char                            buf[1024] = {0};
    PSINGLE_LINK_ENTRY              pSLinkEntry       = NULL;
    ULONG                           ulIndex = 0;

    AnscSListInitializeHeader( &option_list );

    if (fp)
    {
        while (fgets(buf, sizeof(buf)-1, fp))
        {
            int  opt_len = 0;
            char opt_data[1024] = {0};
            char * p = NULL;

            p_rcv = (DML_DHCPCV6_RECV * )AnscAllocateMemory(sizeof(*p_rcv));
            memset(p_rcv, 0, sizeof(*p_rcv));
            if (!p_rcv)
                break;

            if (sscanf(buf, "%d %lu", &opt_len, &p_rcv->Tag) != 2)
            {
                AnscFreeMemory(p_rcv); /*RDKB-6780, CID-33399, free unused resource*/
                p_rcv = NULL;
                continue;
            }

            /*don't trust opt_len record in file, calculate by self*/
            if ((p = strrchr(buf, ' ')))
            {
                p++;
                if ((unsigned int)opt_len*2 < (strlen(buf)- (p-buf)))
                    opt_len = (strlen(buf)- (p-buf))/2;
            }
            else
            {
                AnscFreeMemory(p_rcv); /*RDKB-6780, CID-33399, free unused resource*/
                p_rcv = NULL;
                continue;
            }

            if ((unsigned int)opt_len*2 >= sizeof(p_rcv->Value))
            {
                AnscFreeMemory(p_rcv); /*RDKB-6780, CID-33399, free unused resource*/
                p_rcv = NULL;
                continue;
            }

            /*finally we are safe, copy string into Value*/
            if (sscanf(buf, "%*d %*d %1023s", p_rcv->Value) != 1)
            {
                AnscFreeMemory(p_rcv); /*RDKB-6780, CID-33399, free unused resource*/
                p_rcv = NULL;
                continue;
            }
            /*we only support one server, hardcode it*/
            strncpy((char*)p_rcv->Server, "Device.DHCPv6.Client.1.Server.1",sizeof(p_rcv->Server)-1);
            AnscSListPushEntryAtBack(&option_list, &p_rcv->Link);

            memset(buf, 0, sizeof(buf));
        }
        fclose(fp);
    }

    if (!AnscSListGetFirstEntry(&option_list))
    {
        *pSize = 0;
        return ANSC_STATUS_SUCCESS;
    }

    *ppEntry = (PDML_DHCPCV6_RECV)AnscAllocateMemory( AnscSListQueryDepth(&option_list) *sizeof(DML_DHCPCV6_RECV) );

    pSLinkEntry = AnscSListGetFirstEntry(&option_list);

    for ( ulIndex = 0; ulIndex < option_list.Depth; ulIndex++ )
    {
        p_rcv = ACCESS_DHCPV6_RECV_LINK_OBJECT(pSLinkEntry);
        pSLinkEntry = AnscSListGetNextEntry(pSLinkEntry);

        AnscCopyMemory( *ppEntry + ulIndex, p_rcv, sizeof(*p_rcv) );
        AnscFreeMemory(p_rcv);
    }

    *pSize = AnscSListQueryDepth(&option_list);

    /* we need keep this for reference in server*/
    if ( g_recv_options )
        AnscFreeMemory(g_recv_options);

    g_recv_option_num = *pSize;
    g_recv_options = (DML_DHCPCV6_RECV *)AnscAllocateMemory( sizeof(DML_DHCPCV6_RECV) * g_recv_option_num );
    AnscCopyMemory(g_recv_options, *ppEntry, sizeof(DML_DHCPCV6_RECV) * g_recv_option_num);

    return ANSC_STATUS_SUCCESS;
}

static int DHCPv6sDmlTriggerRestart(BOOL OnlyTrigger)
{
    int fd = 0;
    char str[32] = "restart";

  #if defined(CISCO_CONFIG_DHCPV6_PREFIX_DELEGATION) && ! defined(DHCPV6_PREFIX_FIX)
    sysevent_set(sysevent_fd, sysevent_token, "dhcpv6_server-restart", "" , 0);
  #else

    //not restart really.we only need trigger pthread to check whether there is pending action.

    fd= open(DHCPS6V_SERVER_RESTART_FIFO, O_RDWR);

    if (fd < 0)
    {
        fprintf(stderr, "open dhcpv6 server restart fifo when writing.\n");
        return 1;
    }
    write( fd, str, sizeof(str) );
    close(fd);

  #endif
    return 0;
}

void
WanMgr_DmlDhcpv6Remove(ANSC_HANDLE hContext)
{
    UNREFERENCED_PARAMETER(hContext);
    /*remove backend memory*/
    if (g_sent_options)
        AnscFreeMemory(g_sent_options);
    return;
}

int dhcpv6_assign_global_ip(char * prefix, char * intfName, char * ipAddr)
{
    unsigned int    length           = 0;
    char            globalIP[64]     = {0};
    char           *pMac             = NULL;
    unsigned int    prefixLen           = 0;
    unsigned int    iteratorI,iteratorJ = 0;
    char            cmd[256]         = {0};
    char            out[256]         = {0};
    char            tmp[8]           = {0};


    _ansc_strcpy( globalIP, prefix);

    /* Prepare the first part. */

    prefixLen = _ansc_strlen(globalIP);

    while( (globalIP[prefixLen-1] != '/') && (prefixLen>0) ) prefixLen--;

    if ( prefixLen == 0 ){
        AnscTrace("error, there is not '/' in prefix:%s\n", prefix);
        return 1;
    }

    length = atoi(&globalIP[prefixLen]);

    if ( length > 64 ){
        AnscTrace("error, length is bigger than 64. prefix:%s, length:%d\n", prefix, length);
        return 1;
    }

    globalIP[prefixLen-1] = '\0';

    prefixLen = prefixLen-1;

    if ( (globalIP[prefixLen-1]!=':') && (globalIP[prefixLen-2]!=':') ){
        AnscTrace("error, there is not '::' in prefix:%s\n", prefix);
        return 1;
    }

    iteratorI = prefixLen-2;
    iteratorJ = 0;
    while( iteratorI > 0 ){
        if ( globalIP[iteratorI-1] == ':' )
            iteratorJ++;
        iteratorI--;
    }

    if ( iteratorJ == 3 )
    {
        globalIP[prefixLen-1] = '\0';
        prefixLen = prefixLen - 1;
    }

    AnscTrace("the first part is:%s\n", globalIP);


    /* prepare second part */
    snprintf(cmd, sizeof(cmd)-1, "ifconfig %s | grep HWaddr\n", intfName );
    _get_shell_output(cmd, out, sizeof(out));
    pMac =_ansc_strstr(out, "HWaddr");
    if ( pMac == NULL ){
        AnscTrace("error, this interface has not a mac address .\n");
        return 1;
    }

    pMac += _ansc_strlen("HWaddr");
    while( pMac && (pMac[0] == ' ') )
        pMac++;

    /* switch 7bit to 1*/
    tmp[0] = pMac[1];

    iteratorJ = strtol(tmp, (char **)NULL, 16);

    iteratorJ = iteratorJ ^ 0x2;
    if ( iteratorJ < 10 )
        iteratorJ += '0';
    else
        iteratorJ += 'A' - 10;

    pMac[1] = iteratorJ;
    pMac[17] = '\0';

    //00:50:56: FF:FE:  92:00:22
    _ansc_strncpy(out, pMac, 9);
    out[9] = '\0';
    strncat(out, "FF:FE:",sizeof(out)-1);
    strncat(out, pMac+9,sizeof(out)-1);

    for(iteratorJ=0,iteratorI=0; out[iteratorI]; iteratorI++){
        if ( out[iteratorI] == ':' )
            continue;
        globalIP[prefixLen++] = out[iteratorI];
        if ( ++iteratorJ == 4 ){
           globalIP[prefixLen++] = ':';
           iteratorJ = 0;
        }
    }

    globalIP[prefixLen-1] = '\0';

    AnscTrace("the full part is:%s\n", globalIP);

    _ansc_strncpy(ipAddr, globalIP, sizeof(globalIP) - 1);

    /* This IP should be unique. If not I have no idea. */
    return 0;
}

#define POS_PREFIX_DELEGATION 7
int CalcIPv6Prefix(char *GlobalPref, char *pref,int index)
{
    unsigned char buf[sizeof(struct in6_addr)];
    int domain, s;
    char str[INET6_ADDRSTRLEN];
   domain = AF_INET6;
   s = inet_pton(domain, GlobalPref, buf);
    if (s <= 0) {
        if (s == 0)
            fprintf(stderr, "Not in presentation format");
        else
            perror("inet_pton");
           return 0;
    }
   buf[POS_PREFIX_DELEGATION] = buf[POS_PREFIX_DELEGATION] + index;
   if (inet_ntop(domain, buf, str, INET6_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
            return 0;
    }
    printf("%s\n", str);
    strncpy(pref,str,sizeof(str));
    return 1;
}

int GenIPv6Prefix(char *ifName,char *GlobalPref, char *pref)
{
int len = 0;
int index = 0;
char cmd[100];
char out[100];
static int interface_num = 4; // Reserving first 4 /64s for dhcp configurations
    if(ifName == NULL)
    return 0;

    memset(cmd,0,sizeof(cmd));
    memset(out,0,sizeof(out));
    _ansc_sprintf(cmd, "%s%s",ifName,"_ipv6_index");
    sysevent_get(sysevent_fd, sysevent_token,cmd, out, sizeof(out));
    if(strlen(out) != 0)
    {
        index = atoi(out);
    }

    len = strlen(GlobalPref);
    strncpy(pref,GlobalPref, len + 1);
    if(index == 0)
    {
        if(CalcIPv6Prefix(GlobalPref,pref,interface_num)== 0)
        return 0;
        memset(cmd,0,sizeof(cmd));
        snprintf(cmd, sizeof(cmd)-1, "%s%s",ifName,"_ipv6_index");
        _ansc_sprintf(out, "%d",interface_num);
        sysevent_set(sysevent_fd, sysevent_token, cmd, out , 0);
        interface_num++;
    }
    else
    {
        if(CalcIPv6Prefix(GlobalPref,pref,index)==0 )
        return 0;
    }
    strncat(pref,"/64",sizeof("/64"));
        CcspTraceInfo(("%s: pref %s\n", __func__, pref));
return 1;

}
/* This thread is added to handle the LnF interface IPv6 rule, because LnF is coming up late in XB6 devices.
This thread can be generic to handle the operations depending on the interfaces. Other interface and their events can be register here later based on requirement */
static int sysevent_fd_1;
static token_t sysevent_token_1;
static pthread_t InfEvtHandle_tid;
static void *InterfaceEventHandler_thrd(void *data)
{
    UNREFERENCED_PARAMETER(data);
    async_id_t interface_asyncid;
    async_id_t interface_XHS_asyncid;

    CcspTraceWarning(("%s started\n",__FUNCTION__));
    sysevent_fd_1 = sysevent_open("127.0.0.1", SE_SERVER_WELL_KNOWN_PORT, SE_VERSION, "Interface_evt_handler", &sysevent_token_1);

    sysevent_set_options(sysevent_fd_1, sysevent_token_1, "multinet_6-status", TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_fd_1, sysevent_token_1, "multinet_6-status",  &interface_asyncid);
    sysevent_set_options(sysevent_fd_1, sysevent_token_1, "multinet_2-status", TUPLE_FLAG_EVENT);
    sysevent_setnotification(sysevent_fd_1, sysevent_token_1, "multinet_2-status",  &interface_XHS_asyncid);
    while(1)
    {
         async_id_t getnotification_asyncid;
         int err;
         unsigned char name[25], val[42],buf[128],cmd[128];
         int namelen = sizeof(name);
         int vallen  = sizeof(val);
        err = sysevent_getnotification(sysevent_fd_1, sysevent_token_1, name, &namelen,  val, &vallen, &getnotification_asyncid);

            if (err)
            {
                   CcspTraceWarning(("sysevent_getnotification failed with error: %d %s\n", err,__FUNCTION__));
                   CcspTraceWarning(("sysevent_getnotification failed name: %s val : %s\n", name,val));
            if ( 0 != system("pidof syseventd")) {

                       CcspTraceWarning(("%s syseventd not running ,breaking the receive notification loop \n",__FUNCTION__));
                break;
            }
            }
            else
            {

                   CcspTraceWarning(("%s Recieved notification event  %s\n",__FUNCTION__,name));
            if(strcmp((const char*)name,"multinet_6-status") == 0)
            {
                if(strcmp((const char*)val, "ready") == 0)
                {
                                        sysevent_get(sysevent_fd, sysevent_token,"br106_ipaddr_v6", buf, sizeof(buf));
                                        memset(cmd,0,sizeof(cmd));
                                        snprintf(cmd, sizeof(cmd)-1, "ip -6 route add %s dev br106",buf);
                    system(cmd);
                    #ifdef _COSA_INTEL_XB3_ARM_
                                        memset(cmd,0,sizeof(cmd));
                                        _ansc_sprintf(cmd, "ip -6 route add %s dev br106 table erouter",buf);
                    system(cmd);
                    #endif
                                        memset(cmd,0,sizeof(cmd));
                                        snprintf(cmd,sizeof(cmd)-1, "ip -6 rule add iif br106 lookup erouter");
                                        system(cmd);
                }

            }
            if(strcmp((const char*)name,"multinet_2-status") == 0)
            {
                if(strcmp((const char*)val, "ready") == 0)
                {
                    char *Inf_name = NULL;
                    int retPsmGet = CCSP_SUCCESS;
                    retPsmGet = PSM_Get_Record_Value2(bus_handle,g_Subsystem, "dmsb.l2net.2.Port.1.Name", NULL, &Inf_name);
                            if (retPsmGet == CCSP_SUCCESS)
                    {
                        char tbuff[100];
                                        memset(cmd,0,sizeof(cmd));
                        memset(tbuff,0,sizeof(tbuff));
                        snprintf(cmd, sizeof(cmd)-1,"sysctl net.ipv6.conf.%s.autoconf",Inf_name);
                        _get_shell_output(cmd, tbuff, sizeof(tbuff));
                        if(tbuff[strlen(tbuff)-1] == '0')
                        {
                            memset(cmd,0,sizeof(cmd));
                            snprintf(cmd, sizeof(cmd)-1, "sysctl -w net.ipv6.conf.%s.autoconf=1",Inf_name);
                            system(cmd);
                            memset(cmd,0,sizeof(cmd));
                            snprintf(cmd, sizeof(cmd)-1,"ifconfig %s down;ifconfig %s up",Inf_name,Inf_name);
                            system(cmd);
                        }

                                        memset(cmd,0,sizeof(cmd));
                                        snprintf(cmd,sizeof(cmd)-1, "%s_ipaddr_v6",Inf_name);
                                        sysevent_get(sysevent_fd, sysevent_token,cmd, buf, sizeof(buf));
                                        memset(cmd,0,sizeof(cmd));
                                        snprintf(cmd,sizeof(cmd)-1, "ip -6 route add %s dev %s",buf,Inf_name);
                        system(cmd);
                        #ifdef _COSA_INTEL_XB3_ARM_
                                        memset(cmd,0,sizeof(cmd));
                                        _ansc_sprintf(cmd, "ip -6 route add %s dev %s table erouter",buf,Inf_name);
                        system(cmd);
                        #endif
                                        memset(cmd,0,sizeof(cmd));
                                        snprintf(cmd,sizeof(cmd)-1, "ip -6 rule add iif %s lookup erouter",Inf_name);
                                        system(cmd);
                        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(Inf_name);
                    }
                    else
                    {
                        CcspTraceWarning(("%s PSM get failed for interface name\n", __FUNCTION__));
                    }
                }

            }

        }
    }

}
static int sysevent_fd_global = 0;
static token_t sysevent_token_global;

static void *
dhcpv6c_dbg_thrd(void * in)
{
    UNREFERENCED_PARAMETER(in);
    int fd=0 , fd1=0;
    char msg[1024] = {0};
    int i;
    char * p = NULL;
    char globalIP2[128] = {0};
    char out[128] = {0};
    //When PaM restart, this is to get previous addr.
    sysevent_get(sysevent_fd, sysevent_token,"lan_ipaddr_v6", globalIP2, sizeof(globalIP2));
    if ( globalIP2[0] )
        CcspTraceWarning(("%s  It seems there is old value(%s)\n", __FUNCTION__, globalIP2));

    sysevent_fd_global = sysevent_open("127.0.0.1", SE_SERVER_WELL_KNOWN_PORT, SE_VERSION, "Multinet Status", &sysevent_token_global);

    CcspTraceWarning(("%s sysevent_fd_global is %d\n", __FUNCTION__, sysevent_fd_global));

    unsigned char lan_multinet_state[16] ;

    int return_val=0;

    fd_set rfds;
    struct timeval tm;

    fd= open(CCSP_COMMON_FIFO, O_RDWR);

    if (fd< 0)
    {
        fprintf(stderr,"open common fifo!!!!!!!!!!!!\n");
        goto EXIT;
    }

    while (1)
    {
    int retCode = 0;
        tm.tv_sec  = 60;
        tm.tv_usec = 0;

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        retCode = select( (fd+1), &rfds, NULL, NULL, &tm);
        /* When return -1, it's error.
           When return 0, it's timeout
           When return >0, it's the number of valid fds */
        if (retCode < 0) {
        fprintf(stderr, "dbg_thrd : select returns error \n" );

            if (errno == EINTR)
                continue;

            CcspTraceWarning(("%s -- select(): %s", __FUNCTION__, strerror(errno)));
            goto EXIT;
        }
    else if(retCode == 0 )
        continue;
        if ( FD_ISSET(fd, &rfds) )
        {
         memset(msg, 0, sizeof(msg));
             read(fd, msg, sizeof(msg));

        }
    else
        continue;

        if (msg[0] != 0)
        {
            CcspTraceInfo(("%s: get message %s\n", __func__, msg));
        } else {
        //Message is empty. Wait 5 sec before trying the select again.
            sleep(5);
            continue;
        }

        if (!strncmp(msg, "dibbler-client", strlen("dibbler-client")))
        {
            char v6addr[64] = {0};
            char v6pref[128] = {0};
            char v6pref_addr[128] = {0};
            char v6Tpref[128] = {0};
            int pref_len = 0;

            char iana_t1[32]    = {0};
            char iana_t2[32]    = {0};
            char iana_iaid[32]  = {0};
            char iana_pretm[32] = {0};
            char iana_vldtm[32] = {0};

            char iapd_t1[32]    = {0};
            char iapd_t2[32]    = {0};
            char iapd_iaid[32]  = {0};
            char iapd_pretm[32] = {0};
            char iapd_vldtm[32] = {0};

            int t1 = 0;
            int idx = 0;
            char action[64] = {0};
            char objName[128] = {0};
            char globalIP[128] = {0};
            BOOL bRestartLan = FALSE;
            int  ret = 0;
#ifdef _HUB4_PRODUCT_REQ_
            char ula_address[64] = {0};
#endif
            /*the format is :
             add 2000::ba7a:1ed4:99ea:cd9f :: 0 t1
             action, address, prefix, pref_len 3600
            now action only supports "add", "del"*/

            p = msg+strlen("dibbler-client");
            while(isblank(*p)) p++;

            fprintf(stderr, "%s -- %d !!! get event from v6 client: %s \n", __FUNCTION__, __LINE__,p);

            if (sscanf(p, "%63s %63s %31s %31s %31s %31s %31s %63s %d %31s %31s %31s %31s %31s",
                       action, v6addr,    iana_iaid, iana_t1, iana_t2, iana_pretm, iana_vldtm,
                       v6pref, &pref_len, iapd_iaid, iapd_t1, iapd_t2, iapd_pretm, iapd_vldtm ) == 14)
            {
                if (!strncmp(action, "add", 3))
                {
                    CcspTraceInfo(("%s: add\n", __func__));

// Waiting until private lan interface is ready , so that we can assign global ipv6 address and also start dhcp server.
            while (1)
            {

                    memset(lan_multinet_state,0,sizeof(lan_multinet_state));
                            return_val=sysevent_get(sysevent_fd_global, sysevent_token_global, "multinet_1-status", lan_multinet_state, sizeof(lan_multinet_state));

                            CcspTraceWarning(("%s multinet_1-status is %s, ret val is %d\n",__FUNCTION__,lan_multinet_state,return_val));

                if(strcmp((const char*)lan_multinet_state, "ready") == 0)
                {
                    break;
                }

                sleep(5);
              }

                    /*for now we only support one address, one prefix notify, if need multiple addr/prefix, must modify dibbler-client code*/
                    if (strncmp(v6addr, "::", 2) != 0)
                    {
                        if (strncmp(v6addr, "''", 2) == 0)
                        {
                            sysevent_set(sysevent_fd, sysevent_token, COSA_DML_DHCPV6C_ADDR_SYSEVENT_NAME, "" , 0);
                        }
                        else
                        {
                            sysevent_set(sysevent_fd, sysevent_token, COSA_DML_DHCPV6C_ADDR_SYSEVENT_NAME, v6addr , 0);
                        }
                        sysevent_set(sysevent_fd, sysevent_token, COSA_DML_DHCPV6C_ADDR_IAID_SYSEVENT_NAME,  iana_iaid , 0);
                        sysevent_set(sysevent_fd, sysevent_token, COSA_DML_DHCPV6C_ADDR_T1_SYSEVENT_NAME,    iana_t1 , 0);
                        sysevent_set(sysevent_fd, sysevent_token, COSA_DML_DHCPV6C_ADDR_T2_SYSEVENT_NAME,    iana_t2 , 0);
                        sysevent_set(sysevent_fd, sysevent_token, COSA_DML_DHCPV6C_ADDR_PRETM_SYSEVENT_NAME, iana_pretm , 0);
                        sysevent_set(sysevent_fd, sysevent_token, COSA_DML_DHCPV6C_ADDR_VLDTM_SYSEVENT_NAME, iana_vldtm , 0);
                    }

                    if (strncmp(v6pref, "::", 2) != 0)
                    {
            memset(v6Tpref,0,sizeof(v6Tpref));
            strncpy(v6Tpref,v6pref,sizeof(v6Tpref));
                        /*We just delegate longer and equal 64bits. Use zero to fill in the slot naturally. */
#if defined (MULTILAN_FEATURE)
                        sprintf(v6pref+strlen(v6pref), "/%d", pref_len);
#else
            if ( pref_len >= 64 )
                            sprintf(v6pref+strlen(v6pref), "/%d", pref_len);
                        else
                        {
#if defined(CISCO_CONFIG_DHCPV6_PREFIX_DELEGATION)
                            sprintf(v6pref+strlen(v6pref), "/%d", pref_len);
#else
                            sprintf(v6pref+strlen(v6pref), "/%d", 64);
#endif
                        }
#endif
    char cmd[100];
#if defined(CISCO_CONFIG_DHCPV6_PREFIX_DELEGATION) && defined(_CBR_PRODUCT_REQ_)
#else
            char out1[100];
            char *token = NULL;char *pt;
            char s[2] = ",";
            if(pref_len < 64)
            {
                memset(out,0,sizeof(out));
                memset(cmd,0,sizeof(cmd));
                memset(out1,0,sizeof(out1));
                snprintf(cmd,sizeof(cmd)-1, "syscfg get IPv6subPrefix");
                      _get_shell_output(cmd, out, sizeof(out));
                if(!strcmp(out,"true"))
                {
                                static int first = 0;

                memset(out,0,sizeof(out));
                memset(cmd,0,sizeof(cmd));
                snprintf(cmd,sizeof(cmd)-1, "syscfg get IPv6_Interface");
                      _get_shell_output(cmd, out, sizeof(out));
                pt = out;
                while((token = strtok_r(pt, ",", &pt)))
                 {

                    if(GenIPv6Prefix(token,v6Tpref,out1))
                    {
                        char tbuff[100];
                        memset(cmd,0,sizeof(cmd));
                        _ansc_sprintf(cmd, "%s%s",token,"_ipaddr_v6");
                        sysevent_set(sysevent_fd, sysevent_token, cmd, out1 , 0);
                        memset(cmd,0,sizeof(cmd));
                        memset(tbuff,0,sizeof(tbuff));
                        sprintf(cmd,"sysctl net.ipv6.conf.%s.autoconf",token);
                        _get_shell_output(cmd, tbuff, sizeof(tbuff));
                        if(tbuff[strlen(tbuff)-1] == '0')
                        {
                            memset(cmd,0,sizeof(cmd));
                            snprintf(cmd, sizeof(cmd)-1, "sysctl -w net.ipv6.conf.%s.autoconf=1",token);
                            system(cmd);
                            memset(cmd,0,sizeof(cmd));
                            sprintf(cmd,"ifconfig %s down;ifconfig %s up",token,token);
                            system(cmd);
                        }
                        memset(cmd,0,sizeof(cmd));
                        sprintf(cmd, "ip -6 route add %s dev %s", out1, token);
                                    system(cmd);
                        #ifdef _COSA_INTEL_XB3_ARM_
                        memset(cmd,0,sizeof(cmd));
                        sprintf(cmd, "ip -6 route add %s dev %s table erouter", out1, token);
                                    system(cmd);
                        #endif
                        memset(cmd,0,sizeof(cmd));
                        sprintf(cmd, "ip -6 rule add iif %s lookup erouter",token);
                        system(cmd);
                        memset(out1,0,sizeof(out1));
                    }
                }
                                        memset(out,0,sizeof(out));
                                        if(first == 0)
                                        {       first = 1;
                                                pthread_create(&InfEvtHandle_tid, NULL, InterfaceEventHandler_thrd, NULL);
                                        }
                }
            }
#endif

            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6C_PREF_SYSEVENT_NAME, v6pref , 0);
            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6C_PREF_IAID_SYSEVENT_NAME,  iapd_iaid , 0);
            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6C_PREF_IAID_SYSEVENT_NAME,  iapd_iaid , 0);
            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6C_PREF_T1_SYSEVENT_NAME,    iapd_t1 , 0);
            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6C_PREF_T2_SYSEVENT_NAME,    iapd_t2 , 0);
            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6C_PREF_PRETM_SYSEVENT_NAME, iapd_pretm , 0);
            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6C_PREF_VLDTM_SYSEVENT_NAME, iapd_vldtm , 0);


#if defined (MULTILAN_FEATURE)
                        if ((v6addr_prev[0] == '\0') || ( _ansc_strcmp(v6addr_prev, v6pref ) !=0))
                        {
                            _ansc_strncpy( v6addr_prev, v6pref, sizeof(v6pref));
                            sysevent_set(sysevent_fd, sysevent_token,"ipv6-restart", "1", 0);
                        }
                        else
                        {
                            sysevent_set(sysevent_fd, sysevent_token,"ipv6_addr-set", "", 0);
                        }
#endif

#ifdef MULTILAN_FEATURE
/* Service IPv6 will assign IP address and prefix allocation,
   for all lan interfaces.
*/
#if !defined(INTEL_PUMA7) && !defined(_COSA_INTEL_XB3_ARM_)
                        // not the best place to add route, just to make it work
                        // delegated prefix need to route to LAN interface
            memset(cmd,0,sizeof(cmd));
                        sprintf(cmd, "ip -6 route add %s dev %s", v6pref, COSA_DML_DHCPV6_SERVER_IFNAME);
                        system(cmd);
            #ifdef _COSA_INTEL_XB3_ARM_
            memset(cmd,0,sizeof(cmd));
                        sprintf(cmd, "ip -6 route add %s dev %s table erouter", v6pref, COSA_DML_DHCPV6_SERVER_IFNAME);
                        system(cmd);
            #endif
            memset(cmd,0,sizeof(cmd));
                        /* we need save this for zebra to send RA
                           ipv6_prefix           // xx:xx::/yy
                         */
                        sprintf(cmd, "sysevent set ipv6_prefix %s \n",v6pref);
                        system(cmd);
                        CcspTraceWarning(("!run cmd1:%s", cmd));

                        DHCPv6sDmlTriggerRestart(FALSE);
#if defined(_COSA_BCM_ARM_) || defined(INTEL_PUMA7)
                        CcspTraceWarning((" %s dhcpv6_assign_global_ip to brlan0 \n", __FUNCTION__));
                        ret = dhcpv6_assign_global_ip(v6pref, "brlan0", globalIP);
#elif defined _COSA_BCM_MIPS_
                        ret = dhcpv6_assign_global_ip(v6pref, COSA_DML_DHCPV6_SERVER_IFNAME, globalIP);
#else
                        /*We need get a global ip addres */
                        ret = dhcpv6_assign_global_ip(v6pref, "l2sd0", globalIP);
#endif
                        CcspTraceWarning(("%s: globalIP %s globalIP2 %s\n", __func__,
                            globalIP, globalIP2));
                        if ( _ansc_strcmp(globalIP, globalIP2 ) ){
                            bRestartLan = TRUE;

                            //PaM may restart. When this happen, we should not overwrite previous ipv6
                            if ( globalIP2[0] )
                               sysevent_set(sysevent_fd, sysevent_token,"lan_ipaddr_v6_prev", globalIP2, 0);

                            _ansc_strcpy(globalIP2, globalIP);
                        }else{
                                char lanrestart[8] = {0};
                                sysevent_get(sysevent_fd, sysevent_token,"lan_restarted",lanrestart, sizeof(lanrestart));
                                fprintf(stderr,"lan restart staus is %s \n",lanrestart);
                                      if (strcmp("true",lanrestart) == 0)
                                    bRestartLan = TRUE;
                                else
                                    bRestartLan = FALSE;
                        }
                        CcspTraceWarning(("%s: bRestartLan %d\n", __func__, bRestartLan));

                        fprintf(stderr, "%s -- %d !!! ret:%d bRestartLan:%d %s %s \n", __FUNCTION__, __LINE__,ret,  bRestartLan,  globalIP, globalIP2);

                        if ( ret != 0 )
                        {
                            AnscTrace("error, assign global ip error.\n");
                        }else if ( bRestartLan == FALSE ){
                            AnscTrace("Same global IP, Need not restart.\n");
                        }else{
                            /* This is for IP.Interface.1. use */
                            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6S_ADDR_SYSEVENT_NAME, globalIP, 0);

                            /*This is for brlan0 interface */
                            sysevent_set(sysevent_fd, sysevent_token,"lan_ipaddr_v6", globalIP, 0);
                            _ansc_sprintf(cmd, "%d", pref_len);
                            sysevent_set(sysevent_fd, sysevent_token,"lan_prefix_v6", cmd, 0);

                            sysevent_set(sysevent_fd, sysevent_token,"lan-restart", "1", 0);
                        }
#endif
#else
#ifdef _HUB4_PRODUCT_REQ_
                        if ((iapd_vldtm[0]=='\0') || (iapd_pretm[0]=='\0')){
                            strncpy(iapd_pretm, "forever", sizeof(iapd_pretm));
                            strncpy(iapd_vldtm, "forever", sizeof(iapd_vldtm));
                        }
                        sysevent_get(sysevent_fd, sysevent_token,SYSEVENT_FIELD_IPV6_ULA_ADDRESS, ula_address, sizeof(ula_address));
                        if(ula_address[0] != '\0') {
                            snprintf(cmd, sizeof(cmd)-1, "ip -6 addr add %s/64 dev %s", ula_address, COSA_DML_DHCPV6_SERVER_IFNAME);
                            system(cmd);
                        }
                        ret = dhcpv6_assign_global_ip(v6pref, COSA_DML_DHCPV6_SERVER_IFNAME, globalIP);
                        if(ret != 0) {
                            CcspTraceInfo(("Assign global ip error \n"));
                        }
                        else {
                            sysevent_set(sysevent_fd, sysevent_token,"lan_ipaddr_v6", globalIP, 0);
                            snprintf(cmd, sizeof(cmd) -1, "ip -6 addr add %s/64 dev %s valid_lft %s preferred_lft %s",
                                globalIP, COSA_DML_DHCPV6_SERVER_IFNAME, iapd_vldtm, iapd_pretm);
                            CcspTraceInfo(("Going to execute: %s \n", cmd));
                            system(cmd);
                        }
                        if(strlen(v6pref) > 0) {
                            strncpy(v6pref_addr, v6pref, (strlen(v6pref)-5));
                            CcspTraceInfo(("Going to set ::1 address on brlan0 interface \n"));
                            snprintf(cmd,sizeof(cmd) - 1,  "ip -6 addr add %s::1/64 dev %s valid_lft %s preferred_lft %s",
                                v6pref_addr, COSA_DML_DHCPV6_SERVER_IFNAME, iapd_vldtm, iapd_pretm);
                            CcspTraceInfo(("Going to execute: %s \n", cmd));
                            system(cmd);
                        }
                        // send an event to Sky-pro app manager that Global-prefix is set
                        sysevent_set(sysevent_fd, sysevent_token,"lan_prefix_set", globalIP, 0);
                        /**
                        * Send data to wanmanager.
                        */
                        int ipv6_wan_status = 0;
                        char dns_server[256] = {'\0'};
                        unsigned int prefix_pref_time = 0;
                        unsigned int prefix_valid_time = 0;
                        char c;
                        WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceDataByName_locked(PHY_WAN_IF_NAME);
                        if(pWanDmlIfaceData != NULL)
                        {
                            DML_WAN_IFACE* pIfaceData = &(pWanDmlIfaceData->data);
                            if(pIfaceData->IP.pIpcIpv6Data == NULL)
                            {
                                pIfaceData->IP.pIpcIpv6Data = (ipc_dhcpv6_data_t*) malloc(sizeof(ipc_dhcpv6_data_t));
                                if(pIfaceData->IP.pIpcIpv6Data != NULL)
                                {
                                    strncpy(pIfaceData->IP.pIpcIpv6Data->ifname, pIfaceData->Wan.Name, sizeof(pIfaceData->IP.pIpcIpv6Data->ifname));
                                    if(strlen(v6pref) == 0)
                                    {
                                        pIfaceData->IP.pIpcIpv6Data->isExpired = TRUE;
                                    }
                                    else
                                    {
                                        pIfaceData->IP.pIpcIpv6Data->isExpired = FALSE;
                                        pIfaceData->IP.pIpcIpv6Data->prefixAssigned = TRUE;
                                        strncpy(pIfaceData->IP.pIpcIpv6Data->sitePrefix, v6pref, sizeof(pIfaceData->IP.pIpcIpv6Data->sitePrefix));
                                        strncpy(pIfaceData->IP.pIpcIpv6Data->pdIfAddress, "", sizeof(pIfaceData->IP.pIpcIpv6Data->pdIfAddress));
                                        /** DNS servers. **/
                                        sysevent_get(sysevent_fd, sysevent_token,SYSEVENT_FIELD_IPV6_DNS_SERVER, dns_server, sizeof(dns_server));
                                        if (strlen(dns_server) != 0)
                                        {
                                            pIfaceData->IP.pIpcIpv6Data->dnsAssigned = TRUE;
                                            sscanf (dns_server, "%s %s", pIfaceData->IP.pIpcIpv6Data->nameserver,
                                                                         pIfaceData->IP.pIpcIpv6Data->nameserver1);
                                        }
                                        sscanf(iapd_pretm, "%c%u%c", &c, &prefix_pref_time, &c);
                                        sscanf(iapd_vldtm, "%c%u%c", &c, &prefix_valid_time, &c);
                                        pIfaceData->IP.pIpcIpv6Data->prefixPltime = prefix_pref_time;
                                        pIfaceData->IP.pIpcIpv6Data->prefixVltime = prefix_valid_time;
                                        pIfaceData->IP.pIpcIpv6Data->maptAssigned = FALSE;
                                        pIfaceData->IP.pIpcIpv6Data->mapeAssigned = FALSE;
                                        pIfaceData->IP.pIpcIpv6Data->prefixCmd = 0;
                                    }
                                    if (wanmgr_handle_dchpv6_event_data(pIfaceData) != ANSC_STATUS_SUCCESS)
                                    {
                                        CcspTraceError(("[%s-%d] Failed to send dhcpv6 data to wanmanager!!! \n", __FUNCTION__, __LINE__));
                                    }
                                }
                            }
                            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                        }
#endif
                        // not the best place to add route, just to make it work
                        // delegated prefix need to route to LAN interface
                        sprintf(cmd, "ip -6 route add %s dev %s", v6pref, COSA_DML_DHCPV6_SERVER_IFNAME);
                        system(cmd);
            #ifdef _COSA_INTEL_XB3_ARM_
            memset(cmd,0,sizeof(cmd));
                        sprintf(cmd, "ip -6 route add %s dev %s table erouter", v6pref, COSA_DML_DHCPV6_SERVER_IFNAME);
                        system(cmd);
            #endif
                        /* we need save this for zebra to send RA
                           ipv6_prefix           // xx:xx::/yy
                         */
#ifndef _HUB4_PRODUCT_REQ_
                        sprintf(cmd, "sysevent set ipv6_prefix %s \n",v6pref);
                        system(cmd);
#else
                        sprintf(cmd, "sysevent set zebra-restart \n");
                        system(cmd);
#endif
                        CcspTraceWarning(("!run cmd1:%s", cmd));

                        DHCPv6sDmlTriggerRestart(FALSE);

                        /*We need get a global ip addres */
#if defined(_COSA_BCM_ARM_) || defined(INTEL_PUMA7)
                        /*this is for tchxb6*/
                        CcspTraceWarning((" %s dhcpv6_assign_global_ip to brlan0 \n", __FUNCTION__));
                        ret = dhcpv6_assign_global_ip(v6pref, "brlan0", globalIP);
#elif defined _COSA_BCM_MIPS_
                        ret = dhcpv6_assign_global_ip(v6pref, COSA_DML_DHCPV6_SERVER_IFNAME, globalIP);
#else
                        ret = dhcpv6_assign_global_ip(v6pref, "l2sd0", globalIP);
#endif
                        CcspTraceWarning(("%s: globalIP %s globalIP2 %s\n", __func__,
                            globalIP, globalIP2));
                        if ( _ansc_strcmp(globalIP, globalIP2 ) ){
                            bRestartLan = TRUE;

                            //PaM may restart. When this happen, we should not overwrite previous ipv6
                            if ( globalIP2[0] )
                               sysevent_set(sysevent_fd, sysevent_token,"lan_ipaddr_v6_prev", globalIP2, 0);

                            _ansc_strcpy(globalIP2, globalIP);
                        }else{
                char lanrestart[8] = {0};
                    sysevent_get(sysevent_fd, sysevent_token,"lan_restarted",lanrestart, sizeof(lanrestart));
                fprintf(stderr,"lan restart staus is %s \n",lanrestart);
                   if (strcmp("true",lanrestart) == 0)
                    bRestartLan = TRUE;
                    else
                                bRestartLan = FALSE;
            }
                        CcspTraceWarning(("%s: bRestartLan %d\n", __func__, bRestartLan));

                        fprintf(stderr, "%s -- %d !!! ret:%d bRestartLan:%d %s %s \n", __FUNCTION__, __LINE__,ret,  bRestartLan,  globalIP, globalIP2);

                        if ( ret != 0 )
                        {
                            AnscTrace("error, assign global ip error.\n");
                        }else if ( bRestartLan == FALSE ){
                            AnscTrace("Same global IP, Need not restart.\n");
                        }else{
                            /* This is for IP.Interface.1. use */
                            sysevent_set(sysevent_fd, sysevent_token,COSA_DML_DHCPV6S_ADDR_SYSEVENT_NAME, globalIP, 0);

                            /*This is for brlan0 interface */
                            sysevent_set(sysevent_fd, sysevent_token,"lan_ipaddr_v6", globalIP, 0);
                            _ansc_sprintf(cmd, "%d", pref_len);
                            sysevent_set(sysevent_fd, sysevent_token,"lan_prefix_v6", cmd, 0);

                            sysevent_set(sysevent_fd, sysevent_token,"lan-restart", "1", 0);
                        }
#endif
                    }
                }
                else if (!strncmp(action, "del", 3))
                {
                    /*todo*/
                }
#if defined(CISCO_CONFIG_DHCPV6_PREFIX_DELEGATION) && (defined(_CBR_PRODUCT_REQ_) || defined(_BCI_FEATURE_REQ))

#else
        system("sysevent set zebra-restart");
#endif
            }

        }
#ifdef _DEBUG
        else if (!strncmp(msg, "mem", 3))
        {
            /*add the test funcs in the run time.*/

            AnscTraceMemoryTable();
        }
#endif
    }

EXIT:
    if(fd>=0) {
        close(fd);
    }

    if(fd1>=0) {
        close(fd1);
    }

    return NULL;
}



static ANSC_STATUS wanmgr_dchpv6_get_ipc_msg_info(WANMGR_IPV6_DATA* pDhcpv6Data, ipc_dhcpv6_data_t* pIpcIpv6Data)
{
    if((pDhcpv6Data == NULL) || (pIpcIpv6Data == NULL))
    {
        return ANSC_STATUS_FAILURE;
    }
    memcpy(pDhcpv6Data->ifname, pIpcIpv6Data->ifname, BUFLEN_32);
    memcpy(pDhcpv6Data->address, pIpcIpv6Data->address, BUFLEN_48);
    memcpy(pDhcpv6Data->pdIfAddress, pIpcIpv6Data->pdIfAddress, BUFLEN_48);
    memcpy(pDhcpv6Data->nameserver, pIpcIpv6Data->nameserver, BUFLEN_128);
    memcpy(pDhcpv6Data->nameserver1, pIpcIpv6Data->nameserver1, BUFLEN_128);
    memcpy(pDhcpv6Data->domainName, pIpcIpv6Data->domainName, BUFLEN_64);
    memcpy(pDhcpv6Data->sitePrefix, pIpcIpv6Data->sitePrefix, BUFLEN_48);
    pDhcpv6Data->prefixPltime = pIpcIpv6Data->prefixPltime;
    pDhcpv6Data->prefixVltime = pIpcIpv6Data->prefixVltime;
    memcpy(pDhcpv6Data->sitePrefixOld, pIpcIpv6Data->sitePrefixOld, BUFLEN_48);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS wanmgr_handle_dchpv6_event_data(DML_WAN_IFACE* pIfaceData)
{
    if(NULL == pIfaceData)
    {
       return ANSC_STATUS_FAILURE;
    }

    ipc_dhcpv6_data_t* pNewIpcMsg = pIfaceData->IP.pIpcIpv6Data;
    WANMGR_IPV6_DATA* pDhcp6cInfoCur = &(pIfaceData->IP.Ipv6Data);
    if((NULL == pDhcp6cInfoCur) || (NULL == pNewIpcMsg))
    {
       return ANSC_STATUS_BAD_PARAMETER;
    }

    BOOL connected = FALSE;
    char set_value[BUFLEN_64];

    memset(set_value, 0, sizeof(set_value));

    CcspTraceInfo(("prefixAssigned=%d prefixCmd=%d sitePrefix=%s pdIfAddress=%s \n"
                   "prefixPltime=%d prefixVltime=%d\n"
                   "addrAssigned=%d addrCmd=%d address=%s ifname=%s\n"
                   "maptAssigned=%d mapeAssigned=%d\n"
                   "dnsAssigned=%d nameserver=%s,%s aftrAssigned=%d aftr=%s isExpired=%d \n",
                   pNewIpcMsg->prefixAssigned, pNewIpcMsg->prefixCmd, pNewIpcMsg->sitePrefix,
                   pNewIpcMsg->pdIfAddress, pNewIpcMsg->prefixPltime, pNewIpcMsg->prefixVltime,
                   pNewIpcMsg->addrAssigned, pNewIpcMsg->addrCmd, pNewIpcMsg->address, pNewIpcMsg->ifname,
                   pNewIpcMsg->maptAssigned, pNewIpcMsg->mapeAssigned,
                   pNewIpcMsg->dnsAssigned, pNewIpcMsg->nameserver, pNewIpcMsg->nameserver1, pNewIpcMsg->aftrAssigned, pNewIpcMsg->aftr, pNewIpcMsg->isExpired));

    /*Check lease expiry*/
    if (pNewIpcMsg->isExpired)
    {
        CcspTraceInfo(("DHCP6LeaseExpired\n"));
        // update current IPv6 data
        wanmgr_dchpv6_get_ipc_msg_info(&(pIfaceData->IP.Ipv6Data), pNewIpcMsg);
        WanManager_UpdateInterfaceStatus(pIfaceData, WANMGR_IFACE_CONNECTION_IPV6_DOWN);

        //Free buffer
        if (pIfaceData->IP.pIpcIpv6Data != NULL )
        {
            //free memory
            free(pIfaceData->IP.pIpcIpv6Data);
            pIfaceData->IP.pIpcIpv6Data = NULL;
        }

        return ANSC_STATUS_SUCCESS;
    }

    /* dhcp6c receives an IPv6 address for WAN interface */
    if (pNewIpcMsg->addrAssigned)
    {
        if (pNewIpcMsg->addrCmd == IFADDRCONF_ADD)
        {
            CcspTraceInfo(("assigned IPv6 address \n"));
            connected = TRUE;
            if (strcmp(pDhcp6cInfoCur->address, pNewIpcMsg->address))
            {
                syscfg_set_string(SYSCFG_FIELD_IPV6_ADDRESS, pNewIpcMsg->address);
            }
        }
        else /* IFADDRCONF_REMOVE */
        {
            CcspTraceInfo(("remove IPv6 address \n"));
            syscfg_set_string(SYSCFG_FIELD_IPV6_ADDRESS, "");
        }
    }

    /* dhcp6c receives prefix delegation for LAN */
    if (pNewIpcMsg->prefixAssigned && !IS_EMPTY_STRING(pNewIpcMsg->sitePrefix))
    {
        if (pNewIpcMsg->prefixCmd == IFADDRCONF_ADD &&
            pNewIpcMsg->prefixPltime != 0 && pNewIpcMsg->prefixVltime != 0)
        {
            CcspTraceInfo(("assigned prefix=%s \n", pNewIpcMsg->sitePrefix));
            connected = TRUE;

            /* Update the WAN prefix validity time in the persistent storage */
            if (pDhcp6cInfoCur->prefixVltime != pNewIpcMsg->prefixVltime)
            {
                snprintf(set_value, sizeof(set_value), "%d", pNewIpcMsg->prefixVltime);
                sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIELD_IPV6_PREFIXVLTIME, set_value, 0);
            }

            if (pDhcp6cInfoCur->prefixPltime != pNewIpcMsg->prefixPltime)
            {
                snprintf(set_value, sizeof(set_value), "%d", pNewIpcMsg->prefixPltime);
                sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIELD_IPV6_PREFIXPLTIME, set_value, 0);
            }

            if (strcmp(pDhcp6cInfoCur->sitePrefixOld, pNewIpcMsg->sitePrefixOld))
            {
                syscfg_set_string(SYSCFG_FIELD_PREVIOUS_IPV6_PREFIX, pNewIpcMsg->sitePrefixOld);
            }

            if (strcmp(pDhcp6cInfoCur->sitePrefix, pNewIpcMsg->sitePrefix))
            {
                syscfg_set_string(SYSCFG_FIELD_IPV6_PREFIX, pNewIpcMsg->sitePrefix);
            }

            // create global IPv6 address (<prefix>::1)
            char prefix[BUFLEN_64] = {0};
            memset(prefix, 0, sizeof(prefix));

            int index = strcspn(pNewIpcMsg->sitePrefix, "/");
            if (index < strlen(pNewIpcMsg->sitePrefix) && index < sizeof(prefix))
            {
                strncpy(prefix, pNewIpcMsg->sitePrefix, index);                                            // only copy prefix without the prefix length
                snprintf(set_value, sizeof(set_value), "%s1", prefix);                                        // concatenate "1" onto the prefix, which is in the form "xxxx:xxxx:xxxx:xxxx::"
                snprintf(pNewIpcMsg->pdIfAddress, sizeof(pNewIpcMsg->pdIfAddress), "%s/64", set_value); // concatenate prefix address with length "/64"

                if (strcmp(pDhcp6cInfoCur->pdIfAddress, pNewIpcMsg->pdIfAddress))
                {
                    syscfg_set_string(SYSCFG_FIELD_IPV6_PREFIX_ADDRESS, pNewIpcMsg->pdIfAddress);
                    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIELD_TR_BRLAN0_DHCPV6_SERVER_ADDRESS, set_value, 0);
                }

                if (strcmp(pDhcp6cInfoCur->sitePrefix, pNewIpcMsg->sitePrefix) != 0)
                {
                    CcspTraceInfo(("%s %d new prefix = %s, current prefix = %s \n", __FUNCTION__, __LINE__, pNewIpcMsg->sitePrefix, pDhcp6cInfoCur->sitePrefix));
                    strncat(prefix, "/64",sizeof(prefix)-1);
                    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIELD_IPV6_PREFIX, prefix, 0);
                }
            }
        }
        else /* IFADDRCONF_REMOVE: prefix remove */
        {
            /* Validate if the prefix to be removed is the same as the stored prefix */
            if (strcmp(pDhcp6cInfoCur->sitePrefix, pNewIpcMsg->sitePrefix) == 0)
            {
                CcspTraceInfo(("remove prefix \n"));
                syscfg_set_string(SYSCFG_FIELD_IPV6_PREFIX, "");
                syscfg_set_string(SYSCFG_FIELD_PREVIOUS_IPV6_PREFIX, "");
                syscfg_set_string(SYSCFG_FIELD_IPV6_PREFIX_ADDRESS, "");
                WanManager_UpdateInterfaceStatus(pIfaceData, WANMGR_IFACE_CONNECTION_IPV6_DOWN);
            }
        }
    }

    /* dhcp6c receives domain name information */
    if (pNewIpcMsg->domainNameAssigned && !IS_EMPTY_STRING(pNewIpcMsg->domainName))
    {
        CcspTraceInfo(("assigned domain name=%s \n", pNewIpcMsg->domainName));

        if (strcmp(pDhcp6cInfoCur->domainName, pNewIpcMsg->domainName))
        {
            sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIELD_IPV6_DOMAIN, pNewIpcMsg->domainName, 0);
        }
    }

    /* Even when dhcp6c is not used to get the WAN interface IP address,
     *  * use this message as a trigger to check the WAN interface IP.
     *   * Maybe we've been assigned an address by SLAAC.*/
    if (!pNewIpcMsg->addrAssigned)
    {
        char guAddr[IP_ADDR_LENGTH] = {0};
        char guAddrPrefix[IP_ADDR_LENGTH] = {0};
        uint32_t prefixLen = 0;
        ANSC_STATUS r2;

        r2 = WanManager_getGloballyUniqueIfAddr6(pIfaceData->Wan.Name, guAddr, &prefixLen);

        if (ANSC_STATUS_SUCCESS == r2)
        {
            snprintf(guAddrPrefix,sizeof(guAddrPrefix)-1, "%s/%d", guAddr, prefixLen);
            CcspTraceInfo(("Detected GloballyUnique Addr6 %s, mark connection up! \n", guAddrPrefix));
            connected = TRUE;

            if (strcmp(pDhcp6cInfoCur->address, guAddrPrefix))
            {
                syscfg_set_string(SYSCFG_FIELD_IPV6_ADDRESS, guAddrPrefix);
            }
        }
    }

    /*
     * dhcp6c receives AFTR information
     * TODO: should we update aftr even WAN is not connected?
     */
    if (connected && pNewIpcMsg->aftrAssigned && !IS_EMPTY_STRING(pNewIpcMsg->aftr))
    {
        CcspTraceInfo(("assigned aftr=%s \n", pNewIpcMsg->aftr));
    }

    if (connected)
    {
        WANMGR_IPV6_DATA Ipv6DataTemp;
        wanmgr_dchpv6_get_ipc_msg_info(&(Ipv6DataTemp), pNewIpcMsg);

        if (strcmp(Ipv6DataTemp.address, pDhcp6cInfoCur->address) ||
                strcmp(Ipv6DataTemp.pdIfAddress, pDhcp6cInfoCur->pdIfAddress) ||
                strcmp(Ipv6DataTemp.nameserver, pDhcp6cInfoCur->nameserver) ||
                strcmp(Ipv6DataTemp.nameserver1, pDhcp6cInfoCur->nameserver1) ||
                strcmp(Ipv6DataTemp.sitePrefix, pDhcp6cInfoCur->sitePrefix))
        {
            CcspTraceInfo(("IPv6 configuration has been changed \n"));
            pIfaceData->IP.Ipv6Changed = TRUE;
        }
        else
        {
            /*TODO: Revisit this*/
            //call function for changing the prlft and vallft
            if ((WanManager_Ipv6AddrUtil(LAN_BRIDGE_NAME, SET_LFT, pNewIpcMsg->prefixPltime, pNewIpcMsg->prefixVltime) < 0))
            {
                CcspTraceError(("Life Time Setting Failed"));
            }
            sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_RADVD_RESTART, NULL, 0);
#ifdef FEATURE_IPOE_HEALTH_CHECK
            pIfaceData->IP.Ipv6Renewed = TRUE;
#endif
        }
        // update current IPv6 Data
        memcpy(&(pIfaceData->IP.Ipv6Data), &(Ipv6DataTemp), sizeof(WANMGR_IPV6_DATA));
        pIfaceData->IP.Ipv6Status = WAN_IFACE_IPV6_STATE_UP;
    }

#ifdef FEATURE_MAPT
    ipc_mapt_data_t dhcp6cMAPTMsgBodyPrvs;
    CcspTraceNotice(("FEATURE_MAPT: MAP-T Enable %d\n", pNewIpcMsg->maptAssigned));
    if (pNewIpcMsg->maptAssigned)
    {
#ifdef FEATURE_MAPT_DEBUG
        MaptInfo("--------- Got a new event in Wanmanager for MAPT_CONFIG ---------");
#endif
        //get MAP-T previous data
        memcpy(&dhcp6cMAPTMsgBodyPrvs, &(pIfaceData->MAP.dhcp6cMAPTparameters), sizeof(ipc_mapt_data_t));

        if (memcmp(&(pNewIpcMsg->mapt), &dhcp6cMAPTMsgBodyPrvs, sizeof(ipc_mapt_data_t)) != 0)
        {
            pIfaceData->MAP.MaptChanged = TRUE;
            CcspTraceInfo(("MAPT configuration has been changed \n"));
        }

        // store MAP-T parameters locally
        memcpy(&(pIfaceData->MAP.dhcp6cMAPTparameters), &(pNewIpcMsg->mapt), sizeof(ipc_mapt_data_t));

        // update MAP-T flags
        WanManager_UpdateInterfaceStatus(pIfaceData, WANMGR_IFACE_MAPT_START);
    }
    else
    {
#ifdef FEATURE_MAPT_DEBUG
        MaptInfo("--------- Got an event in Wanmanager for MAPT_STOP ---------");
#endif
        // reset MAP-T parameters
        memset(&(pIfaceData->MAP.dhcp6cMAPTparameters), 0, sizeof(ipc_mapt_data_t));
        WanManager_UpdateInterfaceStatus(pIfaceData, WANMGR_IFACE_MAPT_STOP);
    }
#endif // FEATURE_MAPT

    //Free buffer
    if (pIfaceData->IP.pIpcIpv6Data != NULL )
    {
        //free memory
        free(pIfaceData->IP.pIpcIpv6Data);
        pIfaceData->IP.pIpcIpv6Data = NULL;
    }

    return ANSC_STATUS_SUCCESS;
} /* End of ProcessDhcp6cStateChanged() */

void* IPV6CPStateChangeHandler (void *arg)
{
    const char *dhcpcInterface = (char *) arg;
    if(NULL == dhcpcInterface)
    {
        return (void *)ANSC_STATUS_FAILURE;
    }

    pthread_detach(pthread_self());
    WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceDataByName_locked(dhcpcInterface);
    if(pWanDmlIfaceData != NULL)
    {
        DML_WAN_IFACE* pIfaceData = &(pWanDmlIfaceData->data);
        switch (pIfaceData->PPP.IPV6CPStatus)
        {
            case WAN_IFACE_IPV6CP_STATUS_UP:
                pIfaceData->IP.Dhcp6cPid = WanManager_StartDhcpv6Client(dhcpcInterface);
                CcspTraceInfo(("%s %d - Started dibbler-client on interface %s, pid %d \n", __FUNCTION__, __LINE__, dhcpcInterface, pIfaceData->IP.Dhcp6cPid));
                break;
            case WAN_IFACE_IPV6CP_STATUS_DOWN:
                WanManager_StopDhcpv6Client(dhcpcInterface);
                break;
        }

        WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
    }

    free (arg);

    return (void *)ANSC_STATUS_SUCCESS;
}
