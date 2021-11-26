/* ---- Include Files ---------------------------------------- */
#include <unistd.h>
#include <linux/rtnetlink.h>
#include "ipc_msg.h"
#include "network_route_monitor.h"
#include "ansc_platform.h"
#include "ansc_string_util.h"
#include <sysevent/sysevent.h>
/* ---- Global Variables ------------------------------------ */
int sysevent_fd = -1;
token_t sysevent_token;

static int maxFd = 50;
int netlinkRouteMonitorFd = -1; //subscribe to route change events
fd_set readFdsMaster;
static fd_set errorFdsMaster;
static bool g_toggle_flag = TRUE;
#define UPDATE_MAXFD(f) (maxFd = (f > maxFd) ? f : maxFd)

#define LOOP_TIMEOUT 100000 // timeout in milliseconds. This is the state machine loop interval
#define SYSEVENT_IPV6_TOGGLE        "ipv6Toggle"
#define SYSEVENT_OPEN_MAX_RETRIES   6
#define SE_SERVER_WELL_KNOWN_PORT   52367
#define SE_VERSION                  1
#define NETMONITOR_SYSNAME          "netmonitor"
#define SYS_IP_ADDR                 "127.0.0.1"

/* ---- Private Function Prototypes -------------------------- */
static void parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len); // Parse the route entries
static ANSC_STATUS isDefaultGatewaypresent(struct nlmsghdr* nlmsgHdr); // check for default gateway

static void parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
    memset(tb, 0, sizeof(struct rtattr *) * (max + 1));

    while (RTA_OK(rta, len)) {
        if (rta->rta_type <= max) {
            tb[rta->rta_type] = rta;
        }

        rta = RTA_NEXT(rta,len);
    }
}

static ANSC_STATUS isDefaultGatewaypresent(struct nlmsghdr* nlmsgHdr)
{
    struct rtmsg* route_entry = NLMSG_DATA(nlmsgHdr);
    struct rtattr* tb[RTA_MAX+1];
    ANSC_STATUS ret = ANSC_STATUS_FAILURE;

    //DBG_MONITOR_PRINT("%s-%d: Enter \n", __FUNCTION__, __LINE__);

    int len = nlmsgHdr->nlmsg_len - NLMSG_LENGTH(sizeof(*route_entry));

    if (len < 0) {
        DBG_MONITOR_PRINT("%s Wrong message length \n", __FUNCTION__);
        return ret;
    }

    // We are just interested in main routing table
    if (route_entry->rtm_table != RT_TABLE_MAIN) {
        return ret;
    }

    parse_rtattr(tb, RTA_MAX, RTM_RTA(route_entry), len);

    if ( (tb[RTA_DST] == 0)  && (route_entry->rtm_dst_len == 0) ) {
        DBG_MONITOR_PRINT("%s-%d: Found Default gateway in route event \n", __FUNCTION__, __LINE__);
        ret = ANSC_STATUS_SUCCESS;
    }

    return ret;
}

static ANSC_STATUS NetMonitor_InitNetlinkRouteMonitorFd()
{
    struct sockaddr_nl addr;

    DBG_MONITOR_PRINT("%s Enter \n", __FUNCTION__);

    netlinkRouteMonitorFd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (netlinkRouteMonitorFd < 0)
    {
        DBG_MONITOR_PRINT("%s Failed to create netlink socket: %s\n", __FUNCTION__, strerror(errno));
        return ANSC_STATUS_FAILURE;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV6_ROUTE;
    if (0 > bind(netlinkRouteMonitorFd, (struct sockaddr *) &addr, sizeof(addr)))
    {
        DBG_MONITOR_PRINT("%s Failed to bind netlink socket: %s\n", __FUNCTION__, strerror(errno));
        close(netlinkRouteMonitorFd);
        return ANSC_STATUS_FAILURE;
    }
    DBG_MONITOR_PRINT("%s Bound netlinkRouteMonitorFd to 0x%x  \n", __FUNCTION__, addr.nl_groups);

    return ANSC_STATUS_SUCCESS;
}

static void NetMonitor_DoToggleV6Status(bool flag)
{
    DBG_MONITOR_PRINT("%s-%d: Enter \n", __FUNCTION__, __LINE__);

    g_toggle_flag = flag;

    if (g_toggle_flag == TRUE)
    {
        DBG_MONITOR_PRINT("%s-%d: Toggle Needed \n", __FUNCTION__, __LINE__);
        sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_IPV6_TOGGLE, "TRUE", 0);
        g_toggle_flag = FALSE;
    }
    else
    {
        DBG_MONITOR_PRINT("%s-%d: No Toggle Needed \n", __FUNCTION__, __LINE__);
    }
}


static void netMonitor_SyseventInit()
{
    int try = 0;

    /* Open sysevent descriptor to send messages */
    while(try < SYSEVENT_OPEN_MAX_RETRIES)
    {
       sysevent_fd =  sysevent_open(SYS_IP_ADDR, SE_SERVER_WELL_KNOWN_PORT, SE_VERSION, NETMONITOR_SYSNAME, &sysevent_token);
       if(sysevent_fd >= 0)
       {
          break;
       }
       try++;
       usleep(50000);
    }
        DBG_MONITOR_PRINT("%s-%d: Started \n", __FUNCTION__, __LINE__);
}

