FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://fbtft.cfg;subdir=fragments"
KERNEL_CONFIG_FRAGMENTS:append = " ${WORKDIR}/fragments/fbtft.cfg"

SRC_URI += "file://0001-enable-ili9488-on-spi6.patch \
            file://0002-enable-button-detect.patch \
            "