import json
import os

from conans import ConanFile, CMake, tools
from six import StringIO


class AalConan(ConanFile):
    name = "aac-system-audio-lib"
    url = "https://github.com/alexa/alexa-auto-sdk"
    license = "Apache-2.0"
    description = "AAL (Audio Abstraction Layer)"

    settings = "os", "compiler", "build_type", "arch"
    generators = "pkg_config"
    exports_sources = "*"

    options = {
        "gstreamer": ["system", "static", "dynamic"],
    }

    def set_version(self):
        if self.version == None:
            self.version = "dev"

    @property
    def is_gstreamer_supported(self):
        return self.settings.os == "Linux" or self.settings.os == "Macos"

    @property
    def is_omxal_qsa_supported(self):
        return self.settings.os == "Neutrino"

    def configure(self):
        if not self.options.gstreamer:
            if self.settings.os == "Linux":
                self.options.gstreamer = "system"
            else:
                self.options.gstreamer = "static"

    def requirements(self):
        gstreamer_version = "1.18.4"
        if self.is_gstreamer_supported:
            if self.options.gstreamer in ("static", "dynamic"):
                gst_deps = [
                    "gstreamer",
                    "gst-plugins-base",
                    "gst-plugins-good",
                    "gst-plugins-bad",
                ]
                for dep in gst_deps:
                    self.requires(f"{dep}/{gstreamer_version}@aac-sdk/{self.version}")

                if self.options.gstreamer == "static":
                    # gstreamer libraries should compile as static library
                    for dep in gst_deps:
                        self.options[dep].shared = False
                else:
                    # gstreamer libraries should compile as shared library
                    for dep in gst_deps:
                        self.options[dep].shared = True

    def _configure_cmake(self):
        cmake = CMake(self)

        defs = {}
        if self.is_gstreamer_supported:
            defs["ENABLE_GSTREAMER"] = "ON"
        elif self.is_omxal_qsa_supported:
            defs["ENABLE_OMXAL"] = "ON"
            defs["ENABLE_QSA"] = "ON"
        else:
            raise Exception("could not find supported audio system!")

        if self.options.gstreamer == "static":
            defs["BUNDLE_GST_PLUGINS"] = "ON"

        cmake.configure(defs=defs)
        return cmake

    def build(self):
        with tools.environment_append({"PKG_CONFIG_PATH": [os.getcwd()]}):
            cmake = self._configure_cmake()
            cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)