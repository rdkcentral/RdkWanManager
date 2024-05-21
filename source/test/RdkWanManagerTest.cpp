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
using namespace std;
using ::testing::_;
using ::testing::Return;
using ::testing::UnitTest;
using ::testing::Eq;
using ::testing::StrEq;
using ::testing::IsNull;
using ::testing::NotNull;

extern WANMGR_DATA_ST gWanMgrDataBase;
char    g_Subsystem[32]         = {0};

string ifaceName[4][2] = 
{   "vdsl0", "VDSL",
    "erouter0", "DOCSIS",
    "ethwan0","WANOE",
    "brRWAN","REMOTE_LTE"
};

class MockWanMgr 
{
public:
    MOCK_METHOD(void, WanMgrDml_GetIfaceData_release, (WanMgr_Iface_Data_t* pWanIfaceData), ());
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t *mutex), ());
    MOCK_METHOD(void, t2_event_d, (char *Telemtrylog, int a));

};

MATCHER_P2(StrCmpLen, expected_str, n, "") {
    return strncmp(arg, expected_str, n) == 0;
}

MockWanMgr *mockWanMgr = nullptr;

class WanMgrWCCTest : public ::testing::Test 
{
protected:
    MockWanMgr mock;

    WanMgrWCCTest() {}

    virtual ~WanMgrWCCTest() {}

    //Initializing WanManager mutex and Interface objects for the test
    virtual void SetUp() 
    {
        cout << "SetUp() : Test Suite Name: " << UnitTest::GetInstance()->current_test_info()->test_suite_name() 
            << " Test Case Name: " << UnitTest::GetInstance()->current_test_info()->name() << endl;

        mockWanMgr = &mock;
        //Initialise mutex attributes
        pthread_mutexattr_t     muttex_attr;
        pthread_mutexattr_init(&muttex_attr);
        pthread_mutexattr_settype(&muttex_attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&(gWanMgrDataBase.gDataMutex), &(muttex_attr));

        //init mock interfaces
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        pWanIfaceCtrl->pIface = (WanMgr_Iface_Data_t*) AnscAllocateMemory( sizeof(WanMgr_Iface_Data_t) * MAX_WAN_INTERFACE_ENTRY);
        memset( pWanIfaceCtrl->pIface, 0, ( sizeof(WanMgr_Iface_Data_t) * MAX_WAN_INTERFACE_ENTRY ) );
        pWanIfaceCtrl->ulTotalNumbWanInterfaces = 4;
        for(int idx = 0 ; idx <  pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++ )
        {
            WanMgr_Iface_Data_t*  pIfaceData  = &(pWanIfaceCtrl->pIface[idx]);

            WanMgr_IfaceData_Init(pIfaceData, idx);
            for(int i=0; i< pIfaceData->data.NoOfVirtIfs; i++)
            {
                DML_VIRTUAL_IFACE* p_VirtIf = (DML_VIRTUAL_IFACE *) AnscAllocateMemory( sizeof(DML_VIRTUAL_IFACE) );
                WanMgr_VirtIface_Init(p_VirtIf, i);
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
    virtual void TearDown() 
    {
        cout << " TearDown() : Test Suite Name: " << UnitTest::GetInstance()->current_test_info()->test_suite_name() 
            << " Test Case Name: " << UnitTest::GetInstance()->current_test_info()->name() << endl;

        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        for(int idx = 0 ; idx <  pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++ )
        {
            WanMgr_Iface_Data_t*  pIfaceData  = &(pWanIfaceCtrl->pIface[idx]);

            for(int i=0; i< pIfaceData->data.NoOfVirtIfs; i++)
            {
                DML_VIRTUAL_IFACE* virIface = pIfaceData->data.VirtIfList;
                while(virIface != NULL)
                {
                    DML_VIRTUAL_IFACE* temp = virIface;
                    virIface = virIface->next;
                    free(temp);
                }
            }
        }
        free(pWanIfaceCtrl->pIface);
        pWanIfaceCtrl->pIface = NULL;
        pthread_mutex_destroy(&(gWanMgrDataBase.gDataMutex));
        mockWanMgr = nullptr;
    }
};

/* Test for WanMgr_GetVirtIfDataByAlias_locked() function.
 * WanMgr_GetVirtIfDataByAlias_locked expected to return a NULL pointer when Alias name is not matched with the interface list.
 */
TEST_F(WanMgrWCCTest, TestGetVirtualIfWrongAlias) 
{
    char* alias = "WrongName";

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked(alias);
    EXPECT_THAT(result, IsNull());
    WanMgr_VirtualIfaceData_release(result);
}

/* Test for WanMgr_GetVirtIfDataByAlias_locked() function.
 * WanMgr_GetVirtIfDataByAlias_locked expected to return a NULL pointer when Alias parameter passed is NULL.
 */
TEST_F(WanMgrWCCTest, TestGetVirtualIfNullAlias) 
{
    char* alias = NULL;

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked(alias);
    EXPECT_THAT(result, IsNull());
    WanMgr_VirtualIfaceData_release(result);
}

/* Test for WanMgr_GetVirtIfDataByAlias_locked() function.
 * WanMgr_GetVirtIfDataByAlias_locked expected to return correct Interface.
 */
TEST_F(WanMgrWCCTest, TestGetVirtualIfVirtulName) 
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
TEST_F(WanMgrWCCTest, TestConfigureTADWithDnsIpv4) 
{

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked("DOCSIS");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("erouter0", result->Name);
    strncpy (result->IP.Ipv4Data.dnsServer, "8.8.8.8", BUFLEN_64);
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_START));

    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_RESTART));
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_STOP));
    WanMgr_VirtualIfaceData_release(result);
}

