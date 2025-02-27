#ifndef _WAN_TELEMETRY_MARKER_H_
#define _WAN_TELEMETRY_MARKER_H_
#include <telemetry_busmessage_sender.h>
#include <stdlib.h>
#include <string.h>
#include "wanmgr_rdkbus_utils.h"

#define DELIMITER ","

#define PHY_INT_STRING       "PHY_INT"
#define WAN_INT_STRING       "WAN_INT"
#define VIRT_WAN_INT_STRING  "VIRT_WAN_INT"
#define SPLIT_VAL_STRING     "SPLIT_VAL"

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

static const char * const WanMgr_TelemetryEventStr[] =
{
        [WAN_INFO_PHY_TYPE] = "WAN_INFO_PHY_TYPE_split",
        [WAN_ERROR_PHY_DOWN] = "WAN_ERROR_PHY_DOWN_split",
        [WAN_INFO_PHY_UP] = "WAN_INFO_PHY_UP_split",
        [WAN_WARN_IP_OBTAIN_TIMER_EXPIRED] = "WAN_WARN_IP_OBTAIN_TIMER_EXPIRED_split",
        [WAN_INFO_WAN_UP] = "WAN_INFO_WAN_UP_split",
        [WAN_INFO_WAN_STANDBY] = "WAN_INFO_WAN_STANDBY_split",
        [WAN_ERROR_WAN_DOWN] = "WAN_ERROR_WAN_DOWN_split",
        [WAN_INFO_IP_CONFIG_TYPE] = "WAN_INFO_IP_CONFIG_TYPE_split",
        [WAN_INFO_IPv4_UP] = "WAN_INFO_IPv4_UP_split",
        [WAN_ERROR_IPv4_DOWN] = "WAN_ERROR_IPv4_DOWN_split",
        [WAN_INFO_IPv6_UP] = "WAN_INFO_IPv6_UP_split",
        [WAN_ERROR_IPv6_DOWN] = "WAN_ERROR_IPv6_DOWN_split",
        [WAN_INFO_IP_MODE] = "WAN_INFO_IP_MODE_split",
        [WAN_INFO_CONNECTIVITY_CHECK_TYPE] = "WAN_INFO_CONNECTIVITY_CHECK_TYPE_split",
        [WAN_INFO_CONNECTIVITY_CHECK_STATUS_UP] = "WAN_INFO_CONNECTIVITY_CHECK_STATUS_UP_split",
        [WAN_ERROR_CONNECTIVITY_CHECK_STATUS_DOWN] = "WAN_ERROR_CONNECTIVITY_CHECK_STATUS_DOWN_split",
        [WAN_WARN_CONNECTIVITY_CHECK_STATUS_FAILED] = "WAN_WARN_CONNECTIVITY_CHECK_STATUS_FAILED_split",
        [WAN_INFO_MAPT_STATUS_UP] = "WAN_INFO_MAPT_STATUS_UP_split",
        [WAN_ERROR_MAPT_STATUS_DOWN] = "WAN_ERROR_MAPT_STATUS_DOWN_split",
        [WAN_ERROR_MAPT_STATUS_FAILED] = "WAN_ERROR_MAPT_STATUS_FAILED_split",
        [WAN_ERROR_VLAN_DOWN] = "WAN_ERROR_VLAN_DOWN_split",
        [WAN_ERROR_VLAN_CREATION_FAILED] = "WAN_ERROR_VLAN_CREATION_FAILED_split"
};

typedef  struct _WANMGR_TELEMETRY_MARKER_
{
        WanMgr_TelemetryEvent_t enTelemetryMarkerID;
        char acPhysicalInterface[32];
        char acWANInterface[32];
        char acVirtualWANInterface[32];
        char acSplitValue[32];

} WanMgr_Telemetry_Marker_t;

void wanmgr_telemetry_event_string(WanMgr_Telemetry_Marker_t *Marker);
#endif //_WAN_TELEMETRY_MARKER_H_


