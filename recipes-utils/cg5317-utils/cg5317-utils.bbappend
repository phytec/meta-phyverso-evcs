DESCRIPTION = "Layer includes the services to initialize CG5317 in host loading mode"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://../COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
	file://mtbf-resets.service \
	file://cg5317_0.cfg \
	file://cg5317_1.cfg \
	file://26-seth1.network \
	file://COPYING.MIT \
"

do_install:append() {
	install -d ${D}${sysconfdir}/systemd/system/
	install -m 0644 ${WORKDIR}/mtbf-resets.service ${D}${sysconfdir}/systemd/system
        install -d ${D}${sysconfdir}/lumissil
        install -m 0644 ${WORKDIR}/cg5317_0.cfg ${D}${sysconfdir}/lumissil/
        install -m 0644 ${WORKDIR}/cg5317_1.cfg ${D}${sysconfdir}/lumissil/
        install -d ${D}${systemd_unitdir}/network
        install -m 0644 ${WORKDIR}/26-seth1.network  ${D}${systemd_unitdir}/network/
}

FILES:${PN} += "${systemd_unitdir}/**"
FILES:${PN} += "${libdir}/firmware/*"
FILES:${PN} += "${ROOT_HOME}/**"
FILES:${PN} += "${sysconfdir}/lumissil/*"


SYSTEMD_SERVICE:${PN} = "cg5317-host@1.service"
