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
#include "cosa_mapt_apis.h"
#include "secure_wrapper.h"
/*
 * Macro definitions
 */
#define STATUS_NULL         NULL
#define FIRST_BYTE(byte)    (byte & 0xFF)
#define SECND_BYTE(byte)    (byte & 0xFF00)
#define THIRD_BYTE(byte)    (byte & 0xFF0000)
#define FORTH_BYTE(byte)    (byte & 0xFF000000)


#define SET_BIT_MASK(maskBits, shiftBits) (                                                    \
                                            (ULONGLONG)(0xFFFFFFFFFFFFFFFF << (64-maskBits))   \
                                            >> (64-maskBits-shiftBits)                         \
                                          )

#define SET_RSHIFT_MASK(maskBits) ((ULONGLONG)(0xFFFFFFFFFFFFFFFF >> (64-maskBits)))

#define STRING_TO_HEX(pStr) ( (pStr-'a'<0)? (pStr-'A'<0)? pStr-'0' : pStr-'A'+10 : pStr-'a'+10 )

#define ERR_CHK(x) 
/*
 * Static function prototypes
 */
//PARTHIBAN : disable_mapt_accel
static RETURN_STATUS CosaDmlMaptPrintConfig  (VOID);
//static RETURN_STATUS CosaDmlMaptResetConfig  (VOID);
//static RETURN_STATUS CosaDmlMaptResetClient  (VOID);
//static RETURN_STATUS CosaDmlMaptResetEvents  (VOID);
//static RETURN_STATUS CosaDmlMaptStopServices (VOID);
//static RETURN_STATUS CosaDmlMaptDisplayFeatureStatus (VOID); //PARTHIBAN : check other features
//static RETURN_STATUS CosaDmlMaptFlushDNSv4Entries (VOID);
//static RETURN_STATUS CosaDmlMaptRollback (RB_STATE eState); //PARTHIBAN : Check IGD
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

