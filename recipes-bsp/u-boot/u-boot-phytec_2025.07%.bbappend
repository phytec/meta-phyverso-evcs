FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
FILESEXTRAPATHS:prepend := "${THISDIR}/features:"

SRC_URI:append:am62-phyverso-evcs = " \
	file://0001-u-boot-enable-cold-reboot-in-PMIC.patch \
    file://0001-u-boot-dts-enable-USB0-VBUS.patch \
"

SRC_URI:append:am62-phyverso-evcs-k3r5 = " \
	file://0001-u-boot-dts-enable-USB0-VBUS.patch \
"


COMPATIBLE_MACHINE:am62-phyverso-evcs = "am62-phyverso-evcs"
COMPATIBLE_MACHINE:am62-phyverso-evcs-k3r5 = "am62-phyverso-evcs-k3r5"

