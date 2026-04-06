SUMMARY = "Simple STM32MP257 Button kernel module"
DESCRIPTION = "Kernel module to read button state on STM32MP257"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

inherit module

SRC_URI = "file://button-detect.c file://Makefile file://button-detect.h"

S = "${WORKDIR}"

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0644 ${S}/button-detect.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/
}

INSANE_SKIP:${PN} += "buildpaths"
INSANE_SKIP:${PN}-dbg += "buildpaths"