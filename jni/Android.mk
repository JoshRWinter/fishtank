LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := freetype
ifeq "$(TARGET_ARCH_ABI)" "armeabi-v7a"
	LOCAL_SRC_FILES := ../../freetype-2.5.3/objs/freetype-armeabi-v7a.a
else ifeq "$(TARGET_ARCH_ABI)" "x86"
	LOCAL_SRC_FILES := ../../freetype-2.5.3/objs/freetype-x86.a
endif
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ogg-vorbis
ifeq "$(TARGET_ARCH_ABI)" "armeabi-v7a"
	LOCAL_SRC_FILES := ../../libogg-1.3.2/lib/ogg-vorbis-armeabi-v7a.a
else ifeq "$(TARGET_ARCH_ABI)" "x86"
	LOCAL_SRC_FILES := ../../libogg-1.3.2/lib/ogg-vorbis-x86.a
endif
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := main
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../freetype-2.5.3/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../libogg-1.3.2/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../libvorbis-1.3.4/include
O_FILES := $(wildcard $(LOCAL_PATH)/object/*.cpp)
FILES := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := glesutil.c $(FILES:$(LOCAL_PATH)/%=%) $(O_FILES:$(LOCAL_PATH)/%=%)
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2 -lz -lOpenSLES
LOCAL_CFLAGS := -O3
LOCAL_STATIC_LIBRARIES := android_native_app_glue freetype ogg-vorbis
include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)
