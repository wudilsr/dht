LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fsck.exfat
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_SRC_FILES := tools/fsck.exfat
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := mkfs.exfat
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_SRC_FILES := tools/mkfs.exfat
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := dump.exfat
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_SRC_FILES := tools/dump.exfat
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := mount.exfat
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_SRC_FILES := tools/mount.exfat
include $(BUILD_PREBUILT)