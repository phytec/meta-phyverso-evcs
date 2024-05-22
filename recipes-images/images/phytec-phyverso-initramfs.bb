SUMMARY = "Phytec phyVERSO initramfs image"
DESCRIPTION = "A small image capable of allowing a device to boot and \
update the eMMC. The kernel includes \
the Minimal RAM-based Initial Root Filesystem (initramfs), which finds the \
first 'init' program more efficiently. \
On phyVERSO, this can be used for booting via ethernet to flash new \
software to the eMMC."

PACKAGE_INSTALL = " \
    initramfs-module-rootfs \
    initramfs-module-udev \
    initramfs-module-network \
    busybox \
    packagegroup-hwtools-init \
    util-linux \
    coreutils \
    ${MACHINE_EXTRA_RDEPENDS} \
    pv \
    partup \
    bash \
    packagegroup-core-ssh-openssh \
"

# Do not pollute the initrd image with rootfs features
#IMAGE_FEATURES = "empty-root-password"

export IMAGE_BASENAME = "phytec-phyVERSO-initramfs-image"
IMAGE_LINGUAS = ""

LICENSE = "MIT"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
inherit core-image

IMAGE_ROOTFS_SIZE = "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "0"

PACKAGE_EXCLUDE = "kernel-image-*"

BAD_RECOMMENDATIONS += "busybox-syslog"

#busybox
#initramfs-module-network
#base-passwd
#initramfs-framework-base
#${ROOTFS_BOOTSTRAP_INSTALL}
#${VIRTUAL-RUNTIME_login_manager}

#initramfs-module-udev
#initramfs-module-network
#busybox
#packagegroup-hwtools-init
#base-files
#util-linux
#coreutils
#packagegroup-hwtools-diagnostic
#partup
