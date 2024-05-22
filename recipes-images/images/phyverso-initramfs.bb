SUMMARY = "Phytec phyVERSO initramfs image"
DESCRIPTION = "A small image capable of allowing a device to boot and \
update the eMMC. The kernel includes \
the Minimal RAM-based Initial Root Filesystem (initramfs), which finds the \
first 'init' program more efficiently. \
On phyVERSO, this can be used for booting via ethernet to flash new \
software to the eMMC."

IMAGE_INSTALL = " \
    initramfs-framework-base \
    initramfs-module-udev \
    initramfs-module-network \
    initramfs-module-provisioninginit \
    busybox \
    packagegroup-hwtools-init \
    util-linux \
    coreutils \
    ${MACHINE_EXTRA_RDEPENDS} \
    bash \
    partup \
    openssh \
    sshd-init \
"

# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = " empty-root-password"
#IMAGE_FEATURES = ""
IMAGE_LINGUAS = ""

export IMAGE_BASENAME = "phyverso-initramfs-image"

LICENSE = "MIT"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
inherit core-image image_types

IMAGE_ROOTFS_SIZE = "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "0"

PACKAGE_EXCLUDE = "kernel-image-*"

BAD_RECOMMENDATIONS += " \
    initramfs-module-rootfs \
    initramfs-module-finish \
    busybox-syslog \
"
