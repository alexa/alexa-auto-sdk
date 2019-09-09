SUMMARY = "mbed TLS (formerly known as PolarSSL) makes it trivially easy for developers to include cryptographic and SSL/TLS capabilities in their (embedded) products, facilitating this functionality with a minimal coding footprint."
HOMEPAGE = "https://tls.mbed.org/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=302d50a6369f5f22efdb674db908167a"

SRC_URI = "https://tls.mbed.org/download/${P}-apache.tgz"
SRC_URI[md5sum] = "37cdec398ae9ebdd4640df74af893c95"
SRC_URI[sha256sum] = "a6834fcd7b7e64b83dfaaa6ee695198cb5019a929b2806cb0162e049f98206a4"

inherit cmake

EXTRA_OECMAKE += "-DCMAKE_BUILD_TYPE=RELEASE \
                  -DENABLE_TESTING=false \
                  -DUSE_SHARED_MBEDTLS_LIBRARY=ON"