#include "wanmgr_telemetry.h"

static char buf[128] = {0};

/*append api appends key value in pairs, separated by DELIMITER*/
static void wanmgr_telemetry_append_key_value(char* key, const char* value)
{
    if(value != NULL)
    {
        if(strlen(buf)>0)
            strcat(buf,WANMGR_T2_TELEMETRY_MARKER_ARG_DELIMITER);
        
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
    switch(Marker->enTelemetryMarkerID)
    {
        case WAN_INFO_IP_MODE:
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_IpModeStr[pVirtIntf->IP.Mode]);
            break;
        case WAN_INFO_IP_CONFIG_TYPE:
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_IpSourceStr[pVirtIntf->IP.IPv4Source]);
            break;
        case WAN_ERROR_VLAN_DOWN:
        case WAN_ERROR_VLAN_CREATION_FAILED:
            wanmgr_telemetry_append_key_value(WANMGR_T2_VIRT_WAN_INTERFACE_STRING,pVirtIntf->Name);
            break;
        case WAN_INFO_CONNECTIVITY_CHECK_TYPE:
            wanmgr_telemetry_append_key_value(WANMGR_T2_WANMGR_SPLIT_VAL_STRING,WanMgr_Telemetry_ConnectivityTypeStr[pVirtIntf->IP.ConnectivityCheckType]);
            break;
        default:
    }
    strcat(buf,"\0");
    return ANSC_STATUS_SUCCESS;
}

/*A generic api for telemetry marker,
 * it gets the necessary parameter structs and
 * sends them to appropriate api to be processed.*/

ANSC_STATUS wanmgr_telemetry_event(WanMgr_Telemetry_Marker_t *Marker)
{
#ifdef ENABLE_FEATURE_TELEMETRY2_0
    if(Marker == NULL)
    {
        return ANSC_STATUS_FAILURE; 
    }
    
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
            if(ANSC_STATUS_FAILURE == wanmgr_process_T2_telemetry_event(Marker))
            {
                return ANSC_STATUS_FAILURE;
            }
            t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
            break;
        //int type markers
        //float type markers
        default:
    }

/*#else
 * Further Telemetry versions to be handled here
 */
#endif
    return ANSC_STATUS_SUCCESS;
}
