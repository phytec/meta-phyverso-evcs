FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append:phyverso-evcs = "\
    file://0001-board-phytec-phytec_am62x-Enable-cold-reboot-in-PMI.patch \
    file://0001-u-boot-dts-enable-USB0-VBUS.patch \
"

COMPATIBLE_MACHINE:append = "|phyverso-evcs"
