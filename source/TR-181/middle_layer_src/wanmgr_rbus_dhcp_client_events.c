/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include "wanmgr_data.h"
#include "wanmgr_rbus_handler_apis.h"
#include "ipc_msg.h"
#include "wanmgr_interface_sm.h"

#define DHCP_MGR_DHCPv4_TABLE "Device.DHCPv4.Client"
#define DHCP_MGR_DHCPv6_TABLE "Device.DHCPv6.Client"


static void copyDhcpv4Data(WANMGR_IPV4_DATA* pDhcpv4Data, const DHCP_MGR_IPV4_MSG* leaseInfo) 
{
    strncpy(pDhcpv4Data->ifname, leaseInfo->ifname, sizeof(pDhcpv4Data->ifname) - 1);
    strncpy(pDhcpv4Data->ip, leaseInfo->address, sizeof(pDhcpv4Data->ip) - 1);
    strncpy(pDhcpv4Data->mask, leaseInfo->netmask, sizeof(pDhcpv4Data->mask) - 1);
    strncpy(pDhcpv4Data->gateway, leaseInfo->gateway, sizeof(pDhcpv4Data->gateway) - 1);
    strncpy(pDhcpv4Data->dnsServer, leaseInfo->dnsServer, sizeof(pDhcpv4Data->dnsServer) - 1);
    strncpy(pDhcpv4Data->dnsServer1, leaseInfo->dnsServer1, sizeof(pDhcpv4Data->dnsServer1) - 1);
    strncpy(pDhcpv4Data->timeZone, leaseInfo->timeZone, sizeof(pDhcpv4Data->timeZone) - 1);
    pDhcpv4Data->mtuSize = leaseInfo->mtuSize;
    pDhcpv4Data->timeOffset = leaseInfo->timeOffset;
    pDhcpv4Data->isTimeOffsetAssigned = leaseInfo->isTimeOffsetAssigned;
    pDhcpv4Data->upstreamCurrRate = leaseInfo->upstreamCurrRate;
}

