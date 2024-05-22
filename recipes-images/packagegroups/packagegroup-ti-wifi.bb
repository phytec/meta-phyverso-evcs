DESCRIPTION = "Phytec wifi software"
LICENSE = "MIT"

PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

# wpa_supplicant and wireless-tools are already install in packagegroup-base-wifi

RDEPENDS:${PN} = " \
    wpa-supplicant \
    iw \
    hostapd \
    linux-firmware-iwlwifi \
    ti-cc33xx-firmware \
"
