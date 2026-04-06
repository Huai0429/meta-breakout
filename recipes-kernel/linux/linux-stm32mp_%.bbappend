FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://fbtft.cfg;subdir=fragments"
KERNEL_CONFIG_FRAGMENTS:append = " ${WORKDIR}/fragments/fbtft.cfg"