# Copyright (C) 2019 PHYTEC Messtechnik GmbH,
# Author: Alexander Bauer <a.bauer@phytec.de>

#require recipes-kernel/linux/linux-common-append.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/features:"

SRC_URI:append = " \
		file://k3-am625-phyverso-evcs.dts \
                file://0001-add-distec-dd-0700-mc01-display-settings.patch \
                file://0001-add-distec-dd-0700-mc01-dpi-display-settings.patch \
		file://cc33xx_kernel.patch \
		file://0001-cc33xx-disable-irq-on-missing-calibration-data.patch \
		file://cc33xx.cfg \
"

do_configure:append(){
 cp ${WORKDIR}/k3-am625-phyverso-evcs.dts ${S}/arch/arm64/boot/dts/ti
}

COMPATIBLE_MACHINE:am62-phyverso-evcs = "am62-phyverso-evcs"
