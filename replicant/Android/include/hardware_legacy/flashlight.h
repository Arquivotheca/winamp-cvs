/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HARDWARE_FLASHLIGHT_H
#define _HARDWARE_FLASHLIGHT_H

#if __cplusplus
extern "C" {
#endif

int get_flashlight_enabled();
int set_flashlight_enabled(int on);
int enable_camera_flash(int milliseconds);

#if __cplusplus
} // extern "C"
#endif

#endif // _HARDWARE_FLASHLIGHT_H
