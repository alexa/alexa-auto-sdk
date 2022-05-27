from conans import ConanFile, Meson, tools
import os

class LibsoupConan(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"

    name = "libsoup"
    version = "2.72"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "pkg_config"
    build_requires = ["meson/0.56.2","pkgconf/1.7.3"]

    _meson = None
    _source_subfolder = "source_subfolder"
    _build_subfolder = "build_subfolder"

    def requirements(self):
        if hasattr(self, 'settings_build') and tools.cross_building(self, skip_x64_x86=True):
            pass # use libpsl, libxml2, and sqlite3 provided by the toolchain
        else:
            self.requires("libpsl/0.21.1")
            self.requires("libxml2/2.9.10#7293e7b3f9703b324258194bb749ce85")
            self.requires("sqlite3/3.37.2#8e4989a1ee5d3237a25a911fbcb19097")

        self.requires(f"glib/2.67.1@{self.user}/{self.channel}")
        self.requires(f"glib-networking/2.68.2@{self.user}/{self.channel}")

    def source(self):
        source_url = f"https://download.gnome.org/sources/{self.name}/{self.version}/{self.name}-{self.version}.0.tar.xz"
        tools.get(source_url)
        os.rename(f"{self.name}-{self.version}.0", self._source_subfolder)

    def _configure_meson(self):
        if self._meson:
            return self._meson
        meson = Meson(self)

        # Disable it since the check won't work without a load TLS backend
        meson.options["tls_check"] = "false"
        # Disable it for compilation error related to introspection
        meson.options["introspection"] = "disabled"
        # Disable it to avoid introducing brotli dependency
        meson.options["brotli"] = "disabled"

        meson.options["pkg_config_path"] = f"{os.getcwd()}:{os.getenv('PKG_CONFIG_PATH')}"
        meson.configure(build_folder=self._build_subfolder,
                        source_folder=self._source_subfolder,
                        args=['--wrap-mode=nofallback'])
        self._meson = meson
        return self._meson

    def build(self):
        meson = self._configure_meson()
        meson.build()

    def package(self):
        meson = self._configure_meson()
        meson.install()

    def package_info(self):
        pkg_config_name = f"{self.name}-2.4"
        self.cpp_info.name = pkg_config_name
        self.cpp_info.includedirs = [f"include/{pkg_config_name}"]
        self.cpp_info.libs = tools.collect_libs(self)

