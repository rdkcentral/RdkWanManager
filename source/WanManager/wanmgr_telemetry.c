#include "wanmgr_telemetry.h"
#include "wanmgr_rdkbus_utils.h"
#ifdef ENABLE_FEATURE_TELEMETRY2_0
#include "wanmgr_t2_telemetry.h"
#endif

/*A generic api for telemetry marker,
 * it gets the necessary parameter structs and
 * sends them to appropriate api to be processed.*/

void wanmgr_telemetry_event(WanMgr_Telemetry_Marker_t *Marker)
{
    CcspTraceInfo(("%s %d: Enter.\n",__FUNCTION__, __LINE__));	
    if(Marker == NULL)
    {
        return ANSC_STATUS_FAILURE; 
    }
//    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
    switch(Marker->enTelemetryMarkerID)
    {
        case WAN_INFO_PHY_UP:
        case WAN_ERROR_PHY_DOWN:
        case WAN_WARN_IP_OBTAIN_TIMER_EXPIRED:
        case WAN_INFO_WAN_UP:
        case WAN_INFO_WAN_STANDBY:
        case WAN_ERROR_WAN_DOWN:
        case WAN_INFO_CONNECTIVITY_CHECK_STATUS_UP_IPV4:
        case WAN_INFO_CONNECTIVITY_CHECK_STATUS_UP_IPV6:
        case WAN_ERROR_CONNECTIVITY_CHECK_STATUS_DOWN:
        case WAN_WARN_CONNECTIVITY_CHECK_STATUS_FAILED_IPV4:
        case WAN_WARN_CONNECTIVITY_CHECK_STATUS_FAILED_IPV6:
        case WAN_WARN_CONNECTIVITY_CHECK_STATUS_IDLE_IPV4:
        case WAN_WARN_CONNECTIVITY_CHECK_STATUS_IDLE_IPV6:
        case WAN_INFO_IP_MODE:
        case WAN_INFO_IPv4_UP:
        case WAN_ERROR_IPv4_DOWN:
        case WAN_INFO_IPv6_UP:
        case WAN_ERROR_IPv6_DOWN:
        case WAN_ERROR_MAPT_STATUS_DOWN:
        case WAN_INFO_MAPT_STATUS_UP:
        case WAN_ERROR_MAPT_STATUS_FAILED:
        case WAN_INFO_IPv4_CONFIG_TYPE:
        case WAN_INFO_IPv6_CONFIG_TYPE:
        case WAN_INFO_CONNECTIVITY_CHECK_TYPE:
        case WAN_ERROR_VLAN_DOWN:
        case WAN_ERROR_VLAN_CREATION_FAILED:
//		CcspTraceInfo(("%s %d Kavya In CASE\n",__FUNCTION__, __LINE__));
#ifdef ENABLE_FEATURE_TELEMETRY2_0
            if(ANSC_STATUS_FAILURE == wanmgr_process_T2_telemetry_event(Marker))
            {
//		    CcspTraceInfo(("%s %d Kavya Failed to send marker\n",__FUNCTION__, __LINE__));
                CcspTraceError(("%s %d: ERROR processing telemetry event %s.\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]));
            }
	    else
	    {
//		    CcspTraceInfo(("%s %d Kavya Sent marker\n",__FUNCTION__, __LINE__));
                CcspTraceInfo(("%s %d: SUCCESS processing telemetry event %s.\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]));
	    }
#endif	    
//	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            break;
        default:
	    ;
    }
}
