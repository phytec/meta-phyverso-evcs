SUMMARY = "firmware for cc33xx radio module"
SECTION = "kernel"

inherit allarch

#LICENSE = "TI"
#NO_GENERIC_LICENSE[Laird] = "LICENSE"
#LIC_FILES_CHKSUM = "file://LICENSE;md5=53d3628b28a0bc3caea61587feade5f9"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://../COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

ROOT_HOME = "/root"


SRC_URI = "file://cc33xx_2nd_loader.bin \
           file://cc33xx_fw.bin \
           file://cc33xx-conf.bin \
           file://COPYING.MIT \
"

do_install() {
    install -d ${D}/lib/firmware/ti-connectivity/

    install -m 0644 ${WORKDIR}/cc33xx_2nd_loader.bin ${D}/lib/firmware/ti-connectivity/
    install -m 0644 ${WORKDIR}/cc33xx_fw.bin ${D}/lib/firmware/ti-connectivity/
    install -m 0644 ${WORKDIR}/cc33xx-conf.bin ${D}/lib/firmware/ti-connectivity/
}

FILES:${PN}:append = " \
    lib/firmware/ti-connectivity/cc33xx_2nd_loader.bin \
    lib/firmware/ti-connectivity/cc33xx_fw.bin \
    lib/firmware/ti-connectivity/cc33xx-conf.bin \
"

