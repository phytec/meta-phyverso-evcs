SUMMARY = "Phytec's FIT-Image base"
LICENSE = "MIT"

inherit fitimage

FITIMAGE_SLOT_kernel ?=  "${PREFERRED_PROVIDER_virtual/kernel}"
FITIMAGE_SLOT_kernel[type] ?= "kernel"
FITIMAGE_SLOT_kernel[file] ?= "${KERNEL_IMAGETYPE}"

FITIMAGE_SLOT_fdt ?= "${PREFERRED_PROVIDER_virtual/kernel}"
FITIMAGE_SLOT_fdt[type] ?= "fdt"
FITIMAGE_SLOT_fdt[file] ?= "${@' '.join(os.path.basename(dtb) for dtb in d.getVar('KERNEL_DEVICETREE').split() if dtb.endswith('.dtb'))}"

FITIMAGE_SLOT_fdto ?= "${PREFERRED_PROVIDER_virtual/kernel}"
FITIMAGE_SLOT_fdto[type] ?= "fdto"
FITIMAGE_SLOT_fdto[file] ?= "${@' '.join(os.path.basename(dtb) for dtb in d.getVar('KERNEL_DEVICETREE').split() if dtb.endswith('.dtbo'))}"


