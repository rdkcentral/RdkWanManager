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

#include <gtest/gtest.h>
#include <gmock/gmock.h>


extern "C" {
#include "wanmgr_data.h"
#include "wanmgr_wan_failover.h"
}

#include "RdkWanManagerTest.h"
ANSC_HANDLE                 bus_handle               = NULL;
extern WANMGR_DATA_ST gWanMgrDataBase;
char    g_Subsystem[32]         = {0};


string ifaceName[4][2] =
{   "vdsl0", "VDSL",
    "erouter0", "DOCSIS",
    "ethwan0","WANOE",
    "brRWAN","REMOTE_LTE"
};

MockWanMgr *mockWanMgr = nullptr;
rbusMock *g_rbusMock = nullptr;
SecureWrapperMock * g_securewrapperMock = nullptr;
CapMock * g_capMock = nullptr;
AnscMemoryMock * g_anscMemoryMock = nullptr;
MessageBusMock * g_messagebusMock = nullptr;
PlatformHalMock *g_platformHALMock =nullptr;

WanMgrBase::WanMgrBase()
{
}

void WanMgrBase::SetUp()
{
    cout << "SetUp() : Test Suite Name: " << UnitTest::GetInstance()->current_test_info()->test_suite_name()
        << " Test Case Name: " << UnitTest::GetInstance()->current_test_info()->name() << endl;

    mockWanMgr = &mockWanUtils;
    g_rbusMock = &mockedRbus;
    g_securewrapperMock = &mockSecurewrapperMock;
    g_capMock = &mockCap;
    g_anscMemoryMock = &mockAnscMemory;
    g_messagebusMock = &mockMessagebus;
    g_platformHALMock = &mockPlatformHAL;
    //Initialise mutex attributes
    pthread_mutexattr_t     muttex_attr;
    pthread_mutexattr_init(&muttex_attr);
    pthread_mutexattr_settype(&muttex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&(gWanMgrDataBase.gDataMutex), &(muttex_attr));

    //init mock interfaces
    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
    pWanIfaceCtrl->pIface = (WanMgr_Iface_Data_t*) malloc( sizeof(WanMgr_Iface_Data_t) * MAX_WAN_INTERFACE_ENTRY);
    memset( pWanIfaceCtrl->pIface, 0, ( sizeof(WanMgr_Iface_Data_t) * MAX_WAN_INTERFACE_ENTRY ) );
    pWanIfaceCtrl->ulTotalNumbWanInterfaces = 4;
    for(int idx = 0 ; idx <  pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++ )
    {
        WanMgr_Iface_Data_t*  pIfaceData  = &(pWanIfaceCtrl->pIface[idx]);

        WanMgr_IfaceData_Init(pIfaceData, idx);
        for(int i=0; i< pIfaceData->data.NoOfVirtIfs; i++)
        {
            DML_VIRTUAL_IFACE* p_VirtIf = (DML_VIRTUAL_IFACE *) malloc( sizeof(DML_VIRTUAL_IFACE) );
            memset( p_VirtIf, 0, sizeof(DML_VIRTUAL_IFACE) );
            WanMgr_VirtIface_Init(p_VirtIf, i);
            p_VirtIf->next = NULL;
            WanMgr_AddVirtualToList(&(pIfaceData->data.VirtIfList), p_VirtIf);
        }
        strncpy(pIfaceData->data.AliasName, ifaceName[idx][1].c_str(),sizeof(pIfaceData->data.AliasName));
        strncpy(pIfaceData->data.DisplayName, ifaceName[idx][1].c_str(),sizeof(pIfaceData->data.DisplayName));
        strncpy(pIfaceData->data.VirtIfList->Name, ifaceName[idx][0].c_str(),sizeof(pIfaceData->data.VirtIfList->Name));
        strncpy(pIfaceData->data.VirtIfList->Alias, ifaceName[idx][1].c_str(),sizeof(pIfaceData->data.VirtIfList->Alias));
        //cout << " VirtIfList->Alias : " << pIfaceData->data.VirtIfList->Alias << endl;
    }
}

