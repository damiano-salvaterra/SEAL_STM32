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

#ifndef BNO08X_APP_H
#define BNO08X_APP_H

#include <stdint.h>
#include "imu.h" //need to include this because this module act as an adapter between the HAL middleware and the high level thread funcitons

uint8_t bno08x_RVC_init(void);
uint8_t bno08x_get_imu_data(uint8_t max_frames, IMUValue_t* out_buffer);
void bno08x_RVC_close();

#endif
