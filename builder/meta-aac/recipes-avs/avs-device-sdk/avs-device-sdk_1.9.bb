require avs-device-sdk.inc

SRC_URI[md5sum] = "b28cf44c13d3c88918bc67386b856b9c"
SRC_URI[sha256sum] = "472f774af977769e64338c7dd4dff42cf08f513abbda9c9e2606d8c78df5d853"

# Alexa Auto modifications
# * SpeechEncoder + OPUS support
SRC_URI += "file://0001-SpeechEncoder-Introduce-SpeechEncoder-with-OPUS-supp.patch \
            file://0002-AIP-Only-16kHz-is-supported-for-OPUS-now.patch \
            file://0003-AIP-Initial-SpeechEncoder-support.patch \
            file://0004-Alerts-stop-delay-fix.patch \
            file://0005-Add-config-support-to-TemplateRuntime_Agent.patch \
            file://0006-Modify_TemplateRuntime_Timeout_Defaults.patch \
            file://0007-Alerts-Detailed-Info-Interface.patch \
            file://0008-AVS-Changes-for-HTTP-Get-and-Delete-Support.patch \
            file://0009-ExternalMediaPlayer-1.1-for-AAC-MACC.patch \
            file://0010-Override-Publish-Capabilities-Check.patch"

# Enable OPUS by default
PACKAGECONFIG += "opus"
PACKAGECONFIG[opus] = "-DOPUS=ON,,libopus"