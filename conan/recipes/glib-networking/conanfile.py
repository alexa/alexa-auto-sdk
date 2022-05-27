from conans import ConanFile, CMake, tools, Meson
import os


class GlibnetworkingConan(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"
    name = "glib-networking"
    version = "2.68.2"
    description = "The GLib Networking package contains Network related gio modules for GLib."
    homepage = "https://gitlab.gnome.org/GNOME/glib-networking"
    license = "LGPLv2Plus"
    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False]}
    default_options = {
        "shared": False
    }
    generators = "pkg_config"
    requires = ["openssl/1.1.1i#b843148d42054bebfdca6e9561a35d77"]
    build_requires = ["meson/0.56.2","pkgconf/1.7.3"]

    _meson = None
    _source_subfolder = "source_subfolder"
    _build_subfolder = "build_subfolder"

    def requirements(self):
        self.requires(f"glib/2.67.1@{self.user}/{self.channel}")

    def source(self):
        maj_ver = '.'.join(self.version.split('.')[0:2])
        tarball_name = '{name}-{version}.tar'.format(
            name=self.name, version=self.version)
        archive_name = f"{tarball_name}.xz"
        source_url = f"https://download.gnome.org/sources/glib-networking/{maj_ver}/{archive_name}"
        tools.get(source_url)
        os.rename(f"{self.name}-{self.version}", self._source_subfolder)

    def _configure_meson(self):
        if self._meson:
            return self._meson
        meson = Meson(self)

        # Disable unused modules
        meson.options["libproxy"] = "disabled"
        meson.options["gnome_proxy"] = "disabled"
        meson.options["gnutls"] = "disabled"
        # Use OpenSSL backend
        meson.options["openssl"] = "enabled"
        meson.options["static_modules"] = "false" if self.options["shared"] else "true"

        meson.options["pkg_config_path"] = f"{os.getcwd()}:{os.getenv('PKG_CONFIG_PATH')}"
        meson.configure(build_folder=self._build_subfolder,
                        source_folder=self._source_subfolder,
                        args=[])
        self._meson = meson
        return self._meson

    def build(self):
        meson = self._configure_meson()
        meson.build()

    def package(self):
        meson = self._configure_meson()
        meson.install()
        # Remove all shared libraries to keep only static ones
        tools.remove_files_by_mask(self.package_folder, "*.so")

    def package_info(self):
        self.cpp_info.libdirs.append(os.path.join(
            self.package_folder, "lib", "gio", "modules"))
        self.cpp_info.libs = tools.collect_libs(self)
