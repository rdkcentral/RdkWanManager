#include "wanmgr_t2_telemetry.h"
#include "wanmgr_rdkbus_utils.h"

static char buf[128] = {0};
static int sendEventOnActiveOnly;

/*append api appends key value in pairs, separated by DELIMITER*/
static void wanmgr_telemetry_append_key_value(char* key, const char* value)
{
    if(value != NULL)
    {
        if(strlen(buf)>0)
        {
            strcat(buf,WANMGR_T2_TELEMETRY_MARKER_ARG_DELIMITER);
        }

        strcat(buf,key);
        strcat(buf,WANMGR_T2_TELEMETRY_MARKER_KEY_VALUE_DELIMITER);
        strcat(buf,value);
    }
}
/* Send Telemetry event based on interface is active or not*/
/*
void wanmgr_send_T2_telemetry_event(WanMgr_Telemetry_Marker_t *Marker)
{
	CcspTraceInfo(("%s %d Kavya Enter\n",__FUNCTION__, __LINE__));
    DML_WAN_IFACE *pIntf = Marker->pInterface;
    if(sendEventOnActiveOnly)
    {
	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
        if(pIntf->Selection.Status == WAN_IFACE_ACTIVE)
	{
		CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            CcspTraceInfo(("%s %d: Kavya Sent Telemetry event [%s] with arguments = [%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));		
	}
	else
	{
		CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            CcspTraceInfo(("%s %d:Kavya Interface not active, not sending telemetry event for [%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]));
	}
    }
    else
    {
	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
        t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
        CcspTraceInfo(("%s %d: Kavya Telemetry event [%s] with arguments = [%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));	    
    }
}*/

/*This api processes the Marker struct,
 * gets the data required to send to T2 marker*/
ANSC_STATUS wanmgr_process_T2_telemetry_event(WanMgr_Telemetry_Marker_t *Marker)
{
	CcspTraceInfo(("%s %d: Kavya Enter\n",__FUNCTION__, __LINE__));
    DML_WAN_IFACE *pIntf = Marker->pInterface;
    DML_VIRTUAL_IFACE *pVirtIntf = Marker->pVirtInterface;
    memset(buf,0,sizeof(buf));
    char tempStr[128] = {0};
CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
    if(pIntf == NULL && pVirtIntf == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }
CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
    if(pIntf)
    {
        wanmgr_telemetry_append_key_value(WANMGR_T2_PHY_INTERFACE_STRING,pIntf->DisplayName);
        wanmgr_telemetry_append_key_value(WANMGR_T2_WAN_INTERFACE_STRING,pIntf->Name);
    }
    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
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
            wanmgr_telemetry_append_key_value(WANMGR_T2_PHY_INTERFACE_STRING,pIntf->DisplayName);
            wanmgr_telemetry_append_key_value(WANMGR_T2_WAN_INTERFACE_STRING,pIntf->Name);
        }
    }
    else
    {
        pVirtIntf = pIntf->VirtIfList ;
    }
    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
    //By default, send T2 event for active interface only.
    sendEventOnActiveOnly = 1;
