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

    module: cosa_mapt_apis.c

        For COSA Data Model Library Development.

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        12/07/2021    initial revision.


**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sysevent/sysevent.h>

#include "ccsp_psm_helper.h"
#include "ansc_platform.h"
#include <syscfg/syscfg.h>
#include "ccsp_psm_helper.h"
#include "sys_definitions.h"
//#include "ccsp_custom.h"
#include "cosa_mapt_apis.h"
#include "plugin_main_apis.h"
#include "secure_wrapper.h"
#include "safec_lib_common.h"
#include "cosa_drg_common.h"
/*
 * Macro definitions
 */
#define STATUS_NULL         NULL
#define FIRST_BYTE(byte)    (byte & 0xFF)
#define SECND_BYTE(byte)    (byte & 0xFF00)
#define THIRD_BYTE(byte)    (byte & 0xFF0000)
#define FORTH_BYTE(byte)    (byte & 0xFF000000)

#define RESOLV_CONF_FILE    "/etc/resolv.conf"
#define RESOLV_CONF_FILE_BK "/tmp/resolv_conf.bk"

#define SET_BIT_MASK(maskBits, shiftBits) (                                                    \
                                            (ULONGLONG)(0xFFFFFFFFFFFFFFFF << (64-maskBits))   \
                                            >> (64-maskBits-shiftBits)                         \
                                          )

#define SET_RSHIFT_MASK(maskBits) ((ULONGLONG)(0xFFFFFFFFFFFFFFFF >> (64-maskBits)))

#define STRING_TO_HEX(pStr) ( (pStr-'a'<0)? (pStr-'A'<0)? pStr-'0' : pStr-'A'+10 : pStr-'a'+10 )

/*
 * Static function prototypes
 */
static RETURN_STATUS CosaDmlMaptApplyConfig  (VOID);
static RETURN_STATUS CosaDmlMaptSetEvents    (VOID);
static RETURN_STATUS CosaDmlMaptPrintConfig  (VOID);
static RETURN_STATUS CosaDmlMaptResetConfig  (VOID);
static RETURN_STATUS CosaDmlMaptResetClient  (VOID);
static RETURN_STATUS CosaDmlMaptResetEvents  (VOID);
static RETURN_STATUS CosaDmlMaptStopServices (VOID);
static RETURN_STATUS CosaDmlMaptDisplayFeatureStatus (VOID);
static RETURN_STATUS CosaDmlMaptFlushDNSv4Entries (VOID);
static RETURN_STATUS CosaDmlMaptRollback (RB_STATE eState);
static RETURN_STATUS CosaDmlMaptParseResponse (PUCHAR pOptionBuf, UINT16 uiOptionBufLen);
static RETURN_STATUS CosaDmlMaptGetIPv6StringFromHex (PUCHAR pIPv6AddrH, PCHAR pIPv6AddrS);
static RETURN_STATUS CosaDmlMaptFormulateIPv4Address (UINT32 ipv4Suffix, PCHAR pIPv4AddrString);
static RETURN_STATUS CosaDmlMaptConvertStringToHexStream (PUCHAR pOptionBuf,
                                                          PUINT16 uiOptionBufLen);
static RETURN_STATUS CosaDmlMaptFormulateIPv6Address (PCHAR pPdIPv6Prefix, PCHAR pIPv4AddrSting,
                                                      UINT16 psid, PCHAR pIPv6AddrString);
static RETURN_STATUS CosaDmlMaptComputePsidAndIPv4Suffix (PCHAR pPdIPv6Prefix,
                         UINT16 ui16PdPrefixLen, UINT16 ui16v6PrefixLen, UINT16 ui16v4PrefixLen,
                         PUINT16 pPsid, PUINT16 pPsidLen, PUINT32 pIPv4Suffix);

static PVOID CosaDmlMaptSetUPnPIGDService (PVOID arg);

/*
 * Global definitions
 */
static COSA_DML_MAPT_DATA   g_stMaptData;
static volatile UINT8 g_bEnableUPnPIGD;
static UINT8 g_bRollBackInProgress;
static UINT s_Option95CheckSum = 0;
extern ANSC_HANDLE bus_handle;

/*
 * Static function definitions
 */
static UINT
CosaDmlMaptCalculateChecksum(unsigned char *buf)
{
    unsigned int checksum = 0;

    while (*buf) {
        checksum += *buf;
        buf++;
    }
    return checksum;
}

static RETURN_STATUS
CosaDmlMaptFormulateIPv4Address
(
    UINT32         ipv4Suffix,
    PCHAR          pIPv4AddrString
)
{
  UCHAR   ipv4AddrBytes[BUFLEN_4];
  struct  in_addr ipv4Addr;
  UINT32  ipv4Hex = 0;
  errno_t rc      = -1;

  MAPT_LOG_INFO("Entry");

  if ( inet_pton(AF_INET, g_stMaptData.RuleIPv4Prefix, &ipv4Addr) <= 0 )
  {
       MAPT_LOG_ERROR("Invalid IPv4 address = %s", g_stMaptData.RuleIPv4Prefix);
       return STATUS_FAILURE;
  }

  ipv4Hex = htonl(ipv4Addr.s_addr);

  *(PUINT32)ipv4AddrBytes = FIRST_BYTE((FIRST_BYTE(ipv4Hex) + FIRST_BYTE(ipv4Suffix)))
                          | SECND_BYTE((SECND_BYTE(ipv4Hex) + SECND_BYTE(ipv4Suffix)))
                          | THIRD_BYTE((THIRD_BYTE(ipv4Hex) + THIRD_BYTE(ipv4Suffix)))
                          | FORTH_BYTE((FORTH_BYTE(ipv4Hex) + FORTH_BYTE(ipv4Suffix)));

  rc = sprintf_s (pIPv4AddrString, BUFLEN_16, "%d.%d.%d.%d",
             ipv4AddrBytes[3], ipv4AddrBytes[2], ipv4AddrBytes[1], ipv4AddrBytes[0]);
  ERR_CHK(rc);

  return STATUS_SUCCESS;
}


static RETURN_STATUS
CosaDmlMaptFormulateIPv6Address
(
    PCHAR          pPdIPv6Prefix,
    PCHAR          pIPv4AddrSting,
    UINT16         psid,
    PCHAR          pIPv6AddrString
)
{
  UCHAR   ipv4AddrBytes[BUFLEN_4];
  struct  in6_addr ipv6Addr;
  struct  in_addr  ipv4Addr;
  UINT32  ipv4Hex = 0;
  errno_t rc      = -1;

  MAPT_LOG_INFO("Entry");

  if ( inet_pton(AF_INET, pIPv4AddrSting, &ipv4Addr) <= 0)
  {
       MAPT_LOG_ERROR("Invalid IPv4 address = %s", pIPv4AddrSting);
       return STATUS_FAILURE;
  }

  ipv4Hex = htonl(ipv4Addr.s_addr);

  *(PUINT32)ipv4AddrBytes = FIRST_BYTE(ipv4Hex)
                          | SECND_BYTE(ipv4Hex)
                          | THIRD_BYTE(ipv4Hex)
                          | FORTH_BYTE(ipv4Hex);

  if ( inet_pton(AF_INET6, pPdIPv6Prefix, &ipv6Addr) <= 0)
  {
       MAPT_LOG_ERROR("Invalid IPv6 address = %s", pPdIPv6Prefix);
       return STATUS_FAILURE;
  }

  rc = sprintf_s (pIPv6AddrString, BUFLEN_40,
            "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
             ipv6Addr.s6_addr[0], ipv6Addr.s6_addr[1], ipv6Addr.s6_addr[2], ipv6Addr.s6_addr[3],
             ipv6Addr.s6_addr[4], ipv6Addr.s6_addr[5], ipv6Addr.s6_addr[6], ipv6Addr.s6_addr[7],
             0x0, 0x0, ipv4AddrBytes[3], ipv4AddrBytes[2], ipv4AddrBytes[1], ipv4AddrBytes[0],
             SECND_BYTE(psid)>>8, FIRST_BYTE(psid));
  ERR_CHK(rc);

  MAPT_LOG_INFO("IPv6AddrString is : %s", pIPv6AddrString);

  return STATUS_SUCCESS;
}


