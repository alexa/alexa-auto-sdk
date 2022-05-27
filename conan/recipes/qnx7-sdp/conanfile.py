import os

from conans import ConanFile, tools
from conans.errors import ConanInvalidConfiguration


class QNX7SDPConan(ConanFile):
    name = "qnx7-sdp"
    version = "7.x"
    description = "Cross-compiling with QNX 7 SDP"

    settings = {
        "os": ["Linux", "Macos"],
        "arch": ["x86_64", "armv8"]
    }

    options = {"qnx7sdp_path": "ANY"}
    default_options = {"qnx7sdp_path": ""}

    exports = "*.cmake"

    @property
    def _build_os(self):
        settings_build = getattr(self, "settings_build", None)
        return settings_build.os if settings_build else self.settings.os

    @property
    def _platform(self):
        return {"Macos": "darwin",
                "Linux": "linux"}.get(str(self._build_os))

    @property
    def _qnx_version(self):
        if self.settings_target.os.version == "7.0":
            return "700"
        elif self.settings_target.os.version == "7.1":
            return "710"
        raise ConanInvalidConfiguration(
            f"Unsupported QNX version: {self.settings_target.os.version}"
        )

    @property
    def _qnx_base(self):
        if self.options.qnx7sdp_path != "":
            print("Using QNX SDP from %s" % self.options.qnx7sdp_path)
            return str(self.options.qnx7sdp_path)
        else:
            qnx_folder_name = "qnx" + self._qnx_version
            return os.path.join(os.getenv("HOME"), qnx_folder_name)

    @property
    def _qnx_host(self):
        return os.path.join(self._qnx_base, "host", str(self._platform), str(self.settings_build.arch))

    @property
    def _qnx_target(self):
        return os.path.join(self._qnx_base, "target", f"qnx{self.version[0]}")

    @property
    def _qnx_arch(self):
        return {"x86_64": "x86_64",
                "armv8": "aarch64le"}.get(str(self.settings_target.arch))

    @property
    def _qnx_toolchain_arch(self):
        return {"x86_64": "x86_64",
                "armv8": "aarch64"}.get(str(self.settings_target.arch))

    def configure(self):
        if self.settings_target.arch not in ["x86_64", "armv8"]:
            raise ConanInvalidConfiguration(f"Invalid arch {self.settings_target.arch}")

    def package(self):
        self.copy("*.cmake", src=self.recipe_folder, dst=self.package_folder)

    def package_info(self):
        usr_bin_path = os.path.join(self._qnx_host, "usr", "bin")
        if not os.path.exists(usr_bin_path):
            raise Exception(f"QNX compiler not found at {usr_bin_path}")
        self.env_info.PATH.append(usr_bin_path)

        if not os.path.exists(self._qnx_host):
            raise Exception(f"QNX host not found at {self._qnx_host}")
        self.env_info.QNX_HOST = self._qnx_host

        if not os.path.exists(self._qnx_target):
            raise Exception(f"QNX target not found at {self._qnx_target}")
        self.env_info.QNX_TARGET = self._qnx_target

        toolchain_prefix = f"nto{self._qnx_toolchain_arch}"

        self.env_info.CC = f"qcc -Vgcc_nto{self._qnx_arch}"
        self.env_info.CFLAGS = "-D_QNX_SOURCE"
        self.env_info.CXX = f"q++ -Vgcc_nto{self._qnx_arch}"
        self.env_info.CXXFLAGS = f"{self.env_info.CFLAGS} -Y_cxx"
        self.env_info.CPP = f"{toolchain_prefix}-cpp"
        self.env_info.AR = f"{toolchain_prefix}-ar"
        self.env_info.RANLIB = f"{toolchain_prefix}-ranlib"
        self.env_info.LD = f"{toolchain_prefix}-ld"
        self.env_info.AS = f"{toolchain_prefix}-as"
        self.env_info.GDB = f"{toolchain_prefix}-gdb"
        self.env_info.STRIP = f"{toolchain_prefix}-strip"
        self.env_info.OBJCOPY = f"{toolchain_prefix}-objcopy"
        self.env_info.OBJDUMP = f"{toolchain_prefix}-objdump"
        self.env_info.READELF = f"{toolchain_prefix}-readelf"
        self.env_info.NM = f"{toolchain_prefix}-nm"

        self.env_info.CMAKE_TOOLCHAIN_FILE = os.path.join(self.package_folder, f"qnx7_toolchain_{self.settings_target.arch}.cmake")

        self.env_info.QNX_HOST = self._qnx_host
        self.env_info.QNX_TARGET = self._qnx_target

        self.output.info(f"Done setting up cross-compiling with {self.env_info.QNX_HOST}")
