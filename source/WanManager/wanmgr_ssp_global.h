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
/*********************************************************************************

    description:

        This is the template file of wanmgr_ssp_global.h for XxxxSsp.
        You may add the header files you want to include.

    ------------------------------------------------------------------------------

    revision:

        09/08/2011    initial revision.

**********************************************************************************/

#ifndef  _WANMGR_SSP_GLOBAL_H
#define  _WANMGR_SSP_GLOBAL_H

#include <time.h>

#include "ansc_platform.h"
#include "slap_definitions.h"

#include "ccsp_message_bus.h"

#include "ccsp_base_api.h"

#include "dslh_cpeco_interface.h"
#include "dslh_cpeco_exported_api.h"

#include "slap_vco_exported_api.h"
#include "wanmgr_ssp_messagebus_interface.h"

#include "dslh_ifo_mpa.h"
#include "dslh_dmagnt_interface.h"
#include "dslh_dmagnt_exported_api.h"

#include "wanmgr_ssp_internal.h"
#include "ccsp_ifo_ccd.h"
#include "ccc_ifo_mbi.h"

#include "messagebus_interface_helper.h"

/*
 *  Define custom trace module ID
 */
#ifdef   ANSC_TRACE_MODULE_ID
    #undef  ANSC_TRACE_MODULE_ID
#endif

#define  ANSC_TRACE_MODULE_ID                       ANSC_TRACE_ID_SSP

#endif //_WANMGR_SSP_GLOBAL_H
