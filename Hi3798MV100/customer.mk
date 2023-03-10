#
# Copyright (C) 2011 The Android Open-Source Project
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
#

PRODUCT_TARGET := aosp

#Quick Boot Support
BOARD_QBSUPPORT := false

#support sdcardfs.
SUPPORT_SDCARDFS := true

#support fuse
SUPPORT_FUSE := fuse

#SUPPROT UI for wipe data or update from USB when pressed remote control power button on boot
SUPPORT_REMOTE_RECOVERY := false

# Whether fastplay should be played completely or not: true or false
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.fastplay.fullyplay=true

# Enable low RAM config or not: true or false
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.low_ram=true

# Enable wallpaper or not for low_ram: true or false
PRODUCT_PROPERTY_OVERRIDES += \
    persist.low_ram.wp.enable=true

# Whether bootanimation should be played or not: true or false
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.bootanim.enable=true

# Enable TDE compose or not: true or false
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.tde_compose=true

#set video output rate for telecom and unicom, defalt 4:3
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.video.cvrs= 0

#if thirdparty dhcp service is needed to obtain ip, please set this property true
# default value is false
PRODUCT_PROPERTY_OVERRIDES += \
    ro.thirdparty.dhcp=true

# smart_suspend, deep_launcher, deep_restart, deep_resume;
PRODUCT_PROPERTY_OVERRIDES += \
     persist.suspend.mode=deep_resume

# Output format adaption for 2D streams
# false -> disable; true -> enable
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.video.adaptformat=false

PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.zygote.optimize=true \
    persist.sys.boot.optimize=true \
    persist.sys.preload.fork=true
    
# Whether CVBS is enabled or not when HDMI is plugged in
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.cvbs.and.hdmi=true

# apk control mode: none/whitelist/signature/md5
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sys.apkcontrol.mode=whitelist
    
# Whether display format self-adaption is enabled or not when HDMI is plugged in
# 0 -> disable; 1 -> enable
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.optimalfmt.enable=1

# display hdmi cec
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.hdmi.cec=true

# Preferential display format: native, i50hz, p50hz, i60hz, p60hz or max_fmt
# persist.sys.optimalfmt.perfer is valid only if persist.sys.optimalfmt.enable=1
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.optimalfmt.perfer=p60hz

# Preferential hiplayer cache setting
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.player.cache.type=time \
    persist.sys.player.cache.low=500 \
    persist.sys.player.cache.high=12000 \
    persist.sys.player.cache.total=20000 \
    persist.sys.player.bufmaxsize=80

# Preferential hiplayer buffer seek
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.player.buffer.seek=0 \
    persist.sys.player.leftbuf.min=10 \
    persist.sys.player.avsync.min=500

# Preferential hiplayer rtsp timeshift support for sichuan mobile
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.hiplayer.rtspusetcp=false

# hdrmode set 0:SDR, 1:Dolby, 2:HDR10, 3:auto
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.hdrmode=3
