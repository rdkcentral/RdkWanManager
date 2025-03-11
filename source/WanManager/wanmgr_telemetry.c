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
ANSC_STATUS wanmgr_send_T2_telemetry_event(WanMgr_Telemetry_Marker_t *Marker)
{
        DML_WAN_IFACE *pIntf = Marker->pInterface;
        DML_VIRTUAL_IFACE *pVirtIntf = Marker->pVirtInterface;

        if(pIntf == NULL && pVirtIntf == NULL){
               return ANSC_STATUS_FAILURE; 
        }

        if(pIntf)
        {
                wanmgr_telemetry_append_key_value(WANMGR_PHY_INTERFACE_STRING,pIntf->DisplayName);
                wanmgr_telemetry_append_key_value(WANMGR_WAN_INTERFACE_STRING,pIntf->Name);                      
        }
        if(pVirtIntf)
        {
                if(pIntf == NULL)
                {
                        WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pVirtIntf->baseIfIdx);
                        if(pWanDmlIfaceData != NULL)
                        {
                            pIntf = &(pWanDmlIfaceData->data);
                            WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
                        }   
                        wanmgr_telemetry_append_key_value(WANMGR_PHY_INTERFACE_STRING,pIntf->DisplayName);
                        wanmgr_telemetry_append_key_value(WANMGR_WAN_INTERFACE_STRING,pIntf->Name); 
                }
        }
        else
        {
                pVirtIntf = pIntf->VirtIfList ;
        }
        switch(Marker->enTelemetryMarkerID)
        {
                case WAN_INFO_IP_MODE:                        
                        wanmgr_telemetry_append_key_value(WANMGR_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_IpModeStr[pVirtIntf->IP.Mode]);
                        break;
                case WAN_INFO_IP_CONFIG_TYPE:
//                        wanmgr_telemetry_append_key_value(WANMGR_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_IpSourceStr[pVirtIntf->IPv4Source]);
                        break;
                case WAN_ERROR_VLAN_DOWN:
                case WAN_ERROR_VLAN_CREATION_FAILED:
                        wanmgr_telemetry_append_key_value(WANMGR_VIRT_WAN_INTERFACE_STRING,pVirtIntf->Name);
                        break;
                case WAN_INFO_CONNECTIVITY_CHECK_TYPE:
//                        wanmgr_telemetry_append_key_value(WANMGR_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_ConnectivityTypeStr[pVirtIntf->ConnectivityCheckType]);
                        break;                         
                default:
        }
        strcat(buf,"\0");
        t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
        return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS wanmgr_telemetry_event(WanMgr_Telemetry_Marker_t *Marker)
{
#ifdef ENABLE_FEATURE_TELEMETRY2_0
        memset(buf,sizeof(buf),0);
	switch(Marker->enTelemetryMarkerID)
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
        		if(wanmgr_send_T2_telemetry_event(Marker) != ANSC_STATUS_SUCCESS)
		        {
        		        return ANSC_STATUS_FAILURE;
	        	}
			break;
		default:
	}
/*#else
 * Further Telemetry versions to be handled here
 */
#endif
        return ANSC_STATUS_SUCCESS;
}

