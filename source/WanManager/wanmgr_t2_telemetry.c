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

/*This api processes the Marker struct,
 * gets the data required to send to T2 marker*/
ANSC_STATUS wanmgr_process_T2_telemetry_event(WanMgr_Telemetry_Marker_t *Marker)
{
    DML_WAN_IFACE *pIntf = Marker->pInterface;
    DML_VIRTUAL_IFACE *pVirtIntf = Marker->pVirtInterface;
    memset(buf,0,sizeof(buf));
    char tempStr[128] = {0};

    if(pIntf == NULL && pVirtIntf == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }
    if(pIntf)
    {
        wanmgr_telemetry_append_key_value(WANMGR_T2_PHY_INTERFACE_STRING,pIntf->DisplayName);
        wanmgr_telemetry_append_key_value(WANMGR_T2_WAN_INTERFACE_STRING,pIntf->Name);
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
            wanmgr_telemetry_append_key_value(WANMGR_T2_PHY_INTERFACE_STRING,pIntf->DisplayName);
            wanmgr_telemetry_append_key_value(WANMGR_T2_WAN_INTERFACE_STRING,pIntf->Name);
        }
    }
    else
    {
        pVirtIntf = pIntf->VirtIfList ;
    }

    //By default, send T2 event for active interface only.
    sendEventOnActiveOnly = 1;
    switch(Marker->enTelemetryMarkerID)
    {
        case WAN_INFO_IP_MODE:
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_IpModeStr[pVirtIntf->IP.Mode]);
            break;
        case WAN_INFO_IPv4_CONFIG_TYPE:
	    memset(tempStr,0,sizeof(tempStr));
	    strcat(tempStr,"IPv4Source-");
	    strcat(tempStr,WanMgr_Telemetry_IpSourceStr[pVirtIntf->IP.IPv4Source]);
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING, tempStr);
	    break;
        case WAN_INFO_IPv6_CONFIG_TYPE:
	    memset(tempStr,0,sizeof(tempStr));
            strcat(tempStr,"IPv6Source-");
	    strcat(tempStr,WanMgr_Telemetry_IpSourceStr[pVirtIntf->IP.IPv6Source]);
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,tempStr);
            break;
        case WAN_ERROR_VLAN_DOWN:
        case WAN_ERROR_VLAN_CREATION_FAILED:
            wanmgr_telemetry_append_key_value(WANMGR_T2_VIRT_WAN_INTERFACE_STRING,pVirtIntf->Name);
            break;
        case WAN_INFO_CONNECTIVITY_CHECK_TYPE:
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_ConnectivityTypeStr[pVirtIntf->IP.ConnectivityCheckType]);
            break;
        case WAN_INFO_IPv4_UP:
        case WAN_ERROR_IPv4_DOWN:
        case WAN_INFO_IPv6_UP:
        case WAN_ERROR_IPv6_DOWN:
        case WAN_INFO_MAPT_STATUS_UP:
        case WAN_ERROR_MAPT_STATUS_DOWN:
            char wan_ifname[16] = {0};
            memset(wan_ifname,0,sizeof(wan_ifname));
            syscfg_get(NULL, "wan_physical_ifname", wan_ifname, sizeof(wan_ifname));
            CcspTraceInfo(("%s %d Kavya syscfg wan_ifname = [%s]\n",__FUNCTION__, __LINE__,wan_ifname));
            CcspTraceInfo(("%s %d Kavya pVirtIntf->Name = [%s]\n",__FUNCTION__, __LINE__,pVirtIntf->Name));
            if(strncmp(pVirtIntf->Name,wan_ifname, sizeof(wan_ifname) != 0))
            {
                wanmgr_telemetry_append_key_value(WANMGR_T2_SELECTION_STATUS_STRING,(pIntf->Selection.Status == WAN_IFACE_ACTIVE) ? "Active" : (pIntf->Selection.Status == WAN_IFACE_SELECTED) ? "Selected":"Standby");
            }
            else
	    {
                wanmgr_telemetry_append_key_value(WANMGR_T2_SELECTION_STATUS_STRING,(pIntf->Selection.Status == WAN_IFACE_ACTIVE || pIntf->Selection.Status == WAN_IFACE_VALIDATING || pIntf->Selection.Status == WAN_IFACE_SELECTED ) ? "Active" : "Standby");		    
	    }
	    sendEventOnActiveOnly = 0;
            break;
        default:
	    ;
    }
    strcat(buf,"\0");
    CcspTraceInfo(("%s %d: Kavya Arguments = [%s] \n",__FUNCTION__, __LINE__,buf));
    //Telemetry start
    v_secure_system("echo '**********%s**********' >> /tmp/kavya_syscfg.txt",WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]);
    v_secure_system("syscfg show | grep -i wan >> /tmp/kavya_syscfg.txt");
    v_secure_system("echo '**********%s**********' >> /tmp/kavya_sysevent.txt",WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID]);
    v_secure_system("sysevent show /tmp/sysevent.txt ; cat /tmp/sysevent.txt | grep -i wan >> /tmp/kavya_sysevent.txt");
    
    if(sendEventOnActiveOnly)
    {
        char wan_ifname[16] = {0};
        memset(wan_ifname,0,sizeof(wan_ifname));
        syscfg_get(NULL, "wan_physical_ifname", wan_ifname, sizeof(wan_ifname));
        CcspTraceInfo(("%s %d Kavya syscfg wan_ifname = [%s]\n",__FUNCTION__, __LINE__,wan_ifname));
        CcspTraceInfo(("%s %d Kavya pVirtIntf->Name = [%s]\n",__FUNCTION__, __LINE__,pVirtIntf->Name));

        if(strncmp(pVirtIntf->Name,wan_ifname, sizeof(wan_ifname) != 0))
        {
//            CcspTraceInfo(("%s %d: WANMANAGER_TELEMETRY Not sending event since interface is not active Event-[%s],argument-[%s]\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));
            return ANSC_STATUS_SUCCESS;
  
        }
        else
        {
            t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
            CcspTraceInfo(("%s %d: WANMANAGER_TELEMETRY Kavya Sent Telemetry event [%s] with arguments = [%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));
        }
    }
    else
    {
        t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
        CcspTraceInfo(("%s %d: WANMANAGER_TELEMETRY Kavya Sent Telemetry event [%s] with arguments = [%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));
    }
    
    return ANSC_STATUS_SUCCESS;
}
