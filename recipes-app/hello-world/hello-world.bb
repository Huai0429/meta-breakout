DESCRIPTION = "Simple Hello World example from STMicroelectronics"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://hello-world.c file://send_msg.c"

S = "${WORKDIR}"

do_configure() {
    :
}

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o hello-world hello-world.c
    ${CC} ${CFLAGS} ${LDFLAGS} -o send_msg send_msg.c
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 hello-world ${D}${bindir}
    install -m 0755 send_msg ${D}${bindir}
}
