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

#ifndef _DMSB_TR181_PSM_DEFINITIONS_
#define _DMSB_TR181_PSM_DEFINITIONS_

/**********************************************************************
                       X_RDK_WanManager.
**********************************************************************/
#define PSM_WANMANAGER_WANENABLE                            "dmsb.wanmanager.wanenable"
#define PSM_WANMANAGER_WANMODE                              "dmsb.wanmanager.wanmode"
#define PSM_WANMANAGER_WANPOLICY                            "dmsb.wanmanager.wanpolicy"
#define PSM_WANMANAGER_WANIFCOUNT                           "dmsb.wanmanager.wanifcount"
#define PSM_WANMANAGER_WANIDLETIMEOUT                       "dmsb.wanmanager.wanidletimeout"
#define PSM_WANMANAGER_IF_ENABLE                            "dmsb.wanmanager.if.%d.Enable"
#define PSM_WANMANAGER_IF_NAME                              "dmsb.wanmanager.if.%d.Name"
#define PSM_WANMANAGER_IF_DISPLAY_NAME                      "dmsb.wanmanager.if.%d.DisplayName"
#define PSM_WANMANAGER_IF_TYPE                              "dmsb.wanmanager.if.%d.Type"
#define PSM_WANMANAGER_IF_PRIORITY                          "dmsb.wanmanager.if.%d.Priority"
#define PSM_WANMANAGER_IF_SELECTIONTIMEOUT                  "dmsb.wanmanager.if.%d.SelectionTimeout"
#define PSM_WANMANAGER_IF_DYNTRIGGERENABLE                  "dmsb.wanmanager.if.%d.DynTriggerEnable"
#define PSM_WANMANAGER_IF_DYNTRIGGERDELAY                   "dmsb.wanmanager.if.%d.DynTriggerDelay"
#define PSM_WANMANAGER_IF_WAN_ENABLE_MAPT                   "dmsb.wanmanager.if.%d.EnableMAPT"
#define PSM_WANMANAGER_IF_WAN_ENABLE_DSLITE                 "dmsb.wanmanager.if.%d.EnableDSLite"
#define PSM_WANMANAGER_IF_WAN_ENABLE_IPOE                   "dmsb.wanmanager.if.%d.EnableIPoE"
#define PSM_WANMANAGER_IF_WAN_VALIDATION_DISCOVERY_OFFER    "dmsb.wanmanager.if.%d.Validation.DiscoveryOffer"
#define PSM_WANMANAGER_IF_WAN_VALIDATION_SOLICIT_ADVERTISE  "dmsb.wanmanager.if.%d.Validation.SolicitAdvertise"
#define PSM_WANMANAGER_IF_WAN_VALIDATION_RS_RA              "dmsb.wanmanager.if.%d.Validation.RsRa"
#define PSM_WANMANAGER_IF_WAN_VALIDATION_PADI_PADO          "dmsb.wanmanager.if.%d.Validation.PadiPado"
#define PSM_WANMANAGER_IF_WAN_PPP_ENABLE                    "dmsb.wanmanager.if.%d.PPPEnable"
#define PSM_WANMANAGER_IF_WAN_PPP_LINKTYPE                  "dmsb.wanmanager.if.%d.PPPLinkType"
#define PSM_WANMANAGER_IF_WAN_PPP_IPCP_ENABLE               "dmsb.wanmanager.if.%d.PPPIPCPEnable"
#define PSM_WANMANAGER_IF_WAN_PPP_IPV6CP_ENABLE             "dmsb.wanmanager.if.%d.PPPIPV6CPEnable"
#define PSM_SELFHEAL_REBOOT_STATUS                          "dmsb.selfheal.rebootstatus"

/**********************************************************************
                       Interface.{i}.Marking.{i}.
**********************************************************************/
#define PSM_MARKING_LIST                     "dmsb.wanmanager.if.%d.Marking.List"
#define PSM_MARKING_ALIAS                    "dmsb.wanmanager.if.%d.Marking.%s.Alias"
#define PSM_MARKING_SKBPORT                  "dmsb.wanmanager.if.%d.Marking.%s.SKBPort"
#define PSM_MARKING_SKBMARK                  "dmsb.wanmanager.if.%d.Marking.%s.SKBMark"
#define PSM_MARKING_ETH_PRIORITY_MASK        "dmsb.wanmanager.if.%d.Marking.%s.EthernetPriorityMark"

#endif
