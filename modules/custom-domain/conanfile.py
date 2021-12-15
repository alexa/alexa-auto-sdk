from conans import ConanFile

class AutoSdkModulePkg(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkModule"
    module_name = "custom-domain"
    module_requires = ["core", "alexa"] 
    # the package name must be defined in sub-class for some conan commands to work
    name = f"aac-module-{module_name}"