static RETURN_STATUS
CosaDmlMaptGetIPv6StringFromHex
(
    PUCHAR         pIPv6AddrH,
    PCHAR          pIPv6AddrS
)
{
  errno_t rc  = -1;
  MAPT_LOG_INFO("Entry");

  if ( !(pIPv6AddrH && pIPv6AddrS) )
  {
       MAPT_LOG_ERROR("NULL inputs(%s - %s) for IPv6 hex to string conversion!",
                       pIPv6AddrH, pIPv6AddrS);
       return STATUS_FAILURE;
  }

  rc = memset_s (pIPv6AddrS, BUFLEN_40, 0, BUFLEN_40);
  ERR_CHK(rc);

  if ( !inet_ntop(AF_INET6, pIPv6AddrH, pIPv6AddrS, BUFLEN_40) )
  {
       MAPT_LOG_ERROR("Invalid IPv6 hex address");
       return STATUS_FAILURE;
  }

  return STATUS_SUCCESS;
}


static RETURN_STATUS
CosaDmlMaptApplyConfig
(
    VOID
)
{
  MAPT_LOG_INFO("Entry");
#if defined (_COSA_BCM_ARM_) && defined (_XB6_PRODUCT_REQ_)
  if ( access("/proc/sys/net/flowmgr/disable_mapt_accel", F_OK) )
  {
       MAPT_LOG_ERROR("Mapt packet acceleration not supported!");
  } else if ( v_secure_system("echo 0 > /proc/sys/net/flowmgr/disable_mapt_accel") )
  {
       MAPT_LOG_ERROR("Failed to enable mapt packet acceleration!");
  }
#endif

  if ( v_secure_system("insmod /lib/modules/`uname -r`/extra/nat46.ko zero_csum_pass=1") )
  {
       MAPT_LOG_ERROR("Failed to load nat46 module!");
       return STATUS_FAILURE;
  }

  if ( v_secure_system("echo add %s > /proc/net/nat46/control", MAPT_INTERFACE) )
  {
       MAPT_LOG_ERROR("Failed to add %s iface in nat46 module!", MAPT_INTERFACE);
       return STATUS_FAILURE;
  }

  if ( v_secure_system("echo config %s local.v4 %s/%d local.v6 %s/%d "
                       "local.style MAP local.ea-len %d local.psid-offset %d "
                       "remote.v4 0.0.0.0/0 remote.v6 %s/%d remote.style RFC6052 "
                       "remote.ea-len 0 remote.psid-offset 0 debug 0 "
                       "local.pd %s/%d > /proc/net/nat46/control",
                       MAPT_INTERFACE,
                       g_stMaptData.RuleIPv4Prefix, g_stMaptData.RuleIPv4PrefixLen,
                       g_stMaptData.RuleIPv6Prefix, g_stMaptData.RuleIPv6PrefixLen,
                       g_stMaptData.EaLen,          g_stMaptData.PsidOffset,
                       g_stMaptData.BrIPv6Prefix,   g_stMaptData.BrIPv6PrefixLen,
                       g_stMaptData.PdIPv6Prefix,   g_stMaptData.PdIPv6PrefixLen) )
  {
       MAPT_LOG_ERROR("Failed to configure map0 iface!");
       return STATUS_FAILURE;
  }
  MAPT_LOG_INFO("Nat46 module loaded and configured successfully.");

  if ( v_secure_system("ip link set %s up", MAPT_INTERFACE) )
  {
       MAPT_LOG_ERROR("Failed to set %s link up!", MAPT_INTERFACE);
       return STATUS_FAILURE;
  }

  if ( v_secure_system("ip route del default") )
  {
       MAPT_LOG_WARNING("Failed to delete default route!");
  }

  if ( v_secure_system("ip ro rep default dev %s mtu %s", MAPT_INTERFACE , MAPT_V4_MTU_SIZE) )
  {
       MAPT_LOG_ERROR("Failed to set %s as default route!", MAPT_INTERFACE);
       return STATUS_FAILURE;
  }

  if ( v_secure_system("ip link set dev %s mtu %s", MAPT_INTERFACE, MAPT_MTU_SIZE) )
  {
       MAPT_LOG_ERROR("Failed to set mtu %s on %s!", MAPT_MTU_SIZE, MAPT_INTERFACE);
       return STATUS_FAILURE;
  }

  if ( v_secure_system("ip -6 route add %s dev %s metric 256 mtu %s",
                        g_stMaptData.IPv6AddrString, MAPT_INTERFACE, MAPT_MTU_SIZE) )
  {
       MAPT_LOG_ERROR("Failed to add %s route on %s!",
                       g_stMaptData.IPv6AddrString, MAPT_INTERFACE);
       return STATUS_FAILURE;
  }
  MAPT_LOG_INFO("Ip routes modified successfully.");

  // override udp timeout for mapt
  if ( v_secure_system("sysctl -w net.netfilter.nf_conntrack_udp_timeout=30") )
  {
       MAPT_LOG_ERROR("Failed to set nf_conntrack_udp_timeout!");
  }

  if ( v_secure_system("sysctl -w net.netfilter.nf_conntrack_udp_timeout_stream=30") )
  {
       MAPT_LOG_ERROR("Failed to set nf_conntrack_udp_timeout_stream!");
  }

  return STATUS_SUCCESS;
}


static PVOID
CosaDmlMaptSetUPnPIGDService
(
     PVOID          arg
)
{
  INT size = 0;
  INT ret  = CCSP_FAILURE;
  extern CHAR g_Subsystem[BUFLEN_32];
  CHAR dst_pathname_cr[BUFLEN_64] = {0};
  errno_t safec_ret = -1;
  PCHAR faultParam  = NULL;
  componentStruct_t**  ppComponents = NULL;
  parameterValStruct_t value = {"Device.UPnP.Device.UPnPIGD",
                                (PCHAR)arg,
                                ccsp_boolean};

  CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;

  pthread_detach(pthread_self());

  safec_ret = sprintf_s(dst_pathname_cr, sizeof(dst_pathname_cr), "%s%s", g_Subsystem, CCSP_DBUS_INTERFACE_CR);
  ERR_CHK(safec_ret);

  ret = CcspBaseIf_discComponentSupportingNamespace(bus_handle
                                     , dst_pathname_cr
                                     , value.parameterName
                                     , g_Subsystem
                                     , &ppComponents
                                     , &size);

  if ( ret != CCSP_SUCCESS || size != 1 ) {
       MAPT_LOG_ERROR("Error: '%s' does not exist|size:%d", value.parameterName, size);
       return STATUS_NULL;
  }

  ret = CcspBaseIf_setParameterValues(bus_handle
                                     , ppComponents[0]->componentName
                                     , ppComponents[0]->dbusPath
                                     , 0
                                     , 0
                                     , (PVOID)&value
                                     , 1
                                     , TRUE
                                     , &faultParam);

  if ( CCSP_SUCCESS != ret )
  {
       MAPT_LOG_ERROR("CcspBaseIf set param failed! ret %d", ret);
       if ( faultParam )
       {
            bus_info->freefunc(faultParam);
       }
       return STATUS_NULL;
  }
  free_componentStruct_t(bus_handle, 1, ppComponents);

  {
       INT cResult = -1;

       safec_ret = strcmp_s((PCHAR)arg, strlen((PCHAR)arg), "true", &cResult);
       ERR_CHK(safec_ret);

       if ( !safec_ret && !cResult )
       {
            g_bEnableUPnPIGD = 0;
            MAPT_LOG_INFO("### Mapt UPnP_IGD service reset.");
       }
       else
       {
            g_bEnableUPnPIGD = 1;
       }
  }

  return STATUS_NULL;
}


