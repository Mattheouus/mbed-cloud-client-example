// ----------------------------------------------------------------------------
// Copyright 2016-2020 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#ifdef TARGET_LIKE_MBED
#if defined (MEMORY_TESTS_HEAP)

// used by print_heap_stats only
#include "mbed_stats.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "mbed-client/m2mbase.h"

// fixup the compilation on AMRCC for PRIu32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "memory_tests.h"

#include "mbed-client/m2mblockmessage.h"
#include "mbed-client/m2mdevice.h"
#include "mbed-client/m2minterfacefactory.h"
#include "mbed-client/m2mobject.h"
#include "mbed-client/m2mserver.h"
#include "mbed-client/m2msecurity.h"
#include "source/include/m2mreporthandler.h"

#include "mbed.h"
#include "mbed_stats.h"

#include <assert.h>
#endif

#if defined (MEMORY_TESTS_HEAP)
void print_heap_stats()
{
    mbed_stats_heap_t stats;
    mbed_stats_heap_get(&stats);
    printf("**** current_size: %" PRIu32 "\n", stats.current_size);
    printf("**** max_size    : %" PRIu32 "\n", stats.max_size);
}

void create_m2mobject_test_set(M2MObjectList& object_list)
{
    printf("*************************************\n");

    mbed_stats_heap_t stats;
    mbed_stats_heap_get(&stats);

    uint32_t initial = stats.current_size;

    const int object_count = 1;
    const int object_id_range_start = 90;
    const int object_id_range_end = object_id_range_start + object_count;

    const int object_instance_count = 5;
    const int resource_count = 5;

    int total_object_count = 0;
    int total_object_instance_count = 0;
    int total_resource_count = 0;

    for (int object_id = object_id_range_start; object_id < object_id_range_end; object_id++) {

        char buff[64];
        sprintf(buff, "%d", object_id);
        M2MObject *obj = M2MInterfaceFactory::create_object(buff);

        for (int object_instance_id = 0; object_instance_id < object_instance_count; object_instance_id++) {

            M2MObjectInstance* obj_inst = obj->create_object_instance(object_instance_id);

            assert(obj_inst != NULL);
            total_object_instance_count++;
            for (int resource_id = 0; resource_id < resource_count; resource_id++) {
                String res_name;
                String res_type;

                res_name.append_int(resource_id);
                res_type.append_int(resource_id);

                M2MResource* resource = obj_inst->create_dynamic_resource(res_name, res_type,
                                                                        M2MResourceInstance::INTEGER, true);

                assert(resource != NULL);

                resource->set_operation(M2MBase::GET_ALLOWED);
                resource->set_value(7);

                total_resource_count++;
            }
        }

        object_list.push_back(obj);
        total_object_count++;
    }

    printf("objects       : %d\n", total_object_count);
    printf("obj instances : %d\n", total_object_instance_count);
    printf("resources     : %d\n", total_resource_count);

    mbed_stats_heap_get(&stats);
    printf("heap used     : %" PRIu32 "\n", stats.current_size - initial);

    printf("*************************************\n");
}

