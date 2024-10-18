#ifndef _WAN_TELEMETRY_MARKER_H_
#define _WAN_TELEMETRY_MARKER_H_

typedef enum _return_status_e
{
    STATUS_SUCCESS,
    STATUS_FAILED,
    STATUS_FEATURE_DISABLED,
    STATUS_UNDEFINED,
    STATUS_INVALID,
    STATUS_UNKNOWN,
}return_status_e;

typedef enum _marker_id_e
{
    WAN_INFO_PHY_TYPE,
    SYS_INFO_EthWanMode,
    WAN_ERROR_PHY_DOWN,
    WAN_INFO_PHY_UP,
    WAN_WARNING_IP_OBTAIN_TIMER_EXPIRED,
    WAN_BTIME_WANINIT,  //exp for int arg
}marker_id_e;

#define wan_telemetry_marker_set(marker_id, data) \
	          __wan_telemetry_marker_set(marker_id, #marker_id, data);

extern return_status_e __wan_telemetry_marker_set(marker_id_e marker_id, char* marker, void **data);

#endif //_WAN_TELEMETRY_MARKER_H_
