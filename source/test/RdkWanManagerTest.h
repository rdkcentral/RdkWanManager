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


class WanMgrBase : public ::testing::Test
{
protected:
    MockWanMgr mock;

    WanMgrBase() {}

    virtual ~WanMgrBase() {}

    //Initializing WanManager mutex and Interface objects for the test
    virtual void SetUp();
    //Clearing Interface objects created for the test
    virtual void TearDown();
};

#endif
