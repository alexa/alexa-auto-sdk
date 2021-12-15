from conans import ConanFile, tools
from conans.errors import ConanInvalidConfiguration
from six import StringIO
import os
import stat
import shutil


class PokySDKConan(ConanFile):
    name = "poky-sdk"
    version = "2.6.1"
    description = "Poky toolchain for Auto SDK"
    short_paths = True
    no_copy_source = True
    exports_sources = "packages/*"

    settings = {
        "os": ["Linux", "Macos"],
        "arch": ["x86", "x86_64", "armv7hf", "armv8"]
    }

    @staticmethod
    def _chmod_plus_x(filename):
        if os.name == 'posix':
            os.chmod(filename, os.stat(filename).st_mode | 0o111)

    def configure(self):
        if self.settings_target.arch not in ["armv7hf", "armv8"]:
            raise ConanInvalidConfiguration("No binaries available for %s architectures" % (self.settings_target.arch))

    @property
    def _local_installer(self):
        return os.path.join(self.source_folder, "packages", f"installer-{self.settings_target.arch}.sh")

    def _check_toolchain_override(self):
        toolchain_override = os.getenv(f"POKY_TOOLCHAIN_{self.settings_target.arch}")
        if toolchain_override and not os.path.exists(toolchain_override):
            raise ConanInvalidConfiguration(f"{toolchain_override} does not exist")
        return toolchain_override

    def source(self):
        if self._check_toolchain_override():
            return

        local_installer = self._local_installer
        tools.download(**self.conan_data["toolchains"][self.version][str(self.settings_target.arch)], filename=local_installer)
        self._chmod_plus_x(local_installer)

    def build(self):
        pass

    @property
    def _toolchain_path(self):
        return f"toolchain-{self.settings_target.arch}"

    @property
    def _target_triplet(self):
        if self.settings_target.arch == "armv7hf":
            return "cortexa8hf-neon-poky-linux-gnueabi"
        else:
            return "aarch64-poky-linux"

    def package(self):
        if self._check_toolchain_override():
            return

        toolchain_path = os.path.join(self.package_folder, self._toolchain_path)
        oetoolchainconfig_path = os.path.join(toolchain_path, "sysroots/x86_64-pokysdk-linux/usr/share/cmake/OEToolchainConfig.cmake")

        # check the license acceptance
        auto_accept_licenses = os.getenv("BUILDER_ACCEPT_LICENSES", "False").lower() == "true"
        if auto_accept_licenses:
            self.run(f"{self._local_installer} -y -d '{toolchain_path}'")
        else:
            self.run(f"{self._local_installer} -d '{toolchain_path}'")

        self.run(f"sed -i.back '10,+2s/^/#/' '{oetoolchainconfig_path}'")

        target_sysroot = os.path.join(toolchain_path, "sysroots", self._target_triplet)

        sudo_exe = os.path.join(target_sysroot, "usr/bin/sudo")
        os.chmod(sudo_exe, os.stat(sudo_exe).st_mode | stat.S_IRUSR)

        # Remove broken links so that Conan won't complain
        for link in ["etc/mtab", "var/lock"]:
            os.remove(os.path.join(target_sysroot, link))

    def _append_path(self, env, path):
        if os.path.isdir(path):
            env.append(path)

    def package_info(self):
        toolchain_override = self._check_toolchain_override()
        if toolchain_override:
            self.env_info.TOOLCHAIN_ROOT = toolchain_override
        else:
            self.env_info.TOOLCHAIN_ROOT = os.path.join(self.package_folder, self._toolchain_path)
        self.env_info.SYSROOT = os.path.join(self.env_info.TOOLCHAIN_ROOT, "sysroots")
        self.env_info.SDKTARGETSYSROOT = os.path.join(self.env_info.SYSROOT, self._target_triplet)

        self.env_info.CONAN_CMAKE_FIND_ROOT_PATH = self.env_info.SDKTARGETSYSROOT

        self._append_path(self.env_info.PATH, os.path.join(self.env_info.SYSROOT, "x86_64-pokysdk-linux/usr/bin"))
        self._append_path(self.env_info.PATH, os.path.join(self.env_info.SYSROOT, "x86_64-pokysdk-linux/usr/sbin"))
        self._append_path(self.env_info.PATH, os.path.join(self.env_info.SYSROOT, "x86_64-pokysdk-linux/bin"))
        self._append_path(self.env_info.PATH, os.path.join(self.env_info.SYSROOT, "x86_64-pokysdk-linux/sbin"))

        if self.settings_target.arch == "armv7hf":
            self._append_path(self.env_info.PATH, os.path.join(self.env_info.SYSROOT, "x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi"))
            self._append_path(self.env_info.PATH, os.path.join(self.env_info.SYSROOT, "x86_64-pokysdk-linux/usr/bin/arm-poky-linux-musl"))
            self.env_info.CONFIG_SITE = self.env_info.TOOLCHAIN_ROOT + "/site-config-cortexa8hf-neon-poky-linux-gnueabi"
            self.env_info.OECORE_TARGET_ARCH = "arm"
            self.env_info.OECORE_TARGET_OS = "linux-gnueabi"
            self.env_info.CC = "arm-poky-linux-gnueabi-gcc -march=armv7-a -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a8 --sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.CXX = "arm-poky-linux-gnueabi-g++ -march=armv7-a -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a8 --sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.CPP = "arm-poky-linux-gnueabi-gcc -E -march=armv7-a -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a8 --sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.AS = "arm-poky-linux-gnueabi-as"
            self.env_info.LD = "arm-poky-linux-gnueabi-ld --sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.GDB = "arm-poky-linux-gnueabi-gdb"
            self.env_info.STRIP = "arm-poky-linux-gnueabi-strip"
            self.env_info.RANLIB = "arm-poky-linux-gnueabi-ranlib"
            self.env_info.OBJCOPY = "arm-poky-linux-gnueabi-objcopy"
            self.env_info.OBJDUMP = "arm-poky-linux-gnueabi-objdump"
            self.env_info.AR = "arm-poky-linux-gnueabi-ar"
            self.env_info.NM = "arm-poky-linux-gnueabi-nm"
            self.env_info.M4 = "m4"
            self.env_info.TARGET_PREFIX = "arm-poky-linux-gnueabi-"
            self.env_info.CONFIGURE_FLAGS = "--target=arm-poky-linux-gnueabi --host=arm-poky-linux-gnueabi --build=x86_64-linux --with-libtool-sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.LDFLAGS = "-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed"
            self.env_info.ARCH = "arm"
        else:
            self._append_path(self.env_info.PATH, os.path.join(self.env_info.SYSROOT, "x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux"))
            self._append_path(self.env_info.PATH, os.path.join(self.env_info.SYSROOT, "x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux-musl"))
            self.env_info.CONFIG_SITE = self.env_info.TOOLCHAIN_ROOT + "/site-config-aarch64-poky-linux"
            self.env_info.OECORE_TARGET_ARCH = "aarch64"
            self.env_info.OECORE_TARGET_OS = "linux"
            self.env_info.CC = "aarch64-poky-linux-gcc -march=armv8-a+crc -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.CXX = "aarch64-poky-linux-g++ -march=armv8-a+crc -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.CPP = "aarch64-poky-linux-gcc -E -march=armv8-a+crc -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.AS = "aarch64-poky-linux-as"
            self.env_info.LD = "aarch64-poky-linux-ld --sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.GDB = "aarch64-poky-linux-gdb"
            self.env_info.STRIP = "aarch64-poky-linux-strip"
            self.env_info.RANLIB = "aarch64-poky-linux-ranlib"
            self.env_info.OBJCOPY = "aarch64-poky-linux-objcopy"
            self.env_info.OBJDUMP = "aarch64-poky-linux-objdump"
            self.env_info.READELF = "aarch64-poky-linux-readelf"
            self.env_info.AR = "aarch64-poky-linux-ar"
            self.env_info.NM = "aarch64-poky-linux-nm"
            self.env_info.M4 = "m4"
            self.env_info.TARGET_PREFIX = "aarch64-poky-linux-"
            self.env_info.CONFIGURE_FLAGS = "--target=aarch64-poky-linux --host=aarch64-poky-linux --build=x86_64-linux --with-libtool-sysroot=" + self.env_info.SDKTARGETSYSROOT
            self.env_info.LDFLAGS = "-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -fstack-protector-strong -Wl,-z,relro,-z,now"
            self.env_info.ARCH = "arm64"

        # Common config between armv7hf and armv8

        self._append_path(self.env_info.PKG_CONFIG_PATH, os.path.join(self.env_info.SDKTARGETSYSROOT, "usr/lib/pkgconfig"))
        self._append_path(self.env_info.PKG_CONFIG_PATH, os.path.join(self.env_info.SDKTARGETSYSROOT, "usr/share/pkgconfig"))

        self.env_info.CONFIG_SITE = self.env_info.TOOLCHAIN_ROOT + "/site-config-aarch64-poky-linux"
        self.env_info.OECORE_NATIVE_SYSROOT = self.env_info.SYSROOT + "/x86_64-pokysdk-linux"
        self.env_info.OECORE_TARGET_SYSROOT = self.env_info.SDKTARGETSYSROOT
        self.env_info.OECORE_ACLOCAL_OPTS = "-I " + self.env_info.SYSROOT + "/usr/share/aclocal"
        self.env_info.OECORE_BASELIB = "lib"
        self.env_info.CPPFLAGS = ""
        self.env_info.CFLAGS = "-O2 -pipe -g -feliminate-unused-debug-types"
        self.env_info.CXXFLAGS = "-O2 -pipe -g -feliminate-unused-debug-types"
        self.env_info.KCFLAGS = "--sysroot=" + self.env_info.SDKTARGETSYSROOT
        self.env_info.OECORE_DISTRO_VERSION = self.version
        self.env_info.OECORE_SDK_VERSION = self.version

        self.env_info.OE_CMAKE_TOOLCHAIN_FILE = self.env_info.OECORE_NATIVE_SYSROOT + "/usr/share/cmake/OEToolchainConfig.cmake"
