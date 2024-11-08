/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#ifndef _WANMGR_GTEST_H_
#define _WANMGR_GTEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mocks/mock_rbus.h>
#include <mocks/mock_platform_hal.h>
#include <mocks/mock_messagebus.h>
#include <mocks/mock_securewrapper.h>
#include <mocks/mock_ansc_memory.h>
#include <mocks/mock_cap.h>
#include <mocks/mock_psm.h>
#include <mocks/mock_ansc_task.h>
#include <mocks/mock_SysInfoRepository.h>
#include <mocks/mock_ansc_timer_scheduler.h>
#include <mocks/mock_ansc_crypto.h>
#include <mocks/mock_ansc_xml.h>
#include <mocks/mock_ansc_file_io.h>
#include <mocks/mock_ansc_co.h>
#include <mocks/mock_dslh_dmagnt_exported.h>
#include <mocks/mock_ccsp_dmapi.h>
#include <mocks/mock_user_runtime.h>
#include <mocks/mock_ansc_debug.h>
#include <mocks/mock_trace.h>
#include <mocks/mock_file_io.h>
#include <mocks/mock_base_api.h>
#include <mocks/mock_usertime.h>
#include <mocks/mock_ansc_wrapper_api.h>



#include "wanmgr_interface_sm.h"

using namespace std;
using ::testing::_;
using ::testing::Return;
using ::testing::UnitTest;
using ::testing::Eq;
using ::testing::StrEq;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::SetArgReferee;
using ::testing::SetArgPointee;
using ::testing::DoAll;

class MockWanMgr
{
public:
    MOCK_METHOD(void, t2_event_d, (char *Telemtrylog, int a));
    //MOCK_METHOD(const char* , rbusProperty_GetName, (rbusProperty_t property));
    //MOCK_METHOD(WanMgr_Iface_Data_t*, WanMgr_GetIfaceData_locked, (UINT iface_index));
    MOCK_METHOD(void, WanMgr_UpdateIpFromCellularMgr, (WanMgr_IfaceSM_Controller_t* pWanIfaceCtrl));
    MOCK_METHOD(ANSC_STATUS, IPCPStateChangeHandler ,(DML_VIRTUAL_IFACE* pVirtIf));
    MOCK_METHOD(ANSC_STATUS, wanmgr_handle_dhcpv4_event_data, (DML_VIRTUAL_IFACE* pVirtIf));

};
MATCHER_P2(StrCmpLen, expected_str, n, "") {
    return strncmp(arg, expected_str, n) == 0;
}


class WanMgrBase : public ::testing::Test
{
public:
   rbusHandle_t handle;

protected:

    MockWanMgr mockWanUtils;
    rbusMock mockedRbus;
    AnscCoMock mockedAnscCo;
    SecureWrapperMock mockSecurewrapperMock;
    CapMock  mockCap;
    AnscMemoryMock  mockAnscMemory;
    MessageBusMock mockMessagebus;
    PlatformHalMock mockPlatformHAL;
    PsmMock mockPSM;
    AnscTaskMock mockanscTaskMock;
    SysInfoRepositoryMock mocksysInfoRepositoryMock;
    AnscTimerSchedulerMock mockanscTimerSchedulerMock;
    AnscCryptoMock mockanscCryptoMock;
    AnscXmlMock mockAnscXml;
    AnscFileIOMock mockAnscFileIOMock;
    DslhDmagntExportedMock mockdslhDmagntExported;
    CcspDmApiMock mockccspDmApiMock;
    UserRuntimeMock mockedUserRuntime;
    AnscDebugMock mockedAnscDebug;
    TraceMock mockedTrace;
    FileIOMock mockedFileIO;
    BaseAPIMock mockedbaseapi;
    UserTimeMock mockedUserTime;
    AnscWrapperApiMock mockedanscWrapperApi;


    WanMgrBase();

    virtual ~WanMgrBase() {}

    //Initializing WanManager mutex and Interface objects for the test
    virtual void SetUp();
    //Clearing Interface objects created for the test
    virtual void TearDown();
};

#endif
