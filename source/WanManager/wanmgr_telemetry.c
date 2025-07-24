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
	CcspTraceInfo(("%s %d: KAVYA.\n",__FUNCTION__, __LINE__));
    if(Marker == NULL)
    {
	    CcspTraceInfo(("%s %d: KAVYA.\n",__FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE; 
    }
    CcspTraceInfo(("%s %d: KAVYA.\n",__FUNCTION__, __LINE__));
#ifdef ENABLE_FEATURE_TELEMETRY2_0
    CcspTraceInfo(("%s %d: KAVYA.\n",__FUNCTION__, __LINE__));
    if(ANSC_STATUS_FAILURE == wanmgr_process_T2_telemetry_event(Marker))
    {
        CcspTraceError(("%s %d: ERROR processing telemetry event %s.\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]));
    }
    else
    {
        CcspTraceInfo(("%s %d: SUCCESS processing telemetry event %s.\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]));
    }
#endif	    
    CcspTraceInfo(("%s %d: KAVYA.\n",__FUNCTION__, __LINE__));
    return ANSC_STATUS_SUCCESS;
}
