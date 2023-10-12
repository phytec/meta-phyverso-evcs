FILESEXTRAPATHS:prepend := "${THISDIR}/${BPN}:"

SRC_URI += " \
    file://10-watchdog.conf \
"

do_install:append() {
      install -m 0644 ${WORKDIR}/10-watchdog.conf ${D}${systemd_unitdir}/system.conf.d/10-watchdog.conf

}
#FILES:${PN} += "
#    ${systemd_system_unitdir}
#    ${sysconfdir}/udev/rules.d
#"
#}
