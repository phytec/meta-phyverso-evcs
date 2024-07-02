# Copyright (C) 2019 PHYTEC Messtechnik GmbH,
# Author: Alexander Bauer <a.bauer@phytec.de>

#require recipes-kernel/linux/linux-common-append.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/features:"

SRC_URI:append = " \
		file://k3-am625-phyverso-evcs-1.dts \
                file://0001-add-distec-dd-0700-mc01-display-settings.patch \
                file://0001-add-distec-dd-0700-mc01-dpi-display-settings.patch \
		file://cc33xx_kernel.patch \
		file://0001-cc33xx-disable-irq-on-missing-calibration-data.patch \
		file://cc33xx.cfg \
"

do_configure:append(){
 cp ${WORKDIR}/k3-am625-phyverso-evcs-1.dts ${S}/arch/arm64/boot/dts/ti
}

#file://0001-Add-fortec-displays.patch 
#file://0002-Add-qwello-displays.patch 
#file://0001-add-etml1010g3dra-lvds.patch
#file://0003-Add-support-for-qwello-display-on-the-dpi-port.patch 
#file://0001-add_etml1010g3dra_rgb.patch
#file://0001-add-etml1010g3dra-lvds.patch
#file://0001-add_etml1010g3dra_rgb.patch

COMPATIBLE_MACHINE:am62-phyverso-evcs-1 = "am62-phyverso-evcs-1"