static RETURN_STATUS
CosaDmlMaptFlushDNSv4Entries
(
     VOID
)
{
  /* Flush v4 DNS enteries from resolv conf */
  if ( !access(RESOLV_CONF_FILE, F_OK) )
  {
       struct in_addr ipv4Addr;
       FILE*  fp = NULL;
       FILE*  tp = NULL;
       UINT8  overWrite = 0;
       INT32  ret = -1;
       INT32  resComp = -1;
       CHAR   nmSrv[BUFLEN_32]    = {0};
       CHAR   dnsIP[BUFLEN_64]    = {0};
       CHAR   leftOut[BUFLEN_128] = {0};

       if ( !(fp = fopen(RESOLV_CONF_FILE, "r")) )
       {
            MAPT_LOG_ERROR("Error opening %s ", RESOLV_CONF_FILE);
            return STATUS_FAILURE;
       }

       if ( !(tp = fopen(RESOLV_CONF_FILE_BK, "w")) )
       {
            MAPT_LOG_ERROR("Error opening %s", RESOLV_CONF_FILE_BK);
            /* CID 277328 fix */
            fclose(fp);
            return STATUS_FAILURE;
       }

       MAPT_LOG_INFO("#### Remove DNS Entries ####");
       while (memset(nmSrv,   0, sizeof(nmSrv)),
              memset(dnsIP,   0, sizeof(dnsIP)),
              memset(leftOut, 0, sizeof(leftOut)),
              /* CID 277314 Calling risky function fix : Use correct precision specifiers */
              fscanf(fp, "%31s %63s%127[^\n]s\n", nmSrv, dnsIP, leftOut) != EOF)
       {
              ret = strcmp_s(nmSrv, sizeof(nmSrv), "nameserver", &resComp);
              ERR_CHK(ret);

              if ( !ret && !resComp )
              {
                   if ( inet_pton(AF_INET, dnsIP, &ipv4Addr) > 0 )
                   {
                        MAPT_LOG_INFO("DEL %s", dnsIP);
                        overWrite = 1;
                        continue;
                   }
              }
              fprintf(tp, "%s %s%s\n", nmSrv, dnsIP, leftOut);
       }
       MAPT_LOG_INFO("############################");

       fclose(tp);
       fclose(fp);

       if ( overWrite )
       {
            char buf[BUFLEN_256] = {0};
            
            if ( !(fp = fopen(RESOLV_CONF_FILE, "w")) ||
                 !(tp = fopen(RESOLV_CONF_FILE_BK, "r")))
            {
                 MAPT_LOG_ERROR("Copy failed %s -> %s", RESOLV_CONF_FILE, RESOLV_CONF_FILE_BK);
                 /* CID 277328 fix - Resource Leak */
                 if(fp != NULL)
		     fclose(fp);
                 return STATUS_FAILURE;
            }

            while (fgets(buf, sizeof(buf), tp))
            {
                   fputs(buf, fp);
            }

            fclose(tp);
            fclose(fp);
       }

       if ( remove(RESOLV_CONF_FILE_BK) )
       {
            MAPT_LOG_WARNING("Couldn't remove file: %s", RESOLV_CONF_FILE_BK);
       }
  }
  else
  {
     MAPT_LOG_ERROR("File not found: %s", RESOLV_CONF_FILE);
     return STATUS_FAILURE;
  }

  return STATUS_SUCCESS;
}


static RETURN_STATUS
CosaDmlMaptStopServices
(
    VOID
)
{
  char eth_wan_enable_flag[BUFLEN_8] = {0};
  int strcmp_ret = -1;
  errno_t rc = -1;

  /* Bring down DHCPv4 client */
  if ( v_secure_system("service_wan dhcp-release") )
  {
       MAPT_LOG_ERROR("Wan Dhcp Release Failed !!");
  }
  if ( v_secure_system("service_wan dhcp-stop") )
  {
       MAPT_LOG_ERROR("Wan Dhcp Stop Failed !!");
  }

  commonSyseventSet ("current_wan_ipaddr", "0.0.0.0");
  /* Try validating the service stop. status may be? */
  MAPT_LOG_INFO("DHCPv4 client is made down.");

  /* FIXME: Issue in DNS ipv6 resolution when ethwan enabled */
  /* Workaround: We keep the ipv4 entries for name resolution */
  if (0 == syscfg_get(NULL, "eth_wan_enabled", eth_wan_enable_flag, sizeof(eth_wan_enable_flag)))
  {
      rc = strcmp_s(eth_wan_enable_flag, 5, "false", &strcmp_ret);
      ERR_CHK(rc);
      if (0 == strcmp_ret)
      {
          /* Better to have this done in service_wan generically.
	   * For now we will live with this, as this is required
	   * for MAPT devices.
	   */
	  if ( CosaDmlMaptFlushDNSv4Entries() != STATUS_SUCCESS)
	  {
	      MAPT_LOG_ERROR("Failed to flush DNS v4 nameservers!");
	  }
      }
      else
      {
      	MAPT_LOG_INFO("Ethwan mode , bypassing CosaDmlMaptFlushDNSv4Entries()");
      }
  }
  /* Stopping UPnP, if mapt ratio is not 1:1 */
  if ( g_stMaptData.Ratio > 1 )
  {
       CHAR isEnabled[BUFLEN_4] = {0};

       if ( !syscfg_get(NULL, SYSCFG_UPNP_IGD_ENABLED, isEnabled, sizeof(isEnabled))
            && isEnabled[0] == '1')
       {
            pthread_t th_id;
            MAPT_LOG_INFO("Stopping UPnP_IGD service, as MAPT ratio is higher");

            if ( pthread_create(&th_id, NULL, &CosaDmlMaptSetUPnPIGDService, (PVOID)"false") )
            {
                 MAPT_LOG_ERROR("pthread create Failed, to stop UPnP_IGD Service!");
            }
       }
  }

  return STATUS_SUCCESS;
}


static RETURN_STATUS
CosaDmlMaptSetEvents
(
    VOID
)
{
  CHAR    eValue[BUFLEN_128] = {0};
  INT32   ret = 0;
  errno_t rc  = -1;

  MAPT_LOG_INFO("Entry");

  ret |= commonSyseventSet (EVENT_MAPT_TRANSPORT_MODE, "MAPT");
  MAPT_LOG_NOTICE("MAP_MODE: MAPT");

  ret |= commonSyseventSet (EVENT_MAPT_CONFIG_FLAG, "set");

  rc = sprintf_s(eValue, BUFLEN_128, "%u", g_stMaptData.EaLen);
  ERR_CHK(rc);
  ret |= commonSyseventSet(EVENT_MAPT_EA_LENGTH, eValue);

  rc = sprintf_s(eValue, BUFLEN_128, "%u", g_stMaptData.Ratio);
  ERR_CHK(rc);
  ret |= commonSyseventSet (EVENT_MAPT_RATIO, eValue);

  rc = sprintf_s(eValue, BUFLEN_128, "%u", g_stMaptData.Psid);
  ERR_CHK(rc);
  ret |= commonSyseventSet (EVENT_MAPT_PSID_VALUE, eValue);

  rc = sprintf_s(eValue, BUFLEN_128, "%u", g_stMaptData.PsidLen);
  ERR_CHK(rc);
  ret |= commonSyseventSet (EVENT_MAPT_PSID_LENGTH, eValue);

  rc = sprintf_s(eValue, BUFLEN_128, "%u", g_stMaptData.PsidOffset);
  ERR_CHK(rc);
  ret |= commonSyseventSet (EVENT_MAPT_PSID_OFFSET, eValue);

  ret |= commonSyseventSet (EVENT_MAPT_IPADDRESS, g_stMaptData.IPv4AddrString);

  ret |= commonSyseventSet (EVENT_MAPT_IPV6_ADDRESS, g_stMaptData.IPv6AddrString);

  rc = sprintf_s(eValue, BUFLEN_128, "%s/%u", g_stMaptData.BrIPv6Prefix
                                            , g_stMaptData.BrIPv6PrefixLen);
  ERR_CHK(rc);
  ret |= commonSyseventSet (EVENT_MAPT_BR_IPV6_PREFIX, eValue);

  rc = sprintf_s(eValue, BUFLEN_128, "%s/%u", g_stMaptData.RuleIPv4Prefix
                                            , g_stMaptData.RuleIPv4PrefixLen);
  ERR_CHK(rc);
  ret |= commonSyseventSet (EVENT_MAPT_RULE_IPADDRESS, eValue);

  rc = sprintf_s(eValue, BUFLEN_128, "%s/%u", g_stMaptData.RuleIPv6Prefix
                                            , g_stMaptData.RuleIPv6PrefixLen);
  ERR_CHK(rc);
  ret |= commonSyseventSet (EVENT_MAPT_RULE_IPV6_ADDRESS, eValue);
  ret |= commonSyseventSet(EVENT_MAPT_IS_FMR, g_stMaptData.bFMR?"TRUE":"FALSE");

  return ret? STATUS_FAILURE : STATUS_SUCCESS;
}

/*
                :           :           ___/       :
                |  p bits   |          /  q bits   :
                +-----------+         +------------+
                |IPv4 suffix|         |Port Set ID |
                +-----------+         +------------+
                 \          /    ____/    ________/
                   \       :  __/   _____/
                     \     : /     /
 |     n bits         |  o bits   | s bits  |   128-n-o-s bits      |
 +--------------------+-----------+---------+------------+----------+
 |  Rule IPv6 prefix  |  EA bits  |subnet ID|     interface ID      |
 +--------------------+-----------+---------+-----------------------+
 |<---  End-user IPv6 prefix  --->|

EA-bits:
+-------------------+---------+
|IPV4 Address Suffix|   PSID  |
+-------------------+---------+
|--------p----------|----q----+
|--------------o--------------|
*/

