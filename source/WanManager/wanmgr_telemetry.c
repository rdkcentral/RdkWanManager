#include "wanmgr_telemetry.h"

static char buf[128] = {0};

/*append api appends key value in pairs, separated by DELIMITER*/
static void wanmgr_append(char* key, char* value)
{
        if(strlen(buf)>0)
                strcat(buf,DELIMITER);

        strcat(buf,key);
        strcat(buf,":");
        strcat(buf,value);
}

/*int type*/
static void wanmgr_telemetry_event_int(char *marker, int value)
{
        if(value == 0)
                value = 1;

        //This api further to be developed
        t2_event_d(marker,value);
}

/*float type*/
static void wanmgr_telemetry_event_float( char *marker, double value)
{
        if (value == 0)
                value = 1;
        //This api further to be developed
        t2_event_f(marker, value);
}

/* wanmgr_telemetry_event_string - This api is to send string type telemetry marker. */
void wanmgr_telemetry_event_string(WanMgr_Telemetry_Marker_t *Marker)
{
        memset(buf,sizeof(buf),0);

        if(strlen(Marker->acPhysicalInterface) > 0)
            wanmgr_append(PHY_INT_STRING,Marker->acPhysicalInterface);

        if(strlen(Marker->acWANInterface) > 0)
            wanmgr_append(WAN_INT_STRING,Marker->acWANInterface);

        if(strlen(Marker->acVirtualWANInterface) > 0)
            wanmgr_append(VIRT_WAN_INT_STRING,Marker->acVirtualWANInterface);

        if(strlen(Marker->acSplitValue) > 0 )
            wanmgr_append(SPLIT_VAL_STRING,Marker->acSplitValue);

        strcat(buf,"\0");
        //eg: buf = [PHY_INT:DSL,WAN_INT:dsl,VIRT_WAN_INT:vdsl0,SPLIT_VAL:STATIC] -> key value pair ':' separated, arguments ',' separted
        t2_event_s(WanMgr_TelemetryEventStr[Marker->enTelemetryMarkerID],buf);
}
