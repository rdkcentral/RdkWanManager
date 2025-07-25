#include "wanmgr_telemetry.h"
#include "wanmgr_rdkbus_utils.h"
#include "wanmgr_data.h"
#ifdef ENABLE_FEATURE_TELEMETRY2_0
#include "wanmgr_t2_telemetry.h"
#endif

/*A generic api for telemetry marker,
 * it gets the necessary parameter structs and
 * sends them to appropriate api to be processed.*/

ANSC_STATUS WanMgr_ProcessTelemetryMarker( DML_VIRTUAL_IFACE *pVirtIf , WanMgr_TelemetryEvent_t telemetry_marker)
{
	CcspTraceInfo(("%s %d: KAVYA.\n",__FUNCTION__, __LINE__));
    if(pVirtIf == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }
CcspTraceInfo(("%s %d: KAVYA. pVirtIf = [%lu] \n",__FUNCTION__, __LINE__,pVirtIf));
CcspTraceInfo(("%s %d: KAVYA. pVirtIf->Name = [%s] \n",__FUNCTION__, __LINE__,pVirtIf->Name));
CcspTraceInfo(("%s %d: KAVYA. pVirtIf->baseIfIdx = [%d] \n",__FUNCTION__, __LINE__,pVirtIf->baseIfIdx));
    WanMgr_Iface_Data_t* pWanDmlIfaceData = WanMgr_GetIfaceData_locked(pVirtIf->baseIfIdx);
    if(pWanDmlIfaceData == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }
CcspTraceInfo(("%s %d: KAVYA.\n",__FUNCTION__, __LINE__));
    DML_WAN_IFACE *pIntf = &(pWanDmlIfaceData->data);
    WanMgrDml_GetIfaceData_release(pWanDmlIfaceData);
CcspTraceInfo(("%s %d: KAVYA. pIntf = [%lu]\n",__FUNCTION__, __LINE__,pIntf));
CcspTraceInfo(("%s %d: KAVYA pIntf->DisplayName  = [%s].\n",__FUNCTION__, __LINE__,pIntf->DisplayName));
CcspTraceInfo(("%s %d: KAVYA pIntf->Name  = [%s].\n",__FUNCTION__, __LINE__,pIntf->Name));

    WanMgr_Telemetry_Marker_t Marker = {0};
    Marker.pVirtInterface = pVirtIf;
    Marker.pInterface = pIntf;
    Marker.enTelemetryMarkerID = telemetry_marker;
CcspTraceInfo(("%s %d: KAVYA pVirtInterface = [%lu].\n",__FUNCTION__, __LINE__,Marker.pVirtInterface));
CcspTraceInfo(("%s %d: KAVYA pInterface  = [%lu].\n",__FUNCTION__, __LINE__,Marker.pInterface));
CcspTraceInfo(("%s %d: KAVYA Marker.pInterface->DisplayName  = [%s].\n",__FUNCTION__, __LINE__,Marker.pInterface->DisplayName));
CcspTraceInfo(("%s %d: KAVYA Marker.pInterface->Name  = [%s].\n",__FUNCTION__, __LINE__,Marker.pInterface->Name));
//    wanmgr_telemetry_event(&Marker);
#ifdef ENABLE_FEATURE_TELEMETRY2_0

    CcspTraceInfo(("%s %d: KAVYA.\n",__FUNCTION__, __LINE__));
    if(ANSC_STATUS_FAILURE == wanmgr_process_T2_telemetry_event(&Marker))
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
/*
ANSC_STATUS wanmgr_telemetry_event(WanMgr_Telemetry_Marker_t *Marker)
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
*/
