import os
import glob
import logging
from conans import ConanFile, tools, Meson
from conans.errors import ConanInvalidConfiguration


class GStPluginsBaseConan(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"
    name = "gst-plugins-base"
    version = "1.18.4"
    description = "GStreamer is a development framework for creating applications like media players, video editors, " \
                  "streaming media broadcasters and so on"
    topics = ("conan", "gstreamer", "multimedia", "video", "audio", "broadcasting", "framework", "media")
    url = "https://github.com/bincrafters/conan-gst-plugins-base"
    homepage = "https://gstreamer.freedesktop.org/"
    license = "GPL-2.0-only"
    settings = "os", "arch", "compiler", "build_type"
    build_requires = ["meson/0.56.2", "pkgconf/1.7.3"]

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_libalsa": [True, False],
        "with_gl": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_libalsa": True,
        "with_gl": True
    }
    _source_subfolder = "source_subfolder"
    _build_subfolder = "build_subfolder"
    exports_sources = ["patches/*.patch"]

    generators = "pkg_config"

    def configure(self):
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd
        self.options['gstreamer'].shared = self.options.shared
        if tools.Version(self.version) >= "1.18.2" and\
           self.settings.compiler == "gcc" and\
           tools.Version(self.settings.compiler.version) < "5":
            raise ConanInvalidConfiguration(
                f"gst-plugins-base {self.version} does not support gcc older than 5"
            )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
        if self.settings.os != "Linux":
            del self.options.with_libalsa
        if self.settings.os != "Macos":
            self.options.with_gl = False

    def requirements(self):
        self.requires("opus/1.3.1#5132ab8db7b69dd8e26466e0b3b017dd")
        self.requires("ogg/1.3.4")
        self.requires(f"gstreamer/{self.version}@{self.user}/{self.channel}")
        if self.settings.os == "Linux":
            if self.options.with_libalsa:
                self.requires("libalsa/1.1.9")

    def build_requirements(self):
        if self.settings.os == 'Windows':
            self.build_requires("winflexbison/2.5.22")
        else:
            self.build_requires("bison/3.7.1")
            self.build_requires("flex/2.6.4")

    def source(self):
        tools.get(**self.conan_data["sources"][self.version])
        os.rename(f"{self.name}-{self.version}", self._source_subfolder)

    def _apply_patches(self):
        for filename in sorted(glob.glob("patches/*.patch")):
            logging.info(f"applying patch: {filename}")
            tools.patch(base_path=self._source_subfolder, patch_file=filename)

    def _configure_meson(self):
        defs = dict()

        def add_flag(name, value):
            if name in defs:
                defs[name] += " " + value
            else:
                defs[name] = value

        def add_compiler_flag(value):
            add_flag("c_args", value)
            add_flag("cpp_args", value)

        def add_linker_flag(value):
            add_flag("c_link_args", value)
            add_flag("cpp_link_args", value)

        meson = Meson(self)
        if self.settings.compiler == "Visual Studio":
            add_linker_flag("-lws2_32")
            add_compiler_flag(f"-{self.settings.compiler.runtime}")
            if int(str(self.settings.compiler.version)) < 14:
                add_compiler_flag("-Dsnprintf=_snprintf")
        if self.settings.get_safe("compiler.runtime"):
            defs["b_vscrt"] = str(self.settings.compiler.runtime).lower()
        defs["tools"] = "disabled"
        defs["examples"] = "disabled"
        defs["benchmarks"] = "disabled"
        defs["tests"] = "disabled"
        defs["wrap_mode"] = "nofallback"
        defs["gl"] = "enabled" if self.options.with_gl else "disabled"
        if self.settings.os != "Linux":
            meson.options["introspection"] = "disabled"
            meson.options["orc"] = "disabled"
        meson.configure(build_folder=self._build_subfolder, source_folder=self._source_subfolder, defs=defs)
        return meson

    def build(self):
        self._apply_patches()
        meson = self._configure_meson()
        meson.build()

    def package(self):
        meson = self._configure_meson()
        meson.install()

    def package_info(self):

        gst_plugin_path = os.path.join(self.package_folder, "lib", "gstreamer-1.0")
        if self.options.shared:
            logging.info(f"Appending GST_PLUGIN_PATH env var: {gst_plugin_path}")
            self.env_info.GST_PLUGIN_PATH.append(gst_plugin_path)
        else:
            comp = self.cpp_info.components["gst-plugins-base-gstreamer-1.0"]
            comp.libdirs.append(gst_plugin_path)
            comp.libs.extend(tools.collect_libs(self, os.path.join("lib", "gstreamer-1.0")))
            comp.includedirs = [os.path.join("include", "gstreamer-1.0")]
            comp.defines.append("GST_PLUGINS_BASE_STATIC")
            comp.requires = ["opus::opus", "ogg::ogg", "gstreamer::gstreamer"]
            if self.settings.os == "Linux":
                comp.requires.append("libalsa::libalsa")

            for lib in ["allocators", "app", "audio", "fft", "pbutils", "riff", "rtp", "rtsp", "sdp", "tag", "video"]:
                comp = self.cpp_info.components[f"gstreamer-{lib}-1.0"]
                comp.libs = [f"gst{lib}-1.0"]
                comp.includedirs = [os.path.join("include", "gstreamer-1.0")]
                comp.requires = ["gstreamer::gstreamer"]

        self.env_info.PKG_CONFIG_PATH.append(os.path.join(gst_plugin_path, "pkgconfig"))
        self.env_info.PKG_CONFIG_PATH.append(os.path.join(self.package_folder, "lib", "pkgconfig"))