// Note: the mbed-os needs to be compiled with MEMORY_TESTS_HEAP to get
// functional heap stats, or the mbed_stats_heap_get() will return just zeroes.
void print_m2mobject_stats()
{
    printf("\n*** M2M object sizes in bytes ***\n");
    printf("M2MBase: %lu\n", (unsigned long)sizeof(M2MBase));
    printf("M2MObject: %lu\n", (unsigned long)sizeof(M2MObject));
    printf("M2MObjectInstance: %lu\n", (unsigned long)sizeof(M2MObjectInstance));
    printf("M2MResource: %lu\n", (unsigned long)sizeof(M2MResource));
    printf("M2MResourceInstance: %lu\n", (unsigned long)sizeof(M2MResourceInstance));
    printf("M2MDevice: %lu\n", (unsigned long)sizeof(M2MDevice));
    printf("M2MServer: %lu\n", (unsigned long)sizeof(M2MServer));
    printf("M2MSecurity: %lu\n", (unsigned long)sizeof(M2MSecurity));
    printf("M2MBlockMessage: %lu\n", (unsigned long)sizeof(M2MBlockMessage));
    printf("M2MReportHandler: %lu\n", (unsigned long)sizeof(M2MReportHandler));
    printf("*************************************\n\n");

    mbed_stats_heap_t stats;
    mbed_stats_heap_get(&stats);

    printf("*** M2M heap stats in bytes***\n");
    uint32_t initial = stats.current_size;

    // M2MDevice
    M2MDevice *device_object = M2MInterfaceFactory::create_device();
    if (device_object) {
        mbed_stats_heap_get(&stats);
        printf("M2MDevice heap size: %" PRIu32 "\n", stats.current_size - initial);
        M2MDevice::delete_instance();
        mbed_stats_heap_get(&stats);
        if (initial != stats.current_size) {
            printf("M2MDevice leaked: %" PRIu32 "bytes\n", stats.current_size - initial);
        }
    } else {
        printf("Could not get M2MDevice\n");
    }

    // M2MServer
    initial = stats.current_size;
    M2MServer *server = M2MInterfaceFactory::create_server();
    if (server) {
        mbed_stats_heap_get(&stats);
        printf("M2MServer heap size: %" PRIu32 "\n", stats.current_size - initial);
        delete server;
        mbed_stats_heap_get(&stats);
        if (initial != stats.current_size) {
            printf("M2MServer leaked: %" PRIu32 "bytes\n", stats.current_size - initial);
        }
    } else {
        printf("Could not get M2MServer\n");
    }

    // M2MSecurity
    initial = stats.current_size;
    M2MSecurity *security = M2MInterfaceFactory::create_security(M2MSecurity::M2MServer);
    if (security) {
        mbed_stats_heap_get(&stats);
        printf("M2MSecurity heap size: %" PRIu32 "\n", stats.current_size - initial);

        // If the MbedCloudClient is already instantiated, this corrupts heap as it frees the
        // singleton of M2MSecurity, which is still used by it. Use with care.
        M2MSecurity::delete_instance();
        mbed_stats_heap_get(&stats);
        if (initial != stats.current_size) {
            printf("M2MSecurity leaked: %" PRIu32 "bytes\n", stats.current_size - initial);
        }
    } else {
        printf("Could not get M2MSecurity\n");
    }

    // Basic object creation
    initial = stats.current_size;
    uint32_t before_object = initial;
    M2MObject *obj = M2MInterfaceFactory::create_object("1");
    if (!obj) {
        printf("Could not create M2MObject\n");
        return;
    }
    mbed_stats_heap_get(&stats);
    printf("M2MObject heap size: %" PRIu32 "\n", stats.current_size - initial);
    initial = stats.current_size;

    M2MObjectInstance* obj_inst = obj->create_object_instance();
    if (!obj_inst) {
        printf("Could not create M2MObjectInstance\n");
        delete obj;
        return;
    }
    mbed_stats_heap_get(&stats);
    printf("M2MObjectInstance heap size: %" PRIu32 "\n", stats.current_size - initial);

    initial = stats.current_size;
    M2MResource* res = obj_inst->create_dynamic_resource("1", "1", M2MResourceInstance::STRING, false);
    if (!res) {
        printf("Could not create M2MResource\n");
        delete obj;
        return;
    }
    mbed_stats_heap_get(&stats);
    printf("M2MResource heap size: %" PRIu32 "\n", stats.current_size - initial);

    delete obj;
    mbed_stats_heap_get(&stats);
    if (before_object != stats.current_size) {
        printf("Resource leaked: %" PRIu32 "bytes\n", stats.current_size - before_object);
    }
    printf("*************************************\n\n");
}

#endif // MEMORY_TESTS_HEAP
#endif // TARGET_LIKE_MBED
