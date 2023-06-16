from conans import ConanFile


class AutoSdkModulePkg(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkModule"
    module_name = "alexa"
    module_requires = ["core"]
    # the package name must be defined in sub-class for some conan commands to work
    name = f"aac-module-{module_name}"

    module_options = {
         "with_captions": [True, False]
    }

    module_default_options = {
        "with_captions": True
    }

    def requirements(self):
        super(AutoSdkModulePkg,self).requirements()
        self.requires(f"avs-device-sdk/1.26.0@aac-sdk/{self.version}")
        self.requires(f"smart-screen-sdk/2.9.2@aac-sdk/{self.version}")
        # enable the build test options if we are building the module with unit tests
        self.options["avs-device-sdk"].build_testing = self.options.get_safe("with_unit_tests", default=False)
        self.options["avs-device-sdk"].with_captions = self.options.get_safe("with_captions", default=True)

    def get_cmake_definitions(self):
        cmake_defs = super(AutoSdkModulePkg, self).get_cmake_definitions()

        if self.options.with_captions:
            cmake_defs["AAC_CAPTIONS"] = "On"

        return cmake_defs

    def package_info(self):
        super(AutoSdkModulePkg, self).package_info()
        if self.options["avs-device-sdk"].with_captions == True:
            # Set a preprocessor definition so that any consumer will know that captions is available
            self.cpp_info.defines.append("AAC_CAPTIONS")