static void copyDhcpv6Data(WANMGR_IPV6_DATA* pDhcpv6Data, const DHCP_MGR_IPV6_MSG* leaseInfo) 
{
    CcspTraceInfo(("%s %d :new IPv6 lease Info:\n"
        "ifname=%s\n"  "address=%s\n"  "nameserver=%s\n"  "nameserver1=%s\n" "domainName=%s\n"  "sitePrefix=%s\n"  "prefixPltime=%u\n"  "prefixVltime=%u\n"  "addrAssigned=%d\n"   "prefixAssigned=%d\n"   "domainNameAssigned=%d\n",
        __FUNCTION__, __LINE__, leaseInfo->ifname, leaseInfo->address, leaseInfo->nameserver, leaseInfo->nameserver1,
        leaseInfo->domainName, leaseInfo->sitePrefix, leaseInfo->prefixPltime, leaseInfo->prefixVltime,
        leaseInfo->addrAssigned, leaseInfo->prefixAssigned, leaseInfo->domainNameAssigned));

    strncpy(pDhcpv6Data->ifname, leaseInfo->ifname, sizeof(pDhcpv6Data->ifname) - 1);
    strncpy(pDhcpv6Data->address, leaseInfo->address, sizeof(pDhcpv6Data->address) - 1);
    strncpy(pDhcpv6Data->nameserver, leaseInfo->nameserver, sizeof(pDhcpv6Data->nameserver) - 1);
    strncpy(pDhcpv6Data->nameserver1, leaseInfo->nameserver1, sizeof(pDhcpv6Data->nameserver1) - 1);
    strncpy(pDhcpv6Data->domainName, leaseInfo->domainName, sizeof(pDhcpv6Data->domainName) - 1);
    strncpy(pDhcpv6Data->sitePrefix, leaseInfo->sitePrefix, sizeof(pDhcpv6Data->sitePrefix) - 1);
    pDhcpv6Data->prefixPltime = leaseInfo->prefixPltime;
    pDhcpv6Data->prefixVltime = leaseInfo->prefixVltime;
    pDhcpv6Data->addrAssigned = leaseInfo->addrAssigned;
    pDhcpv6Data->prefixAssigned = leaseInfo->prefixAssigned;
    pDhcpv6Data->domainNameAssigned = leaseInfo->domainNameAssigned;
}
void WanMgr_DhcpClientEventsHandler(rbusHandle_t handle, rbusEvent_t const* event, rbusEventSubscription_t* subscription)
{
    (void)handle;
    (void)subscription;

    const char* eventName = event->name;

    if((eventName == NULL))
    {
        CcspTraceError(("%s : FAILED , value is NULL\n",__FUNCTION__));
        return;
    }

    CcspTraceInfo(("%s %d: Received %s\n", __FUNCTION__, __LINE__, eventName));
    if (strstr(eventName, DHCP_MGR_DHCPv4_TABLE))
    {
        rbusValue_t value;
        value = rbusObject_GetValue(event->data, "IfName");
        char virtIfName[20] = {0};
        strncpy(virtIfName , rbusValue_GetString(value, NULL),sizeof(virtIfName)-1);
        CcspTraceInfo(("%s-%d : DHCP client event received for  %s\n", __FUNCTION__, __LINE__,  virtIfName));

        value = rbusObject_GetValue(event->data, "MsgType");
        DHCP_MESSAGE_TYPE type = rbusValue_GetUInt32(value);


        DML_VIRTUAL_IFACE* pVirtIf = WanMgr_GetVIfByName_VISM_running_locked(virtIfName);
        if(pVirtIf != NULL)
        {

            if(type == DHCP_CLIENT_STARTED)
            {
                CcspTraceInfo(("%s-%d : DHCPv4 client started for  %s\n", __FUNCTION__, __LINE__,  pVirtIf->Name));
            }
            else if(type == DHCP_CLIENT_STOPPED)
            {
                CcspTraceInfo(("%s-%d : DHCPv4 client Stopped for  %s\n", __FUNCTION__, __LINE__,  pVirtIf->Name));
            }
            else if(type == DHCP_CLIENT_FAILED)
            {
                CcspTraceInfo(("%s-%d : DHCPv4 client Stopped for  %s\n", __FUNCTION__, __LINE__,  pVirtIf->Name));
            }
            else if(type == DHCP_LEASE_RENEW)
            {
                //TODO: Check for syevents
                pVirtIf->IP.Ipv4Renewed = TRUE;
                CcspTraceInfo(("%s-%d : DHCPv4 lease renewed for  %s\n", __FUNCTION__, __LINE__,  pVirtIf->Name));
                WanManager_UpdateInterfaceStatus(pVirtIf, WANMGR_IFACE_CONNECTION_UP);
            }
            else if(type == DHCP_LEASE_DEL)
            {

                CcspTraceInfo(("%s-%d : DHCPv4 lease expired for  %s\n", __FUNCTION__, __LINE__,  pVirtIf->Name));   
                WanManager_UpdateInterfaceStatus(pVirtIf, WANMGR_IFACE_CONNECTION_DOWN);
            }
            else if(type == DHCP_LEASE_UPDATE)
            {
                CcspTraceInfo(("%s-%d : DHCPv4 lease updated  for  %s\n", __FUNCTION__, __LINE__,  pVirtIf->Name));   
                value = rbusObject_GetValue(event->data, "LeaseInfo");
                int bytes_len=0;
                uint8_t byteArray[sizeof(DHCP_MGR_IPV4_MSG)];
                DHCP_MGR_IPV4_MSG leaseInfo;

                uint8_t const* ptr = rbusValue_GetBytes(value, &bytes_len);
                memset(&leaseInfo, 0, sizeof(leaseInfo));
                CcspTraceInfo(("%s-%d : DHCPv4 lease length %d and expected %d\n", __FUNCTION__, __LINE__,  bytes_len,sizeof(DHCP_MGR_IPV4_MSG) ));   

                if((size_t)bytes_len == sizeof(DHCP_MGR_IPV4_MSG))
                {
                    memcpy(&leaseInfo, ptr, bytes_len);
                    copyDhcpv4Data(&(pVirtIf->IP.Ipv4Data), &leaseInfo);
                    pVirtIf->IP.Ipv4Changed = TRUE;
                    WanManager_UpdateInterfaceStatus(pVirtIf, WANMGR_IFACE_CONNECTION_UP);
                } 
            }
            WanMgr_VirtualIfaceData_release(pVirtIf);
        }
    }
    else if (strstr(eventName, DHCP_MGR_DHCPv6_TABLE)) // Handle DHCPv6 events
    {
        rbusValue_t value;
        value = rbusObject_GetValue(event->data, "IfName");
        char virtIfName[20] = {0};
        strncpy(virtIfName, rbusValue_GetString(value, NULL), sizeof(virtIfName) - 1);
        CcspTraceInfo(("%s-%d : DHCPv6 client event received for %s\n", __FUNCTION__, __LINE__, virtIfName));

        value = rbusObject_GetValue(event->data, "MsgType");
        DHCP_MESSAGE_TYPE type = rbusValue_GetUInt32(value);

        DML_VIRTUAL_IFACE* pVirtIf = WanMgr_GetVIfByName_VISM_running_locked(virtIfName);
        if (pVirtIf != NULL)
        {
            if (type == DHCP_CLIENT_STARTED)
            {
                CcspTraceInfo(("%s-%d : DHCPv6 client started for %s\n", __FUNCTION__, __LINE__, pVirtIf->Name));
            }
            else if (type == DHCP_CLIENT_STOPPED)
            {
                CcspTraceInfo(("%s-%d : DHCPv6 client stopped for %s\n", __FUNCTION__, __LINE__, pVirtIf->Name));
            }
            else if (type == DHCP_CLIENT_FAILED)
            {
                CcspTraceInfo(("%s-%d : DHCPv6 client failed for %s\n", __FUNCTION__, __LINE__, pVirtIf->Name));
            }
            else if (type == DHCP_LEASE_RENEW)
            {
                pVirtIf->IP.Ipv6Renewed = TRUE;
                CcspTraceInfo(("%s-%d : DHCPv6 lease renewed for %s\n", __FUNCTION__, __LINE__, pVirtIf->Name));
                WanManager_UpdateInterfaceStatus(pVirtIf, WANMGR_IFACE_CONNECTION_IPV6_UP);
            }
            else if (type == DHCP_LEASE_DEL)
            {
                CcspTraceInfo(("%s-%d : DHCPv6 lease expired for %s\n", __FUNCTION__, __LINE__, pVirtIf->Name));
                WanManager_UpdateInterfaceStatus(pVirtIf, WANMGR_IFACE_CONNECTION_IPV6_DOWN);
            }
            else if (type == DHCP_LEASE_UPDATE)
            {
                CcspTraceInfo(("%s-%d : DHCPv6 lease updated for %s\n", __FUNCTION__, __LINE__, pVirtIf->Name));
                value = rbusObject_GetValue(event->data, "LeaseInfo");
                int bytes_len = 0;
                uint8_t byteArray[sizeof(DHCP_MGR_IPV6_MSG)];
                DHCP_MGR_IPV6_MSG leaseInfo;

                uint8_t const* ptr = rbusValue_GetBytes(value, &bytes_len);
                memset(&leaseInfo, 0, sizeof(leaseInfo));
                CcspTraceInfo(("%s-%d : DHCPv6 lease length %d and expected %d\n", __FUNCTION__, __LINE__, bytes_len, sizeof(DHCP_MGR_IPV6_MSG)));

                if ((size_t)bytes_len == sizeof(DHCP_MGR_IPV6_MSG))
                {
                    memcpy(&leaseInfo, ptr, bytes_len);
                    copyDhcpv6Data(&(pVirtIf->IP.Ipv6Data), &leaseInfo);
                    pVirtIf->IP.Ipv6Changed = TRUE;
                    WanManager_UpdateInterfaceStatus(pVirtIf, WANMGR_IFACE_CONNECTION_IPV6_UP);
                }
            }
            WanMgr_VirtualIfaceData_release(pVirtIf);
        }
    }
}

