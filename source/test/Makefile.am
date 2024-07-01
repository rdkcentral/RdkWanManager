#############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################

AM_CFLAGS = -D_ANSC_LINUX
AM_CFLAGS += -D_ANSC_USER

AM_CPPFLAGS = -Wall -g -Werror
AM_CXXFLAGS = -std=c++11

ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS = RdkWanManager_gtest.bin

wanmanager_DEPENDENCIES = \
        $(EXTRA_DEPENDENCIES) \
        ${top_builddir}/source/TR-181/middle_layer_src/libCcspWanManager_middle_layer_src.la


RdkWanManager_gtest_bin_CPPFLAGS = -I$(PKG_CONFIG_SYSROOT_DIR)$(includedir)/gtest \
-I${top_srcdir}/gtest/include \
-I${top_srcdir}/source \
-I${top_srcdir}/source/include \
-I${top_srcdir}/source/WanManager \
-I$(top_srcdir)/source/TR-181/include \
-I$(top_srcdir)/source/TR-181/middle_layer_src \
-I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/dbus-1.0 \
-I${PKG_CONFIG_SYSROOT_DIR}$(libdir)/dbus-1.0/include \
-I${PKG_CONFIG_SYSROOT_DIR}$(includedir)/ccsp 
RdkWanManager_gtest_bin_LDFLAGS = -lgtest -lgmock -lgcov

#TODO:Compiling all files due to limited time. This should be modified to compile only the required files
RdkWanManager_gtest_bin_SOURCES =  ../WanManager/wanmgr_webconfig_apis.c ../WanManager/wanmgr_webconfig.c  ../WanManager/wanmgr_ssp_action.c  ../WanManager/wanmgr_core.c ../WanManager/wanmgr_controller.c ../WanManager/wanmgr_data.c ../WanManager/wanmgr_sysevents.c ../WanManager/wanmgr_interface_sm.c ../WanManager/wanmgr_utils.c ../WanManager/wanmgr_net_utils.c ../WanManager/wanmgr_dhcpv4_apis.c ../WanManager/wanmgr_dhcpv6_apis.c ../WanManager/wanmgr_ipc.c ../WanManager/wanmgr_dhcpv4_internal.c ../WanManager/wanmgr_dhcpv6_internal.c ../WanManager/wanmgr_policy_autowan_impl.c ../WanManager/wanmgr_policy_auto_impl.c ../WanManager/dm_pack_datamodel.c ../WanManager/wanmgr_wan_failover.c ../WanManager/wanmgr_policy_parallel_scan_impl.c ../WanManager/wanmgr_dhcp_legacy_apis.c gtest_main.cpp \
 RdkWanManagerTest.cpp  Test_wanmgr_rbus_handler_apis.cpp

RdkWanManager_gtest_bin_LDADD = $(wanmanager_DEPENDENCIES) -lmock_ccsp_common -lrdkloggers  -lmock_platform_hal -lapi_dhcpv4c -lnanomsg -lwebconfig_framework -lmsgpackc -ldhcp_client_utils -lmock_privilege -lmock_rbus -lmock_secure_wrapper





#RdkWanManager_gtest_bin_SOURCES =  ../WanManager/wanmgr_webconfig_apis.c ../WanManager/wanmgr_webconfig.c  ../WanManager/wanmgr_ssp_action.c ../WanManager/wanmgr_ssp_messagebus_interface.c ../WanManager/wanmgr_core.c ../WanManager/wanmgr_controller.c ../WanManager/wanmgr_data.c ../WanManager/wanmgr_sysevents.c ../WanManager/wanmgr_interface_sm.c ../WanManager/wanmgr_utils.c ../WanManager/wanmgr_net_utils.c ../WanManager/wanmgr_dhcpv4_apis.c ../WanManager/wanmgr_dhcpv6_apis.c ../WanManager/wanmgr_ipc.c ../WanManager/wanmgr_dhcpv4_internal.c ../WanManager/wanmgr_dhcpv6_internal.c ../WanManager/wanmgr_policy_autowan_impl.c ../WanManager/wanmgr_policy_auto_impl.c ../WanManager/dm_pack_datamodel.c ../WanManager/wanmgr_wan_failover.c ../WanManager/wanmgr_policy_parallel_scan_impl.c ../WanManager/wanmgr_dhcp_legacy_apis.c gtest_main.cpp RdkWanManagerTest.cpp
#RdkWanManager_gtest_bin_LDADD = $(wanmanager_DEPENDENCIES) -lccsp_common -lrdkloggers $(DBUS_LIBS) $(SYSTEMD_LDFLAGS) -lhal_platform -lapi_dhcpv4c -lnanomsg -lwebconfig_framework -lmsgpackc -ldhcp_client_utils -lprivilege -lrbus -lsecure_wrapper
#-ldhcp_client_utils
#-lmsgpackc
#-lwebconfig_framework
#-lnanomsg
#-lrdkloggers

