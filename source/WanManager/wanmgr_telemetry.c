#include "wanmgr_telemetry.h"

static char buf[128] = {0};

/*append api appends key value in pairs, separated by DELIMITER*/
static void wanmgr_telemetry_append_key_value(char* key, const char* value)
{
        if(value != NULL)
        {
                if(strlen(buf)>0)
                        strcat(buf,WANMGR_TELEMETRY_MARKER_ARG_DELIMITER);

                strcat(buf,key);
                strcat(buf,WANMGR_TELEMETRY_MARKER_KEY_VALUE_DELIMITER);
                strcat(buf,value);
        }
}

void wanmgr_get_data_from_interface_data(DML_WAN_IFACE *pInterface,WanMgr_TelemetryEvent_t MarkerID)
{
        wanmgr_telemetry_append_key_value(WANMGR_PHY_INTERFACE_STRING,pInterface->DisplayName);
        wanmgr_telemetry_append_key_value(WANMGR_WAN_INTERFACE_STRING,pInterface->Name);
        switch(MarkerID)
        {
                case WAN_INFO_IP_MODE:
//                      wanmgr_telemetry_append_key_value(WANMGR_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_IpModeStr[pInterface->VirtIfList->IP.Mode]);
                        break;
                default:
        }
        strcat(buf,"\0");
        t2_event_s(WanMgr_TelemetryEventStr[MarkerID],buf);

}
void wanmgr_get_data_from_virt_interface_data(DML_VIRTUAL_IFACE *p_VirtIf, WanMgr_TelemetryEvent_t MarkerID)
{
        DML_WAN_IFACE *pInterface = NULL;
        WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked(p_VirtIf->baseIfIdx);
        if(pWanDmlIfaceData != NULL)
        {
                pInterface = &(pWanDmlIfaceData->data);
                WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
        }
        wanmgr_telemetry_append_key_value(WANMGR_PHY_INTERFACE_STRING,pInterface->DisplayName);
        wanmgr_telemetry_append_key_value(WANMGR_WAN_INTERFACE_STRING,pInterface->Name);

        switch(MarkerID)
        {
                case WAN_INFO_IP_CONFIG_TYPE:
//                      wanmgr_telemetry_append_key_value(WANMGR_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_IpSourceStr[p_VirtIf->IPv4Source]);
                        break;
                case WAN_ERROR_VLAN_DOWN:
                case WAN_ERROR_VLAN_CREATION_FAILED:
                        wanmgr_telemetry_append_key_value(WANMGR_VIRT_WAN_INTERFACE_STRING,p_VirtIf->Name);
                        break;
                case WAN_INFO_CONNECTIVITY_CHECK_TYPE:
//                      wanmgr_telemetry_append_key_value(WANMGR_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_ConnectivityTypeStr[p_VirtIf->ConnectivityCheckType]);
                        break;
                default:
        }
        strcat(buf,"\0");
        t2_event_s(WanMgr_TelemetryEventStr[MarkerID],buf);

}

void wanmgr_telemetry_event(void *pStruct, WanMgr_TelemetryEvent_t MarkerID)
{
#ifdef ENABLE_FEATURE_TELEMETRY2_0
        memset(buf,sizeof(buf),0);

        switch(MarkerID)
        {
                case WAN_INFO_PHY_UP:
                case WAN_ERROR_PHY_DOWN:
                case WAN_WARN_IP_OBTAIN_TIMER_EXPIRED:
                case WAN_INFO_WAN_UP:
                case WAN_INFO_WAN_STANDBY:
                case WAN_ERROR_WAN_DOWN:
                case WAN_INFO_CONNECTIVITY_CHECK_STATUS_UP:
                case WAN_ERROR_CONNECTIVITY_CHECK_STATUS_DOWN:
                case WAN_WARN_CONNECTIVITY_CHECK_STATUS_FAILED:
                case WAN_INFO_IP_MODE:
                        wanmgr_get_data_from_interface_data((DML_WAN_IFACE *)pStruct,MarkerID);
                        break;
                case WAN_INFO_IPv4_UP:
                case WAN_ERROR_IPv4_DOWN:
                case WAN_INFO_IPv6_UP:
                case WAN_ERROR_IPv6_DOWN:
                case WAN_ERROR_MAPT_STATUS_DOWN:
                case WAN_INFO_MAPT_STATUS_UP:
                case WAN_ERROR_MAPT_STATUS_FAILED:
                case WAN_INFO_IP_CONFIG_TYPE:
                case WAN_INFO_CONNECTIVITY_CHECK_TYPE:
                case WAN_ERROR_VLAN_DOWN:
                case WAN_ERROR_VLAN_CREATION_FAILED:
                        wanmgr_get_data_from_virt_interface_data((DML_VIRTUAL_IFACE*)pStruct,MarkerID);
                        break;
                default:

        }
/*#else
 * Further Telemetry versions to be handled here
 */
#endif
}