/* Test for TestConfigureTADWithDnsIpv4() function.
 * Get interface object with WanMgr_GetVirtIfDataByAlias_locked.
 * WanMgr_Configure_TAD_WCC expected to return ANSC_STATUS_SUCCESS for WCC_START, WCC_RESTART  and WCC_STOP operations when the interface has valid IPv6 DNS
 */
TEST_F(WanMgrWCCTest, TestConfigureTADWithDnsIpv6) 
{

    DML_VIRTUAL_IFACE* result = WanMgr_GetVirtIfDataByAlias_locked("DOCSIS");
    ASSERT_THAT(result, NotNull());
    EXPECT_STREQ("erouter0", result->Name);
    strncpy (result->IP.Ipv6Data.nameserver, "fd01:1010:b066:798e:5528:ef02:7af4:e28", BUFLEN_64);
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_START));
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_RESTART));
    EXPECT_EQ(ANSC_STATUS_SUCCESS, WanMgr_Configure_TAD_WCC( result,  WCC_STOP));
    WanMgr_VirtualIfaceData_release(result);
}

/* Test for TestConfigureTADWithDnsIpv4() function.
 * Get interface object with WanMgr_GetVirtIfDataByAlias_locked.
 * WanMgr_Configure_TAD_WCC expected to return ANSC_STATUS_FAILURE when the interface doesn't have valid dns.
 */
TEST_F(WanMgrWCCTest, TestConfigureTADWithoutDns) 
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
class WanfailOver : public WanMgrWCCTest
{
protected:

   WanMgr_FailOver_Controller_t  FWController; 
    WanfailOver() {}

    virtual ~WanfailOver() {}

    virtual void SetUp()
    {
        WanMgrWCCTest::SetUp();
        memset(&FWController, 0, sizeof(FWController));
    }

    virtual void TearDown() 
    {

        WanMgrWCCTest::TearDown();
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
    auto& exp1 =EXPECT_CALL(mock,t2_event_d(StrCmpLen("WFO_FAILOVER_SUCCESS: Total Time Taken", strlen("WFO_FAILOVER_SUCCESS: Total Time Taken")), Eq(1)))
        .Times(1);
    EXPECT_CALL(mock,t2_event_d(StrCmpLen("WAN_FAILOVER_SUCCESS_COUNT", strlen("WAN_FAILOVER_SUCCESS_COUNT")), Eq(1)))
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
    auto& exp1 =EXPECT_CALL(mock,t2_event_d(StrCmpLen("WFO_RESTORE_SUCCESS: Total Time Taken", strlen("WFO_RESTORE_SUCCESS: Total Time Taken")), Eq(1)))
        .Times(1);
    EXPECT_CALL(mock,t2_event_d(StrCmpLen("WAN_RESTORE_SUCCESS_COUNT", strlen("WAN_RESTORE_SUCCESS_COUNT")), Eq(1)))
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
    EXPECT_CALL(mock,t2_event_d(StrCmpLen("WAN_RESTORE_FAIL_COUNT", strlen("WAN_RESTORE_FAIL_COUNT")), Eq(1)))
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
    EXPECT_CALL(mock,t2_event_d(StrCmpLen("WAN_FAILOVER_FAIL_COUNT", strlen("WAN_FAILOVER_FAIL_COUNT")), Eq(1)))
        .Times(1);
    FWController.TelemetryEvent = WAN_FAILOVER_FAIL;
    WanMgr_TelemetryEventTrigger(&FWController);
}

