SUMMARY = "Add sshd init script for initramfs"
DESCRIPTION = "This recipe adds the initialization for sshd in initramfs"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"



SRC_URI = "file://90-sshdinit"

RDEPENDS_${PN} += "bash"
S = "${WORKDIR}" 

do_install () {
    install -d -m 750 ${D}/init.d
    install -p -m 750 90-sshdinit ${D}/init.d/
}

FILES:${PN} += "/init.d/90-sshdinit \
"