static RETURN_STATUS
CosaDmlMaptComputePsidAndIPv4Suffix
(
    PCHAR          pPdIPv6Prefix,
    UINT16         ui16PdPrefixLen,
    UINT16         ui16v6PrefixLen,
    UINT16         ui16v4PrefixLen,
    PUINT16        pPsid,
    PUINT16        pPsidLen,
    PUINT32        pIPv4Suffix
)
{
  UINT8 ui8v4BitIdxLen = 0, ui8PsidBitIdxLen = 0, ui8EaStartByteSetBits = 0;
  UINT8 ui8EaLen       = 0, ui8EaStartByte   = 0, ui8EaLastByte        = 0;
  ULONG ulEaBytes      = 0;
  struct in6_addr ipv6Addr;

  MAPT_LOG_INFO("Entry");

  // V4 suffix bits length
  ui8v4BitIdxLen = BUFLEN_32 - ui16v4PrefixLen;

  // EA bits length
  ui8EaLen = ui16PdPrefixLen - ui16v6PrefixLen;

  // PSID length
  ui8PsidBitIdxLen = ui8EaLen - ui8v4BitIdxLen;

  MAPT_LOG_INFO("<<<Trace>>> ui8v4BitIdxLen(IPV4 Suffix Bits): %u", ui8v4BitIdxLen);
  MAPT_LOG_INFO("<<<Trace>>> ui8EaLen (EA bits)                    : %u", ui8EaLen);
  MAPT_LOG_INFO("<<<Trace>>> ui8PsidBitIdxLen(PSID length)         : %u", ui8PsidBitIdxLen);

  if (ui8EaLen != g_stMaptData.EaLen)
  {
       MAPT_LOG_INFO("Calculated EA-bits and received MAP EA-bits does not match!");
       return STATUS_FAILURE;
  }

  if ( ui16PdPrefixLen < ui16v6PrefixLen )
  {
       MAPT_LOG_ERROR("Invalid MAPT option, ui16PdPrefixLen(%d) < ui16v6PrefixLen(%d)",
		       ui16PdPrefixLen, ui16v6PrefixLen);
       return STATUS_FAILURE;
  }

  if ( inet_pton(AF_INET6, pPdIPv6Prefix, &ipv6Addr) <= 0 )
  {
       MAPT_LOG_ERROR("Invalid IPv6 address = %s", pPdIPv6Prefix);
       return STATUS_FAILURE;
  }

  if ( ui8EaLen )
  {
       UINT8 idx = 0;
       ui8EaStartByte        = ui16v6PrefixLen / 8;
       ui8EaLastByte         = ui8EaStartByte + ui8EaLen/8 + ((ui8EaLen % 8)?1:0);
       ui8EaStartByteSetBits = ui16v6PrefixLen % 8;

       MAPT_LOG_INFO("<<<Trace>>> ui8EaStartByte       : %u", ui8EaStartByte);
       MAPT_LOG_INFO("<<<Trace>>> ui8EaLastByte        : %u", ui8EaLastByte);
       MAPT_LOG_INFO("<<<Trace>>> ui8EaStartByteSetBits : %u", ui8EaStartByteSetBits);

       /* Extracting ui8EaLen/8 bytes of EA bits from Pd IPv6 prefix */
       do
       {
            ulEaBytes = ulEaBytes << 8 | ipv6Addr.s6_addr[ui8EaStartByte + idx];
       } while (idx++, (idx < (ui8EaLastByte - ui8EaStartByte)));

       MAPT_LOG_INFO("<<<Trace>>> No.of bytes extracted: %d, ulEaBytes: 0x%lX", idx, ulEaBytes);

       // If prefix is not aa multiple of 8, get the extra byte and extract the bits
       if ( ui8EaStartByteSetBits )
       {
          MAPT_LOG_INFO("MAP V6 Prefix not in multiples of 8, Prefix = %d", ui16v6PrefixLen);
          ulEaBytes <<= 8;
          ulEaBytes  |= (ipv6Addr.s6_addr[ui8EaLastByte]);

          // push the extra bits out from the last byte
          ulEaBytes >>= (((ui8EaLastByte - ui8EaStartByte) * 8 - ui8EaLen) + (8-ui8EaStartByteSetBits));

          // clear the extra bits from the first EA byte
          ulEaBytes = (ulEaBytes & SET_RSHIFT_MASK(ui8EaLen));

          MAPT_LOG_INFO("<<<Trace>>> ulEaBytes: 0x%lX", ulEaBytes);
       }
       else
       {
          // push the extra bits out from the last byte
          ulEaBytes = (ulEaBytes >> (((ui8EaLastByte-ui8EaStartByte)*8) - ui8EaLen));
          MAPT_LOG_INFO("<<<Trace>>> ulEaBytes: 0x%lX\n", ulEaBytes);
       }

       // p-bits
       *pIPv4Suffix = (ulEaBytes >> ui8PsidBitIdxLen) & SET_RSHIFT_MASK(ui8v4BitIdxLen);

       // q-bits
       *pPsid = (ulEaBytes & (SET_RSHIFT_MASK(ui8PsidBitIdxLen)));
       *pPsidLen = ui8PsidBitIdxLen;
  }

  MAPT_LOG_INFO("IPv4Suffix: %u | Psid: %u | Psidlen: %u",*pIPv4Suffix, *pPsid, *pPsidLen);

  return STATUS_SUCCESS;
}