static void NetMonitor_ProcessNetlinkRouteMonitorFd()
{
    struct sockaddr_nl local;   // local addr struct
    char buf[8192];             // message buffer
    struct iovec iov;           // message structure
    iov.iov_base = buf;         // set message buffer as io
    iov.iov_len = sizeof(buf);  // set size
    struct nlmsghdr *nl_msgHdr;
    static bool gw_v6_flag = FALSE;

    // initialize protocol message header
    struct msghdr msg;
    {
        msg.msg_name = &local;                  // local address
        msg.msg_namelen = sizeof(local);        // address size
        msg.msg_iov = &iov;                     // io vector
        msg.msg_iovlen = 1;                     // io size
    }

    //DBG_MONITOR_PRINT("%s-%d: \n", __FUNCTION__, __LINE__);

    ssize_t status = recvmsg(netlinkRouteMonitorFd, &msg, 0);
    if (status <= 0) {
        DBG_MONITOR_PRINT("%s-%d: Received Message Status Failed %d \n", __FUNCTION__, __LINE__, status);
        return;
    }

    for(nl_msgHdr = (struct nlmsghdr *) buf; NLMSG_OK (nl_msgHdr, (unsigned int)status); nl_msgHdr = NLMSG_NEXT (nl_msgHdr, status))
    {
        /* Finish reading */
        if (nl_msgHdr->nlmsg_type == NLMSG_DONE)
        {
            return;
        }
        /* Message is some kind of error */
        if (nl_msgHdr->nlmsg_type == NLMSG_ERROR)
        {
            DBG_MONITOR_PRINT("%s netlink message error \n", __FUNCTION__);
            return;
        }

        DBG_MONITOR_PRINT("%s-%d: msg type %d , gw_v6=%d \n", __FUNCTION__, __LINE__, nl_msgHdr->nlmsg_type, gw_v6_flag);
        switch(nl_msgHdr->nlmsg_type)
        {

            case RTM_NEWROUTE:
                 {
                     if(isDefaultGatewaypresent(nl_msgHdr) == ANSC_STATUS_SUCCESS){
                         if(gw_v6_flag == FALSE){
                             DBG_MONITOR_PRINT(" %s  IPv6 Default route update - ADD \n", __FUNCTION__);
                             NetMonitor_DoToggleV6Status(FALSE);
                             gw_v6_flag = TRUE;
                         }
                     }
                     break;
                 }
            case RTM_DELROUTE:
                 {
                     if(isDefaultGatewaypresent(nl_msgHdr) == ANSC_STATUS_SUCCESS){
                         if(gw_v6_flag == TRUE){
                             DBG_MONITOR_PRINT(" %s  IPv6 Default route update - DEL \n", __FUNCTION__);
                             NetMonitor_DoToggleV6Status(TRUE);
                             gw_v6_flag = FALSE;
                         }
                     }
                     break;
                }
            default:
                break;
        }
    }
    return;
}

static void NetMonitor_DeInitNetlinkRouteMonitorFd()
{
    if (netlinkRouteMonitorFd >= 0)
    {
        DBG_MONITOR_PRINT("%s-%d: \n", __FUNCTION__, __LINE__);
        close(netlinkRouteMonitorFd);
        netlinkRouteMonitorFd = -1;
    }
}

int main(int argc, char* argv[])
{
    //event handler
    int n = 0;
    struct timeval tv;

    DBG_MONITOR_PRINT("%s-%d: \n", __FUNCTION__, __LINE__);

    fd_set readFds;
    fd_set errorFds;

    /* Route events : set up all the fd stuff for select */
    FD_ZERO(&readFdsMaster);
    FD_ZERO(&errorFdsMaster);

    netMonitor_SyseventInit();

    NetMonitor_InitNetlinkRouteMonitorFd();
    if (netlinkRouteMonitorFd != -1)
    {
        FD_SET(netlinkRouteMonitorFd, &readFdsMaster);
        UPDATE_MAXFD(netlinkRouteMonitorFd);
    }
    while(1)
    {
        tv.tv_sec = 0;
        tv.tv_usec = LOOP_TIMEOUT;

        readFds = readFdsMaster;
        errorFds = errorFdsMaster;

        n = select(maxFd+1, &readFds, NULL, &errorFds, &tv);
        if (n < 0)
        {
            /* interrupted by signal or something, continue */
            continue;
        }
        if ((netlinkRouteMonitorFd != -1) && FD_ISSET(netlinkRouteMonitorFd, &readFds))
        {
            DBG_MONITOR_PRINT("%s-%d: Process NetLink Route Monitor \n", __FUNCTION__, __LINE__);
            NetMonitor_ProcessNetlinkRouteMonitorFd();
        }
    }
    NetMonitor_DeInitNetlinkRouteMonitorFd();
    return 0;
}
