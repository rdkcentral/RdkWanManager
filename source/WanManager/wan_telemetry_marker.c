#include "wan_telemetry_marker.h"
#ifdef ENABLE_FEATURE_TELEMETRY2_0
#include <telemetry_busmessage_sender.h>
#endif
#include <stddef.h>
#include "ccsp_trace.h"

struct marker
{
    char *t1_marker_string;
    return_status_e (*telemetry_marker_set)(struct marker *marker_p, char *marker, void **data);
};

static return_status_e telemetry_marker_t2_string(struct marker *marker_p, char *marker, void **data)
{
    char buf[128] = {0};
    char **data_p = (char **)data;
    
    if (data != NULL)
    {
	snprintf(buf, 128, marker_p->t1_marker_string, data_p[0], data_p[1]);
    }
    else
    {
        strcpy(buf, marker_p->t1_marker_string);
    }
    CcspTraceInfo(("%s-%d: %s-%s", __FUNCTION__, __LINE__, marker, buf));
#ifdef ENABLE_FEATURE_TELEMETRY2_0
    t2_event_s(marker, data_p);
#endif
    return STATUS_SUCCESS;
}


static return_status_e telemetry_marker_t1_string(struct marker *marker_p, char *marker, void **data)
{
    char buf[128] = {0};

    if (data != NULL)
    {
        char **data_p = (char **)data;
        snprintf(buf, 128, marker_p->t1_marker_string, data_p[0], data_p[1]);
    }
    else
    {
        strcpy(buf, marker_p->t1_marker_string);
    }
    CcspTraceInfo(("%s-%d: %s-%s", __FUNCTION__, __LINE__, marker, buf));
    return STATUS_SUCCESS;
}

static return_status_e telemetry_marker_t2_int(struct marker *marker_p, char *marker, void **data)
{
    char buf[128] = {0};
    int **data_p = (int **)data;

    if (data != NULL)
    {
        snprintf(buf, 128, marker_p->t1_marker_string, (**data_p));
    }
    else
    {
        strcpy(buf, marker_p->t1_marker_string);
    }
    CcspTraceInfo(("%s-%d: %s-%s", __FUNCTION__, __LINE__, marker, buf));
#ifdef ENABLE_FEATURE_TELEMETRY2_0
    t2_event_d(marker, (**data_p));
#endif
    return STATUS_SUCCESS;
}


static struct marker marker_table[] = 
{
    {
        //marker id : WAN_INFO_PHY_TYPE
        .t1_marker_string = "Selecting Base Interface:%s Phy_Type:%s",
        .telemetry_marker_set = telemetry_marker_t2_string,
    },
    {
        //marker id : SYS_INFO_EthWanMode
        .t1_marker_string = "Ethernet WAN is enabled",
        .telemetry_marker_set = telemetry_marker_t1_string,
    },
    {
        //marker id : WAN_ERROR_PHY_DOWN
        .t1_marker_string = "Received Base Interface:%s down",
        .telemetry_marker_set = telemetry_marker_t2_string,
    },
    {
        //marker id : WAN_INFO_PHY_UP
        .t1_marker_string = "Received Base Interface:%s up",
        .telemetry_marker_set = telemetry_marker_t2_string,
    },
    {
        //marker id : WAN_WARNING_IP_OBTAIN_TIMER_EXPIRED
        .t1_marker_string = "Wan Router Interface:%s, IP Obtaining Timer Expired %s",
        .telemetry_marker_set = telemetry_marker_t2_string,
    },
    {
        //marker id : WAN_BTIME_WANINIT
        .t1_marker_string = "Wan Boot Time Init %d",
        .telemetry_marker_set = telemetry_marker_t2_int,
    },
    { },
};



return_status_e __wan_telemetry_marker_set(marker_id_e marker_id, char* marker, void **data)
{
    return_status_e return_status;
    return_status = marker_table[marker_id].telemetry_marker_set(&marker_table[marker_id], marker, data);
    return return_status;
}


