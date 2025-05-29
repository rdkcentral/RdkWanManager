/*********************************************************************************
 If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 Deutsche Telekom AG.
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

*********************************************************************************/
#include <sys/inotify.h>
#include <sysevent/sysevent.h>
#include <math.h>
#include "secure_wrapper.h"
#include "wanmgr_apis.h"
#include "wanmgr_net_utils.h"
#include "wanmgr_map_apis.h"

//Global Variables
extern INT            sysevent_fd;
extern token_t        sysevent_token;
PWAN_PRI_MAP_DOM_FULL g_pDomainFull  = NULL;
static ULONG          maxDomainCount = 0;

//Macros
#define TUNNEL_MTU            1540
#define TUNNEL_NAME           "ip6tnl"
#define NET_STATS_FILE        "/proc/net/dev"
#define BUFF_SIZE_16          16
#define BUFF_SIZE_64          64
#define BUFF_SIZE_1024        1024
#define IPV4_ADDR_LEN_IN_BITS 32
#define BUF_LEN (10 * (sizeof(struct inotify_event) + 255 + 1))

// API declaration 
static INT WanManager_CalculateMAPEPsid(CHAR *pdIPv6Prefix, INT v6PrefixLen, INT v4PrefixLen, INT ea_length,
                                                  USHORT *psidValue, UINT *ipv4IndexValue, UINT *psidLen);
static VOID syscfg_SetDefault(VOID);
static unsigned long long strtoul_custom(char *number);

#define MAX_ULONG 4294967295
static unsigned long long strtoul_custom(char *number)
{
    unsigned long long int value;
    unsigned long int ret;

    if (!number)
    {
        CcspTraceWarning(("Input parameter  is NULL\n"));
        return 0;
    }

    value = strtoull(number, NULL, 10);
    if(value < MAX_ULONG) {
        return value;
    }

    ret = (value - ((value / MAX_ULONG) *MAX_ULONG));
    return ret;
}

