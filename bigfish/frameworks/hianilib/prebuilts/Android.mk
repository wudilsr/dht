LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := hiani:hiani.jar
include $(BUILD_MULTI_PREBUILT)
