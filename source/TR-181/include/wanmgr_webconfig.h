 /*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/


#ifndef  _WAN_MANAGER_WEBCONFIG_H
#define  _WAN_MANAGER_WEBCONFIG_H

/* ---- Include Files ---------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>

#include <time.h>
#include <errno.h>
#include <msgpack.h>
#include <stdarg.h>
#include "ansc_platform.h"
#include "ansc_string_util.h"
#include "webconfig_framework.h"

/* Buffer Length Macros */

#define BUFFER_LENGTH_4                 4
#define BUFFER_LENGTH_16                16
#define BUFFER_LENGTH_32                32
#define BUFFER_LENGTH_64                64
#define BUFFER_LENGTH_128               128
#define BUFFER_LENGTH_256               256
#define BUFFER_LENGTH_512               512
#define BUFFER_LENGTH_1024              1024
#define BUFFER_LENGTH_2048              2048

#define WANDATA_DEFAULT_TIMEOUT  15

#define WANDATA_SUBDOC_COUNT     2
 /***********************************
     Actual definition declaration
 ************************************/
 /*
     WEBCONFIG Part
 */

#define match(p, s) strncmp((p)->key.via.str.ptr, s, (p)->key.via.str.size)
#define member_size(type, member) sizeof(((type *)0)->member)

typedef struct
{
    unsigned int            uiMarkingInstanceNumber;
    char                    alias[BUFFER_LENGTH_16];
    int                     ethernetPriorityMark;
} WebConfig_Wan_Marking_Table_t;

typedef struct
{
    unsigned int                    uiIfInstanceNumber;
    char                            name[BUFFER_LENGTH_64];
    unsigned int                    markingCount;
    WebConfig_Wan_Marking_Table_t   *markingTable;
} WebConfig_Wan_Interface_Table_t;

typedef struct 
{
    bool                    AllowRemoteIface;
} WebConfig_Wan_FailOverData_t;

typedef struct {
    char                              subdocName[BUFFER_LENGTH_64];
    unsigned int                      version;
    unsigned short int                transactionId;
    unsigned int                      ifCount;
    WebConfig_Wan_Interface_Table_t   *ifTable;
    WebConfig_Wan_FailOverData_t      *pWanFailOverData;
} WanMgr_WebConfig_t;

ANSC_STATUS WanMgr_WebConfig_Process_Wanmanager_Params(msgpack_object obj, WanMgr_WebConfig_t *pWebConfig);
ANSC_STATUS WanMgr_WebConfig_Process_ifParams( WebConfig_Wan_Interface_Table_t *e, msgpack_object_map *map );
ANSC_STATUS WaneMgr_WebConfig_Process_MarkingEntry(WebConfig_Wan_Marking_Table_t *e, msgpack_object_map *map);

#endif /* _WAN_MANAGER_WEBCONFIG_H */