static RETURN_STATUS
CosaDmlMaptParseResponse
(
    PUCHAR         pOptionBuf,
    UINT16         uiOptionBufLen
)
{
  RETURN_STATUS retStatus = STATUS_SUCCESS;
  PCOSA_DML_MAPT_OPTION  pStartBuf  = NULL;
  PCOSA_DML_MAPT_OPTION  pNxtOption = NULL;
  PCOSA_DML_MAPT_OPTION  pEndBuf    = NULL;
  UINT16  uiOptionLen = 0;
  UINT16  uiOption    = 0;
  PUCHAR  pCurOption  = 0;
  errno_t rc = -1;

  MAPT_LOG_INFO("Entry");

  if ( !(pOptionBuf && *(pOptionBuf+1)) )
  {
       MAPT_LOG_ERROR("MAPT Option-95 Response is NULL !!");
       return STATUS_FAILURE;
  }

  pStartBuf = (PCOSA_DML_MAPT_OPTION)pOptionBuf;
  pEndBuf   = (PCOSA_DML_MAPT_OPTION)((PCHAR)pOptionBuf + uiOptionBufLen);
MAPT_LOG_INFO("<<<TRACE>>> Start : %p | End : %p", pStartBuf,pEndBuf);
  for ( ;pStartBuf + 1 <= pEndBuf; pStartBuf = pNxtOption )
  {
       uiOptionLen = ntohs(pStartBuf->OptLen);
       uiOption    = ntohs(pStartBuf->OptType);

       pCurOption  = (PUCHAR)(pStartBuf + 1);
       pNxtOption  = (PCOSA_DML_MAPT_OPTION)(pCurOption + uiOptionLen);
MAPT_LOG_INFO("<<<TRACE>>> Cur : %p | Nxt : %p", pCurOption,pNxtOption);
MAPT_LOG_INFO("<<<TRACE>>> Opt : %u | OpLen : %u", uiOption,uiOptionLen);

       /* option length field overrun */
       if ( pNxtOption > pEndBuf )
       {
            MAPT_LOG_ERROR("Malformed MAP options!");
            retStatus = STATUS_FAILURE; //STATUS_BAD_PAYLOAD
            break;
       }

       switch ( uiOption )
       {
            case MAPT_OPTION_S46_RULE:
            {
               UINT8 bytesLeftOut  = 0;
               UINT8 v6ByteLen     = 0;
               UINT8 v6BitLen      = 0;
               UCHAR v6Addr[BUFLEN_24];

               g_stMaptData.bFMR    = (*pCurOption & 0x01)? TRUE : FALSE;
               g_stMaptData.EaLen   =  *++pCurOption;
               g_stMaptData.RuleIPv4PrefixLen =  *++pCurOption;
               pCurOption++;

               rc = sprintf_s (g_stMaptData.RuleIPv4Prefix, sizeof(g_stMaptData.RuleIPv4Prefix),
                               "%d.%d.%d.%d",
                               pCurOption[0], pCurOption[1], pCurOption[2], pCurOption[3]);
               ERR_CHK(rc);

               g_stMaptData.RuleIPv6PrefixLen = *(pCurOption += 4);
               v6ByteLen = g_stMaptData.RuleIPv6PrefixLen / 8;
               v6BitLen  = g_stMaptData.RuleIPv6PrefixLen % 8;
               pCurOption++;

               rc = memset_s (&v6Addr, BUFLEN_24, 0, sizeof(v6Addr));
               ERR_CHK(rc);
               rc = memcpy_s (&v6Addr, BUFLEN_24, pCurOption,
                                                  (v6ByteLen+(v6BitLen?1:0))*sizeof(CHAR));
               ERR_CHK(rc);

               if ( v6BitLen )
               {
                    *((PCHAR)&v6Addr + v6ByteLen) &= 0xFF << (8 - v6BitLen);
               }

               rc = memcpy_s (&g_stMaptData.RuleIPv6PrefixH, BUFLEN_24, v6Addr,
                                                         (v6ByteLen+(v6BitLen?1:0))*sizeof(CHAR));
               ERR_CHK(rc);
               CosaDmlMaptGetIPv6StringFromHex (g_stMaptData.RuleIPv6PrefixH,
                                                                     g_stMaptData.RuleIPv6Prefix);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.bFMR              : %d", g_stMaptData.bFMR);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.EaLen             : %u", g_stMaptData.EaLen);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.RuleIPv4PrefixLen : %u", g_stMaptData.RuleIPv4PrefixLen);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.RuleIPv4Prefix    : %s", g_stMaptData.RuleIPv4Prefix);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.RuleIPv6PrefixLen : %u", g_stMaptData.RuleIPv6PrefixLen);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.RuleIPv6Prefix    : %s", g_stMaptData.RuleIPv6Prefix);
               MAPT_LOG_INFO("Parsing OPTION_S46_RULE Successful.");

               /*
                * check port parameter option:
                * prefix6_len, located at 8th byte in rule option, specifies
                * the length of following v6prefix. So the length of port
                * param option, if any, must be rule_opt_len minus 8 minus v6ByteLen minus (1 or 0)
                */
               bytesLeftOut = uiOptionLen - 8 - v6ByteLen - (v6BitLen?1:0);

               g_stMaptData.Ratio = 1 << (g_stMaptData.EaLen -
                                                    (BUFLEN_32 - g_stMaptData.RuleIPv4PrefixLen));
               /* RFC default */
               g_stMaptData.PsidOffset = 6;
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.Ratio             : %u", g_stMaptData.Ratio);
MAPT_LOG_INFO("<<<TRACE>>> bytesLeftOut                   : %u", bytesLeftOut);
               if ( bytesLeftOut > 0 )
               {
                    /* this rule option includes port param option */
                    if ( bytesLeftOut == 8 ) // Only support one port param option per rule option
                    {
                         UINT16 uiSubOptionLen = 0;
                         UINT16 uiSubOption    = 0;

                         pCurOption += v6ByteLen + (v6BitLen?1:0);
                         uiSubOptionLen = ntohs(((PCOSA_DML_MAPT_OPTION)pCurOption)->OptLen);
                         uiSubOption    = ntohs(((PCOSA_DML_MAPT_OPTION)pCurOption)->OptType);

                         if ( uiSubOption == MAPT_OPTION_S46_PORT_PARAMS &&
                              uiSubOptionLen == 4 )
                         {
                              g_stMaptData.PsidOffset  = (*(pCurOption += uiSubOptionLen))?
                                                                 *pCurOption:6;
                              g_stMaptData.PsidLen     = *++pCurOption;

                              if ( !g_stMaptData.EaLen )
                              {
                                   g_stMaptData.Ratio = 1 << g_stMaptData.PsidLen;
                              }
                              /*
                               * RFC 7598: 4.5: 16 bits long. The first k bits on the left of
                               * this field contain the PSID binary value. The remaining (16 - k)
                               * bits on the right are padding zeros.
                               */
                              g_stMaptData.Psid = ntohs(*((PUINT16)++pCurOption)) >>
                                                                      (16 - g_stMaptData.PsidLen);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.Psid       : %u", g_stMaptData.Psid);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.PsidLen    : %u", g_stMaptData.PsidLen);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.PsidOffset : %u", g_stMaptData.PsidOffset);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.Ratio      : %u", g_stMaptData.Ratio);
                              MAPT_LOG_INFO("Parsing OPTION_S46_PORT_PARAM Successful.");
                         }
                         else
                         {
                              MAPT_LOG_WARNING("OPTION_S46_PORT_PARAM option length(%d) varies!"
                                              , uiSubOptionLen);
                         }
                    }
                    else
                    {
                         MAPT_LOG_WARNING("Port param option length(%d) not equal to 8"
                                         , bytesLeftOut);
                    }
               }
               break;
            }

            case MAPT_OPTION_S46_BR:
            {
               MAPT_LOG_WARNING("Parsing OPTION_S46_BR is not supported !");
               //retStatus = STATUS_NOT_SUPPORTED;
               break;
            }

            case MAPT_OPTION_S46_DMR:
            {
               UCHAR ipv6Addr[BUFLEN_24];

               g_stMaptData.BrIPv6PrefixLen = *pCurOption++;

               /* RFC 6052: 2.2: g_stMaptData.BrIPv6PrefixLen%8 must be 0! */
               rc = memset_s (&ipv6Addr, BUFLEN_24, 0, sizeof(ipv6Addr));
               ERR_CHK(rc);
               rc = memcpy_s (&ipv6Addr, BUFLEN_24, pCurOption, g_stMaptData.BrIPv6PrefixLen/8);
               ERR_CHK(rc);

               CosaDmlMaptGetIPv6StringFromHex (ipv6Addr, g_stMaptData.BrIPv6Prefix);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.BrIPv6PrefixLen : %u", g_stMaptData.BrIPv6PrefixLen);
MAPT_LOG_INFO("<<<TRACE>>> g_stMaptData.BrIPv6Prefix    : %s", g_stMaptData.BrIPv6Prefix);
               MAPT_LOG_INFO("Parsing OPTION_S46_DMR Successful.");
               break;
            }

            default:
               MAPT_LOG_ERROR("Unknown or unexpected MAP option : %d | option len : %d"
                             , uiOption, uiOptionLen);
               //retStatus = STATUS_NOT_SUPPORTED;
               break;
       }
  }

  /* Check a parameter from each mandatory options */
  if ( !g_stMaptData.RuleIPv6PrefixLen || !g_stMaptData.BrIPv6PrefixLen)
  {
       MAPT_LOG_ERROR("Mandatory mapt options are missing !");
       retStatus = STATUS_FAILURE;
  }

  return retStatus;
}


static RETURN_STATUS
CosaDmlMaptPrintConfig
(
    VOID
)
{
  CHAR ruleIPv4Prefix[BUFLEN_40] = "\0";
  CHAR ruleIPv6Prefix[BUFLEN_40] = "\0";
  CHAR pdIPv6Prefix[BUFLEN_40]   = "\0";
  CHAR brIPv6Prefix[BUFLEN_40]   = "\0";
  errno_t rc = -1;

  rc = sprintf_s (ruleIPv4Prefix, BUFLEN_40, "%s/%u", g_stMaptData.RuleIPv4Prefix
                                               , g_stMaptData.RuleIPv4PrefixLen);
  ERR_CHK(rc);
  rc = sprintf_s (ruleIPv6Prefix, BUFLEN_40, "%s/%u", g_stMaptData.RuleIPv6Prefix
                                               , g_stMaptData.RuleIPv6PrefixLen);
  ERR_CHK(rc);
  rc = sprintf_s (pdIPv6Prefix,   BUFLEN_40, "%s/%u", g_stMaptData.PdIPv6Prefix
                                               , g_stMaptData.PdIPv6PrefixLen);
  ERR_CHK(rc);
  rc = sprintf_s (brIPv6Prefix,   BUFLEN_40, "%s/%u", g_stMaptData.BrIPv6Prefix
                                               , g_stMaptData.BrIPv6PrefixLen);
  ERR_CHK(rc);

  MAPT_LOG_INFO("\r"
     "+-------------------------------------------------------------+%15s\n"
     "|                      MAP-T Configuration                    |\n"
     "+-------------------+-----------------------------------------+","");
  MAPT_LOG_INFO("\r"
     "| fmr               | %-40s"                                 "|%15s\n"
     "| ealen             | %-40u"                                 "|\n"
     "| psid              | %-40u"                                 "|"
     , g_stMaptData.bFMR?"true":"false", ""
     , g_stMaptData.EaLen
     , g_stMaptData.Psid);
  MAPT_LOG_INFO("\r"
     "| psidlen           | %-40u"                                 "|%15s\n"
     "| psid offset       | %-40u"                                 "|\n"
     "| ratio             | %-40u"                                 "|"
     , g_stMaptData.PsidLen, ""
     , g_stMaptData.PsidOffset
     , g_stMaptData.Ratio);
  MAPT_LOG_INFO("\r"
     "| rule ipv4 prefix  | %-40s"                                 "|%15s\n"
     "| rule ipv6 prefix  | %-40s"                                 "|\n"
     "| ipv6 prefix       | %-40s"                                 "|"
     , ruleIPv4Prefix, ""
     , ruleIPv6Prefix
     , pdIPv6Prefix);
  MAPT_LOG_INFO("\r"
     "| br ipv6 prefix    | %-40s"                                 "|%15s\n"
     "| ipv4 suffix       | %-40u"                                 "|\n"
     "| map ipv4 address  | %-40s"                                 "|"
     , brIPv6Prefix, ""
     , g_stMaptData.IPv4Suffix
     , g_stMaptData.IPv4AddrString);
  MAPT_LOG_INFO("\r"
     "| map ipv6 address  | %-40s"                                 "|%15s\n"
     "+-------------------+-----------------------------------------+\n"
     , g_stMaptData.IPv6AddrString, "");

  return STATUS_SUCCESS;
}