CcspTraceInfo(("%s %d Kavya sendEventOnActiveOnly = [%d]\n",__FUNCTION__, __LINE__,sendEventOnActiveOnly));
    switch(Marker->enTelemetryMarkerID)
    {
        case WAN_INFO_IP_MODE:
		CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_IpModeStr[pVirtIntf->IP.Mode]);
            break;
        case WAN_INFO_IPv4_CONFIG_TYPE:
	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
	    memset(tempStr,0,sizeof(tempStr));
	    strcat(tempStr,"IPv4Source-");
	    strcat(tempStr,WanMgr_Telemetry_IpSourceStr[pVirtIntf->IP.IPv4Source]);
	    CcspTraceInfo(("%s %d Kavya tempStr = [%s]\n",__FUNCTION__, __LINE__,tempStr));
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING, tempStr);
	    break;
        case WAN_INFO_IPv6_CONFIG_TYPE:
	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
	    memset(tempStr,0,sizeof(tempStr));
            strcat(tempStr,"IPv6Source-");
	    strcat(tempStr,WanMgr_Telemetry_IpSourceStr[pVirtIntf->IP.IPv6Source]);
	    CcspTraceInfo(("%s %d Kavya tempStr = [%s]\n",__FUNCTION__, __LINE__,tempStr));
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,tempStr);
            break;
        case WAN_ERROR_VLAN_DOWN:
        case WAN_ERROR_VLAN_CREATION_FAILED:
	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            wanmgr_telemetry_append_key_value(WANMGR_T2_VIRT_WAN_INTERFACE_STRING,pVirtIntf->Name);
            break;
        case WAN_INFO_CONNECTIVITY_CHECK_TYPE:
	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_ConnectivityTypeStr[pVirtIntf->IP.ConnectivityCheckType]);
            break;
        case WAN_INFO_IPv4_UP:
        case WAN_ERROR_IPv4_DOWN:
        case WAN_INFO_IPv6_UP:
        case WAN_ERROR_IPv6_DOWN:
        case WAN_INFO_MAPT_STATUS_UP:
        case WAN_ERROR_MAPT_STATUS_DOWN:
	    CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            wanmgr_telemetry_append_key_value(WANMGR_T2_SELECTION_STATUS_STRING,(pIntf->Selection.Status == WAN_IFACE_ACTIVE) ? "Active" : (pIntf->Selection.Status == WAN_IFACE_SELECTED) ? "Selected":"Standby");
	    sendEventOnActiveOnly = 0;
	    CcspTraceInfo(("%s %d Kavya sendEventOnActiveOnly = [%d]\n",__FUNCTION__, __LINE__,sendEventOnActiveOnly));
            break;
        default:
	    ;
    }
    strcat(buf,"\0");
    CcspTraceInfo(("%s %d: Kavya Arguments = [%s] \n",__FUNCTION__, __LINE__,buf));
    CcspTraceInfo(("%s %d: Kavya SelectionStatus = [%d] \n",__FUNCTION__, __LINE__,pIntf->Selection.Status));
    CcspTraceInfo(("%s %d: Kavya SelectionEnable = [%d] \n",__FUNCTION__, __LINE__,pIntf->Selection.Enable));
    CcspTraceInfo(("%s %d: Kavya BaseInterfaceStatus = [%d] \n",__FUNCTION__, __LINE__,pIntf->BaseInterfaceStatus));
    //Telemetry start
    v_secure_system("echo '*****%s*****' >> /tmp/kavya_syscfg.txt",WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]);
    v_secure_system("syscfg show | grep -i wan >> /tmp/kavya_syscfg.txt");
    v_secure_system("echo '**********' >> /tmp/kavya_syscfg.txt");
    v_secure_system("echo '*****%s*****' >> /tmp/kavya_sysevent.txt",WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]);
    v_secure_system("sysevent show /tmp/sysevent.txt ; cat /tmp/sysevent.txt | grep -i wan >> /tmp/kavya_sysevent.txt");
    v_secure_system("echo '**********' >> /tmp/kavya_sysevent.txt");
    
    CcspTraceInfo(("%s %d Kavya Send Marker\n",__FUNCTION__, __LINE__));
    if(sendEventOnActiveOnly)
    {
            CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
        if(pIntf->Selection.Status == WAN_IFACE_ACTIVE || pIntf->Selection.Status == WAN_IFACE_SELECTED)
        {
                CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
            CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            CcspTraceInfo(("%s %d: Kavya Sent Telemetry event [%s] with arguments = [%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));
        }
        else
        {
                CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
            CcspTraceInfo(("%s %d:Kavya Interface not active [%s], arguments:[%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));
        }
    }
    else
    {
            CcspTraceInfo(("%s %d Kavya\n",__FUNCTION__, __LINE__));
        t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
        CcspTraceInfo(("%s %d: Kavya Telemetry event [%s] with arguments = [%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));
    }
    
//    wanmgr_send_T2_telemetry_event(Marker);
    CcspTraceInfo(("%s %d Kavya SUCCESS\n",__FUNCTION__, __LINE__));
    return ANSC_STATUS_SUCCESS;
}
