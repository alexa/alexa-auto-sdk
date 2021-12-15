from conans import ConanFile
import logging, json, os

class AutoSdkModulePkg(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkModule"
    module_name = "system-audio"
    module_requires = ["core", "alexa"]
    # the package name must be defined in sub-class for some conan commands to work
    name = f"aac-module-{module_name}"

    supported_os = ["Linux", "Macos", "Neutrino"]

    def requirements(self):
        super(AutoSdkModulePkg, self).requirements()
        if self.settings.os in self.supported_os:
            self.requires(f"aac-system-audio-lib/{self.version}")

    def build(self):
        if self.settings.os in self.supported_os:
            super(AutoSdkModulePkg, self).build()
        else:
            logging.warning(
                f"Operating system not supported (settings.os={self.settings.os}). Module is being skipped.")

    def package(self):
        super(AutoSdkModulePkg,self).package()
        self.copy("*", src=f"configs/{str(self.settings.os).lower()}", dst="config", keep_path=False)

    def package_info(self):
        super(AutoSdkModulePkg,self).package_info()
        self.user_info.aac_export_shared = json.dumps([{
            "pattern": "*.json",
            "src": os.path.abspath(os.path.join(self.package_folder,"config")),
            "dst": "system-audio/config"
        }])
