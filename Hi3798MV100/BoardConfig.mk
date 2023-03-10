#
# BoardConfig.mk
# Product-specific compile-time definitions.
#

#################################################################################
##  Variable configuration definition
################################################################################

# SDK configure
HISI_SDK_ANDROID_CFG := hi3798mdmo_hi3798mv100_android_cfg.mak
#HISI_SDK_ANDROID_VMX_CFG := hi3798mdmo_hi3798mv100_android_vmx_cfg.mak
#HISI_SDK_SECURE_CFG := hi3798mdmo_hi3798mv100_android_security_cfg.mak
#HISI_SDK_TEE_CFG := hi3798mdmo_hi3798mv100_android_tee_cfg.mak
#HISI_SDK_RECOVERY_CFG := hi3798mdmo_hi3798mv100_android_recovery_cfg.mak
#HISI_SDK_TEE_VMX_CFG := hi3798mdmo_hi3798mv100_android_tee_vmx_cfg.mak
SDK_CFG_DIR := configs/hi3798mv100

# Kernel configure
RAMDISK_ENABLE := true
ANDROID_KERNEL_CONFIG := hi3798mv100_android_defconfig
RECOVERY_KERNEL_CONFIG := hi3798mv100_android_recovery_defconfig

# emmc fastboot configure
EMMC_BOOT_CFG_NAME := hi3798mdmo1g_hi3798mv100_ddr3_1gbyte_16bitx2_4layers_emmc.cfg
EMMC_BOOT_REG_NAME := hi3798mdmo1g_hi3798mv100_ddr3_1gbyte_16bitx2_4layers_emmc.reg
EMMC_BOOT_ENV_STARTADDR :=0x100000
EMMC_BOOT_ENV_SIZE=0x10000
EMMC_BOOT_ENV_RANGE=0x10000

#
# ext4 file system configure
# the ext4 file system just use in the eMMC
#
# BOARD_FLASH_BLOCK_SIZE :              we do not need to change it,but needed
# BOARD_SYSTEMIMAGE_PARTITION_SIZE:     system size,
# BOARD_USERDATAIMAGE_PARTITION_SIZE:   userdata size,
# BOARD_CACHEIMAGE_PARTITION_SIZE :     cache size
# BOARD_SDCARDIMAGE_PARTITION_SIZE :    sdcard size
# 524288000 represent 524288000/1024/1024 = 500MB
# system,userdata,cache size should be consistent with the bootargs
#

TARGET_USERIMAGES_USE_EXT4 := true
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 838860800

ifeq ($(SUPPORT_SDCARDFS),true)
    BOARD_USERDATAIMAGE_PARTITION_SIZE := 1912602624
else
BOARD_USERDATAIMAGE_PARTITION_SIZE := 1073741824
BOARD_SDCARDIMAGE_PARTITION_SIZE := 838860800
endif
BOARD_CACHEIMAGE_PARTITION_SIZE := 524288000
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_PRIVATEIMAGE_PARTITION_SIZE := 52428800
BOARD_FLASH_BLOCK_SIZE := 4096

# Enable WiFi Only module used in the board.
BOARD_WLAN_DEVICE_RTL8188FTV := y

################################################################################
##  Stable configuration definitions
################################################################################

# The generic product target doesn't have any hardware-specific pieces.
TARGET_NO_BOOTLOADER := true
ifeq ($(RAMDISK_ENABLE),false)
TARGET_NO_KERNEL := true
else
TARGET_NO_KERNEL := false
endif
BOARD_KERNEL_BASE :=0x3000000
BOARD_KERNEL_PAGESIZE :=16384
TARGET_NO_RECOVERY := true
TARGET_NO_RADIOIMAGE := true
TARGET_ARCH := arm

# Note: we build the platform images for ARMv7-A _without_ NEON.
#
# Technically, the emulator supports ARMv7-A _and_ NEON instructions, but
# emulated NEON code paths typically ends up 2x slower than the normal C code
# it is supposed to replace (unlike on real devices where it is 2x to 3x
# faster).
#
# What this means is that the platform image will not use NEON code paths
# that are slower to emulate. On the other hand, it is possible to emulate
# application code generated with the NDK that uses NEON in the emulator.
#

TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a9
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
ARCH_ARM_HAVE_TLS_REGISTER := true

BOARD_USES_GENERIC_AUDIO := true

# no hardware camera
USE_CAMERA_STUB := true

# Build OpenGLES emulation guest and host libraries
#BUILD_EMULATOR_OPENGL := true

# Build and enable the OpenGL ES View renderer. When running on the emulator,
# the GLES renderer disables itself if host GL acceleration isn't available.
USE_OPENGL_RENDERER := true

#
#  Hisilicon Platform
#

# Buildin Hisilicon GPU gralloc and GPU libraries.
BUILDIN_HISI_GPU := true

# Buildin Hisilicon NDK extensions
BUILDIN_HISI_EXT := true

# Configure buildin Hisilicon smp
TARGET_CPU_SMP := true

# Disable use system/core/rootdir/init.rc
# HiSilicon use device/hisilicon/bigfish/etc/init.rc
TARGET_PROVIDES_INIT_RC := true

# Configure Board Platform name
TARGET_BOARD_PLATFORM := bigfish
TARGET_BOOTLOADER_BOARD_NAME := bigfish

# Define Hisilicon platform path.
HISI_PLATFORM_PATH := device/hisilicon/bigfish
# Define Hisilicon sdk path.
SDK_DIR := $(HISI_PLATFORM_PATH)/sdk
# Define sdk boot table configures directory
SDK_BOOTCFG_DIR := $(SDK_DIR)/source/boot/sysreg
# Configure Hisilicon Linux Kernel Version
HISI_LINUX_KERNEL := linux-3.10.y
# Define Hisilicon linux kernel source path.
HISI_KERNEL_SOURCE := $(SDK_DIR)/source/kernel/$(HISI_LINUX_KERNEL)

# wpa_supplicant and hostapd build configuration
# wpa_supplicant is used for WiFi STA, hostapd is used for WiFi SoftAP
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_WLAN_DEVICE := bcmdhd

# Set /system/bin/sh to mksh, not ash, to test the transition.
TARGET_SHELL := mksh
INTERGRATE_THIRDPARTY_APP := true
SUPPORT_IPV6 := true

# widevine Level setting
BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 3
