LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := create.ko
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/modules
LOCAL_SRC_FILES := create_$(CHIPNAME).ko
include $(BUILD_PREBUILT)

