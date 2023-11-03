# Copyright (C) 2019 PHYTEC Messtechnik GmbH,
# Author: Alexander Bauer <a.bauer@phytec.de>

#include add-costum-dts.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/features:"

SRC_URI:append:am62-ksp0728-1 = " \
	file://0001-u-boot-dts-enable-USB0-VBUS.patch \
"

SRC_URI:append:am62-ksp0728-1-k3r5 = " \
	file://0001-u-boot-dts-enable-USB0-VBUS.patch \
"

COMPATIBLE_MACHINE:am62-ksp0728-1 = "am62-ksp0728-1"
COMPATIBLE_MACHINE:am62-ksp0728-1-k3r5 = "am62-ksp0728-1-k3r5"

