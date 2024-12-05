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

#ifndef  _DHCPV6_MSG_APIS_H
#define  _DHCPV6_MSG_APIS_H


#include "ipc_msg.h"

/*
 * DHCP MAPT options macro definitions
 */
#define MAPT_OPTION_S46_RULE            89
#define MAPT_OPTION_S46_BR              90
#define MAPT_OPTION_S46_DMR             91
#define MAPT_OPTION_S46_PORT_PARAMS     93
#define MAPT_OPTION_S46_CONT_MAPT       95

#define BUFLEN_4                        4
#define BUFLEN_8                        8
#define BUFLEN_16                       16
#define BUFLEN_24                       24
#define BUFLEN_32                       32
#define BUFLEN_40                       40
#define BUFLEN_64                       64
#define BUFLEN_128                      128
#define BUFLEN_256                      256
#define BUFLEN_512                      512
#define BUFLEN_1024                     1024

/*
 * Data type Macro definitions
 */
#ifndef VOID
 #define VOID        void
#endif

#ifndef UINT8
 #define UINT8       unsigned char
#endif

#ifndef UINT16
 #define UINT16      unsigned short
#endif

#ifndef UINT32
 #define UINT32      unsigned int
#endif

#ifndef UCHAR
 #define UCHAR       unsigned char
#endif

#ifndef ULONG
 #define ULONG       unsigned long
#endif

#ifndef ULONGLONG
 #define ULONGLONG   unsigned long long
#endif

#ifndef CHAR
 #define CHAR        char
#endif

#ifndef INT8
 #define INT8        char
#endif

#ifndef INT16
 #define INT16       short
#endif

#ifndef INT32
 #define INT32       int
#endif

#ifndef LONG
 #define LONG        long
#endif

#ifndef PVOID
 #define PVOID       void*
#endif

#ifndef PCHAR
 #define PCHAR       char*
#endif

#ifndef PUCHAR
 #define PUCHAR      unsigned char*
#endif

#ifndef PUINT8
 #define PUINT8      unsigned char*
#endif

#ifndef PUINT16
 #define PUINT16     unsigned short*
#endif

#ifndef PUINT32
 #define PUINT32     unsigned int*
#endif

#ifndef PULONG
 #define PULONG      unsigned long*
#endif

#ifndef BOOLEAN
 #define BOOLEAN     unsigned char
#endif

#define MAPT_LOG_INFO(format, ...)     \
                              CcspTraceInfo   (("%s - "format"\n", __FUNCTION__, ##__VA_ARGS__))
#define MAPT_LOG_ERROR(format, ...)    \
                              CcspTraceError  (("%s - "format"\n", __FUNCTION__, ##__VA_ARGS__))
#define MAPT_LOG_NOTICE(format, ...)   \
                              CcspTraceNotice (("%s - "format"\n", __FUNCTION__, ##__VA_ARGS__))
#define MAPT_LOG_WARNING(format, ...)  \
                              CcspTraceWarning(("%s - "format"\n", __FUNCTION__, ##__VA_ARGS__))

#define COSA_DML_WANIface_PREF_SYSEVENT_NAME           "tr_%s_dhcpv6_client_v6pref"
#define COSA_DML_WANIface_PREF_IAID_SYSEVENT_NAME      "tr_%s_dhcpv6_client_pref_iaid"
#define COSA_DML_WANIface_PREF_T1_SYSEVENT_NAME        "tr_%s_dhcpv6_client_pref_t1"
#define COSA_DML_WANIface_PREF_T2_SYSEVENT_NAME        "tr_%s_dhcpv6_client_pref_t2"
#define COSA_DML_WANIface_PREF_PRETM_SYSEVENT_NAME     "tr_%s_dhcpv6_client_pref_pretm"
#define COSA_DML_WANIface_PREF_VLDTM_SYSEVENT_NAME     "tr_%s_dhcpv6_client_pref_vldtm"

#define COSA_DML_WANIface_ADDR_SYSEVENT_NAME           "tr_%s_dhcpv6_client_v6addr"
#define COSA_DML_WANIface_ADDR_IAID_SYSEVENT_NAME      "tr_%s_dhcpv6_client_addr_iaid"
#define COSA_DML_WANIface_ADDR_T1_SYSEVENT_NAME        "tr_%s_dhcpv6_client_addr_t1"
#define COSA_DML_WANIface_ADDR_T2_SYSEVENT_NAME        "tr_%s_dhcpv6_client_addr_t2"
#define COSA_DML_WANIface_ADDR_PRETM_SYSEVENT_NAME     "tr_%s_dhcpv6_client_addr_pretm"
#define COSA_DML_WANIface_ADDR_VLDTM_SYSEVENT_NAME     "tr_%s_dhcpv6_client_addr_vldtm"

/*
 * Enums and structure definition
 */
typedef enum
_RETURN_STATUS
{
   STATUS_SUCCESS = 0,
   STATUS_FAILURE = -1
} RETURN_STATUS;

typedef struct
_COSA_DML_MAPT_DATA
{
   CHAR       RuleIPv4Prefix[BUFLEN_16];
   CHAR       RuleIPv6Prefix[BUFLEN_40];
   CHAR       IPv4AddrString[BUFLEN_16];
   CHAR       IPv6AddrString[BUFLEN_40];
   CHAR       PdIPv6Prefix[BUFLEN_40];
   CHAR       BrIPv6Prefix[BUFLEN_40];
   UCHAR      RuleIPv6PrefixH[BUFLEN_24];
   UINT16     RuleIPv4PrefixLen;
   UINT16     RuleIPv6PrefixLen;
   UINT16     BrIPv6PrefixLen;
   UINT16     PdIPv6PrefixLen;
   UINT16     EaLen;
   UINT16     Psid;
   UINT16     PsidLen;
   UINT32     PsidOffset;
   UINT32     IPv4Suffix;
   UINT16     IPv4Psid;
   UINT16     IPv4PsidLen;
   UINT32     Ratio;
   BOOLEAN    bFMR;
} COSA_DML_MAPT_DATA, *PCOSA_DML_MAPT_DATA;


typedef struct
_COSA_DML_MAPT_OPTION
{
   UINT16     OptType;
   UINT16     OptLen;
} __attribute__ ((__packed__)) COSA_DML_MAPT_OPTION, *PCOSA_DML_MAPT_OPTION;

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
    );
#endif
