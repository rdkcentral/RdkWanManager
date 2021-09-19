##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

bin_PROGRAMS = wanmanager

AM_CPPFLAGS = -I$(top_srcdir)/hal/include \
        -I$(top_srcdir)/source/LogComponent \
        -I$(top_srcdir)/source/TR-181/middle_layer_src \
        -I$(top_srcdir)/source/TR-181/include \
        -I$(top_srcdir)/../CcspCommonLibrary/source/ccsp/components/include

AM_LDFLAGS = -lwebconfig_framework

wanmanager_DEPENDENCIES = \
        $(EXTRA_DEPENDENCIES) \
        ${top_builddir}/source/TR-181/middle_layer_src/libCcspWanManager_middle_layer_src.la

wanmanager_CFLAGS = -D_ANSC_LINUX -D_ANSC_USER -D_ANSC_LITTLE_ENDIAN_ -DFEATURE_SUPPORT_RDKLOG $(DBUS_CFLAGS) $(SYSTEMD_CFLAGS)
wanmanager_SOURCES = wanmgr_webconfig_apis.c wanmgr_webconfig.c wanmgr_main.c  wanmgr_ssp_action.c wanmgr_ssp_messagebus_interface.c wanmgr_core.c wanmgr_controller.c wanmgr_data.c wanmgr_sysevents.c wanmgr_policy_fm_impl.c wanmgr_policy_fmob_impl.c wanmgr_policy_pp_impl.c wanmgr_policy_ppob_impl.c wanmgr_interface_sm.c wanmgr_utils.c wanmgr_net_utils.c wanmgr_dhcpv4_apis.c wanmgr_dhcpv6_apis.c wanmgr_ipc.c wanmgr_dhcpv4_internal.c wanmgr_dhcpv6_internal.c wanmgr_policy_autowan_impl.c wanmgr_policy_auto_impl.c dm_pack_datamodel.c
wanmanager_LDFLAGS = -lccsp_common -lrdkloggers $(DBUS_LIBS) $(SYSTEMD_LDFLAGS) -lhal_platform -lapi_dhcpv4c -lbreakpadwrapper -lnanomsg -lwebconfig_framework -lmsgpackc -ldhcp_client_utils
wanmanager_LDADD =  $(wanmanager_DEPENDENCIES)