//Clearing Interface objects created for the test
void WanMgrBase::TearDown()
{
    cout << " TearDown() : Test Suite Name: " << UnitTest::GetInstance()->current_test_info()->test_suite_name()
        << " Test Case Name: " << UnitTest::GetInstance()->current_test_info()->name() << endl;

    WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
    for(int idx = 0 ; idx <  pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++ )
    {
        WanMgr_Iface_Data_t*  pIfaceData  = &(pWanIfaceCtrl->pIface[idx]);

        DML_VIRTUAL_IFACE* virIface = pIfaceData->data.VirtIfList;
        while(virIface != NULL)
        {
            DML_VIRTUAL_IFACE* temp = virIface;
            virIface = virIface->next;
            free(temp);
        }
    }
    free(pWanIfaceCtrl->pIface);
    pWanIfaceCtrl->pIface = NULL;
    pthread_mutex_destroy(&(gWanMgrDataBase.gDataMutex));
}

/* Test for WanMgr_GetVirtIfDataByAlias_locked() function.
 * WanMgr_GetVirtIfDataByAlias_locked expected to return a NULL pointer when Alias name is not matched with the interface list.
 */
TEST_F(WanMgrBase, TestGetVirtualIfWrongAlias) 
{
    char* alias = "WrongName";

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked(alias);
    EXPECT_THAT(result, IsNull());
    WanMgr_VirtualIfaceData_release(result);
}

/* Test for WanMgr_GetVirtIfDataByAlias_locked() function.
 * WanMgr_GetVirtIfDataByAlias_locked expected to return a NULL pointer when Alias parameter passed is NULL.
 */
TEST_F(WanMgrBase, TestGetVirtualIfNullAlias) 
{
    char* alias = NULL;

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked(alias);
    EXPECT_THAT(result, IsNull());
    WanMgr_VirtualIfaceData_release(result);
}

/* Test for WanMgr_GetVirtIfDataByAlias_locked() function.
 * WanMgr_GetVirtIfDataByAlias_locked expected to return correct Interface.
 */
TEST_F(WanMgrBase, TestGetVirtualIfVirtulName) 
{

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked("WANOE");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("ethwan0", result->Name);
    WanMgr_VirtualIfaceData_release(result);

    result = WanMgr_GetVirtIfDataByAlias_locked("VDSL");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("vdsl0", result->Name);
    WanMgr_VirtualIfaceData_release(result);

    result = WanMgr_GetVirtIfDataByAlias_locked("DOCSIS");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("erouter0", result->Name);
    WanMgr_VirtualIfaceData_release(result);

    result = WanMgr_GetVirtIfDataByAlias_locked("REMOTE_LTE");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("brRWAN", result->Name);
    WanMgr_VirtualIfaceData_release(result);
}


/* Test for TestConfigureTADWithDnsIpv4() function.
 * Get interface object with WanMgr_GetVirtIfDataByAlias_locked.
 * WanMgr_Configure_TAD_WCC expected to return ANSC_STATUS_SUCCESS for WCC_START, WCC_RESTART  and WCC_STOP operations when the interface has valid IPv4 DNS
 */