/**********************************************************************

    caller:     self

    prototype:

        ULONG
        WanDmlMapInit
            (
                ANSC_HANDLE                 hDml,
                PANSC_HANDLE                phContext
             );
    Description:

        The API initial the entry of MAP in the system.

    Arguments:
                ANSC_HANDLE                 hDml

                PANSC_HANDLE                phContext

    Return:
                  The operation status..

**********************************************************************/
ANSC_STATUS
WanDmlMapInit
    (
        ANSC_HANDLE                 hDml,
        PANSC_HANDLE                phContext
    )
{
    PWAN_DML_MAP_RULE               pRuleList          = NULL;
    ULONG                           ulEntryCount       = 0;
    ULONG                           ulIndex            = 0;
    ULONG                           ulSubIndex         = 0;

    maxDomainCount = 1; //ToDo: Read from PSM

    g_pDomainFull = (PWAN_PRI_MAP_DOM_FULL)AnscAllocateMemory(sizeof(WAN_PRI_MAP_DOM_FULL) * maxDomainCount);

    if ( !g_pDomainFull )
    {
         return ANSC_STATUS_FAILURE;
    }

    for ( ulIndex = 0; ulIndex < maxDomainCount; ulIndex++ )
    {
        g_pDomainFull[ulIndex].Cfg.InstanceNumber   = ulIndex + 1;
        g_pDomainFull[ulIndex].Cfg.bEnabled         = TRUE;
        g_pDomainFull[ulIndex].Cfg.DSCPMarkPolicy   = 0;
        g_pDomainFull[ulIndex].Cfg.IfaceEnable      = TRUE;
        AnscCopyString((CHAR *)g_pDomainFull[ulIndex].Cfg.Alias, "Domain_mape");
        AnscCopyString((CHAR *)g_pDomainFull[ulIndex].Cfg.IfaceAlias, "Interface_tunnel");

        g_pDomainFull[ulIndex].Info.Status          = WAN_DML_DOMAIN_STATUS_Disabled;
        g_pDomainFull[ulIndex].Info.TransportMode   = WAN_DML_DOMAIN_TRANSPORT_MAPE;
        g_pDomainFull[ulIndex].Info.IfaceStatus     = WAN_DML_INTERFACE_STATUS_Down;
        g_pDomainFull[ulIndex].Info.IfaceLastChange = 0;
        AnscCopyString((CHAR *)g_pDomainFull[ulIndex].Info.Name, "MAPE");
        AnscCopyString((CHAR *)g_pDomainFull[ulIndex].Info.IfaceName, TUNNEL_NAME);

        g_pDomainFull[ulIndex].ulNumOfRule          = 1; //ToDo: Read from PSM

        pRuleList = (PWAN_DML_MAP_RULE)AnscAllocateMemory(sizeof(WAN_DML_MAP_RULE) * (g_pDomainFull[ulIndex].ulNumOfRule));

        if ( !pRuleList )
        {
             return ANSC_STATUS_FAILURE;
        }

        for ( ulSubIndex = 0; ulSubIndex < g_pDomainFull[ulIndex].ulNumOfRule; ulSubIndex++ )
        {
            pRuleList[ulSubIndex].InstanceNumber     = ulSubIndex + 1;
            pRuleList[ulSubIndex].Enable             = TRUE;
            pRuleList[ulSubIndex].Status             = WAN_DML_RULE_STATUS_Disabled;
            pRuleList[ulSubIndex].EABitsLength       = 0;
            pRuleList[ulSubIndex].IsFMR              = FALSE;
            pRuleList[ulSubIndex].PSIDOffset         = 0;
            pRuleList[ulSubIndex].PSIDLength         = 0;
            pRuleList[ulSubIndex].PSID               = 0;
            pRuleList[ulSubIndex].IncludeSystemPorts = FALSE;
            snprintf((CHAR *)pRuleList[ulSubIndex].Alias, DML_BUFF_SIZE_32, "Rule_%d", (ulSubIndex + 1));
            AnscCopyString((CHAR *)pRuleList[ulSubIndex].Origin, "DHCPv6");
        }

        g_pDomainFull[ulIndex].pRuleList = pRuleList;
    }

    /* Set default values for map syscfg parameters */
    syscfg_SetDefault();

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    caller:     self

    prototype:

        ULONG
        WanDmlMapDomGetNumberOfEntries
            (
                ANSC_HANDLE                 hContext
             );
    Description:

        The API retrieves the number of MAP domaian in the system.

    Arguments:
                ANSC_HANDLE                 hContext

    Return:
                The number of entries in the Interface table.

**********************************************************************/
ULONG
WanDmlMapDomGetNumberOfEntries
    (
        ANSC_HANDLE                 hContext
    )
{
    return (maxDomainCount);
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        WanDmlMapDomGetEntry
            (
                ANSC_HANDLE                 hContext,
                ULONG                       ulIndex,
                PWAN_DML_MAP_DOM_FULL       pEntry
             );

    Description:
           The API retrieves the complete info of the MAP domain designated by index.
           The usual process is the caller gets the total number of entries, then iterate
           through those by calling this API.
    Arguments:
               ANSC_HANDLE                 hContext,

               ULONG                       ulIndex,

               PWAN_DML_MAP_DOM_FULL       pEntry

    Return:
        The ulIndex entry of the table

**********************************************************************/
ANSC_STATUS
WanDmlMapDomGetEntry
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulIndex,
        PWAN_DML_DOMAIN_FULL        pEntry
    )
{
    if (NULL ==  pEntry)
    {
        AnscTraceFlow(("%s: Null parameter\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    if ((ulIndex >= 0) && (ulIndex < maxDomainCount))
    {
        AnscCopyMemory(&pEntry->Cfg, &g_pDomainFull[ulIndex].Cfg, sizeof(WAN_DML_DOMAIN_CFG));
        AnscCopyMemory(&pEntry->Info, &g_pDomainFull[ulIndex].Info, sizeof(WAN_DML_DOMAIN_INFO));
    }
    else
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
    }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomGetCfg
           (
                ANSC_HANDLE                 hContext,
                PWAN_DML_DOMAIN_CFG         pCfg
           );

    Description:

        API to get the Domain configuration.

    Arguments:
                ANSC_HANDLE                 hContext,

                PWAN_DML_DOMAIN_CFG         pCfg

    Return:
        The status of the operation.

**********************************************************************/
ANSC_STATUS
WanDmlMapDomGetCfg
    (
        ANSC_HANDLE                 hContext,
        PWAN_DML_DOMAIN_CFG         pCfg
    )
{
    UNREFERENCED_PARAMETER(hContext);

    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;

    if (NULL ==  pCfg)
    {
        AnscTraceFlow(("%s: Null parameter\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    AnscCopyMemory(pCfg, &g_pDomainFull[pCfg->InstanceNumber-1].Cfg, sizeof(WAN_DML_DOMAIN_CFG));

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

       PWAN_DML_DOMAIN_CFG
       WanDmlMapDomGetCfg_Data
           (
                ULONG                       ulInstanceNumber
           );

    Description:

        API to get the Domain configuration address

    Arguments:
                ULONG                       ulInstanceNumber

    Return:
        The Domain configuration address.

**********************************************************************/
PWAN_DML_DOMAIN_CFG
WanDmlMapDomGetCfg_Data
    (
        ULONG                       ulInstanceNumber
    )
{
    PWAN_DML_DOMAIN_CFG pDomCfg = NULL;

    if ((ulInstanceNumber > 0) && (ulInstanceNumber <= maxDomainCount))
    {
        pDomCfg = &g_pDomainFull[ulInstanceNumber-1].Cfg;
    }
    else
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
    }

    return pDomCfg;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomSetCfg
          (
                ANSC_HANDLE                 hContext,
                PWAN_DML_DOMAIN_CFG         pCfg
            );

    Description:

        API to set the Domain configuration.

    Arguments:
                ANSC_HANDLE                 hContext,
                PWAN_DML_DOMAIN_CFG         pCfg

    Return:
        The status of the operation.

**********************************************************************/
ANSC_STATUS
WanDmlMapDomSetCfg
    (
        ANSC_HANDLE                 hContext,
        PWAN_DML_DOMAIN_CFG         pCfg
    )
{
    UNREFERENCED_PARAMETER(hContext);

    if (NULL ==  pCfg)
    {
        AnscTraceFlow(("%s: Null parameter\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PWAN_DML_DOMAIN_CFG             pBuf_cfg     = NULL;

    pBuf_cfg = &g_pDomainFull[pCfg->InstanceNumber-1].Cfg;

    if (0 != strcmp(pCfg->Alias, pBuf_cfg->Alias))
    {
        AnscCopyString(pBuf_cfg->Alias, pCfg->Alias);
    }

    if (pCfg->bEnabled != pBuf_cfg->bEnabled)
    {
        pBuf_cfg->bEnabled = pCfg->bEnabled;
    }

    if (0 != strcmp(pCfg->WANInterface, pBuf_cfg->WANInterface))
    {
        AnscCopyString(pBuf_cfg->WANInterface, pCfg->WANInterface);
    }

    if (0 != strcmp(pCfg->IPv6Prefix, pBuf_cfg->IPv6Prefix))
    {
        AnscCopyString(pBuf_cfg->IPv6Prefix, pCfg->IPv6Prefix);
    }

    if (0 != strcmp(pCfg->BRIPv6Prefix, pBuf_cfg->BRIPv6Prefix))
    {
        AnscCopyString(pBuf_cfg->BRIPv6Prefix, pCfg->BRIPv6Prefix);
    }

    if (pCfg->DSCPMarkPolicy != pBuf_cfg->DSCPMarkPolicy)
    {
        pBuf_cfg->DSCPMarkPolicy = pCfg->DSCPMarkPolicy;
    }

    if (pCfg->IfaceEnable != pBuf_cfg->IfaceEnable)
    {
        pBuf_cfg->IfaceEnable = pCfg->IfaceEnable;
    }

    if (0 != strcmp(pCfg->BRIPv6Prefix, pBuf_cfg->BRIPv6Prefix))
    {
        AnscCopyString(pBuf_cfg->BRIPv6Prefix, pCfg->BRIPv6Prefix);
    }

    if (0 != strcmp(pCfg->IfaceAlias, pBuf_cfg->IfaceAlias))
    {
        AnscCopyString(pBuf_cfg->IfaceAlias, pCfg->IfaceAlias);
    }

    if (0 != strcmp(pCfg->IfaceLowerLayers, pBuf_cfg->IfaceLowerLayers))
    {
        AnscCopyString(pBuf_cfg->IfaceLowerLayers, pCfg->IfaceLowerLayers);
    }

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomGetInfo
           (
                ANSC_HANDLE                 hContext,
                ULONG                       ulInstanceNumber,
                PWAN_DML_DOMAIN_INFO        pInfo
           );

    Description:

        API to get the Domain info.

    Arguments:
                ANSC_HANDLE                 hContext,

                ULONG                       ulInstanceNumber,

                PWAN_DML_DOMAIN_INFO        pInfo

    Return:
        The status of the operation.

**********************************************************************/
ANSC_STATUS
WanDmlMapDomGetInfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PWAN_DML_DOMAIN_INFO        pInfo
    )
{
    UNREFERENCED_PARAMETER(hContext);

    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;

    if (NULL ==  pInfo)
    {
        AnscTraceFlow(("%s: Null parameter\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    if ((ulInstanceNumber > 0) && (ulInstanceNumber <= maxDomainCount))
    {
        AnscCopyMemory(pInfo, &g_pDomainFull[ulInstanceNumber-1].Info, sizeof(WAN_DML_DOMAIN_INFO));
    }
    else
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
    }

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomGetInfo_Data
           (
                ULONG                       ulInstanceNumber
           );

    Description:

        API to get the Domain info address.

    Arguments:
                ULONG                       ulInstanceNumber

    Return:
        The Domain info address.

**********************************************************************/
PWAN_DML_DOMAIN_INFO
WanDmlMapDomGetInfo_Data
    (
        ULONG                       ulInstanceNumber
    )
{
    PWAN_DML_DOMAIN_INFO pDomInfo = NULL;

    if ((ulInstanceNumber > 0) && (ulInstanceNumber <= maxDomainCount))
    {
        pDomInfo = &g_pDomainFull[ulInstanceNumber-1].Info;
    }
    else
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
    }

    return pDomInfo;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomSetInfo
          (
                ANSC_HANDLE                 hContext,
                ULONG                       ulInstanceNumber,
                PWAN_DML_DOMAIN_INFO        pInfo
            );

    Description:

        API to set the Domain info.

    Arguments:
                ANSC_HANDLE                 hContext,
                ULONG                       ulInstanceNumber,
                PWAN_DML_DOMAIN_INFO        pInfo

    Return:
        The status of the operation.

**********************************************************************/
ANSC_STATUS
WanDmlMapDomSetInfo
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulInstanceNumber,
        PWAN_DML_DOMAIN_INFO        pInfo
    )
{
    UNREFERENCED_PARAMETER(hContext);

    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PWAN_DML_DOMAIN_INFO            pBuf_info    = NULL;

    if (NULL ==  pInfo)
    {
        AnscTraceFlow(("%s: Null parameter\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    if ((ulInstanceNumber <= 0) || (ulInstanceNumber > maxDomainCount))
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    pBuf_info = &g_pDomainFull[ulInstanceNumber-1].Info;

    if (pInfo->Status != pBuf_info->Status)
    {
        pBuf_info->Status = pInfo->Status;
    }

    if (0 != strcmp(pInfo->Name, pBuf_info->Name))
    {
        AnscCopyString(pBuf_info->Name, pInfo->Name);
    }

    if (pInfo->TransportMode != pBuf_info->TransportMode)
    {
        pBuf_info->TransportMode = pInfo->TransportMode;
    }

    if (pInfo->IfaceStatus != pBuf_info->IfaceStatus)
    {
        pBuf_info->IfaceStatus = pInfo->IfaceStatus;
    }

    if (0 != strcmp(pInfo->IfaceName, pBuf_info->IfaceName))
    {
        AnscCopyString(pBuf_info->IfaceName, pInfo->IfaceName);
    }

    if (pInfo->IfaceLastChange != pBuf_info->IfaceLastChange)
    {
        pBuf_info->IfaceLastChange = pInfo->IfaceLastChange;
    }

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomGetNumberOfRules
           (
                ANSC_HANDLE                 hContext,
                ULONG                           ulIpIfInstanceNumber,
           );

    Description:

        The API retrieves the number of MAP rules in the system.

    Arguments:
                ANSC_HANDLE                 hContext,

                ULONG                       ulIpIfInstanceNumber

    Return:
        The number of MAP Rules.

**********************************************************************/
ULONG
WanDmlMapDomGetNumberOfRules
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulMapDomInstanceNumber
    )
{
    ULONG                           noOfRules    = -1;

    if((ulMapDomInstanceNumber > 0) && (ulMapDomInstanceNumber <= maxDomainCount))
    {
        noOfRules = g_pDomainFull[ulMapDomInstanceNumber - 1].ulNumOfRule;
    }
    else
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
    }

    return noOfRules;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomGetRule
           (
                ANSC_HANDLE                 hContext,
                ULONG                       ulMapDomInstanceNumber,
                ULONG                       ulIndex,
                PWAN_DML_MAP_RULE           pEntry
           );

    Description:

        The API adds one IP interface into the system.

    Arguments:

        ANSC_HANDLE                 hContext,

        ULONG                       ulMapDomInstanceNumber,

        ULONG                       ulIndex,

        PWAN_DML_MAP_RULE           pEntry

    Return:
        The status of the operation.

**********************************************************************/
ANSC_STATUS
WanDmlMapDomGetRule
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulMapDomInstanceNumber,
        ULONG                       ulIndex,
        PWAN_DML_MAP_RULE           pEntry
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    UINT                            maxRuleCount = -1;

    if (NULL ==  pEntry)
    {
        AnscTraceFlow(("%s: Null parameter\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    if((ulMapDomInstanceNumber > 0) && (ulMapDomInstanceNumber <= maxDomainCount))
    {
        maxRuleCount = g_pDomainFull[ulMapDomInstanceNumber - 1].ulNumOfRule;
        if((ulIndex >= 0) && (ulIndex < maxRuleCount))
        {
            AnscCopyMemory(pEntry, &g_pDomainFull[ulMapDomInstanceNumber - 1].pRuleList[ulIndex], sizeof(WAN_DML_MAP_RULE));
        }
        else
        {
            AnscTraceError(("%s:%d:: Invalid Rule instance number\n", __FUNCTION__, __LINE__));
        }
    }
    else
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
    }

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomGetRule_Data
           (
                ULONG                       ulMapDomInstanceNumber,
                ULONG                       ulIndex
           );

    Description:

        The API to get MAP rule address.

    Arguments:
        ULONG                       ulMapDomInstanceNumber,

        ULONG                       ulIndex

    Return:
        The MAP rule address.

**********************************************************************/
PWAN_DML_MAP_RULE
WanDmlMapDomGetRule_Data
    (
        ULONG                       ulMapDomInstanceNumber,
        ULONG                       ulIndex
    )
{
    PWAN_DML_MAP_RULE               pMapRule     = NULL;
    UINT                            maxRuleCount = -1;

    if((ulMapDomInstanceNumber > 0) && (ulMapDomInstanceNumber <= maxDomainCount))
    {
        maxRuleCount = g_pDomainFull[ulMapDomInstanceNumber - 1].ulNumOfRule;
        if((ulIndex >= 0) && (ulIndex < maxRuleCount))
        {
            pMapRule = &g_pDomainFull[ulMapDomInstanceNumber - 1].pRuleList[ulIndex];
        }
        else
        {
            AnscTraceError(("%s:%d:: Invalid Rule instance number\n", __FUNCTION__, __LINE__));
        }
    }
    else
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
    }

    return pMapRule;
}

/**********************************************************************

    caller:     self

    prototype:

       ANSC_STATUS
       WanDmlMapDomSetRule
           (
                ANSC_HANDLE                 hContext,
                ULONG                       ulMapDomInstanceNumber,
                ULONG                       ulIndex,
                PWAN_DML_MAP_RULE           pEntry
           );

    Description:

        API to set the Rule values.

    Arguments:
                ANSC_HANDLE                 hContext,
                ULONG                       ulMapDomInstanceNumber,
                ULONG                       ulIndex,
                PWAN_DML_MAP_RULE           pEntry

    Return:
        The status of the operation.

**********************************************************************/
ANSC_STATUS
WanDmlMapDomSetRule
    (
        ANSC_HANDLE                 hContext,
        ULONG                       ulMapDomInstanceNumber,
        ULONG                       ulIndex,
        PWAN_DML_MAP_RULE           pEntry
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PWAN_DML_MAP_RULE               pBuf_rule    = NULL;
    UINT                            maxRuleCount = -1;

    if (NULL ==  pEntry)
    {
        AnscTraceFlow(("%s: Null parameter\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    if((ulMapDomInstanceNumber < 0) || (ulMapDomInstanceNumber > maxDomainCount))
    {
        AnscTraceError(("%s:%d:: Invalid Domain instance number\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    maxRuleCount = g_pDomainFull[ulMapDomInstanceNumber - 1].ulNumOfRule;
    if((ulIndex < 0) || (ulIndex >= maxRuleCount))
    {
        AnscTraceError(("%s:%d:: Invalid Rule instance number\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    pBuf_rule = &g_pDomainFull[ulMapDomInstanceNumber - 1].pRuleList[ulIndex];

    if (pEntry->Enable != pBuf_rule->Enable)
    {
        pBuf_rule->Enable = pEntry->Enable;
    }

    if (pEntry->Status != pBuf_rule->Status)
    {
        pBuf_rule->Status = pEntry->Status;
    }

    if (0 != strcmp(pEntry->Alias, pBuf_rule->Alias))
    {
        AnscCopyString(pBuf_rule->Alias, pEntry->Alias);
    }

    if (0 != strcmp(pEntry->Origin, pBuf_rule->Origin))
    {
        AnscCopyString(pBuf_rule->Origin, pEntry->Origin);
    }

    if (0 != strcmp(pEntry->IPv6Prefix, pBuf_rule->IPv6Prefix))
    {
        AnscCopyString(pBuf_rule->IPv6Prefix, pEntry->IPv6Prefix);
    }

    if (0 != strcmp(pEntry->IPv4Prefix, pBuf_rule->IPv4Prefix))
    {
        AnscCopyString(pBuf_rule->IPv4Prefix, pEntry->IPv4Prefix);
    }

    if (pEntry->EABitsLength != pBuf_rule->EABitsLength)
    {
        pBuf_rule->EABitsLength = pEntry->EABitsLength;
    }

    if (pEntry->IsFMR != pBuf_rule->IsFMR)
    {
        pBuf_rule->IsFMR = pEntry->IsFMR;
    }

    if (pEntry->PSIDOffset != pBuf_rule->PSIDOffset)
    {
        pBuf_rule->PSIDOffset = pEntry->PSIDOffset;
    }

    if (pEntry->PSIDLength != pBuf_rule->PSIDLength)
    {
        pBuf_rule->PSIDLength = pEntry->PSIDLength;
    }

    if (pEntry->PSID != pBuf_rule->PSID)
    {
        pBuf_rule->PSID = pEntry->PSID;
    }

    if (pEntry->IncludeSystemPorts != pBuf_rule->IncludeSystemPorts)
    {
        pBuf_rule->IncludeSystemPorts = pEntry->IncludeSystemPorts;
    }

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

    static INT WanManager_MAPEConfiguration
    (
        VOID
    );

    Description:

        API will configure the MAP-E using DMs updated by DhcpManager

    Arguments:
        Nill
    Return:
        The value of ANSC_STATUS

**********************************************************************/

ANSC_STATUS
WanManager_MAPEConfiguration
    (
        VOID
    )
{
    INT ret = 0;
    INT v4_prefix_len = 0, v6_prefix_len = 0, psid_offset = 0, pd_length = 0;
    INT ea_len = 0, ea_offset_byte = 0, ea_offset_remainder = 0;
    UINT ipv4IndexValue = 0, psidLen = 0, ipValue = 0, ea_bytes = 0, ipv4_addr = 0;
    UINT mask                       = 0;
    USHORT psidValue                = 0;
    CHAR *save_opt                  = NULL;
    CHAR *ptr                       = NULL;
    struct in_addr result           = {0};
    struct in6_addr in6             = {0};
    CHAR mape_ipv4_addr[BUFLEN_16]  = {0};
    CHAR ipAddressString[BUFLEN_32] = {0};
    CHAR charBuff[BUFLEN_32]        = {0};
    CHAR v4_prefix[BUFLEN_64]       = {0};
    CHAR tunnel_source[BUFLEN_64]   = {0};
    CHAR v6_prefix[BUFLEN_128]      = {0};
    CHAR br_prefix[BUFLEN_128]      = {0};
    CHAR pd_prefix[BUFLEN_128]      = {0};
    CHAR pd_ipv6_prefix[BUFLEN_128] = {0};
    UCHAR ipAddressBytes[BUFLEN_4]  = {0};

    //To-Do: The MAP-E DMs need to be updated from DhcpManager
    //Get option
    PWAN_DML_DOMAIN_CFG  pMapDomainCfg  = WanDmlMapDomGetCfg_Data(DOM_MAPE_INS_NO);
    PWAN_DML_DOMAIN_INFO pMapDomainInfo = WanDmlMapDomGetInfo_Data(DOM_MAPE_INS_NO);
    PWAN_DML_MAP_RULE    pDomainRule    = WanDmlMapDomGetRule_Data(DOM_MAPE_INS_NO, DOM_MAPE_INS_NO-1);

    ea_len      = pDomainRule->EABitsLength;
    psid_offset = pDomainRule->PSIDOffset;
    AnscCopyString(v4_prefix, pDomainRule->IPv4Prefix);
    AnscCopyString(v6_prefix, pDomainRule->IPv6Prefix);
    AnscCopyString(br_prefix, pMapDomainCfg->BRIPv6Prefix);

    //Get IAPD and v6 length
    sysevent_get(sysevent_fd, sysevent_token,"ipv6_prefix", pd_prefix, sizeof(pd_prefix));

    if(strlen(pd_prefix) == 0)
    {
        CcspTraceError(("%s: %d ipv6_prefix is empty \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }
    else
    {
        strncpy(pMapDomainCfg->IPv6Prefix, pd_prefix, sizeof(pMapDomainCfg->IPv6Prefix));
    }

    ptr = strtok_r(pd_prefix, "/", &save_opt);
    snprintf(pd_ipv6_prefix, sizeof(pd_ipv6_prefix), "%s", ptr);
    pd_length = atoi(save_opt);

    ptr      = NULL;
    save_opt = NULL;
    ptr      = strtok_r(v6_prefix, "/", &save_opt);
    v6_prefix_len = atoi(save_opt);

    /* Get the S46_RULE prefix4-len */
    save_opt = NULL;
    strtok_r(v4_prefix, "/", &save_opt);
    v4_prefix_len = atoi(save_opt);

    //Calculate PSID
    WanManager_CalculateMAPEPsid(pd_ipv6_prefix, v6_prefix_len, v4_prefix_len, ea_len, &psidValue, &ipv4IndexValue, &psidLen);
    pDomainRule->PSIDLength = psidLen;
    pDomainRule->PSID       = psidValue;

    //MAP-E IPv4 Address
    inet_pton(AF_INET,v4_prefix, &(result));
    ipValue = htonl(result.s_addr);

    /* Calculate tunnel IPv4 address by embedding suffix bits from EA bits */
    mask = (1 << (ea_len-psidLen)) - 1;
    ipValue = ipValue & ~mask;
    ipValue |= ipv4IndexValue;

    ipAddressBytes[0] = ipValue & 0xFF;
    ipAddressBytes[1] = (ipValue >> 8) & 0xFF;
    ipAddressBytes[2] = (ipValue >> 16) & 0xFF;
    ipAddressBytes[3] = (ipValue >> 24) & 0xFF;

    snprintf(ipAddressString, sizeof(ipAddressString), "%d.%d.%d.%d", ipAddressBytes[3], ipAddressBytes[2], ipAddressBytes[1], ipAddressBytes[0]);

    inet_pton(AF_INET6, pd_ipv6_prefix, &in6);
    strcpy(mape_ipv4_addr, ipAddressString);
    in6.s6_addr16[7] = htons(psidValue);
    in6.s6_addr16[5] = inet_addr(mape_ipv4_addr) & 0x0000ffff;
    in6.s6_addr16[6] = inet_addr(mape_ipv4_addr) >> 16;
    in6.s6_addr16[4] = 0;
    inet_ntop(AF_INET6, &in6, tunnel_source, sizeof(tunnel_source));

    //tunnel_source
    ret = v_secure_system("ip -6 tunnel add %s mode ip4ip6 remote %s local %s encaplimit none dev %s", TUNNEL_NAME, br_prefix, tunnel_source, "erouter0");
    if(ret)
        return ANSC_STATUS_FAILURE;

    ret = v_secure_system("ifconfig %s add %s/128", TUNNEL_NAME, tunnel_source);
    if(ret)
       return ANSC_STATUS_FAILURE;

    ret = v_secure_system("ifconfig %s mtu %d", TUNNEL_NAME, TUNNEL_MTU);
    if(ret)
        return ANSC_STATUS_FAILURE;

    ret = v_secure_system("ip link set dev %s up", TUNNEL_NAME);
    if(ret)
        return ANSC_STATUS_FAILURE;

    ret = v_secure_system("ifconfig %s %s netmask 255.255.255.255", TUNNEL_NAME, mape_ipv4_addr);
    if(ret)
        return ANSC_STATUS_FAILURE;

    v_secure_system("ip route add to default dev %s", TUNNEL_NAME);

    /* Generate syscfg for psid_offset,psidValue,psidLen,ipAddressString,mape_config_flag to use in firewall.c */
    snprintf(charBuff, sizeof(charBuff), "%d", psid_offset);
    if ( syscfg_set(NULL, "mape_psid_offset", charBuff) != 0 )
    {
        CcspTraceError(("%s: syscfg_set failed for parameter mape_psid_offset\n", __FUNCTION__));
    }

    memset(charBuff, 0, sizeof(charBuff));
    snprintf(charBuff, sizeof(charBuff), "%d", psidValue);
    if ( syscfg_set(NULL, "mape_psid", charBuff) != 0 )
    {
        CcspTraceError(("%s: syscfg_set failed for parameter mape_psid\n", __FUNCTION__));
    }

    memset(charBuff, 0, sizeof(charBuff));
    snprintf(charBuff, sizeof(charBuff), "%d", psidLen);
    if ( syscfg_set(NULL, "mape_psid_len", charBuff) != 0 )
    {
        CcspTraceError(("%s: syscfg_set failed for parameter mape_psid_len\n", __FUNCTION__));
    }

    if ( syscfg_set(NULL, "mape_ipv4_address", ipAddressString) != 0 )
    {
        CcspTraceError(("%s: syscfg_set failed for parameter mape_ipv4_address\n", __FUNCTION__));
    }

    if ( syscfg_set(NULL, "mape_config_flag", "true") != 0 )
    {
        CcspTraceError(("%s: syscfg_set failed for parameter mape_config_flag\n", __FUNCTION__));
    }

    //Restart firewall
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_FIREWALL_RESTART, NULL, 0);
    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_IGD_RESTART, NULL, 0);

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    caller:     self

    prototype:

       INT
       WanDmlMapDomGetIfStats
           (
                CHAR                        *ifname,
                PWAN_DML_INTERFACE_STAT      pStats
           );

    Description:

        API to get the interface statistics.

    Arguments:
                CHAR                        *ifname,
                PWAN_DML_INTERFACE_STAT      pStats

    Return:
        The status of the operation.

**********************************************************************/
INT
WanDmlMapDomGetIfStats
    (
        CHAR *ifname,
        PWAN_DML_INTERFACE_STAT pStats
    )
{
    INT  ret                                  = 0;
    INT  idx                                  = -1;
    FILE *fp                                  = NULL;
    CHAR *p                                   = NULL;
    CHAR BytesReceived[BUFF_SIZE_16]          = {0};
    CHAR PacketsReceived[BUFF_SIZE_16]        = {0};
    CHAR ErrorsReceived[BUFF_SIZE_16]         = {0};
    CHAR DiscardPacketsReceived[BUFF_SIZE_16] = {0};
    CHAR BytesSent[BUFF_SIZE_16]              = {0};
    CHAR PacketsSent[BUFF_SIZE_16]            = {0};
    CHAR ErrorsSent[BUFF_SIZE_16]             = {0};
    CHAR DiscardPacketsSent[BUFF_SIZE_16]     = {0};
    CHAR buffStats[BUFF_SIZE_1024]            = {0};
    CHAR searchStr[BUFF_SIZE_64]              = {0};

    if(NULL != ifname)
    {
        sprintf(searchStr, "%s:", ifname);
    }
    else
    {
        CcspTraceError(("%s: Invalid interface name!\n", __FUNCTION__));
        return ret;
    }

    fp = fopen(NET_STATS_FILE, "r");
    if(NULL != fp)
    {
        idx = 0;
        while(fgets(buffStats, sizeof(buffStats), fp))
        {
            if(++idx <= 2)
                continue;

            if(p = strstr(buffStats, searchStr))
            {
                memset(pStats, 0, sizeof(WAN_DML_INTERFACE_STAT));

                if (8 == sscanf(p, "%*s %s %s %s %s %*lu %*lu %*lu %*lu %s %s %s %s %*lu %*lu %*lu %*lu",
                BytesReceived, PacketsReceived, ErrorsReceived, DiscardPacketsReceived,
                BytesSent, PacketsSent, ErrorsSent, DiscardPacketsSent))
                {
                    pStats->BytesReceived          = strtoul_custom(BytesReceived);
                    pStats->PacketsReceived        = strtoul_custom(PacketsReceived);
                    pStats->ErrorsReceived         = strtoul_custom(ErrorsReceived);
                    pStats->DiscardPacketsReceived = strtoul_custom(DiscardPacketsReceived);
                    pStats->BytesSent              = strtoul_custom(BytesSent);
                    pStats->PacketsSent            = strtoul_custom(PacketsSent);
                    pStats->ErrorsSent             = strtoul_custom(ErrorsSent);
                    pStats->DiscardPacketsSent     = strtoul_custom(DiscardPacketsSent);

                    ret = 1;
                    break;
                }
            }
        }

        fclose(fp);
    }

    return ret;
}

/**********************************************************************

    caller:     self

    prototype:

    static INT WanManager_CalculateMAPEPsid
    (
        CHAR *pdIPv6Prefix,
        INT v6PrefixLen,
        INT v4PrefixLen,
        INT ea_length,
        USHORT *psidValue,
        UINT *ipv4IndexValue,
        UINT *psidLen
    );

    Description:

        API will calculate the psidValue ipv4IndexValue psidLen from given arguments

    Arguments:
        CHAR *pdIPv6Prefix,
        INT v6PrefixLen,
        INT v4PrefixLen,
        INT ea_length,
        USHORT *psidValue,
        UINT *ipv4IndexValue,
        UINT *psidLen

    Return:
        The value of psidValue ipv4IndexValue psidLen

**********************************************************************/

static INT WanManager_CalculateMAPEPsid
    (
        CHAR *pdIPv6Prefix,
        INT v6PrefixLen,
        INT v4PrefixLen,
        INT ea_length,
        USHORT *psidValue,
        UINT *ipv4IndexValue,
        UINT *psidLen
    )
{
    INT  ret                 = RETURN_OK;
    UINT ea_bytes            = 0;
    UINT v4_suffix           = 0;
    UINT psid                = 0;
    INT  ea_offset_byte      = 0;
    INT  ea_offset_remainder = 0;
    INT  bitIndex            = 0;
    INT  psid_len            = 0;
    INT  byteIndex           = 0;
    struct in6_addr in6      = {0};

    ea_offset_byte      = v6PrefixLen/8;
    ea_offset_remainder = v6PrefixLen%8;
    psid_len            = ea_length - (IPV4_ADDR_LEN_IN_BITS - v4PrefixLen);

    inet_pton(AF_INET6, pdIPv6Prefix, &in6);

    for(byteIndex = 0; byteIndex < 4; byteIndex++)
    {
        ea_bytes |= in6.s6_addr[ea_offset_byte + byteIndex]<<(4 - byteIndex - 1)*8;
    }

    if(ea_offset_remainder)
    {
        ea_bytes = ea_bytes<<ea_offset_remainder;
    }

    ea_bytes        = ea_bytes>>(IPV4_ADDR_LEN_IN_BITS - ea_length);
    v4_suffix       = ea_bytes>>psid_len;
    psid            = ea_bytes^(v4_suffix<<psid_len);
    *psidValue      = (USHORT)(psid);
    *ipv4IndexValue = v4_suffix;
    *psidLen        = psid_len;

    return ret;
}

/**********************************************************************

    caller:     self

    prototype:

        VOID
        syscfg_SetDefault
            (
                VOID
            );
    Description:

        The API will set default value for syscfg parameters.

    Arguments:
                VOID

    Return:
                VOID

**********************************************************************/
static VOID syscfg_SetDefault
    (
        VOID
    )
{
    syscfg_unset (NULL, "mape_config_flag");
    syscfg_unset (NULL, "mape_psid_offset");
    syscfg_unset (NULL, "mape_psid");
    syscfg_unset (NULL, "mape_psid_len");
    syscfg_unset (NULL, "mape_ipv4_address");
    syscfg_unset (NULL, "mape_ipv4_address");
}
