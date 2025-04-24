FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

inherit systemd

SRC_URI += " \
    file://37-can-am62.rules \
    file://main_mcan0.network \
    file://mcu_mcan0.network \
"

do_install:append() {
      install -d ${D}${sysconfdir}/udev/rules.d/
      install -m 0644 ${WORKDIR}/37-can-am62.rules ${D}${sysconfdir}/udev/rules.d/
      install -d ${D}${systemd_unitdir}/network/
      for file in $(find ${WORKDIR} -maxdepth 1 -type f -name *.network); do
           install -m 0644 "$file" ${D}${systemd_unitdir}/network/
      done
}

FILES:${PN} += "\
    ${systemd_system_unitdir} \
    ${sysconfdir}/udev/rules.d \
"

