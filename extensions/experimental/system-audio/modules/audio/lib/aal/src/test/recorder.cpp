/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#define G_LOG_DOMAIN "AAL"
#include <aal.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define MODULE_ID 0

static pthread_mutex_t lock;
static bool flag = false;
static aal_handle_t handle;
static int intcount = 0;
static FILE* fp = NULL;

static void request_shutdown() {
    pthread_mutex_lock(&lock);
    flag = true;
    pthread_mutex_unlock(&lock);
}

static void on_start_callback(void* user_data) {
    printf("Recorder started\n");
}

static void on_stop_callback(aal_status_t reason, void* user_data) {
    switch (reason) {
        case AAL_SUCCESS:
            printf("Recorder finished recording\n");
            request_shutdown();
            break;
        case AAL_PAUSED:
            printf("Recorder paused recording\n");
            break;
        case AAL_ERROR:
            printf("Recorder encountered error\n");
            request_shutdown();
            break;
        case AAL_UNKNOWN:
            printf("Recorder stopped recording for unknown reason\n");
            request_shutdown();
            break;
    }
}

static void on_data_callback(const int16_t* data, const size_t length, void* user_data) {
    fwrite(data, sizeof(int16_t), length, fp);
}

static const aal_listener_t listener = {.on_start = on_start_callback,
                                        .on_stop = on_stop_callback,
                                        .on_almost_done = NULL,
                                        .on_data = on_data_callback,
                                        .on_data_requested = NULL};

static void signal_handler(int signo) {
    printf("Call aal_recorder_stop...\n");
    aal_recorder_stop(handle);
    if (intcount++ > 5) {
        request_shutdown();
    }
}

aal_handle_t create_recorder() {
    const aal_attributes_t attr = {.name = "SampleApp",
                                   .device = NULL,
                                   .uri = NULL,
                                   .listener = &listener,
                                   .user_data = NULL,
                                   .module_id = MODULE_ID};

    return aal_recorder_create(&attr, nullptr);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: recorder <output>\n");
        return EXIT_FAILURE;
    }

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        printf("signal failed\n");
        return EXIT_FAILURE;
    }

    if (!aal_initialize(MODULE_ID)) {
        printf("aal_initialize failed\n");
        return EXIT_FAILURE;
    }

    fp = fopen(argv[1], "wb");
    if (!fp) {
        printf("open failed: %s\n", argv[1]);
        goto deinit;
    }

    handle = create_recorder();
    if (!handle) {
        printf("aal_recorder_create failed\n");
        goto deinit;
    }

    aal_recorder_play(handle);

    while (true) {
        pthread_mutex_lock(&lock);
        if (flag) break;
        pthread_mutex_unlock(&lock);
    }

deinit:
    if (handle) aal_recorder_destroy(handle);
    if (fp) fclose(fp);

    aal_deinitialize(MODULE_ID);
    return EXIT_SUCCESS;
}