/*
 * Copyright 2020-21 CEVA, Inc.
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

#include <stdio.h>
#include <stdbool.h>

#include "rvc.h"
#include "rvc_hal.h"


rvc_SensorEvent_t sensorEvent;

// initialize RVC subsystem
//int rvc_init()
//{
//    return RVC_OK;
//}


// open the RVC interface (starts sensor events)
int rvc_open()
{
    return rvc_hal_open();
}

// close the RVC interface (ends sensor events)
void rvc_close()
{
    rvc_hal_close();
}

#define SKIP_COUNT (1000000)

// periodically service the RVC subsystem.
// must be called periodically to service the RVC UART, parse RVC messages
// and call the RVC callback on each sensor event.
uint8_t rvc_service(uint8_t max_frames, rvc_SensorValue_t* out_buffer){
    rvc_SensorEvent_t event;
    uint8_t count = 0;
    bool done = false;
    
    while (!done && (count < max_frames)) {
        
        // Read a single frame 
        int status = rvc_hal_read(&event);

        if (status > 0) {
            // we have a valid frame
            
        
            //if the passed array is valid, directly decode the data and put the in the array
            if (out_buffer != NULL) {
                rvc_decode(&out_buffer[count], &event);
            }
            
            count++;
            
        }
        else {
            done = true;
        }
    }
    
    return count;
}

// Convert from SensorEvent (integer, fixed-point representation)
// to SensorValue (float, degrees and g's)
void rvc_decode(rvc_SensorValue_t *value, const rvc_SensorEvent_t *event)
{
    value->index = event->index;
    value->yaw_deg =   0.01  * event->yaw;
    value->pitch_deg = 0.01  * event->pitch;
    value->roll_deg =  0.01  * event->roll;
    value->acc_x_g =   0.001 * event->acc_x;
    value->acc_y_g =   0.001 * event->acc_y;
    value->acc_z_g =   0.001 * event->acc_z;
    value->mi =        event->mi;
    value->mr =        event->mr;

    value->timestamp_uS = event->timestamp_uS;
}


