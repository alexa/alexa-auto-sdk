import os
import glob
import logging
from conans import ConanFile, tools, Meson

_meson_feature = ["disabled", "enabled", "auto"]
_features = []


class GStPluginsBadConan(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"
    name = "gst-plugins-bad"
    version = "1.18.4"
    description = "GStreamer is a development framework for creating applications like media players, video editors, " \
                  "streaming media broadcasters and so on"
    topics = ("conan", "gstreamer", "multimedia", "video", "audio", "broadcasting", "framework", "media")
    homepage = "https://gstreamer.freedesktop.org/"
    license = "GPL-2.0-only"
    exports = ["LICENSE.md"]
    settings = "os", "arch", "compiler", "build_type"
    options = dict({"shared": [True, False], "fPIC": [ True, False]}, **{f: _meson_feature for f in _features})
    default_options = dict({"shared": False, "fPIC": True}, **{f: "auto" for f in _features})
    exports_sources = ["patches/*.patch"]
    requires = ["openssl/1.1.1i#b843148d42054bebfdca6e9561a35d77", "libxml2/2.9.10#7293e7b3f9703b324258194bb749ce85"]
    build_requires = ["meson/0.56.2", "bison/3.7.1", "flex/2.6.4", "pkgconf/1.7.3"]
    generators = "pkg_config"

    _source_subfolder = "source_subfolder"
    _build_subfolder = "build_subfolder"

    def configure(self):
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd
        self.options['gstreamer'].shared = self.options.shared

    def config_options(self):
        if self.settings.os == 'Windows':
            del self.options.fPIC

    def requirements(self):
        self.requires(f"gst-plugins-base/{self.version}@{self.user}/{self.channel}")
        self.requires(f"faad2/2.10.0@{self.user}/{self.channel}")

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
        for x in ["tools", "examples", "benchmarks", "tests"]:
            defs[x] = "disabled"
        for x in _features:
            defs[x] = self.options.get_safe(x)

        # Disable options that cause build issues on non-Linux systems
        if self.settings.os != 'Linux' or (hasattr(self, 'settings_build') and tools.cross_building(self, skip_x64_x86=True)):
            meson.options["introspection"] = "disabled"
            meson.options["orc"] = "disabled"

        # Disable unused plugins
        for plugin in ["closedcaption", "rsvg", "ttml", "openexr"]:
            meson.options[plugin] = "disabled"

        # Enable hls explicitly for HTTP streaming
        meson.options["hls"] = "enabled"
        meson.options["hls-crypto"] = "openssl"

        # Somehow Meson ignore PKG_CONFIG_PATH env. Force setting it with option.
        meson.options["pkg_config_path"] = os.getenv('PKG_CONFIG_PATH')

        meson.configure(build_folder=self._build_subfolder, source_folder=self._source_subfolder, defs=defs)
        return meson

    def build(self):
        self._apply_patches()
        with tools.environment_append({"PKG_CONFIG_PATH": [os.getcwd()]}):
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
            self.cpp_info.libdirs.append(gst_plugin_path)
            self.cpp_info.libs = tools.collect_libs(self)
