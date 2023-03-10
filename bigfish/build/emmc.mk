include $(CLEAR_VARS)
CHIP_TABLE              := $(CHIPNAME)-sdcardfs-emmc.xml	
CHIP_TABLE_PATH         := $(TOP)/device/hisilicon/$(CHIPNAME)/prebuilts/
PREBUILT_BASEPARAM_PATH := $(TOP)/device/hisilicon/$(CHIPNAME)/prebuilts/
PREBUILT_LOGO_PATH      := $(TOP)/device/hisilicon/$(CHIPNAME)/prebuilts/
PREBUILT_FASTPLAY_PATH  := $(TOP)/device/hisilicon/$(CHIPNAME)/prebuilts/

EMMC_CHIP_TABLE := $(EMMC_PRODUCT_OUT)/$(CHIP_TABLE)
$(EMMC_CHIP_TABLE) : $(EMMC_PRODUCT_OUT)/% : $(CHIP_TABLE_PATH)/% | $(ACP)
	$(transform-prebuilt-to-target)

EMMC_PREBUILT_BASEPARAM_IMG := $(EMMC_PRODUCT_OUT)/baseparam.img
$(EMMC_PREBUILT_BASEPARAM_IMG) : $(EMMC_PRODUCT_OUT)/% : $(PREBUILT_BASEPARAM_PATH)/% | $(ACP)
	$(transform-prebuilt-to-target)

EMMC_PREBUILT_LOGO_IMG := $(EMMC_PRODUCT_OUT)/logo.img
$(EMMC_PREBUILT_LOGO_IMG) : $(EMMC_PRODUCT_OUT)/% : $(PREBUILT_LOGO_PATH)/% | $(ACP)
	$(transform-prebuilt-to-target)

EMMC_PREBUILT_FASTPLAY_IMG := $(EMMC_PRODUCT_OUT)/fastplay.img
$(EMMC_PREBUILT_FASTPLAY_IMG) : $(EMMC_PRODUCT_OUT)/% : $(PREBUILT_FASTPLAY_PATH)/% | $(ACP)
	$(transform-prebuilt-to-target)

EMMC_PREBUILT_IMG := $(EMMC_CHIP_TABLE) \
		$(EMMC_PREBUILT_BASEPARAM_IMG) \
		$(EMMC_PREBUILT_LOGO_IMG) \
		$(EMMC_PREBUILT_FASTPLAY_IMG)

$(EMMC_PREBUILT_IMG) : $(EMMC_CHIP_TABLE) $(EMMC_PREBUILT_BASEPARAM_IMG) $(EMMC_PREBUILT_LOGO_IMG) $(EMMC_PREBUILT_FASTPLAY_IMG)

.PHONY:prebuilt-emmc
prebuilt-emmc:$(EMMC_PREBUILT_IMG)

#----------------------------------------------------------------------
# hiboot
#----------------------------------------------------------------------
include $(CLEAR_VARS)

EMMC_HIBOOT_IMG := fastboot-burn-emmc.bin
EMMC_HIBOOT_OBJ := $(TARGET_OUT_INTERMEDIATES)/EMMC_HIBOOT_OBJ
EMMC_BOOT_ANDROID_CFG := $(HISI_SDK_ANDROID_CFG)

emmc_fastboot_prepare:
	mkdir -p $(EMMC_HIBOOT_OBJ)
	mkdir -p $(EMMC_PRODUCT_OUT)