//PARTHIBAN : Check IGD and Ipv4 DNS
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
  int rc      = -1;

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

  rc = snprintf (pIPv4AddrString, BUFLEN_16, "%d.%d.%d.%d",
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
  int rc      = -1;

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

  rc = snprintf (pIPv6AddrString, BUFLEN_40,
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
  int rc  = -1;
  MAPT_LOG_INFO("Entry");

  if ( !(pIPv6AddrH && pIPv6AddrS) )
  {
       MAPT_LOG_ERROR("NULL inputs(%s - %s) for IPv6 hex to string conversion!",
                       pIPv6AddrH, pIPv6AddrS);
       return STATUS_FAILURE;
  }

  rc = memset(pIPv6AddrS, 0, BUFLEN_40);
  ERR_CHK(rc);

  if ( !inet_ntop(AF_INET6, pIPv6AddrH, pIPv6AddrS, BUFLEN_40) )
  {
       MAPT_LOG_ERROR("Invalid IPv6 hex address");
       return STATUS_FAILURE;
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
  int safec_ret = -1;
  PCHAR faultParam  = NULL;
  componentStruct_t**  ppComponents = NULL;
  parameterValStruct_t value = {"Device.UPnP.Device.UPnPIGD",
                                (PCHAR)arg,
                                ccsp_boolean};

  CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;

  pthread_detach(pthread_self());

  safec_ret = snprintf(dst_pathname_cr, sizeof(dst_pathname_cr), "%s%s", g_Subsystem, CCSP_DBUS_INTERFACE_CR);
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
  int rc = -1;

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

               rc = snprintf (g_stMaptData.RuleIPv4Prefix, sizeof(g_stMaptData.RuleIPv4Prefix),
                               "%d.%d.%d.%d",
                               pCurOption[0], pCurOption[1], pCurOption[2], pCurOption[3]);
               ERR_CHK(rc);

               g_stMaptData.RuleIPv6PrefixLen = *(pCurOption += 4);
               v6ByteLen = g_stMaptData.RuleIPv6PrefixLen / 8;
               v6BitLen  = g_stMaptData.RuleIPv6PrefixLen % 8;
               pCurOption++;

               rc = memset (&v6Addr, 0, sizeof(v6Addr));
               ERR_CHK(rc);
               rc = memcpy (&v6Addr,  pCurOption, (v6ByteLen+(v6BitLen?1:0))*sizeof(CHAR));
               ERR_CHK(rc);

               if ( v6BitLen )
               {
                    *((PCHAR)&v6Addr + v6ByteLen) &= 0xFF << (8 - v6BitLen);
               }

               rc = memcpy (&g_stMaptData.RuleIPv6PrefixH, v6Addr,
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
               rc = memset (&ipv6Addr, 0, sizeof(ipv6Addr));
               ERR_CHK(rc);
               rc = memcpy (&ipv6Addr, pCurOption, g_stMaptData.BrIPv6PrefixLen/8);
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
  int rc = -1;

  rc = snprintf (ruleIPv4Prefix, BUFLEN_40, "%s/%u", g_stMaptData.RuleIPv4Prefix
                                               , g_stMaptData.RuleIPv4PrefixLen);
  ERR_CHK(rc);
  rc = snprintf (ruleIPv6Prefix, BUFLEN_40, "%s/%u", g_stMaptData.RuleIPv6Prefix
                                               , g_stMaptData.RuleIPv6PrefixLen);
  ERR_CHK(rc);
  rc = snprintf (pdIPv6Prefix,   BUFLEN_40, "%s/%u", g_stMaptData.PdIPv6Prefix
                                               , g_stMaptData.PdIPv6PrefixLen);
  ERR_CHK(rc);
  rc = snprintf (brIPv6Prefix,   BUFLEN_40, "%s/%u", g_stMaptData.BrIPv6Prefix
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

/**
 * @brief Parses the MAPT option 95 response.
 *
 * This function processes the MAPT option 95 response, extracts the relevant information and updates ipc_dhcpv6_data_t struct with mapt details
 *
 * @param[in] pPdIPv6Prefix Pointer to the IPv6 prefix.
 * @param[in] pOptionBuf Pointer to the buffer containing the option 95 data.
 * @param[out] dhcpv6_data Pointer to the structure where the parsed DHCPv6 data will be stored.
 *
 * @return ANSC_STATUS indicating the success or failure of the operation.
 */

ANSC_STATUS WanMgr_MaptParseOpt95Response
(
    PCHAR          pPdIPv6Prefix,
    PUCHAR         pOptionBuf,
    ipc_dhcpv6_data_t *dhcpv6_data
)
{
  RETURN_STATUS ret = STATUS_SUCCESS;
  UINT16  uiOptionBufLen = 0;
  int rc = -1;

  MAPT_LOG_INFO("Entry");

  /* Convert the received string buffer into hex stream */
  rc = memset (&g_stMaptData, 0, sizeof(g_stMaptData));
  ERR_CHK(rc);
  if ( CosaDmlMaptConvertStringToHexStream (pOptionBuf, &uiOptionBufLen) != STATUS_SUCCESS )
  {
       MAPT_LOG_ERROR("MAPT string buffer to HexStream conversion Failed !!");
       ret = STATUS_FAILURE;
  }

  /* Store IPv6 prefix and length */
  rc = memcpy (&g_stMaptData.PdIPv6Prefix, pPdIPv6Prefix,
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

     rc = snprintf (dhcpv6_data->mapt.pdIPv6Prefix, BUFLEN_40, "%s", g_stMaptData.PdIPv6Prefix);
     ERR_CHK(rc);
     rc = snprintf (dhcpv6_data->mapt.ruleIPv4Prefix, BUFLEN_40, "%s", g_stMaptData.RuleIPv4Prefix);
     ERR_CHK(rc);
     rc = snprintf (dhcpv6_data->mapt.ruleIPv6Prefix, BUFLEN_40, "%s/%u", g_stMaptData.RuleIPv6Prefix
                                                                      , g_stMaptData.RuleIPv6PrefixLen);
     ERR_CHK(rc);
     rc = snprintf (dhcpv6_data->mapt.brIPv6Prefix,   BUFLEN_40, "%s/%u", g_stMaptData.BrIPv6Prefix
                                                                      , g_stMaptData.BrIPv6PrefixLen);
     ERR_CHK(rc);

     CosaDmlMaptPrintConfig();

     MAPT_LOG_INFO("MAPT configuration complete.\n");
  }

  return ((ret) ? ANSC_STATUS_FAILURE : ANSC_STATUS_SUCCESS);
}
