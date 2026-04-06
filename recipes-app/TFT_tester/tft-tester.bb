DESCRIPTION = "TFT_tester"
LICENSE = "CLOSED"

SRC_URI = "file://TFT_tester.c \
           file://TFT_tester.h\
           file://head.bmp \
           file://arial.ttf \
            "

S = "${WORKDIR}"

DEPENDS += "freetype"

do_configure() {
    :
}

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} \
        -I${STAGING_INCDIR}/freetype2 \
        -I${STAGING_INCDIR} \
        -o TFT_tester ${S}/TFT_tester.c \
        -lfreetype
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 TFT_tester ${D}${bindir}
    
    install -d ${D}${datadir}/tft-tester
    install -m 0644 ${WORKDIR}/head.bmp ${D}${datadir}/tft-tester/head.bmp
    
    install -d ${D}${datadir}/fonts
    install -m 0644 ${WORKDIR}/arial.ttf ${D}${datadir}/fonts/arial.ttf
}
FILES:${PN} += "${datadir}/fonts"