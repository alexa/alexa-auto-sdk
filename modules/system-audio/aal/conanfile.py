import json
import os

from conans import ConanFile, CMake, tools
from six import StringIO


class AalConan(ConanFile):
    name = "aac-system-audio-lib"
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
                gst_deps = ["gstreamer", "gst-plugins-base", "gst-plugins-good", "gst-plugins-bad"]
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

    _build_info = "aal-build-info.txt"

    def build(self):
        with tools.environment_append({"PKG_CONFIG_PATH": [os.getcwd()]}):
            cmake = self._configure_cmake()
            cmake.build()

            # Check gstreamer packages provided by the system or toolchain
            if self.is_gstreamer_supported and self.options.gstreamer == "system":
                pkg_config = tools.which("pkg-config")
                if not pkg_config:
                    raise Exception("pkg-config is not available")

                gstreamer_packages = ["gstreamer", "gstreamer-app", "gstreamer-audio", "gstreamer-plugins-base", "gstreamer-plugins-bad"]
                gstreamer_packages = [f"{p}-1.0" for p in gstreamer_packages]

                gstreamer_pkg_config = StringIO()
                self.run(f"{pkg_config} --libs --cflags {' '.join(gstreamer_packages)}", output=gstreamer_pkg_config)

                with open(self._build_info, "w") as file:
                    json.dump({
                        "gstreamer": gstreamer_pkg_config.getvalue(),
                        "sysroot": os.getenv("SDKTARGETSYSROOT")
                    }, file)

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

        self.copy(self._build_info)

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)

        if self.is_omxal_qsa_supported:
            self.cpp_info.system_libs = ["OpenMAXAL", "asound"]

        if self.settings.os == "Macos":
            # This is necessary for bundling osxaudio plugins statically since the framework
            # dependencies are hidden behind meson.
            self.cpp_info.frameworks.extend(["CoreAudio", "AudioToolbox"])

        # Provide gstreamer libraries as dependencies
        if os.path.exists(self._build_info):
            with open(self._build_info, "r") as file:
                build_info = json.load(file)

                sysroot = build_info["sysroot"] or ""
                flags = build_info["gstreamer"].split()
                self.cpp_info.includedirs.extend([sysroot + flag[2:] for flag in flags if flag.startswith("-I")])
                self.cpp_info.libdirs.extend([sysroot + flag[2:] for flag in flags if flag.startswith("-L")])
                self.cpp_info.libs.extend([flag[2:] for flag in flags if flag.startswith("-l")])
