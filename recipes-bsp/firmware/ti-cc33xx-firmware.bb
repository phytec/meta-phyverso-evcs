SUMMARY = "firmware for cc33xx radio module"
SECTION = "kernel"

inherit allarch

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://../COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

ROOT_HOME = "/root"


SRC_URI = "file://cc33xx_2nd_loader.bin \
           file://cc33xx_fw.bin \
           file://cc33xx-conf.bin \
           file://COPYING.MIT \
"

do_install() {
    install -d ${D}/${nonarch_base_libdir}/firmware/ti-connectivity/

    install -m 0644 ${WORKDIR}/cc33xx_2nd_loader.bin ${D}/${nonarch_base_libdir}/firmware/ti-connectivity/
    install -m 0644 ${WORKDIR}/cc33xx_fw.bin ${D}/${nonarch_base_libdir}/firmware/ti-connectivity/
    install -m 0644 ${WORKDIR}/cc33xx-conf.bin ${D}/${nonarch_base_libdir}/firmware/ti-connectivity/
}

FILES:${PN}:append = " \
    ${nonarch_base_libdir}/firmware/ti-connectivity/cc33xx_2nd_loader.bin \
    ${nonarch_base_libdir}/firmware/ti-connectivity/cc33xx_fw.bin \
    ${nonarch_base_libdir}/firmware/ti-connectivity/cc33xx-conf.bin \
"