static RETURN_STATUS
CosaDmlMaptResetClient
(
    VOID
)
{
  MAPT_LOG_INFO("Entry");

  FILE* fd = NULL;
  CHAR  outBuf[BUFLEN_256] = {0};

  if ( (fd = v_secure_popen("r", "ps | grep udhcp | grep erouter0")) )
  {
       if ( fgets(outBuf, BUFLEN_256, fd) && !strstr(outBuf, "erouter0") )
       {
            if ( v_secure_system("service_wan dhcp-start") )
            {
                 MAPT_LOG_ERROR("Failed to restore dhclient !");
		 v_secure_pclose(fd);
                 return STATUS_FAILURE;
            }
       }
	v_secure_pclose(fd);
   }
  return STATUS_SUCCESS;
}


static RETURN_STATUS
CosaDmlMaptResetEvents
(
    VOID
)
{
  MAPT_LOG_INFO("Entry");
  MAPT_LOG_NOTICE("MAP_MODE: NONE");
  return commonSyseventSet (EVENT_MAPT_TRANSPORT_MODE,    "NONE")

       | commonSyseventSet (EVENT_MAPT_CONFIG_FLAG,       "")

       | commonSyseventSet (EVENT_MAPT_EA_LENGTH,         "")

       | commonSyseventSet (EVENT_MAPT_RATIO,             "")

       | commonSyseventSet (EVENT_MAPT_PSID_VALUE,        "")

       | commonSyseventSet (EVENT_MAPT_PSID_LENGTH,       "")

       | commonSyseventSet (EVENT_MAPT_PSID_OFFSET,       "")

       | commonSyseventSet (EVENT_MAPT_IPADDRESS,         "")

       | commonSyseventSet (EVENT_MAPT_IPV6_ADDRESS,      "")

       | commonSyseventSet (EVENT_MAPT_BR_IPV6_PREFIX,    "")

       | commonSyseventSet (EVENT_MAPT_RULE_IPADDRESS,    "")

       | commonSyseventSet (EVENT_MAPT_RULE_IPV6_ADDRESS, "")

       | commonSyseventSet(EVENT_MAPT_IS_FMR,             "");
}


static RETURN_STATUS
CosaDmlMaptResetConfig
(
    VOID
)
{
  RETURN_STATUS ret = STATUS_SUCCESS;
  FILE* fd = NULL;
  CHAR  outBuf[BUFLEN_64] = {0};
  errno_t rc = -1;

  MAPT_LOG_INFO("Entry");

  if ( (fd = v_secure_popen("r","ip -4 route show | grep default | grep %s", MAPT_INTERFACE)) )
  {
       if ( fgets(outBuf, BUFLEN_64, fd) && strstr(outBuf, MAPT_INTERFACE) )
       {
            v_secure_system("ip route del default");
       }
       v_secure_pclose(fd);
  }

  if ( (fd = v_secure_popen("r","ip -4 route show | grep default | grep erouter0")) )
  {
       rc = memset_s (&outBuf, BUFLEN_64, 0, sizeof(outBuf));
       ERR_CHK(rc);

       if ( !fgets(outBuf, BUFLEN_64, fd) || !strstr(outBuf, "erouter0") )
       {
            if ( v_secure_system("ip ro rep default dev erouter0") )
            {
                 MAPT_LOG_ERROR("Failed to restore ip routes !");
                 ret = STATUS_FAILURE;
            }
       }
       v_secure_pclose(fd);
  }

  if ( (fd = v_secure_popen("r", "lsmod | grep nat46")) )
  {
       rc = memset_s (&outBuf, BUFLEN_64, 0, sizeof(outBuf));
       ERR_CHK(rc);

       if ( fgets(outBuf, BUFLEN_64, fd) && strstr(outBuf, "nat46") )
       {
            if ( v_secure_system("rmmod nat46.ko") )
            {
                 MAPT_LOG_ERROR("Failed to remove nat46 module !");
                 ret = STATUS_FAILURE;
            }
       }
       v_secure_pclose(fd);
  }

#if defined (_COSA_BCM_ARM_) && defined (_XB6_PRODUCT_REQ_)
  if ( !access("/proc/sys/net/flowmgr/disable_mapt_accel", F_OK) )
  {
       if ( v_secure_system("echo 3 > /proc/sys/net/flowmgr/disable_mapt_accel") )
       {
            MAPT_LOG_ERROR("Failed to disable mapt packet acceleration!");
       }
  }
  else
  {
       MAPT_LOG_WARNING("Mapt packet acceleration is not supported!");
  }
#endif

  return ret;
}


static RETURN_STATUS
CosaDmlMaptRollback
(
    RB_STATE       eState
)
{
  RETURN_STATUS ret = STATUS_SUCCESS;
  MAPT_LOG_INFO("Entry");

  if ( eState & RB_EVENTS )
  {
       ret |= CosaDmlMaptResetEvents();
       MAPT_LOG_INFO("### Mapt events reset.")
  }
  if ( eState & RB_FIREWALL )
  {
       ret |= commonSyseventSet (EVENT_FIREWALL_RESTART, NULL);
       MAPT_LOG_INFO("### Mapt firewall reset.")
  }
  if ( eState & RB_CONFIG )
  {
       ret |= CosaDmlMaptResetConfig();
       MAPT_LOG_INFO("### Mapt config reset.")
  }
  if ( eState & RB_DHCPCLIENT )
  {
       ret |= CosaDmlMaptResetClient();
       MAPT_LOG_INFO("### Mapt dhclient reset.")
  }
  if ( eState & RB_UPNPIGD )
  {
       CHAR isEnabled[BUFLEN_4] = {0};

       syscfg_get(NULL, SYSCFG_UPNP_IGD_ENABLED, isEnabled, sizeof(isEnabled));

       if ( isEnabled[0] == '1' )
       {
            g_bEnableUPnPIGD = 0;
       }

       if ( g_bEnableUPnPIGD )
       {
            pthread_t th_id;
            if ( pthread_create(&th_id, NULL, &CosaDmlMaptSetUPnPIGDService, (PVOID)"true") )
            {
                 MAPT_LOG_ERROR("pthread create Failed, to reset UPnP_IGD Service!");
            }
       }
  }

  if ( eState )
  {
       errno_t rt = -1;
       rt = memset_s (&g_stMaptData, sizeof(g_stMaptData), 0, sizeof(g_stMaptData));
       ERR_CHK(rt);

       if ( !g_bRollBackInProgress )
       {
            MAPT_LOG_INFO("##### Mapt rollback complete #####");
       }
  }

  return ret;
}


static RETURN_STATUS
CosaDmlMaptConvertStringToHexStream
(
    PUCHAR         pWriteBf,
    PUINT16        uiOptionBufLen
)
{
  PUCHAR pReadBf  = pWriteBf;
  MAPT_LOG_INFO("Entry");

  if ( !pWriteBf )
  {
       MAPT_LOG_ERROR("MAPT string buffer is empty !!");
       return STATUS_FAILURE;
  }

  if ( *pReadBf == '\'' && pReadBf++ ) {}

  if ( pReadBf[strlen((PCHAR)pReadBf)-1] == '\'' )
  {
       pReadBf[strlen((PCHAR)pReadBf)-1] = '\0';
  }

MAPT_LOG_INFO("<<<Trace>>> pOptionBuf is %p : %s",pReadBf, pReadBf);
  while ( *pReadBf && *(pReadBf+1) )
  {
       if ( *pReadBf == ':' && pReadBf++ ) {}

       *pWriteBf  = (STRING_TO_HEX(*pReadBf) << 4) | STRING_TO_HEX(*(pReadBf+1));

       ++pWriteBf;
       ++pReadBf;
       ++pReadBf;
       ++*uiOptionBufLen;
  }
  *pWriteBf = '\0';
MAPT_LOG_INFO("<<<Trace>>> BufLen : %d", *uiOptionBufLen);

  return STATUS_SUCCESS;
}

