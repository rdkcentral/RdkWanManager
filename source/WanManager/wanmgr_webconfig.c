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

#include "wanmgr_webconfig.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/**
 *  Convert the msgpack map into the WebConfig_Wan_Marking_Table_t structure.
 *
 *  @param e    the marking table entry pointer
 *  @param map  the msgpack map pointer
 *
 *  @return 0 on success, error otherwise
 */
ANSC_STATUS WaneMgr_WebConfig_Process_MarkingEntry(WebConfig_Wan_Marking_Table_t *e, msgpack_object_map *map)
{
    int  i;
    msgpack_object_kv *p = map->ptr;

    //Validate param
    if( NULL == p )
    {
        CcspTraceError(("%s Invalid Pointer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    for(i = 0;i < map->size; i++)
    {
        CcspTraceInfo(("Marking index %d, Field Name[type: %d] :%s\n",
                                    e->uiMarkingInstanceNumber, p->val.type, p->key.via.str.ptr));
        switch (p->val.type)
        {
            case MSGPACK_OBJECT_STR:
                if( 0 == match(p, "Alias") )
                {
                    if(p->val.via.str.size < sizeof(e->alias))
                    {
                        strncpy(e->alias, p->val.via.str.ptr, p->val.via.str.size);
                    }
                    else
                    {
                        CcspTraceInfo(("Alias value size(%d) exeeds limit(%d), truncating..\n",
                                                                   p->val.via.str.size, sizeof(e->alias)-1));
                        strncpy(e->alias, p->val.via.str.ptr, sizeof(e->alias)-1);
                    }
                    CcspTraceInfo(("Marking index %d, Alias :%s\n", e->uiMarkingInstanceNumber, e->alias));
                }
                break;
            case MSGPACK_OBJECT_NEGATIVE_INTEGER:
                if( 0 == match(p, "EthernetPriorityMark") )
                {
                    e->ethernetPriorityMark = (int)p->val.via.i64;
                    CcspTraceInfo(("Marking index %d, EthernetPriorityMark :%d\n",
                                                         e->uiMarkingInstanceNumber, e->ethernetPriorityMark));
                }
                break;
            case MSGPACK_OBJECT_POSITIVE_INTEGER:
                if( 0 == match(p, "EthernetPriorityMark") )
                {
                    e->ethernetPriorityMark = (int)p->val.via.u64;
                    CcspTraceInfo(("Marking index %d, EthernetPriorityMark :%d\n",
                                                         e->uiMarkingInstanceNumber, e->ethernetPriorityMark));
                }
                break;
        }
        p++;
    }

    return ANSC_STATUS_SUCCESS;
}
/**
 *  Convert the msgpack map into the WebConfig_Wan_Interface_Table_t structure.
 *
 *  @param e    interface table entry pointer
 *  @param map  the msgpack map pointer
 *
 *  @return 0 on success, error otherwise
 */
ANSC_STATUS WanMgr_WebConfig_Process_ifParams( WebConfig_Wan_Interface_Table_t *e, msgpack_object_map *map )
{
    int i = 0, j = 0;
    msgpack_object_kv *p = map->ptr;

    //Validate param
    if( NULL == p )
    {
        CcspTraceError(("%s Invalid Pointer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    for(i = 0;i < map->size; i++)
    {
        if (MSGPACK_OBJECT_STR == p->val.type )
        {
            if( 0 == match(p, "Name") )
            {
                strncpy(e->name,p->val.via.str.ptr, p->val.via.str.size);
                CcspTraceInfo(("Interface name :%s\n", e->name));
            }
        }
        else if( MSGPACK_OBJECT_ARRAY == p->val.type )
        {
            if( 0 == match(p, "Marking") )
            {
                e->markingCount = p->val.via.array.size;
                e->markingTable = (WebConfig_Wan_Marking_Table_t *)
                                   malloc( sizeof(WebConfig_Wan_Marking_Table_t) * e->markingCount);
                if( NULL == e->markingTable )
                {
                    e->markingCount = 0;
                    return ANSC_STATUS_FAILURE;
                }

                CcspTraceInfo(("Wan interface %d Marking Table Count %d\n", e->uiIfInstanceNumber, e->markingCount));

                memset( e->markingTable, 0, sizeof(WebConfig_Wan_Marking_Table_t) * e->markingCount );
                for( j = 0; j < e->markingCount; j++ )
                {
                    if( MSGPACK_OBJECT_MAP != p->val.via.array.ptr[j].type )
                    {
                        CcspTraceError(("%s %d - Invalid OBJECT \n",__FUNCTION__,__LINE__));
                        return ANSC_STATUS_FAILURE;
                    }

                    e->markingTable[j].uiMarkingInstanceNumber = j + 1;
                    if( ANSC_STATUS_SUCCESS !=
                        WaneMgr_WebConfig_Process_MarkingEntry(&e->markingTable[j], &p->val.via.array.ptr[j].via.map) )
                    {
                        return ANSC_STATUS_FAILURE;
                    }
                }
            }
        }

        p++;
    }

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS WanMgr_WebConfig_Process_WanFailOver_Params(msgpack_object obj, WanMgr_WebConfig_t *pWebConfig)
{
    int i, j;
    msgpack_object_kv* p = obj.via.map.ptr;

    //Validate param
    if( (NULL == p) || (NULL == pWebConfig))
    {
        CcspTraceError(("%s %d: Invalid Pointer\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("*************** WAN manager wanfailover Data *******************\n"));

    pWebConfig->pWanFailOverData = malloc (sizeof(WebConfig_Wan_FailOverData_t));
    if (NULL == pWebConfig->pWanFailOverData)
    {
        CcspTraceInfo(("%s %d: malloc() failure\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    for(i = 0; i < obj.via.map.size; i++) 
    {
        if (MSGPACK_OBJECT_BOOLEAN == p->val.type)
        {
            if ( 0 == match(p, "allow_remote_interfaces"))
            {
                pWebConfig->pWanFailOverData->AllowRemoteIface =  p->val.via.boolean;
                CcspTraceInfo(("%s %d: pWanFailOverData->AllowRemoteIface = %d\n", __FUNCTION__, __LINE__, pWebConfig->pWanFailOverData->AllowRemoteIface));
            }
        }
        ++p;
    }
    return ANSC_STATUS_SUCCESS;
}

/*
 * Function to deserialize wan manager params from msgpack object
 *
 * @param obj  Pointer to msgpack object
 * @param pWebConfig  Pointer to wan manager ebconfig structure
 *
 * returns 0 on success, error otherwise
 */
ANSC_STATUS WanMgr_WebConfig_Process_Wanmanager_Params(msgpack_object obj, WanMgr_WebConfig_t *pWebConfig)
{
    int i, j;
    msgpack_object_kv* p = obj.via.map.ptr;

    //Validate param
    if( NULL == p )
    {
        CcspTraceError(("%s Invalid Pointer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("*************** WAN manager WebConfig Data *******************\n"));

    for(i = 0;i < obj.via.map.size;i++) {
        if ( 0 == match(p, "CPEInterface") )
        {
            if( MSGPACK_OBJECT_ARRAY == p->val.type )
            {
                pWebConfig->ifCount = p->val.via.array.size;
                pWebConfig->ifTable = (WebConfig_Wan_Interface_Table_t *)
                                      malloc( sizeof(WebConfig_Wan_Interface_Table_t) * pWebConfig->ifCount);
                if( NULL == pWebConfig->ifTable )
                {
                    pWebConfig->ifCount = 0;
                    CcspTraceError(("%s %d - Failed to allocate memory \n",__FUNCTION__,__LINE__));
                    return ANSC_STATUS_FAILURE;
                }

                CcspTraceInfo(("Interface Count %d\n",pWebConfig->ifCount));

                memset( pWebConfig->ifTable, 0, sizeof(WebConfig_Wan_Interface_Table_t) * pWebConfig->ifCount );
                for( j = 0; j < pWebConfig->ifCount; j++ )
                {
                    if( MSGPACK_OBJECT_MAP != p->val.via.array.ptr[j].type )
                    {
                        CcspTraceError(("%s %d - Invalid OBJECT \n",__FUNCTION__,__LINE__));
                        return ANSC_STATUS_FAILURE;
                    }

                    pWebConfig->ifTable[j].uiIfInstanceNumber = j;
                    if( ANSC_STATUS_SUCCESS != WanMgr_WebConfig_Process_ifParams(
                                                 &pWebConfig->ifTable[j], &p->val.via.array.ptr[j].via.map) )
                    {
                        return ANSC_STATUS_FAILURE;
                    }

                }
            }
        }

        ++p;
    }

    CcspTraceInfo(("*************************************************************\n"));

    return ANSC_STATUS_SUCCESS;
}
