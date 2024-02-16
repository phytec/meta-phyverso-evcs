# Copyright (C) 2019 PHYTEC Messtechnik GmbH,
# Author: Alexander Bauer <a.bauer@phytec.de>

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/features:"

SRC_URI:append = " \
		file://k3-am625-ksp0728.dts \
		file://0001-add_etml1010g3dra.patch \
		file://cc33xx_kernel.patch \
		file://cc33xx.cfg \
"

COMPATIBLE_MACHINE:am62-ksp0728-1 = "am62-ksp0728-1"
