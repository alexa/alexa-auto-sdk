import os
import shutil

from conans import ConanFile
from conans.errors import ConanInvalidConfiguration


class QNXOpenSSLConan(ConanFile):
    """
    Allow recipe consumer to compile and link with OpenSSL libraries provided by QNX SDP
    """

    name = "openssl"
    version = "qnx7"
    settings = "os", "compiler", "build_type", "arch"
    description = "Wrapper for OpenSSL libraries provided by QNX SDP"

    options = {
        "shared": [True, False],
        "openssl_version": ["default", "1.1"],
    }

    default_options = {
        "shared": False,
        "openssl_version": "default",
    }

    @property
    def _qnx_ver(self):
        return self.version

    @property
    def _openssl_ver(self):
        return self.options.openssl_version

    @property
    def _openssl_include_path(self):
        ver = self._openssl_ver
        if ver == "default":
            return os.path.join(self._qnx_target, "usr", "include", "openssl")
        elif ver == "1.1":
            return os.path.join(self._qnx_target, "usr", "include", "openssl1_1", "openssl")
        else:
            raise Exception(f"Unsupported OpenSSL version {ver}")

    @property
    def _libssl_name(self):
        ver = self._openssl_ver
        if ver == "default":
            return "libssl"
        elif ver == "1.1":
            return "libssl1_1"
        else:
            raise Exception(f"Unsupported OpenSSL version {ver}")

    @property
    def _libcrypto_name(self):
        ver = self._openssl_ver
        if ver == "default":
            return "libcrypto"
        elif ver == "1.1":
            return "libcrypto1_1"
        else:
            raise Exception(f"Unsupported OpenSSL version {ver}")

    @property
    def _qnx_target(self):
        qnx_target = os.getenv("QNX_TARGET")
        if not qnx_target:
            raise ConanInvalidConfiguration("QNX_TARGET is not exported")
        return qnx_target

    @property
    def _qnx_arch(self):
        return {"x86_64": "x86_64",
                "armv8": "aarch64le"}.get(str(self.settings.arch))

    @property
    def _qnx_target_arch(self):
        return os.path.join(self._qnx_target, self._qnx_arch)

    def package(self):
        if not os.path.exists(self._qnx_target_arch):
            raise Exception(f"QNX target for {self._qnx_arch} not found at {self._qnx_target_arch}")

        shutil.copytree(
            self._openssl_include_path,
            os.path.join(self.package_folder, "include", "openssl"))

        os.makedirs(os.path.join(self.package_folder, "lib"))
        if self.options.shared:
            shutil.copy(
                os.path.join(self._qnx_target_arch, "usr", "lib", self._libssl_name + ".so"),
                os.path.join(self.package_folder, "lib", "libssl.so"))
            shutil.copy(
                os.path.join(self._qnx_target_arch, "usr", "lib", self._libcrypto_name + ".so"),
                os.path.join(self.package_folder, "lib", "libcrypto.so"))
        else:
            shutil.copy(
                os.path.join(self._qnx_target_arch, "usr", "lib", self._libssl_name + ".a"),
                os.path.join(self.package_folder, "lib", "libssl.a"))
            shutil.copy(
                os.path.join(self._qnx_target_arch, "usr", "lib", self._libcrypto_name + ".a"),
                os.path.join(self.package_folder, "lib", "libcrypto.a"))

    def package_info(self):
        self.cpp_info.components["ssl"].libs = ["ssl"]
        self.cpp_info.components["crypto"].libs = ["crypto"]
        self.cpp_info.components["ssl"].requires = ["crypto"]