static RETURN_STATUS
CosaDmlMaptDisplayFeatureStatus
(
    VOID
)
{
  RETURN_STATUS ret = STATUS_SUCCESS;
  CHAR isEnabled[BUFLEN_4] = {0};

  if ( !(ret|=syscfg_get(NULL, SYSCFG_UPNP_IGD_ENABLED, isEnabled, sizeof(isEnabled))) )
  {
       if ( '1' == isEnabled[0] )
       {
            MAPT_LOG_INFO("MAP-T_enabled_UPnP_IGD_enabled");
       }
       else
       {
            MAPT_LOG_INFO("MAP-T_enabled_UPnP_IGD_disabled");
       }
  }
  *(PUINT32)isEnabled = 0;

  if ( !(ret|=syscfg_get(NULL, SYSCFG_DMZ_ENABLED, isEnabled, sizeof(isEnabled))) )
  {
       if ( '1' == isEnabled[0] )
       {
            MAPT_LOG_INFO("MAP-T_enabled_UPnP_DMZ_enabled");
       }
       else
       {
            MAPT_LOG_INFO("MAP-T_enabled_DMZ_disabled");
       }
  }
  *(PUINT32)isEnabled = 0;

  if ( !(ret|=syscfg_get(NULL, SYSCFG_PORT_FORWARDING_ENABLED, isEnabled, sizeof(isEnabled))) )
  {
       if ( '1' == isEnabled[0] )
       {
            MAPT_LOG_INFO("MAP-T_enabled_UPnP_Port_Forwarding_enabled");
       }
       else
       {
            MAPT_LOG_INFO("MAP-T_enabled_Port_Forwarding_disabled");
       }
  }
  *(PUINT32)isEnabled = 0;

  if ( !(ret|=syscfg_get(NULL, SYSCFG_PORT_TRIGGERING_ENABLED, isEnabled, sizeof(isEnabled))) )
  {
       if ( '1' == isEnabled[0] )
       {
            MAPT_LOG_INFO("MAP-T_enabled_UPnP_Port_Triggering_enabled");
       }
       else
       {
            MAPT_LOG_INFO("MAP-T_enabled_Port_Triggering_disabled");
       }
  }
  *(PUINT32)isEnabled = 0;

  {
     RETURN_STATUS ret2 = STATUS_SUCCESS;

     if ( !(ret2=syscfg_get(NULL, SYSCFG_MGMT_HTTP_ENABLED, isEnabled, sizeof(isEnabled))) )
     {
          if ( '1' == isEnabled[0] )
          {
               MAPT_LOG_INFO("MAP-T_enabled_User_Remote_Mgt_http_enabled");
          }
          else
          {
               MAPT_LOG_INFO("MAP-T_enabled_User_Remote_Mgt_http_disabled");
          }
     }
//#if defined(CONFIG_CCSP_WAN_MGMT_ACCESS)
     else
     if ( ret2 &&
          !(ret2=syscfg_get(NULL, SYSCFG_MGMT_HTTP_ENABLED"_ert", isEnabled, sizeof(isEnabled))) )
     {
          if ( '1' == isEnabled[0] )
          {
               MAPT_LOG_INFO("MAP-T_enabled_User_Remote_Mgt_http_enabled");
          }
          else
          {
               MAPT_LOG_INFO("MAP-T_enabled_User_Remote_Mgt_http_disabled");
          }
     }
//#endif
     ret |= ret2;
  }
  *(PUINT32)isEnabled = 0;

  if ( !(ret|=syscfg_get(NULL, SYSCFG_MGMT_HTTPS_ENABLED, isEnabled, sizeof(isEnabled))) )
  {
       if ( '1' == isEnabled[0] )
       {
            MAPT_LOG_INFO("MAP-T_enabled_User_Remote_Mgt_https_enabled");
       }
       else
       {
            MAPT_LOG_INFO("MAP-T_enabled_User_Remote_Mgt_https_disabled");
       }
  }

  return ret? STATUS_FAILURE : STATUS_SUCCESS;
}


/*
 * External api definitions
 */
ANSC_STATUS
CosaDmlMaptProcessOpt95Response
(
    PCHAR          pPdIPv6Prefix,
    PUCHAR         pOptionBuf
)
{
  RETURN_STATUS ret = STATUS_SUCCESS;
  UINT16  uiOptionBufLen = 0;
  errno_t rc = -1;

  UINT prevCheckSum = s_Option95CheckSum;

  s_Option95CheckSum = CosaDmlMaptCalculateChecksum(pOptionBuf);
  MAPT_LOG_INFO("Prev option95 checksum: %d, new option95 checksum: %d", prevCheckSum, s_Option95CheckSum);
  if (s_Option95CheckSum == prevCheckSum)
  {
       MAPT_LOG_INFO("No change in received option95 data. No need to configure MAPT again!");
       return ret;
  }
  MAPT_LOG_INFO("Entry");

  /* Check MAPT configuration, if already active, do rollback RB_ALL */
  if ( g_stMaptData.RuleIPv4Prefix[0] )
  {
       MAPT_LOG_INFO("MAPT is configured already, hence calling mapt-Rollback !");
       g_bRollBackInProgress = 1;
       if ( CosaDmlMaptRollback (RB_EVENTS|RB_FIREWALL|RB_CONFIG) != STATUS_SUCCESS )
       {
            MAPT_LOG_ERROR("MAPT rollback failed !!");
       }
  }

  /* Convert the received string buffer into hex stream */
  rc = memset_s (&g_stMaptData, sizeof(g_stMaptData), 0, sizeof(g_stMaptData));
  ERR_CHK(rc);
  if ( CosaDmlMaptConvertStringToHexStream (pOptionBuf, &uiOptionBufLen) != STATUS_SUCCESS )
  {
       MAPT_LOG_ERROR("MAPT string buffer to HexStream conversion Failed !!");
       ret = STATUS_FAILURE;
  }

  /* Store IPv6 prefix and length */
  rc = memcpy_s (&g_stMaptData.PdIPv6Prefix, BUFLEN_40, pPdIPv6Prefix,
                                              (strchr(pPdIPv6Prefix, '/') - pPdIPv6Prefix));
  ERR_CHK(rc);
  g_stMaptData.PdIPv6PrefixLen = strtol((strchr(pPdIPv6Prefix, '/') + 1), NULL, 10);
MAPT_LOG_INFO("<<<Trace>>> Received PdIPv6Prefix : %s/%u", g_stMaptData.PdIPv6Prefix
                                                         , g_stMaptData.PdIPv6PrefixLen);
  if ( !ret && !g_stMaptData.PdIPv6Prefix[0] )
  {
       MAPT_LOG_ERROR("PdIPv6Prefix is NULL !!");
       ret = STATUS_FAILURE;
  }

  /* Parse the hex buffer for mapt options */
  if ( !ret && CosaDmlMaptParseResponse (pOptionBuf, uiOptionBufLen) != STATUS_SUCCESS )
  {
       MAPT_LOG_ERROR("MAPT Parsing Response Failed !!");
       rc = memset_s (&g_stMaptData, sizeof(g_stMaptData), 0, sizeof(g_stMaptData));
       ERR_CHK(rc);
       ret = STATUS_FAILURE;
  }

  /* Derive IPv4 suffix and PSID value */
  if ( !ret && CosaDmlMaptComputePsidAndIPv4Suffix ( g_stMaptData.PdIPv6Prefix
                                                   , g_stMaptData.PdIPv6PrefixLen
                                                   , g_stMaptData.RuleIPv6PrefixLen
                                                   , g_stMaptData.RuleIPv4PrefixLen
                                                   , &g_stMaptData.Psid
                                                   , &g_stMaptData.PsidLen
                                                   , &g_stMaptData.IPv4Suffix) != STATUS_SUCCESS )
  {
       MAPT_LOG_ERROR("MAPT Psid and IPv4 Suffix Computaion Failed !!");
       rc = memset_s (&g_stMaptData, sizeof(g_stMaptData), 0, sizeof(g_stMaptData));
       ERR_CHK(rc);
       ret = STATUS_FAILURE;
  }

  /* Calculate CE IPv4 Address */
  if ( !ret && CosaDmlMaptFormulateIPv4Address ( g_stMaptData.IPv4Suffix
                                               , g_stMaptData.IPv4AddrString) != STATUS_SUCCESS)
  {
       MAPT_LOG_ERROR("MAPT Ipv4 Configuration Failed !!");
       rc = memset_s (&g_stMaptData, sizeof(g_stMaptData), 0, sizeof(g_stMaptData));
       ERR_CHK(rc);
       ret = STATUS_FAILURE;
  }

  /* Calculate CE IPv6 Address */
  if ( !ret &&  CosaDmlMaptFormulateIPv6Address ( g_stMaptData.PdIPv6Prefix
                                                , g_stMaptData.IPv4AddrString
                                                , g_stMaptData.Psid
                                                , g_stMaptData.IPv6AddrString) != STATUS_SUCCESS)
  {
       MAPT_LOG_ERROR("MAPT Ipv6 Configuration Failed !!");
       rc = memset_s (&g_stMaptData, sizeof(g_stMaptData), 0, sizeof(g_stMaptData));
       ERR_CHK(rc);
       ret = STATUS_FAILURE;
  }

  if ( ret )
  {
       if ( g_bRollBackInProgress )
       {
            g_bRollBackInProgress = 0;
            CosaDmlMaptRollback (RB_DHCPCLIENT|RB_UPNPIGD);
       }
       return ANSC_STATUS_FAILURE;
  }

  /* Stop UPnP_IGD and DHCPv4 Client services */
  CosaDmlMaptStopServices();

  /* set mapt sysevents and restart Firewall */
  if ( CosaDmlMaptSetEvents() )
  {
       MAPT_LOG_ERROR("MAPT set events Failed !!");
       CosaDmlMaptRollback (RB_EVENTS|RB_DHCPCLIENT|RB_UPNPIGD);
       return ANSC_STATUS_FAILURE;
  }

  /* Load nat46 module and configure */
  if ( CosaDmlMaptApplyConfig() )
  {
       MAPT_LOG_ERROR("MAPT Apply Configurations Failed !!");
       CosaDmlMaptRollback (RB_CONFIG|RB_EVENTS|RB_DHCPCLIENT|RB_UPNPIGD);
       return ANSC_STATUS_FAILURE;
  }

  /* Restarting firewall to apply mapt rules */
  MAPT_LOG_INFO("MAPT events are set. Triggering firewall-restart");
  commonSyseventSet (EVENT_FIREWALL_RESTART, NULL);
  
  MAPT_LOG_INFO("Triggering ntpd-restart");
  commonSyseventSet (EVENT_NTPD_RESTART, NULL);

  /* Display port based features' status */
  if ( CosaDmlMaptDisplayFeatureStatus() )
  {
       MAPT_LOG_ERROR("MAPT get(syscfg) feature status Failed!");
  }

  CosaDmlMaptPrintConfig();

  MAPT_LOG_INFO("MAPT configuration complete.\n");

  return ANSC_STATUS_SUCCESS;
}

