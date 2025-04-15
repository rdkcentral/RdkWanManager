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
    if(Marker == NULL)
    {
        return ANSC_STATUS_FAILURE; 
    }

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
#ifdef ENABLE_FEATURE_TELEMETRY2_0
            if(ANSC_STATUS_FAILURE == wanmgr_process_T2_telemetry_event(Marker))
            {
                CcspTraceError(("%s %d: Error sending Telemetry event %s.\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]));
            }
#endif	    
            break;
        default:
    }
}
