from conans import ConanFile

class AutoSdkModulePkg(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkModule"
    module_name = "alexa"
    module_requires = ["core"]
    # the package name must be defined in sub-class for some conan commands to work
    name = f"aac-module-{module_name}"

    def requirements(self):
        super(AutoSdkModulePkg,self).requirements()
        self.requires(f"avs-device-sdk/1.25.0@aac-sdk/{self.version}")
        self.requires(f"smart-screen-sdk/2.8.0@aac-sdk/{self.version}")
        # enable the build test options if we are building the odule with unit tests
        self.options["avs-device-sdk"].build_testing = self.options.get_safe("with_unit_tests", default=False)
