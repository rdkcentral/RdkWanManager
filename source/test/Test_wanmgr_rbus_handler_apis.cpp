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
#include <mocks/mock_rbus.h>

extern "C" {
#include "wanmgr_data.h"
rbusError_t WanMgr_Interface_GetHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts);
}
#include "RdkWanManagerTest.h"

extern WANMGR_DATA_ST gWanMgrDataBase;

extern MockWanMgr *mockWanMgr;

class RbusHandlerTest : public WanMgrBase
{
protected:
    RbusHandlerTest() 
    {
    }
    virtual ~RbusHandlerTest(){}

    virtual void SetUp()
    {
        WanMgrBase::SetUp();
        WanMgr_IfaceCtrl_Data_t* pWanIfaceCtrl = &(gWanMgrDataBase.IfaceCtrl);
        for(int idx = 0 ; idx <  pWanIfaceCtrl->ulTotalNumbWanInterfaces; idx++ )
        {
            WanMgr_Iface_Data_t*  pIfaceData  = &(pWanIfaceCtrl->pIface[idx]);

            strncpy(pIfaceData->data.VirtIfList->IP.Ipv4Data.ip, "9.9.9.9",sizeof(pIfaceData->data.VirtIfList->IP.Ipv4Data.ip));
            strncpy(pIfaceData->data.VirtIfList->IP.Ipv6Data.address, "2a02:c7f:8253:3900::1",sizeof(pIfaceData->data.VirtIfList->IP.Ipv6Data.address));
            strncpy(pIfaceData->data.VirtIfList->IP.Ipv6Data.sitePrefix, "2a02:c7f:8253:3900::1",sizeof(pIfaceData->data.VirtIfList->IP.Ipv6Data.sitePrefix));
        }
    }

    virtual void TearDown()
    {

        WanMgrBase::TearDown();
    }
};


TEST_F(RbusHandlerTest, InterfaceGetHandlerIPV6Address)
{
    rbusHandle_t handle  = nullptr;
    rbusProperty_t property  = nullptr;
    rbusGetHandlerOptions_t *opts = nullptr;
    std::string DmlName = "Device.X_RDK_WanManager.Interface.1.VirtualInterface.1.IP.IPv6Address";
    
    EXPECT_CALL(mockedRbus, rbusValue_Init(_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusValue_SetString(_,StrEq("2a02:c7f:8253:3900::1"))).Times(1);
    EXPECT_CALL(mockedRbus, rbusProperty_SetValue(_,_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusValue_Release(_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusProperty_GetName(Eq(nullptr))).Times(1).WillOnce(Return(DmlName.c_str()));
    
    EXPECT_EQ(RBUS_ERROR_SUCCESS, WanMgr_Interface_GetHandler(handle, property, opts));

}

TEST_F(RbusHandlerTest, InterfaceGetHandlerIPV4Address)
{
    rbusHandle_t handle  = nullptr;
    rbusProperty_t property  = nullptr;
    rbusGetHandlerOptions_t *opts  = nullptr;
    std::string DmlName = "Device.X_RDK_WanManager.Interface.1.VirtualInterface.1.IP.IPv4Address";

    EXPECT_CALL(mockedRbus, rbusValue_Init(_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusValue_Release(_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusProperty_SetValue(_,_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusValue_SetString(_,StrEq("9.9.9.9"))).Times(1);
    EXPECT_CALL(mockedRbus, rbusProperty_GetName(Eq(nullptr))).Times(1).WillOnce(Return(DmlName.c_str()));
    
    EXPECT_EQ(RBUS_ERROR_SUCCESS, WanMgr_Interface_GetHandler(handle, property, opts));
}

TEST_F(RbusHandlerTest, InterfaceGetHandlerIPV6Prefix)
{
    rbusHandle_t handle = nullptr;
    rbusProperty_t property = nullptr;
    rbusGetHandlerOptions_t *opts  = nullptr;
    std::string DmlName = "Device.X_RDK_WanManager.Interface.1.VirtualInterface.1.IP.IPv6Prefix";
    EXPECT_CALL(mockedRbus, rbusValue_Init(_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusValue_SetString(_,StrEq("2a02:c7f:8253:3900::1"))).Times(1);
    EXPECT_CALL(mockedRbus, rbusProperty_SetValue(_,_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusValue_Release(_)).Times(1);
    EXPECT_CALL(mockedRbus, rbusProperty_GetName(_)).Times(1).WillOnce(Return(DmlName.c_str()));
    
    EXPECT_EQ(RBUS_ERROR_SUCCESS, WanMgr_Interface_GetHandler(handle, property, opts));
    cout << "PARTHI " <<__func__<< __LINE__ << endl;
}

