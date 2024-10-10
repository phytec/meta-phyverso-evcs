DESCRIPTION = "Layer includes the services to initialize CG5317 in host loading mode"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://../COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://cg5317-bringup.service \
           file://cg5317-bringup.sh \
           file://cg5317-host.service \
           file://mtbf-resets.service \
           file://resetmodem.sh \
           file://COPYING.MIT \
           "

RDEPENDS:${PN} += "libgpiod"

do_install:append() {
	install -d ${D}${sysconfdir}/systemd/system/
	install -m 0644 ${WORKDIR}/cg5317-bringup.service ${D}${sysconfdir}/systemd/system
	install -m 0644 ${WORKDIR}/cg5317-host.service ${D}${sysconfdir}/systemd/system
	install -m 0644 ${WORKDIR}/mtbf-resets.service ${D}${sysconfdir}/systemd/system

	install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants
	ln -sf ../cg5317-bringup.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/
	ln -sf ../cg5317-host.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/

	install -d ${D}${sysconfdir}/systemd/system/default.target.wants
	ln -sf ../mtbf-resets.service ${D}${sysconfdir}/systemd/system/default.target.wants/

	install -m 0755 ${WORKDIR}/resetmodem.sh ${D}${sysconfdir}/systemd/system
	install -m 0755 ${WORKDIR}/cg5317-bringup.sh ${D}${sysconfdir}/systemd/system
}

FILES:${PN} += "${base_libdir}/systemd/system"
FILES:${PN} += "/lib/firmware/*"

