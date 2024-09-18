# Copyright (C) 2019 PHYTEC Messtechnik GmbH,
# Author: Alexander Bauer <a.bauer@phytec.de>

#include add-costum-dts.inc
#include u-boot-rauc.inc
#include u-boot-fitimage.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/features:"

SRC_URI:append:am62-phyverso-evcs = " \
	file://0001-u-boot-dts-enable-USB0-VBUS.patch \
	file://0001-Enable-RAW_INITRD-for-ramfs.patch \
	file://0001-u-boot-Add-ddr-phy-reg-count.patch \
"

SRC_URI:append:am62-phyverso-evcs-k3r5 = " \
	file://0001-u-boot-dts-enable-USB0-VBUS.patch \
"

COMPATIBLE_MACHINE:am62-phyverso-evcs = "am62-phyverso-evcs"
COMPATIBLE_MACHINE:am62-phyverso-evcs-k3r5 = "am62-phyverso-evcs-k3r5"

