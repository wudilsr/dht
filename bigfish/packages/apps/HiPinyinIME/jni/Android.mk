LOCAL_PATH := $(call my-dir)

### shared library

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	android/com_hisilicon_android_inputmethod_remote_PinyinDecoderService.cpp \
	share/dictbuilder.cpp \
	share/dictlist.cpp \
	share/dicttrie.cpp \
	share/lpicache.cpp \
	share/matrixsearch.cpp \
	share/mystdlib.cpp \
	share/ngram.cpp \
	share/pinyinime.cpp \
	share/searchutility.cpp \
	share/spellingtable.cpp \
	share/spellingtrie.cpp \
	share/splparser.cpp \
	share/userdict.cpp \
	share/utf16char.cpp \
	share/utf16reader.cpp \
	share/sync.cpp

LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)
LOCAL_LDLIBS += -lpthread
LOCAL_MODULE := libjni_remoteime
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
