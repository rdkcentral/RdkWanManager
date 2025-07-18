#include "wanmgr_t2_telemetry.h"
#include "wanmgr_rdkbus_utils.h"

static char buf[128] = {0};

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

    wanmgr_telemetry_append_key_value(WANMGR_T2_PHY_INTERFACE_STRING,pIntf->DisplayName);
    wanmgr_telemetry_append_key_value(WANMGR_T2_WAN_INTERFACE_STRING,pIntf->Name);

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

	case WAN_INFO_CONNECTIVITY_CHECK_STATUS_UP_IPV4:
	case WAN_WARN_CONNECTIVITY_CHECK_STATUS_FAILED_IPV4:
	case WAN_WARN_CONNECTIVITY_CHECK_STATUS_IDLE_IPV4:
	case WAN_ERROR_CONNECTIVITY_CHECK_STATUS_DOWN_IPV4:
	    wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,"IPv4");
	    break;

	case WAN_INFO_CONNECTIVITY_CHECK_STATUS_UP_IPV6:
	case WAN_WARN_CONNECTIVITY_CHECK_STATUS_FAILED_IPV6:
	case WAN_WARN_CONNECTIVITY_CHECK_STATUS_IDLE_IPV6:
	case WAN_ERROR_CONNECTIVITY_CHECK_STATUS_DOWN_IPV6:
	    wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,"IPv6");
	    break;

        case WAN_INFO_IPv4_UP:
        case WAN_ERROR_IPv4_DOWN:
        case WAN_INFO_IPv6_UP:
        case WAN_ERROR_IPv6_DOWN:
        case WAN_INFO_MAPT_STATUS_UP:
        case WAN_ERROR_MAPT_STATUS_DOWN:
            wanmgr_telemetry_append_key_value(WANMGR_T2_SELECTION_STATUS_STRING,(pIntf->Selection.Status == WAN_IFACE_ACTIVE) ? "Active" : (pIntf->Selection.Status == WAN_IFACE_VALIDATING) ? "Validating" : (pIntf->Selection.Status == WAN_IFACE_SELECTED ) ? "Selected" : "Standby");
            break;

        default:
	    ;
    }
    strcat(buf,"\0");

    t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
    //This log is added for our internal testing, to be removed	
CcspTraceInfo(("%s %d: Successfully sent Telemetry event [%s] with arguments = [%s].\n",__FUNCTION__, __LINE__,WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf));

    return ANSC_STATUS_SUCCESS;
}
