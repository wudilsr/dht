LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include $(SDK_DIR)/Android.def

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libvpcodec
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := $(CFG_HI_CFLAGS)
LOCAL_CFLAGS += -DLOG_TAG=\"$(LOCAL_MODULE)\"

LOCAL_SRC_FILES := vp_codec_1_0.c

LOCAL_C_INCLUDES := $(MSP_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_API_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_API_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_DRV_INCLUDE)
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../rtp
#LOCAL_C_INCLUDES += $(LOCAL_PATH)
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include

LOCAL_SHARED_LIBRARIES := libcutils libutils libdl libhi_common libhi_msp

include $(BUILD_SHARED_LIBRARY)