$(EMMC_HIBOOT_IMG): emmc_fastboot_prepare
	cp $(SDK_DIR)/$(SDK_CFG_DIR)/$(EMMC_BOOT_ANDROID_CFG) $(EMMC_HIBOOT_OBJ);\
	sed -i '/CFG_HI_LOADER_SUPPORT/,1d' $(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG); \
	sed -i '/CFG_HI_APPLOADER_SUPPORT/,1d' $(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG); \
	if [ "$(SUPPORT_REMOTE_RECOVERY)" = "true" ]; then \
	sed -i -e '/# CFG_HI_BUILD_WITH_IR /a\CFG_HI_BUILD_WITH_IR = y' -e '/# CFG_HI_BUILD_WITH_IR/d' \
	$(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG); \
	fi
	if [ -n "$(EMMC_BOOT_REG_NAME)" ]; then \
	sed -i s/CFG_HI_BOOT_REG_NAME=.*/CFG_HI_BOOT_REG_NAME=${EMMC_BOOT_REG_NAME}/ $(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG);\
	fi
	if [ -n "$(EMMC_BOOT_ENV_STARTADDR)" ];then \
	sed -i s/CFG_HI_BOOT_ENV_STARTADDR=.*/CFG_HI_BOOT_ENV_STARTADDR=${EMMC_BOOT_ENV_STARTADDR}/ $(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG);\
	fi
	if [ -n "$(EMMC_BOOT_ENV_SIZE)" ];then \
	sed -i s/CFG_HI_BOOT_ENV_SIZE=.*/CFG_HI_BOOT_ENV_SIZE=${EMMC_BOOT_ENV_SIZE}/ $(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG);\
	fi
	if [ -n "$(EMMC_BOOT_ENV_RANGE)" ]; then \
	sed -i s/CFG_HI_BOOT_ENV_RANGE=.*/CFG_HI_BOOT_ENV_RANGE=${EMMC_BOOT_ENV_RANGE}/ $(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG);\
	fi
	cd $(SDK_DIR);$(MAKE) hiboot O=$(ANDROID_BUILD_TOP)/$(EMMC_HIBOOT_OBJ) \
	SDK_CFGFILE=../../../../$(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG);\
	if [ -f "$(ANDROID_BUILD_TOP)/$(EMMC_HIBOOT_OBJ)/miniboot.bin" ]; then \
	cp -avf $(ANDROID_BUILD_TOP)/$(EMMC_HIBOOT_OBJ)/miniboot.bin $(ANDROID_BUILD_TOP)/$(EMMC_PRODUCT_OUT)/fastboot.bin ;\
	elif [ -f "$(ANDROID_BUILD_TOP)/$(EMMC_HIBOOT_OBJ)/fastboot-burn.bin" ]; then \
	cp -avf $(ANDROID_BUILD_TOP)/$(EMMC_HIBOOT_OBJ)/fastboot-burn.bin $(ANDROID_BUILD_TOP)/$(EMMC_PRODUCT_OUT)/fastboot.bin ; \
	else \
	exit; \
	fi
	if [ -f "$(ANDROID_BUILD_TOP)/$(EMMC_HIBOOT_OBJ)/advca_programmer.bin" ]; then \
	cp -avf $(ANDROID_BUILD_TOP)/$(EMMC_HIBOOT_OBJ)/advca_programmer.bin $(ANDROID_BUILD_TOP)/$(EMMC_PRODUCT_OUT)/advca_programmer.bin;\
	fi



.PHONY: hiboot-emmc
hiboot-emmc: $(EMMC_HIBOOT_IMG)

#----------------------------------------------------------------------
EMMC_BOOT_ANDROID_CFG := $(HISI_SDK_ANDROID_CFG)

emmc_fastboot_prepare_2:
	mkdir -p $(EMMC_HIBOOT_OBJ_2)
	mkdir -p $(EMMC_PRODUCT_OUT)

$(EMMC_HIBOOT_IMG_2): emmc_fastboot_prepare_2
	cp $(SDK_DIR)/$(SDK_CFG_DIR)/$(EMMC_BOOT_ANDROID_CFG) $(EMMC_HIBOOT_OBJ_2);\
	sed -i '/CFG_HI_LOADER_SUPPORT/,1d' $(EMMC_HIBOOT_OBJ_2)/$(EMMC_BOOT_ANDROID_CFG); \
	sed -i '/CFG_HI_APPLOADER_SUPPORT/,1d' $(EMMC_HIBOOT_OBJ_2)/$(EMMC_BOOT_ANDROID_CFG); \
	if [ "$(SUPPORT_REMOTE_RECOVERY)" = "true" ]; then \
	sed -i -e '/# CFG_HI_BUILD_WITH_IR /a\CFG_HI_BUILD_WITH_IR = y' -e '/# CFG_HI_BUILD_WITH_IR/d' \
	$(EMMC_HIBOOT_OBJ)/$(EMMC_BOOT_ANDROID_CFG); \
	fi
	if [ -n "$(EMMC_BOOT_ENV_STARTADDR)" ];then \
	sed -i s/CFG_HI_BOOT_ENV_STARTADDR=.*/CFG_HI_BOOT_ENV_STARTADDR=${EMMC_BOOT_ENV_STARTADDR}/ $(EMMC_HIBOOT_OBJ_2)/$(EMMC_BOOT_ANDROID_CFG);\
	fi
	if [ -n "$(EMMC_BOOT_ENV_SIZE)" ];then \
	sed -i s/CFG_HI_BOOT_ENV_SIZE=.*/CFG_HI_BOOT_ENV_SIZE=${EMMC_BOOT_ENV_SIZE}/ $(EMMC_HIBOOT_OBJ_2)/$(EMMC_BOOT_ANDROID_CFG);\
	fi
	if [ -n "$(EMMC_BOOT_ENV_RANGE)" ]; then \
	sed -i s/CFG_HI_BOOT_ENV_RANGE=.*/CFG_HI_BOOT_ENV_RANGE=${EMMC_BOOT_ENV_RANGE}/ $(EMMC_HIBOOT_OBJ_2)/$(EMMC_BOOT_ANDROID_CFG);\
	fi
	cd $(SDK_DIR);$(MAKE) hiboot O=$(ANDROID_BUILD_TOP)/$(EMMC_HIBOOT_OBJ_2) \
	SDK_CFGFILE=../../../../$(EMMC_HIBOOT_OBJ_2)/$(EMMC_BOOT_ANDROID_CFG);

.PHONY: hiboot-emmc-2
hiboot-emmc-2: $(EMMC_HIBOOT_IMG_2)
#----------------------------------------------------------------------
# hiboot END
#


#----------------------------------------------------------------------
# Generate The Update Package
# 1: emmc update.zip
#----------------------------------------------------------------------

include $(CLEAR_VARS)

