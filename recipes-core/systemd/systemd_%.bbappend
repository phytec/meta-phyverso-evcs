FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit systemd

SRC_URI += " \
    file://37-can-am62.rules \
"

do_install:append() {
      install -d ${D}${sysconfdir}/udev/rules.d/
      install -m 0644 ${WORKDIR}/37-can-am62.rules ${D}${sysconfdir}/udev/rules.d/
}

FILES:${PN} += "\
    ${systemd_system_unitdir} \
    ${sysconfdir}/udev/rules.d \
"

