kconfig_del() {
    bbnote "Removing all lines $1 prefixes in configs"
    for config in ${UBOOT_MACHINE}; do
        sed -e "/$1/d" -i "${S}/configs/${config}"
    done
}

kconfig_set() {
    bbnote "Setting $1 in configs to $2"
    if [ "$2" = "n" ]; then
        line="# CONFIG_$1 is not set"
    else
        line="CONFIG_$1=$2"
    fi

    for config in ${UBOOT_MACHINE}; do
    if [ "$(grep -E CONFIG_$1[=\ ] ${S}/configs/${config})" ]; then
        sed -i "/CONFIG_$1[= ]/c\\$line" "${S}/configs/${config}"
    else
        echo "$line" >> "${S}/configs/${config}"
    fi
    done
}

patch_ubootconfig () {
        kconfig_set FIT y
        kconfig_set FIT_ENABLE_SHA256_SUPPORT n
        kconfig_set FIT_BEST_MATCH y
        kconfig_set FIT_VERBOSE n
        kconfig_set LEGACY_IMAGE_FORMAT y
	kconfig_set SUPPORT_RAW_INITRD y

        sed -i "s/\(setenv dofitboot \)[01]/\11/" ${S}/include/configs/phycore_*.h
        sed -e 's/image=Image\\0/image=fitImage\\0/g' -i ${S}/include/configs/phycore_*.h
        sed 's/mmc_load_overlay=/no_extensions=1\\0no_overlays=0\\0mmc_load_overlay=/   g' -i ${S}/include/configs/phycore_*.h
}

do_patch:append () {
    bb.build.exec_func("patch_ubootconfig", d)
}