EMMC_UPDATE_PACKAGE :=$(EMMC_PRODUCT_OUT)/update.zip

$(EMMC_UPDATE_PACKAGE): $(INSTALLED_SYSTEMIMAGE) $(INSTALLED_USERDATAIMAGE_TARGET) $(KERNEL_IMAGE) $(RECOVERY_IMAGE) $(EMMC_PREBUILT_IMG) $(EMMC_HIBOOT_IMG) $(UPDATE_TOOLS)

	@echo ----- Making update package ------
	$(hide) rm -rf $(EMMC_PRODUCT_OUT)/update
	$(hide) mkdir -p $(EMMC_PRODUCT_OUT)/update
	$(hide) mkdir -p $(EMMC_PRODUCT_OUT)/update/file
	$(hide) mkdir -p $(EMMC_PRODUCT_OUT)/update/file/META
	$(hide) cp -af $(call include-path-for, recovery)/etc/recovery.emmc.fstab $(EMMC_PRODUCT_OUT)/update/file/META/recovery.fstab
	$(hide) cp -a $(PRODUCT_OUT)/bootargs_emmc.txt $(EMMC_PRODUCT_OUT)/update/file/META/bootargs.txt
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/fastboot.bin $(EMMC_PRODUCT_OUT)/update/file/fastboot.img
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/recovery.img $(EMMC_PRODUCT_OUT)/update/file/recovery.img
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/baseparam.img $(EMMC_PRODUCT_OUT)/update/file/baseparam.img
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/logo.img $(EMMC_PRODUCT_OUT)/update/file/logo.img
ifneq ($(strip $(BOARD_QBSUPPORT)),true)
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/fastplay.img $(EMMC_PRODUCT_OUT)/update/file/fastplay.img
endif
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/bootargs.bin $(EMMC_PRODUCT_OUT)/update/file/bootargs.img
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/kernel.img $(EMMC_PRODUCT_OUT)/update/file/boot.img
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/pq_param.bin $(EMMC_PRODUCT_OUT)/update/file/pq_param.img
ifeq ($(strip $(BOARD_QBSUPPORT)),true)
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/userapi.bin $(EMMC_PRODUCT_OUT)/update/file/userapi.img
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/hibdrv.bin $(EMMC_PRODUCT_OUT)/update/file/hibdrv.img
endif
	$(hide) mkdir -p $(EMMC_PRODUCT_OUT)/update/file/META-INF/com/google/android
	$(hide) mkdir -p $(EMMC_PRODUCT_OUT)/update/file/META-INF/com/android
	sed -n 's/ro.build.fingerprint/post-build/p' $(PRODUCT_OUT)/system/build.prop  > $(EMMC_PRODUCT_OUT)/update/file/META-INF/com/android/metadata
	$(hide) cp -a $(PRODUCT_OUT)/system/bin/updater $(EMMC_PRODUCT_OUT)/update/file/META-INF/com/google/android/update-binary
	$(hide) cp -af $(PRODUCT_OUT)/system $(EMMC_PRODUCT_OUT)/update/file/system
	$(hide) cp -af $(PRODUCT_OUT)/data $(EMMC_PRODUCT_OUT)/update/file/userdata
	$(hide) cp -a $(call include-path-for, recovery)/etc/META-INF/com/google/android/updater-script-emmc $(EMMC_PRODUCT_OUT)/update/file/META-INF/com/google/android/updater-script
	$(hide) echo "recovery_api_version=$(RECOVERY_API_VERSION)" >$(EMMC_PRODUCT_OUT)/update/file/META/misc_info.txt
	$(hide) echo "fstab_version=$(RECOVERY_FSTAB_VERSION)" >> $(EMMC_PRODUCT_OUT)/update/file/META/misc_info.txt
	$(hide) (cd $(EMMC_PRODUCT_OUT)/update/file && zip -qry ../sor_update.zip .)
	zipinfo -1 $(EMMC_PRODUCT_OUT)/update/sor_update.zip | awk '/^system\// {print}' | $(HOST_OUT_EXECUTABLES)/fs_config > $(EMMC_PRODUCT_OUT)/update/file/META/filesystem_config.txt
	$(hide) (cd $(EMMC_PRODUCT_OUT)/update/file && zip -q ../sor_update.zip META/*)
	java -jar $(SIGNAPK_JAR) -w  $(DEFAULT_SYSTEM_DEV_CERTIFICATE).x509.pem $(DEFAULT_SYSTEM_DEV_CERTIFICATE).pk8 $(EMMC_PRODUCT_OUT)/update/sor_update.zip $(EMMC_PRODUCT_OUT)/update/update.zip
	$(hide) cp -a $(EMMC_PRODUCT_OUT)/update/update.zip  $(EMMC_PRODUCT_OUT)/
	$(hide) rm -rf $(EMMC_PRODUCT_OUT)/update
	@echo ----- Made update package: $@ --------



.PHONY: updatezip-emmc
updatezip-emmc: $(EMMC_UPDATE_PACKAGE)
#----------------------------------------------------------------------
# Generate The Update Package End
#----------------------------------------------------------------------