TEST_F(WanMgrBase, TestConfigureTADWithDnsIpv4) 
{

    //TODO : Set correct expecttations
    EXPECT_CALL(mockedRbus, rbusObject_Init(testing::_, testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusValue_Init(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusValue_SetString(testing::_, testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusObject_SetValue(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusValue_Release(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusObject_Release(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusMethod_Invoke(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mockedRbus, rbusEvent_Subscribe(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mockedRbus, rbusEvent_Unsubscribe(testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(RBUS_ERROR_SUCCESS));

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked("DOCSIS");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("erouter0", result->Name);
    strncpy (result->IP.Ipv4Data.dnsServer, "8.8.8.8", BUFLEN_64);
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_START));

    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_RESTART));
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_STOP));
    sleep(1) ; //sleep for 1 second since Configure_TAD_WCC executed in threads
    WanMgr_VirtualIfaceData_release(result);
}

/* Test for TestConfigureTADWithDnsIpv4() function.
 * Get interface object with WanMgr_GetVirtIfDataByAlias_locked.
 * WanMgr_Configure_TAD_WCC expected to return ANSC_STATUS_SUCCESS for WCC_START, WCC_RESTART  and WCC_STOP operations when the interface has valid IPv6 DNS
 */
TEST_F(WanMgrBase, TestConfigureTADWithDnsIpv6) 
{
    //TODO : Set correct expecttations
    EXPECT_CALL(mockedRbus, rbusValue_Init(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusValue_SetString(testing::_, testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusObject_SetValue(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusValue_Release(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusObject_Release(testing::_)).Times(testing::AnyNumber());
    EXPECT_CALL(mockedRbus, rbusMethod_Invoke(testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mockedRbus, rbusEvent_Subscribe(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(RBUS_ERROR_SUCCESS));
    EXPECT_CALL(mockedRbus, rbusEvent_Unsubscribe(testing::_, testing::_))
        .Times(testing::AnyNumber()).WillRepeatedly(testing::Return(RBUS_ERROR_SUCCESS));

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked("DOCSIS");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("erouter0", result->Name);
    strncpy (result->IP.Ipv6Data.nameserver, "fd01:1010:b066:798e:5528:ef02:7af4:e28", BUFLEN_64);
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_START));
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_RESTART));
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_STOP));
    sleep(1); //sleep for 1 second since Configure_TAD_WCC executed in threads
    WanMgr_VirtualIfaceData_release(result);
}

/* Test for TestConfigureTADWithDnsIpv4() function.
 * Get interface object with WanMgr_GetVirtIfDataByAlias_locked.
 * WanMgr_Configure_TAD_WCC expected to return ANSC_STATUS_FAILURE when the interface doesn't have valid dns.
 */
TEST_F(WanMgrBase, TestConfigureTADWithoutDns) 
{

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked("DOCSIS");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("erouter0", result->Name);
    EXPECT_EQ(ANSC_STATUS_FAILURE, WanMgr_Configure_TAD_WCC( result,  WCC_START));

    WanMgr_VirtualIfaceData_release(result);
}




/**********************************************************************************
 *Class for WanfailOver tetsing, Note: Currently only Telemetry event is tested.*
 **********************************************************************************/
class WanfailOver : public WanMgrBase
{
protected:

   WanMgr_FailOver_Controller_t  FWController; 
    WanfailOver() {}

    virtual ~WanfailOver() {}

    virtual void SetUp()
    {
        WanMgrBase::SetUp();
        memset(&FWController, 0, sizeof(FWController));
    }

    virtual void TearDown() 
    {

        WanMgrBase::TearDown();
    }
};

/* Mock funtion for t2_event_d
 */
extern "C" void t2_event_d(char *Telemtrylog, int a)
{
    if (mockWanMgr) {
        mockWanMgr->t2_event_d(Telemtrylog, a);
    }
}

extern "C" void WanMgr_UpdateIpFromCellularMgr (WanMgr_IfaceSM_Controller_t* pWanIfaceCtrl)
{
    if (mockWanMgr) {
        mockWanMgr->WanMgr_UpdateIpFromCellularMgr(pWanIfaceCtrl);
    }
}

extern "C" ANSC_STATUS IPCPStateChangeHandler (DML_VIRTUAL_IFACE* pVirtIf)
{
    if (mockWanMgr) {
        mockWanMgr->IPCPStateChangeHandler(pVirtIf);
    }
}

extern "C" ANSC_STATUS wanmgr_handle_dhcpv4_event_data(DML_VIRTUAL_IFACE* pVirtIf)
{
    if (mockWanMgr) {
        mockWanMgr->wanmgr_handle_dhcpv4_event_data(pVirtIf);
    }
}
#ifdef ENABLE_FEATURE_TELEMETRY2_0
/* 
 * Unit test for the WanMgr_TelemetryEventTrigger() function.
 * 
 * This test specifically targets the WAN_FAILOVER_SUCCESS event. 
 * When the WAN_FAILOVER_SUCCESS event is triggered, the function is expected to call t2_event_d 
 * to send telemetry events.
 * 
 * Two calls to t2_event_d are expected:
 * 1. The first call should report the telemetry timing.
 * 2. The second call should report the telemetry count.
 */

TEST_F(WanfailOver, TelemetryFailOverOSuccess) 
{
    //Start the FailOver Timer for telemetry
    memset(&(FWController.FailOverTimer), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(FWController.FailOverTimer));
    auto& exp1 =EXPECT_CALL(mockWanUtils,t2_event_d(StrCmpLen("WFO_FAILOVER_SUCCESS: Total Time Taken", strlen("WFO_FAILOVER_SUCCESS: Total Time Taken")), Eq(1)))
        .Times(1);
    EXPECT_CALL(mockWanUtils,t2_event_d(StrCmpLen("WAN_FAILOVER_SUCCESS_COUNT", strlen("WAN_FAILOVER_SUCCESS_COUNT")), Eq(1)))
        .Times(1)
        .After(exp1);
    FWController.TelemetryEvent = WAN_FAILOVER_SUCCESS;
    WanMgr_TelemetryEventTrigger(&FWController); 
}

/* 
 * Unit test for the WanMgr_TelemetryEventTrigger() function.
 * 
 * This test specifically targets the WAN_RESTORE_SUCCESS event. 
 * When the WAN_RESTORE_SUCCESS event is triggered, the function is expected to call t2_event_d 
 * to send telemetry events.
 * 
 * Two calls to t2_event_d are expected:
 * 1. The first call should report the telemetry timing.
 * 2. The second call should report the telemetry count.
 */
TEST_F(WanfailOver, TelemetryRestoreSuccess) 
{
    //Start the FailOver Timer for telemetry
    memset(&(FWController.FailOverTimer), 0, sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC_RAW, &(FWController.FailOverTimer));
    auto& exp1 =EXPECT_CALL(mockWanUtils,t2_event_d(StrCmpLen("WFO_RESTORE_SUCCESS: Total Time Taken", strlen("WFO_RESTORE_SUCCESS: Total Time Taken")), Eq(1)))
        .Times(1);
    EXPECT_CALL(mockWanUtils,t2_event_d(StrCmpLen("WAN_RESTORE_SUCCESS_COUNT", strlen("WAN_RESTORE_SUCCESS_COUNT")), Eq(1)))
        .Times(1)
        .After(exp1);
    FWController.TelemetryEvent = WAN_RESTORE_SUCCESS;
    WanMgr_TelemetryEventTrigger(&FWController); 
}

/* 
 * Unit test for the WanMgr_TelemetryEventTrigger() function.
 * 
 * This test specifically targets the WAN_RESTORE_FAIL event. 
 * When the WAN_RESTORE_FAIL event is triggered, the function is expected to call t2_event_d 
 * to send telemetry events.
 * 
 * One call to t2_event_d is expected:
 * 1. The second call should report the telemetry count.
 */
TEST_F(WanfailOver, TelemetryRestoreFail)
{
    EXPECT_CALL(mockWanUtils,t2_event_d(StrCmpLen("WAN_RESTORE_FAIL_COUNT", strlen("WAN_RESTORE_FAIL_COUNT")), Eq(1)))
        .Times(1);
    FWController.TelemetryEvent = WAN_RESTORE_FAIL;
    WanMgr_TelemetryEventTrigger(&FWController);
}

/* 
 * Unit test for the WanMgr_TelemetryEventTrigger() function.
 * 
 * This test specifically targets the WAN_FAILOVER_FAIL event. 
 * When the WAN_FAILOVER_FAIL event is triggered, the function is expected to call t2_event_d 
 * to send telemetry events.
 * 
 * One call to t2_event_d is expected:
 * 1. The second call should report the telemetry count.
 */
TEST_F(WanfailOver, TelemetryFailOverFail)
{
    EXPECT_CALL(mockWanUtils,t2_event_d(StrCmpLen("WAN_FAILOVER_FAIL_COUNT", strlen("WAN_FAILOVER_FAIL_COUNT")), Eq(1)))
        .Times(1);
    FWController.TelemetryEvent = WAN_FAILOVER_FAIL;
    WanMgr_TelemetryEventTrigger(&FWController);
}

#endif
