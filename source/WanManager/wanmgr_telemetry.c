#include "wanmgr_telemetry.h"

#define DELIMITER ","

#define PHY_INT_DSL          "DSL"
#define WAN_INT_DSL          "dsl0"
#define VIRT_WAN_INT_DSL     "vdsl0"

#define PHY_INT_WANOE        "WANOE"
#define WAN_INT_WANOE        "eth3"
#define VIRT_WAN_INT_WANOE   "erouter0"

#define PHY_INT_ADSL         "ADSL"
#define WAN_INT_ADSL         "pppoa0"
#define VIRT_WAN_INT_ADSL    "pppoa0"

#define PHY_INT_STRING       "PHY_INT"
#define WAN_INT_STRING       "WAN_INT"
#define VIRT_WAN_INT_STRING  "VIRT_WAN_INT"
#define SPLIT_VAL_STRING     "SPLIT_VAL"

char buf[128] = {0};

/*get_phy_int api returns the PHY interface name for corresponding Virtual WAN interface name*/
char* get_phy_int(char *str)
{
        if(strcmp(str,VIRT_WAN_INT_DSL) == 0){
                return PHY_INT_DSL;
        }

        if(strcmp(str,VIRT_WAN_INT_WANOE) == 0){
                return PHY_INT_WANOE;
        }
}

/*get_wan_int api returns the WAN interface name for corresponding Virtual WAN interface name*/
char* get_wan_int(char *str)
{
        if(strcmp(str,VIRT_WAN_INT_DSL) == 0){
                return WAN_INT_DSL;
        }

        if(strcmp(str,VIRT_WAN_INT_WANOE)==0){
                return WAN_INT_WANOE;
        }
}

/*append api appends key value in pairs, separated by DELIMITER*/
void append(char* key, char* value)
{
        if(strlen(buf)>0)
                strcat(buf,DELIMITER);

        strcat(buf,key);
        strcat(buf,":");
        strcat(buf,value);
}

/*int type*/
void wanmgr_t2_event_int(char *marker, int value)
{
        if(value == 0)
                value = 1;

        //This api further to be developed
        t2_event_d(marker,value);
}

/*float type*/
void wanmgr_t2_event_float( char *marker, double value)
{
        if (value == 0)
                value = 1;
        //This api further to be developed
        t2_event_f(marker, value);
}

/* wanmgr_t2_event_string - This api is to send string type telemetry marker.
 * marker_str - T2 telemetry marker to be sent.
 * phy_int - PHY interface name eg: DSL
 * wan_int - WAN interface name(L2 layer name) eg: dsl0
 * virt_wan_int - Virtual WAN interface name(L3 layer name) eg:  vdsl0
 * split_value - any other argument to be sent
 * */

void wanmgr_t2_event_string(char *marker_str, char* phy_int, char *wan_int, char *virt_wan_int, char* split_value)
{
        char *marker;
        marker = strdup(marker_str);
        /*append _split since it is a split marker type*/
        strcat(marker,"_split");
        memset(buf,sizeof(buf),0);

        if(phy_int != NULL)
                append(PHY_INT_STRING,phy_int);

        if(wan_int != NULL)
                append(WAN_INT_STRING,wan_int);

        if(virt_wan_int != NULL)
        {
                /*If only Virtual WAN interface is recieved, then frame the PHY and WAN interface names*/
                if(phy_int == NULL)
                        append(PHY_INT_STRING,get_phy_int(virt_wan_int));

                if(wan_int == NULL)
                        append(WAN_INT_STRING,get_wan_int(virt_wan_int));

                append(VIRT_WAN_INT_STRING,virt_wan_int);
        }
        if(split_value != NULL)
        {
                append(SPLIT_VAL_STRING,split_value);
        }

        strcat(buf,"\0");
        t2_event_s(marker,buf);
        free(marker);

}
