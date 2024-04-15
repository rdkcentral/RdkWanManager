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
/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

/**********************************************************************

    module: plugin_main.h

        For COSA Library development.

    ---------------------------------------------------------------

    description:

        This header file defines the exported apis for COSA Library plugin.

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    author:

        Bin Zhu

    ---------------------------------------------------------------

    revision:

        12/12/2010    initial revision.

**********************************************************************/


#ifndef  _WANMGR_PLUGIN_MAIN_H_
#define  _WANMGR_PLUGIN_MAIN_H_



#define ANSC_EXPORT_API

#ifdef __cplusplus
extern "C"{
#endif

/***************************************************************************
 *
 *  COSA stands for "Cisco Open Service Architecture"
 *
 ***************************************************************************/
int ANSC_EXPORT_API WanManagerDmlInit(
        ULONG                       uMaxVersionSupported,
        void*                       hCosaPlugInfo         /* PCOSA_PLUGIN_INFO passed in by the caller */
    );

BOOL ANSC_EXPORT_API COSA_IsObjectSupported(char* pObjName);

void ANSC_EXPORT_API COSA_Unload(void);

#ifdef __cplusplus
}
#endif

#endif //_WANMGR_PLUGIN_MAIN_H_
