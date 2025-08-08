# Copyright (C) 2019 PHYTEC Messtechnik GmbH,
# Author: Alexander Bauer <a.bauer@phytec.de>

#require recipes-kernel/linux/linux-common-append.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/features:"

SRC_URI:append = " \
		file://k3-am625-phyverso-evcs.dts \
                file://0001-add-distec-dd-0700-mc01-display-settings.patch \
                file://0001-add-distec-dd-0700-mc01-dpi-display-settings.patch \
		file://dd0700mc01_dpi.dtso \
		file://dd0700mc01_lvds.dtso \
		file://k3-am62-phyboard-lyra-oldi-ac209a.dtso \
		file://cc33xx.dtso \
		file://cc33xx.cfg  \
"


do_configure:append(){
 cp ${WORKDIR}/k3-am625-phyverso-evcs.dts ${S}/arch/arm64/boot/dts/ti
}

do_compile:prepend () {
                cp "${WORKDIR}/dd0700mc01_dpi.dtso" \
       "${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/ti/"
                cp "${WORKDIR}/dd0700mc01_lvds.dtso" \
       "${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/ti/"
                cp "${WORKDIR}/k3-am62-phyboard-lyra-oldi-ac209a.dtso" \
       "${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/ti/"
                cp "${WORKDIR}/cc33xx.dtso" \
       "${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/ti/"
}


COMPATIBLE_MACHINE:am62-phyverso-evcs = "am62-phyverso-evcs"
