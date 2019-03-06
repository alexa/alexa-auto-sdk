SUMMARY = "Alexa Auto SDK C++ Sample App"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://SampleApp/src/main.cpp;beginline=4;endline=13;md5=527e9938f0eaf4dbc8d3b17563870ae7"

def get_depends(d):
    if d.getVar('AMAZONLITE', True):
        return "\
          aac-module-core \
          aac-module-alexa \
          aac-module-navigation \
          aac-module-phone-control \
          aac-module-cbl \
          aac-sample-audio \
          aac-module-amazonlite \
        "
    else:
        return "\
          aac-module-core \
          aac-module-alexa \
          aac-module-navigation \
          aac-module-phone-control \
          aac-module-cbl \
          aac-sample-audio \
        "

AMAZONLITE ?= ""

DEPENDS = "${@get_depends(d)}"

EXTRA_OECMAKE += "-DAMAZONLITE=${AMAZONLITE}"

inherit aac-module
