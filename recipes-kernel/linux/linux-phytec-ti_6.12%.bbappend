FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
		file://k3-am625-phyverso-evcs-oldi-ac209a.dtso \
"


do_compile:prepend () {
        cp "${WORKDIR}/k3-am625-phyverso-evcs-oldi-ac209a.dtso" \
       "${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/ti/"
}

COMPATIBLE_MACHINE:append = "|phyverso-evcs"