#if defined(MAPT_UNIFICATION_ENABLED)
ANSC_STATUS
CosaDmlMaptParseOpt95Response
(
    PCHAR          pPdIPv6Prefix,
    PUCHAR         pOptionBuf,
    ipc_dhcpv6_data_t *dhcpv6_data
)
{
  RETURN_STATUS ret = STATUS_SUCCESS;
  UINT16  uiOptionBufLen = 0;
  errno_t rc = -1;

  MAPT_LOG_INFO("Entry");

  /* Convert the received string buffer into hex stream */
  rc = memset_s (&g_stMaptData, sizeof(g_stMaptData), 0, sizeof(g_stMaptData));
  ERR_CHK(rc);
  if ( CosaDmlMaptConvertStringToHexStream (pOptionBuf, &uiOptionBufLen) != STATUS_SUCCESS )
  {
       MAPT_LOG_ERROR("MAPT string buffer to HexStream conversion Failed !!");
       ret = STATUS_FAILURE;
  }

  /* Store IPv6 prefix and length */
  rc = memcpy_s (&g_stMaptData.PdIPv6Prefix, BUFLEN_40, pPdIPv6Prefix,
                                              (strchr(pPdIPv6Prefix, '/') - pPdIPv6Prefix));
  ERR_CHK(rc);
  g_stMaptData.PdIPv6PrefixLen = strtol((strchr(pPdIPv6Prefix, '/') + 1), NULL, 10);
  MAPT_LOG_INFO("<<<Trace>>> Received PdIPv6Prefix : %s/%u", g_stMaptData.PdIPv6Prefix
                                                         , g_stMaptData.PdIPv6PrefixLen);
  if ( !ret && !g_stMaptData.PdIPv6Prefix[0] )
  {
       MAPT_LOG_ERROR("PdIPv6Prefix is NULL !!");
       ret = STATUS_FAILURE;
  }

  /* Parse the hex buffer for mapt options */
  if ( !ret && CosaDmlMaptParseResponse (pOptionBuf, uiOptionBufLen) != STATUS_SUCCESS )
  {
       MAPT_LOG_ERROR("MAPT Parsing Response Failed !!");
       ret = STATUS_FAILURE;
  }

  /* Derive IPv4 suffix and PSID value */
  if ( !ret && CosaDmlMaptComputePsidAndIPv4Suffix ( g_stMaptData.PdIPv6Prefix
                                                   , g_stMaptData.PdIPv6PrefixLen
                                                   , g_stMaptData.RuleIPv6PrefixLen
                                                   , g_stMaptData.RuleIPv4PrefixLen
                                                   , &g_stMaptData.IPv4Psid
                                                   , &g_stMaptData.IPv4PsidLen
                                                   , &g_stMaptData.IPv4Suffix) != STATUS_SUCCESS )
  {
       MAPT_LOG_ERROR("MAPT Psid and IPv4 Suffix Computaion Failed !!");
       ret = STATUS_FAILURE;
  }

  /* Calculate CE IPv4 Address */
  if ( !ret && CosaDmlMaptFormulateIPv4Address ( g_stMaptData.IPv4Suffix
                                               , g_stMaptData.IPv4AddrString) != STATUS_SUCCESS)
  {
       MAPT_LOG_ERROR("MAPT Ipv4 Configuration Failed !!");
       ret = STATUS_FAILURE;
  }

  /* Calculate CE IPv6 Address */
  if ( !ret &&  CosaDmlMaptFormulateIPv6Address ( g_stMaptData.PdIPv6Prefix
                                                , g_stMaptData.IPv4AddrString
                                                , g_stMaptData.Psid
                                                , g_stMaptData.IPv6AddrString) != STATUS_SUCCESS)
  {
       MAPT_LOG_ERROR("MAPT Ipv6 Configuration Failed !!");
       ret = STATUS_FAILURE;
  }

  if( ret == STATUS_SUCCESS )
  {
     //Fill Required MAP-T information
     dhcpv6_data->maptAssigned = TRUE;
     dhcpv6_data->mapt.v6Len = g_stMaptData.RuleIPv6PrefixLen;
     dhcpv6_data->mapt.isFMR = g_stMaptData.bFMR ? TRUE : FALSE;
     dhcpv6_data->mapt.eaLen = g_stMaptData.EaLen;
     dhcpv6_data->mapt.v4Len = g_stMaptData.RuleIPv4PrefixLen;
     dhcpv6_data->mapt.psidOffset = g_stMaptData.PsidOffset;
     dhcpv6_data->mapt.psidLen = g_stMaptData.PsidLen;
     dhcpv6_data->mapt.psid = g_stMaptData.Psid;
     dhcpv6_data->mapt.iapdPrefixLen = g_stMaptData.PdIPv6PrefixLen;
     dhcpv6_data->mapt.ratio = g_stMaptData.Ratio;

     rc = sprintf_s (dhcpv6_data->mapt.pdIPv6Prefix, BUFLEN_40, "%s", g_stMaptData.PdIPv6Prefix);
     ERR_CHK(rc);
     rc = sprintf_s (dhcpv6_data->mapt.ruleIPv4Prefix, BUFLEN_40, "%s", g_stMaptData.RuleIPv4Prefix);
     ERR_CHK(rc);
     rc = sprintf_s (dhcpv6_data->mapt.ruleIPv6Prefix, BUFLEN_40, "%s/%u", g_stMaptData.RuleIPv6Prefix
                                                                      , g_stMaptData.RuleIPv6PrefixLen);
     ERR_CHK(rc);
     rc = sprintf_s (dhcpv6_data->mapt.brIPv6Prefix,   BUFLEN_40, "%s/%u", g_stMaptData.BrIPv6Prefix
                                                                      , g_stMaptData.BrIPv6PrefixLen);
     ERR_CHK(rc);

     CosaDmlMaptPrintConfig();

     MAPT_LOG_INFO("MAPT configuration complete.\n");
  }

  return ((ret) ? ANSC_STATUS_FAILURE : ANSC_STATUS_SUCCESS);
}
#endif /** MAPT_UNIFICATION_ENABLED */
