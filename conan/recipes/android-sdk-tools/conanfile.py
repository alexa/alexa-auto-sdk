from conans import ConanFile, tools, RunEnvironment
import os, logging

class AndroidSdkToolsConanFile(ConanFile):
    name = "android-sdk-tools"
    version = "4.0"
    user = "aac-sdk"
    channel = "stable"
    no_copy_source = True
    exports_sources = ["cmake-wrapper.cmd", "cmake-wrapper"]
    settings = "os", "arch", "compiler", "build_type"
    requires = ["zulu-openjdk/11.0.8"]

    options = {
        "sdk_version": "ANY",
        "ndk_version": "ANY",
        "android_stl": ["c++_shared","c++_static"]
    }
    default_options = {
        "sdk_version": "7302050",
        # Sync with the version specified in `aacs/android/sample-app/alexa-auto-app/build.gradle`.
        "ndk_version": "21.4.7075529",  # r21e
        "android_stl": "c++_shared",
    }

    @staticmethod
    def chmod_plus_x(filename):
        if os.name == "posix":
            os.chmod(filename, os.stat(filename).st_mode | 0o111)

    def fix_permissions(self,root_folder):
        if os.name != "posix":
            return
        for root, _, files in os.walk(root_folder):
            for filename in files:
                filename = os.path.join(root, filename)
                with open(filename, "rb") as f:
                    sig = f.read(4)
                    if type(sig) is str:
                        sig = [ord(s) for s in sig]
                    else:
                        sig = [s for s in sig]
                    if len(sig) > 2 and sig[0] == 0x23 and sig[1] == 0x21:
                        logging.info(f"chmod on script file: {filename}")
                        self.chmod_plus_x(filename)
                    elif sig == [0x7F, 0x45, 0x4C, 0x46]:
                        logging.info(f"chmod on ELF file: {filename}")
                        self.chmod_plus_x(filename)
                    elif sig == [0xCA, 0xFE, 0xBA, 0xBE] or \
                         sig == [0xBE, 0xBA, 0xFE, 0xCA] or \
                         sig == [0xFE, 0xED, 0xFA, 0xCF] or \
                         sig == [0xCF, 0xFA, 0xED, 0xFE] or \
                         sig == [0xFE, 0xEF, 0xFA, 0xCE] or \
                         sig == [0xCE, 0xFA, 0xED, 0xFE]:
                        logging.info(f"chmod on Mach-O file: {filename}")
                        self.chmod_plus_x(filename)

    @property
    def _build_os(self):
        settings_build = getattr(self,"settings_build",None)
        return settings_build.os if settings_build else self.settings.os

    def source(self):
        if self._build_os == "Macos":
            package = f"commandlinetools-mac-{self.options.sdk_version}_latest"
        elif self._build_os == "Linux":
            package = f"commandlinetools-linux-{self.options.sdk_version}_latest"
        else:
            raise Exception( f"settings.os not supported: {self._build_os}" )
        #download the command line tools package
        tools.get( f"https://dl.google.com/android/repository/{package}.zip" )

    def package(self):
        self.copy( "*", src="cmdline-tools", dst="cmdline-tools" )
        self.copy( "cmake-wrapper.cmd" )
        self.copy( "cmake-wrapper" )
        # fix executable permisions for command line tools
        self.fix_permissions(self.package_folder)
        # check the license -- needs to be accepted once
        sdk_manager = os.path.join( self.package_folder, "cmdline-tools", "bin", "sdkmanager" )
        auto_accept_licenses = os.getenv("BUILDER_ACCEPT_LICENSES", "False").lower() == "true"
        env_run = RunEnvironment(self)
        with tools.environment_append( env_run.vars ):
            # check the license -- needs to be accepted once
            check_yes_opt = f"yes | {sdk_manager}" if auto_accept_licenses else sdk_manager
            self.run( f"{check_yes_opt} --sdk_root={self.package_folder} --licenses", run_environment=True )
            # install android sdk
            self.run( f"{sdk_manager} --sdk_root={self.package_folder} 'platform-tools' 'platforms;android-{self.settings_target.os.api_level}'", run_environment=True )
            # install android ndk
            self.run( f"{sdk_manager} --sdk_root={self.package_folder} --install 'ndk;{self.options.ndk_version}'", run_environment=True )

    @property
    def _platform(self):
        return {"Windows": "windows",
                "Macos": "darwin",
                "Linux": "linux"}.get(str(self._build_os))

    @property
    def _android_abi(self):
        return {"x86": "x86",
                "x86_64": "x86_64",
                "armv7hf": "armeabi-v7a",
                "armv8": "arm64-v8a"}.get(str(self.settings_target.arch))

    @property
    def _llvm_triplet(self):
        arch = {'armv7hf': 'arm',
                'armv8': 'aarch64',
                'x86': 'i686',
                'x86_64': 'x86_64'}.get(str(self.settings_target.arch))
        abi = 'androideabi' if self.settings_target.arch == 'armv7hf' else 'android'
        return f"{arch}-linux-{abi}"

    @property
    def _clang_triplet(self):
        arch = {'armv7hf': 'armv7a',
                'armv8': 'aarch64',
                'x86': 'i686',
                'x86_64': 'x86_64'}.get(str(self.settings_target.arch))
        abi = 'androideabi' if self.settings_target.arch == 'armv7hf' else 'android'
        return f"{arch}-linux-{abi}"

    @property
    def _sdk_home(self):
        return os.path.join( self.package_folder )

    @property
    def _ndk_home(self):
        return os.path.join( self.package_folder, "ndk", str(self.options.ndk_version) )

    @property
    def _ndk_root(self):
        return os.path.join(self._ndk_home, "toolchains", "llvm", "prebuilt",  f"{self._platform}-x86_64")

    def _tool_name(self, tool):
        if 'clang' in tool:
            suffix = '.cmd' if self._build_os == 'Windows' else ''
            return f"{self._clang_triplet}{self.settings_target.os.api_level}-{tool}{suffix}"
        else:
            suffix = '.exe' if self._build_os == 'Windows' else ''
            return f"{self._llvm_triplet}-{tool}{suffix}"

    def _define_tool_var(self, name, value):
        ndk_bin = os.path.join(self._ndk_root, 'bin')
        path = os.path.join(ndk_bin, self._tool_name(value))
        logging.info(f"Creating {name} environment variable: {path}")
        return path

    def package_info(self):
        # set the android sdk environment variables
        logging.info(f"Creating ANDROID_SDK_ROOT environment variable: {self._sdk_home}")
        self.env_info.ANDROID_SDK_ROOT = self._sdk_home

        # test shall pass, so this runs also in the build as build requirement context
        # ndk-build: https://developer.android.com/ndk/guides/ndk-build
        self.env_info.PATH.append( self._ndk_home )

        # You should use the ANDROID_NDK_ROOT environment variable to indicate where the NDK is located.
        # That's what most NDK-related scripts use (inside the NDK, and outside of it).
        # https://groups.google.com/g/android-ndk/c/qZjhOaynHXc
        logging.info(f"Creating ANDROID_NDK_ROOT environment variable: {self._ndk_home}")
        self.env_info.ANDROID_NDK_ROOT = self._ndk_home

        # Gradle is complaining about the ANDROID_NDK_HOME environment variable:
        #   WARNING: Support for ANDROID_NDK_HOME is deprecated and will be removed in the future.
        #   Use android.ndkVersion in build.gradle instead.
        # logging.info(f"Creating ANDROID_NDK_HOME environment variable: {self._ndk_home}")
        # self.env_info.ANDROID_NDK_HOME = self._ndk_home

        logging.info(f"Creating NDK_ROOT environment variable: {self._ndk_root}")
        self.env_info.NDK_ROOT = self._ndk_root

        logging.info(f"Creating CHOST environment variable: {self._llvm_triplet}")
        self.env_info.CHOST = self._llvm_triplet

        ndk_sysroot = os.path.join(self._ndk_root, 'sysroot')
        logging.info(f"Creating CONAN_CMAKE_FIND_ROOT_PATH environment variable: {ndk_sysroot}")
        self.env_info.CONAN_CMAKE_FIND_ROOT_PATH = ndk_sysroot

        logging.info(f"Creating SYSROOT environment variable: {ndk_sysroot}")
        self.env_info.SYSROOT = ndk_sysroot

        logging.info(f"Creating self.cpp_info.sysroot: {ndk_sysroot}")
        self.cpp_info.sysroot = ndk_sysroot

        logging.info(f"Creating ANDROID_NATIVE_API_LEVEL environment variable: {self.settings_target.os.api_level}")
        self.env_info.ANDROID_NATIVE_API_LEVEL = str(self.settings_target.os.api_level)

        self.chmod_plus_x(os.path.join(self.package_folder, "cmake-wrapper"))
        cmake_wrapper = "cmake-wrapper.cmd" if self._build_os == "Windows" else "cmake-wrapper"
        cmake_wrapper = os.path.join(self.package_folder, cmake_wrapper)
        logging.info(f"Creating CONAN_CMAKE_PROGRAM environment variable: {cmake_wrapper}")
        self.env_info.CONAN_CMAKE_PROGRAM = cmake_wrapper

        toolchain = os.path.join(self._ndk_home, "build", "cmake", "android.toolchain.cmake")
        logging.info(f"Creating CONAN_CMAKE_TOOLCHAIN_FILE environment variable: {toolchain}")
        self.env_info.CONAN_CMAKE_TOOLCHAIN_FILE = toolchain

        self.env_info.CC = self._define_tool_var('CC', 'clang')
        self.env_info.CXX = self._define_tool_var('CXX', 'clang++')
        self.env_info.LD = self._define_tool_var('LD', 'ld')
        self.env_info.AR = self._define_tool_var('AR', 'ar')
        self.env_info.AS = self._define_tool_var('AS', 'as')
        self.env_info.RANLIB = self._define_tool_var('RANLIB', 'ranlib')
        self.env_info.STRIP = self._define_tool_var('STRIP', 'strip')
        self.env_info.ADDR2LINE = self._define_tool_var('ADDR2LINE', 'addr2line')
        self.env_info.NM = self._define_tool_var('NM', 'nm')
        self.env_info.OBJCOPY = self._define_tool_var('OBJCOPY', 'objcopy')
        self.env_info.OBJDUMP = self._define_tool_var('OBJDUMP', 'objdump')
        self.env_info.READELF = self._define_tool_var('READELF', 'readelf')
        self.env_info.ELFEDIT = self._define_tool_var('ELFEDIT', 'elfedit')

        self.env_info.ANDROID_PLATFORM = f"android-{self.settings_target.os.api_level}"
        self.env_info.ANDROID_TOOLCHAIN = "clang"
        self.env_info.ANDROID_ABI = self._android_abi
        self.env_info.ANDROID_STL = f"{self.options.android_stl}"
        # set the stl shared lib path if specified by the android_stl option
        if self.options.android_stl == "c++_shared":
            self.env_info.ANDROID_STL_SHARED_LIB = f"{os.path.join(ndk_sysroot, 'usr', 'lib', self._llvm_triplet, 'libc++_shared.so')}"
            logging.info(f"Creating ANDROID_STL_SHARED_LIB environment variable: {self.env_info.ANDROID_STL_SHARED_LIB}")

        self.env_info.CMAKE_FIND_ROOT_PATH_MODE_PROGRAM = "BOTH"
        self.env_info.CMAKE_FIND_ROOT_PATH_MODE_LIBRARY = "BOTH"
        self.env_info.CMAKE_FIND_ROOT_PATH_MODE_INCLUDE = "BOTH"
        self.env_info.CMAKE_FIND_ROOT_PATH_MODE_PACKAGE = "BOTH"
