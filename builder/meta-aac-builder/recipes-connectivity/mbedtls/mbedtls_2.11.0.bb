SUMMARY = "mbed TLS (formerly known as PolarSSL) makes it trivially easy for developers to include cryptographic and SSL/TLS capabilities in their (embedded) products, facilitating this functionality with a minimal coding footprint."
HOMEPAGE = "https://tls.mbed.org/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=302d50a6369f5f22efdb674db908167a"

SRC_URI = "https://tls.mbed.org/download/${P}-apache.tgz"
SRC_URI[md5sum] = "3b133b9b4cf0ccf3ac1c91436771bfa5"
SRC_URI[sha256sum] = "2a87061ad770e6d019b3b9cd27ea42a58bd0affccc4c6bfe4f5f0eee9ebf8aa8"

inherit cmake

EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=RELEASE \
                  -DENABLE_TESTING=false"