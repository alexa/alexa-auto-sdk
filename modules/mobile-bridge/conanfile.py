from conans import ConanFile, tools


class AutoSdkModulePkg(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkModule"

    module_name = "mobile-bridge"
    module_requires = ["alexa"]
    name = "aac-module-%s" % (module_name)

    supported_os = ["Linux", "Android", "Macos"]

    requires = [
        # direct dependencies
        "libtins/4.4#9f04bafcd285a7f41d13f079fa3c4a7f",
        "libevent/2.1.12#7a0944d4f2d6a9fd90a63f0747a14783",
        # indirect dependencies
        "libpcap/1.10.1#ac3088dc2decd37cc21cc7cc33d5663c",
    ]

    def configure(self):
        super(AutoSdkModulePkg, self).configure()
        self.options["libtins"].with_ack_tracker = False
        self.options["libtins"].with_wpa2 = False
        self.options["libtins"].with_dot11 = False
        self.options["libevent"].with_openssl = False
        self.options["libevent"].disable_threads = False
