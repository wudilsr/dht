# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# We're moving the emulator-specific platform libs to
# development.git/tools/emulator/. The following test is to ensure
# smooth builds even if the tree contains both versions.
#

LOCAL_PATH := $(call my-dir)

# HAL module implemenation, not prelinked and stored in
# hw/<GPS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_CFLAGS += -DQEMU_HARDWARE
LOCAL_SHARED_LIBRARIES := liblog libcutils libhardware
LOCAL_SRC_FILES := gps_qemu.c
LOCAL_MODULE := gps.bigfish
LOCAL_MODULE_TAGS := optional
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
include $(BUILD_SHARED_LIBRARY)

