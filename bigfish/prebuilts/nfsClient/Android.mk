LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ieee-iab.txt
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc
LOCAL_SRC_FILES := ieee-iab.txt
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := ieee-oui.txt
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc
LOCAL_SRC_FILES := ieee-oui.txt
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := mac-vendor.txt
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc
LOCAL_SRC_FILES := mac-vendor.txt
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := arp-scan
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_SRC_FILES := arp-scan
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAG := optional
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := showmount
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_SRC_FILES := showmount
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAG := optional
include $(BUILD_PREBUILT)
