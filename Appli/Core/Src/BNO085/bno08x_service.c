/*
 * Copyright 2017-2021 CEVA, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License and 
 * any applicable agreements you may have with CEVA, Inc.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Demo App for SH2 devices (BNO08x and FSP200)
 */

// ------------------------------------------------------------------------

// Sensor Application
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "bno08x_service.h"

#include "rvc.h"

// --- Private methods --------------------------------------------------



// --- Public methods -------------------------------------------------


uint8_t bno08x_RVC_init(void)
{
    int status;

    //status = rvc_init();
    //if (status != RVC_OK) {
    //    printf("Error, %d, from rvc_init.\n", status);
    //}

    
    status = rvc_open();
    if (status != RVC_OK) {
        printf("Error, %d, from rvc_open.\n", status);
    }

    return (uint8_t) status;
}


void bno08x_RVC_close()
{
    rvc_close();
}


//Adapter function
uint8_t bno08x_get_imu_data(uint8_t max_frames, IMUValue_t* out_buffer)
{
    if (out_buffer == NULL || max_frames == 0) return 0;

    // Temporary buffer with RVC type
    rvc_SensorValue_t rvc_buffer[max_frames];

    // call low level driver
    uint8_t fetched_frames = rvc_service(max_frames, rvc_buffer);

    // translate RVC structs into high level struct
    for (uint8_t i = 0; i < fetched_frames; i++) {
        out_buffer[i].timestamp_uS = rvc_buffer[i].timestamp_uS;
        out_buffer[i].index        = rvc_buffer[i].index;
        out_buffer[i].yaw_deg      = rvc_buffer[i].yaw_deg;
        out_buffer[i].pitch_deg    = rvc_buffer[i].pitch_deg;
        out_buffer[i].roll_deg     = rvc_buffer[i].roll_deg;
        out_buffer[i].acc_x_g      = rvc_buffer[i].acc_x_g;
        out_buffer[i].acc_y_g      = rvc_buffer[i].acc_y_g;
        out_buffer[i].acc_z_g      = rvc_buffer[i].acc_z_g;
    }

    return fetched_frames;
}