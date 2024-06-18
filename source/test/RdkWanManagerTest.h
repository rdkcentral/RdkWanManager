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

extern "C" {
#include "wanmgr_data.h"
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

#endif
