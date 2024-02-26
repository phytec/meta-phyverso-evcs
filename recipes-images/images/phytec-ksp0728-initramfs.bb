SUMMARY = "PHYTEC provisioning initramfs image for initialization of security features on the device"
LICENSE = "MIT"

inherit core-image image_types

# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = ""
IMAGE_LINGUAS = ""

IMAGE_ROOTFS_SIZE = "8192"
IMAGE_ROOTFS_EXTRA_SPACE = "0"
IMAGE_OVERHEAD_FACTOR = "1.0"

IMAGE_FSTYPES = "cpio.gz"
export IMAGE_BASENAME = "phytec-ksp0728-initramfs"

IMAGE_INSTALL = " \
    initramfs-framework-base \
    initramfs-module-udev \
    initramfs-module-provisioninginit \
    initramfs-module-network \
    initramfs-module-timesync \
    busybox \
    packagegroup-hwtools-init \
    util-linux \
    coreutils \
    ${MACHINE_EXTRA_RDEPENDS} \
    pv \
"

# Remove some packages added via recommendations
BAD_RECOMMENDATIONS += " \
    initramfs-module-rootfs \
    initramfs-module-finish \
    busybox-syslog \
"
