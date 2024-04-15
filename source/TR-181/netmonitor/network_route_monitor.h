#ifndef _NETWORK_ROUTE_MONITOR_H_
#define _NETWORK_ROUTE_MONITOR_H_

#define CONSOLE_LOG_FILE       "/rdklogs/logs/WANMANAGERLog.txt.0"

#define DBG_MONITOR_PRINT(fmt ...)     {\
    FILE     *fp        = NULL;\
    fp = fopen ( CONSOLE_LOG_FILE, "a+");\
    if (fp)\
    {\
        fprintf(fp,fmt);\
        fclose(fp);\
    }\
}\

#endif /* _NETWORK_ROUTE_MONITOR_H_ */
