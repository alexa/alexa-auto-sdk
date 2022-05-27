from conans import ConanFile, CMake, tools
import os
import re

# Basic connan package file for creating an aac sdk module package,
# which extends the BaseSdkModule class in aac-sdk-tools.

class AutoSdkModulePkg(ConanFile):

    # the name of the module which must be overriden for this package
    module_name = "core"
    # the package name must be defined in sub-class for some conan commands to work
    name = f"aac-module-{module_name}"

    # python requires statement to include aac module specific
    # pacakge tools and resources.
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkModule"

    # conan dependencies needed for this package
    requires = [
        "rapidjson/1.1.0",
        "nlohmann_json/3.8.0",
        "sqlite3/3.37.2#8e4989a1ee5d3237a25a911fbcb19097",
    ]

    module_options = {
        "default_logger_enabled": [True,False],
        "default_logger_level": ["Default","Verbose","Info","Metric","Warn","Error","Critical"],
        "default_logger_sink": ["Default","Console","Syslog"],
        "with_colored_logs": [True, False],
        "with_thread_moniker_logs": [True, False],
    }
    module_default_options = {
        "default_logger_enabled": True,
        "default_logger_level": "Default",
        "default_logger_sink": "Default",
        "with_colored_logs": True,
        "with_thread_moniker_logs": True,
        "sqlite3:build_executable": False,
    }

    def configure(self):
        super(AutoSdkModulePkg,self).configure()

    def get_cmake_definitions(self):
        cmake_defs = super(AutoSdkModulePkg,self).get_cmake_definitions()

        cmake_defs["AAC_VERSION"] = self.version if re.match("^\d+\.\d+(\.\d+)?(-.+)?$", self.version) else "4.1.0-" + self.version
        cmake_defs["AAC_DEFAULT_LOGGER_ENABLED"] = self._utils.bool_value(self.options.default_logger_enabled,"On","Off")
        cmake_defs["AAC_DEFAULT_LOGGER_LEVEL"] = self.options.default_logger_level
        cmake_defs["AAC_DEFAULT_LOGGER_SINK"] = self.options.default_logger_sink
        cmake_defs["AAC_CORE_ENGINE_DIRECTORY"] = os.path.join( self._source_path, "engine" )
        if self.options.with_colored_logs:
            cmake_defs["AAC_EMIT_COLOR_LOGS"] = "On"
        if self.options.with_thread_moniker_logs:
            cmake_defs["AAC_EMIT_THREAD_MONIKER_LOGS"] = "On"

        return cmake_defs

