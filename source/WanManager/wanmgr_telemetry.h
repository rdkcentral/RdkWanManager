#ifndef _WAN_TELEMETRY_MARKER_H_
#define _WAN_TELEMETRY_MARKER_H_
#include "wanmgr_rdkbus_utils.h"

typedef enum WanMgr_TelemetryEvent
{
        WAN_INFO_PHY_TYPE = 1,
        WAN_ERROR_PHY_DOWN,
        WAN_INFO_PHY_UP,
        WAN_WARN_IP_OBTAIN_TIMER_EXPIRED,
        WAN_INFO_WAN_UP,
        WAN_INFO_WAN_STANDBY,
        WAN_ERROR_WAN_DOWN,
        WAN_INFO_IP_CONFIG_TYPE,
        WAN_INFO_IPv4_UP,
        WAN_ERROR_IPv4_DOWN,
        WAN_INFO_IPv6_UP,
        WAN_ERROR_IPv6_DOWN,
        WAN_INFO_IP_MODE,
        WAN_INFO_CONNECTIVITY_CHECK_TYPE,
        WAN_INFO_CONNECTIVITY_CHECK_STATUS_UP,
        WAN_ERROR_CONNECTIVITY_CHECK_STATUS_DOWN,
        WAN_WARN_CONNECTIVITY_CHECK_STATUS_FAILED,
        WAN_INFO_MAPT_STATUS_UP,
        WAN_ERROR_MAPT_STATUS_DOWN,
        WAN_ERROR_MAPT_STATUS_FAILED,
        WAN_ERROR_VLAN_DOWN,
        WAN_ERROR_VLAN_CREATION_FAILED
}WanMgr_TelemetryEvent_t;

typedef struct _WANMGR_TELEMETRY_MARKER_
{
    WanMgr_TelemetryEvent_t enTelemetryMarkerID;
    DML_WAN_IFACE *pInterface;
    DML_VIRTUAL_IFACE *pVirtInterface;
} WanMgr_Telemetry_Marker_t;

ANSC_STATUS wanmgr_telemetry_event(WanMgr_Telemetry_Marker_t *);

#endif //_WAN_TELEMETRY_MARKER_H_
