import os
from conans import ConanFile, CMake, AutoToolsBuildEnvironment, tools
from conans.errors import ConanInvalidConfiguration


class Nghttp2Conan(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"

    name = "libnghttp2"
    version = "1.42.0"
    description = "HTTP/2 C Library and tools"
    topics = ("conan", "http")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://nghttp2.org"
    license = "MIT"
    exports_sources = ["CMakeLists.txt", "patches/**"]
    generators = "cmake", "pkg_config"
    settings = "os", "arch", "compiler", "build_type"
    options = {"shared": [True, False],
               "fPIC": [True, False],
               "with_app": [True, False],
               "with_hpack": [True, False],
               "with_jemalloc": [True, False],
               "with_asio": [True, False],
               "openssl_version": ["1.0", "1.1", None],
              }
    default_options = {"shared": False,
                       "fPIC": True,
                       "with_app": True,
                       "with_hpack": False,
                       "with_jemalloc": False,
                       "with_asio": False,
                       "openssl_version": None,
                      }

    _source_subfolder = "source_subfolder"

    @property
    def default_channel(self):
        return "dev"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        if self.options.with_asio and self.settings.compiler == "Visual Studio":
            raise ConanInvalidConfiguration("Build with asio and MSVC is not supported yet, see upstream bug #589")
        if self.settings.compiler == "gcc":
            v = tools.Version(str(self.settings.compiler.version))
            if v < "6.0":
                raise ConanInvalidConfiguration("gcc >= 6.0 required")
        if self.settings.os == "Android":
            self.options.with_app = False

        if self.options.openssl_version:
            self.options["openssl"].openssl_version = self.options.openssl_version

    def requirements(self):
        if self.settings.os == "Neutrino":
            self.requires("openssl/qnx7")
        else:
            self.requires("openssl/1.1.1h") # always set openssl dependency - (aac) fix for with_app=False
        if self.options.with_app:
            self.requires("c-ares/1.17.1")
            self.requires("libev/4.33")
            self.requires("libevent/2.1.12")
            self.requires("libxml2/2.9.10#7293e7b3f9703b324258194bb749ce85")
        if self.options.with_hpack:
            self.requires("jansson/2.13.1")
        if self.options.with_jemalloc:
            self.requires("jemalloc/5.2.1")
        if self.options.with_asio:
            self.requires("boost/1.71.0")

    def source(self):
        tools.get(**self.conan_data["sources"][self.version])
        extracted_folder = "nghttp2-{0}".format(self.version)
        os.rename(extracted_folder, self._source_subfolder)

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["ENABLE_SHARED_LIB"] = "ON" if self.options.shared else "OFF"
        cmake.definitions["ENABLE_STATIC_LIB"] = "OFF" if self.options.shared else "ON"
        cmake.definitions["ENABLE_HPACK_TOOLS"] = "ON" if self.options.with_hpack else "OFF"
        cmake.definitions["ENABLE_APP"] = "ON" if self.options.with_app else "OFF"
        cmake.definitions["ENABLE_EXAMPLES"] = "OFF"
        cmake.definitions["ENABLE_PYTHON_BINDINGS"] = "OFF"
        cmake.definitions["ENABLE_FAILMALLOC"] = "OFF"
        # disable unneeded auto-picked dependencies
        cmake.definitions["WITH_LIBXML2"] = "OFF"
        cmake.definitions["WITH_JEMALLOC"] = "ON" if self.options.with_jemalloc else "OFF"
        cmake.definitions["WITH_SPDYLAY"] = "OFF"

        cmake.definitions["ENABLE_ASIO_LIB"] = "ON" if self.options.with_asio else "OFF"

        if tools.Version(self.version) >= "1.42.0":
            # backward-incompatible change in 1.42.0
            cmake.definitions["STATIC_LIB_SUFFIX"] = "_static"

        # set the open ssl rootpath - (aac) fix for with_app=False
        cmake.definitions["OPENSSL_ROOT_DIR"] = self.deps_cpp_info["openssl"].rootpath

        if self.options.with_asio:
            cmake.definitions["BOOST_ROOT"] = self.deps_cpp_info["boost"].rootpath

        cmake.configure()
        return cmake

    def _patch_sources(self):
        for patch in self.conan_data["patches"][self.version]:
            tools.patch(**patch)
        # tools.replace_in_file(os.path.join(self._source_subfolder, "lib", "CMakeLists.txt"),
        #                       "set_target_properties(nghttp2_static ",
        #                       "target_include_directories(nghttp2_static INTERFACE\n"
        #                       "${CMAKE_CURRENT_BINARY_DIR}/includes\n"
        #                       "${CMAKE_CURRENT_SOURCE_DIR}/includes)\n"
        #                       "set_target_properties(nghttp2_static ")
        target_libnghttp2 = "nghttp2" if self.options.shared else "nghttp2_static"
        tools.replace_in_file(os.path.join(self._source_subfolder, "src", "CMakeLists.txt"),
                              "\n"
                              "link_libraries(\n"
                              "  nghttp2\n",
                              "\n"
                              "link_libraries(\n"
                              "  {} ${{CONAN_LIBS}}\n".format(target_libnghttp2))
        if not self.options.shared:
            tools.replace_in_file(os.path.join(self._source_subfolder, "src", "CMakeLists.txt"),
                                  "\n"
                                  "  add_library(nghttp2_asio SHARED\n",
                                  "\n"
                                  "  add_library(nghttp2_asio\n")
            tools.replace_in_file(os.path.join(self._source_subfolder, "src", "CMakeLists.txt"),
                                  "\n"
                                  "  target_link_libraries(nghttp2_asio\n"
                                  "    nghttp2\n",
                                  "\n"
                                  "  target_link_libraries(nghttp2_asio\n"
                                  "    {}\n".format(target_libnghttp2))

    def build_cmake(self):
        self._patch_sources()
        cmake = self._configure_cmake()
        cmake.build()

    def build_autotools(self):
        autotools = AutoToolsBuildEnvironment(self)
        conf_args = [
            "--enable-lib-only"
        ]
        autotools.configure(configure_dir=self._source_subfolder,args=conf_args)
        autotools.make()
        pass

    def build(self):
        if self.settings.os == "Android":
            self.build_autotools()
        else:
            self.build_cmake()

    def package(self):
        self.copy(pattern="COPYING", dst="licenses", src=self._source_subfolder)
        if self.settings.os == "Android":
            self.copy("*.lib", dst="lib", keep_path=False)
            self.copy("*.dll", dst="bin", keep_path=False)
            self.copy("*.dylib*", dst="lib", keep_path=False)
            self.copy("*.so", dst="lib", keep_path=False)
            self.copy("*.a", dst="lib", keep_path=False)
            self.copy("*.h", src="%s/lib/includes" % self._source_subfolder, dst="include")
        else:
            cmake = self._configure_cmake()
            cmake.install()
            cmake.patch_config_paths()
            tools.rmdir(os.path.join(self.package_folder, 'share'))
            tools.rmdir(os.path.join(self.package_folder, 'lib', 'pkgconfig'))

    def package_info(self):
        suffix = "_static" if tools.Version(self.version) > "1.39.2" and not self.options.shared and not self.settings.os == "Android" else ""
        self.cpp_info.libs = ["nghttp2" + suffix]
        if self.options.with_asio:
            self.cpp_info.libs.insert(0, "nghttp2_asio")
        if self.settings.compiler == "Visual Studio":
            if not self.options.shared:
                self.cpp_info.defines.append("NGHTTP2_STATICLIB")
