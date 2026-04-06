SUMMARY = "Simple STM32MP257 Button kernel module"
DESCRIPTION = "Kernel module to read button state on STM32MP257"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = "file://${THISDIR}/COPYING;md5=08782068476f4e97c8b67ca16da0dccd"

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
