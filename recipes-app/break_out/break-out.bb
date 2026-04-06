DESCRIPTION = "break_out"
LICENSE = "CLOSED"

SRC_URI = "file://break_out.c \
           file://TFT.c \
           file://TFT.h \
           file://utils.c \
           file://utils.h \
           file://head.bmp \
           file://2.bmp \
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
        -o break_out ${S}/break_out.c ${S}/utils.c ${S}/TFT.c\
        -lfreetype
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 break_out ${D}${bindir}
    
    install -d ${D}${datadir}/break-out
    install -m 0644 ${WORKDIR}/head.bmp ${D}${datadir}/break-out/head.bmp
    install -m 0644 ${WORKDIR}/2.bmp ${D}${datadir}/break-out/2.bmp
    
    install -d ${D}${datadir}/fonts
    install -m 0644 ${WORKDIR}/arial.ttf ${D}${datadir}/fonts/arial.ttf
}
FILES:${PN} += "${datadir}/fonts"