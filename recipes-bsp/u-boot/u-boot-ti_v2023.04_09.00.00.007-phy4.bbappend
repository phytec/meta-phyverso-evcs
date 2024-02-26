# Copyright (C) 2019 PHYTEC Messtechnik GmbH,
# Author: Alexander Bauer <a.bauer@phytec.de>

#include add-costum-dts.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/features:"

SRC_URI:append:am62-ksp0728-1 = " \
	file://0001-u-boot-dts-enable-USB0-VBUS.patch \
	file://0001-board-phytec-phycore_am62x-Add-RAUC-to-env.patch \
        file://0002-include-environment-Add-RAUC-boot-logic.patch \
        file://0003-configs-phycore_am62x_a53_defconfig-Move-environment-to-MMC.patch \
"

SRC_URI:append:am62-ksp0728-1-k3r5 = " \
	file://0001-u-boot-dts-enable-USB0-VBUS.patch \
"

COMPATIBLE_MACHINE:am62-ksp0728-1 = "am62-ksp0728-1"
COMPATIBLE_MACHINE:am62-ksp0728-1-k3r5 = "am62-ksp0728-1-k3r5"

