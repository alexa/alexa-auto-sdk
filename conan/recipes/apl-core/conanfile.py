import os
from conans import ConanFile, CMake, tools

class AplCoreConan(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"

    name = "apl-core"
    version = "1.6.2"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "cmake_find_package"
    exports_sources = "CMakeLists.txt", "patches/*"
    requires = []

    options = {
        "with_coverage": [True, False],
        "with_verbose": [True, False],
        "build_testing": [True,False]
    }

    default_options = {
        "with_coverage": False,
        "with_verbose": False,
        "build_testing": False
    }

    _source_subfolder = "source_subfolder"

    def source(self):
        tools.get(f"https://github.com/alexa/apl-core-library/archive/refs/tags/v{self.version}.tar.gz" )
        os.rename(f"apl-core-library-{self.version}", self._source_subfolder)

    def configure(self):
        self.settings.compiler.cppstd = "11"
        if self.settings.os == "Android":
            # Android does not support libraries with version extension (libssl.so.1.1)
            # so used static version off openssl for Android
            self.options["openssl"].shared = False 

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TESTS"] = "ON" if self.options.build_testing else "OFF"
        cmake.definitions["COVERAGE"] = "ON" if self.options.with_coverage else "OFF"
        cmake.definitions["VERBOSE"] = "ON" if self.options.with_verbose else "OFF"
        if self.settings.os == "Android":
            cmake.definitions["ANDROID"] = "OFF" # this is not a mistake!
        cmake.configure(source_folder=self._source_subfolder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.names["pkg_config"] = "APLCoreEngine"
        self.cpp_info.libs = tools.collect_libs(self)