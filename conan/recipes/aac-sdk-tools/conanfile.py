import os, logging, json
from datetime import datetime
from conans import ConanFile, tools
from conans.errors import ConanException, ConanInvalidConfiguration
from pylib import utils, cmake_build, gradle_build, a2ml_build

class AacSdkTools(ConanFile):
    
    name = "aac-sdk-tools"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    exports = "*"

    options = {
        "build_name": "ANY",
        "pkg_modules": "ANY",
        "thirdparty_includes": "ANY",
        "aac_version": "ANY",
        "with_aasb": [True,False],
        "with_unit_tests": [True,False],
        "with_sensitive_logs": [True,False],
        "with_latency_logs": [True,False],
        "with_sampleapp": [True,False],
        "with_docs": [True,False]
    }

    default_options = {
        "build_name": None,
        "pkg_modules": None,
        "aac_version": "dev",
        "with_aasb": False,
        "with_unit_tests": False,
        "with_sensitive_logs": False,
        "with_latency_logs": False,
        "with_sampleapp": False,
        "with_docs": True
    }
 
    def init(self):
        self.logging_level = logging.DEBUG if os.getenv( "ENABLE_VERBOSE_LOGGING" ) == "true" else logging.INFO
        logging.basicConfig( level=self.logging_level )
        logging.getLogger().handlers[0].setFormatter( AacSdkToolsLogFormatter() )

    def requirements(self):
        if self.options.pkg_modules:
            for req in str(self.options.pkg_modules).split(","):
                self.requires( f"{req}/{self.options.aac_version}" )
                # configure options
                self.options[req].with_aasb = self.options.with_aasb
                self.options[req].with_unit_tests = self.options.with_unit_tests
                self.options[req].with_sensitive_logs = self.options.with_sensitive_logs
                self.options[req].with_latency_logs = self.options.with_latency_logs
                self.options[req].with_docs = self.options.with_docs
        if self.options.with_sampleapp:
            if self.settings.os == "Android":
                logging.warning("SampleApp not available on Android. Skipping.")
            else:
                self.requires( f"aac-sampleapp/{self.options.aac_version}" )
                self.options["aac-sampleapp"].with_aasb = self.options.with_aasb
                self.options["aac-sampleapp"].with_unit_tests = self.options.with_unit_tests
                self.options["aac-sampleapp"].with_sensitive_logs = self.options.with_sensitive_logs
                self.options["aac-sampleapp"].with_latency_logs = self.options.with_latency_logs
                if self.options.pkg_modules:
                    self.options["aac-sampleapp"].aac_modules = self.options.pkg_modules

    def imports(self):
        if self.options.pkg_modules:
            # export shared libraries
            self.copy( "*.dylib*", src="@libdirs", dst="lib", keep_path=False )
            self.copy( "*.so*", src="@libdirs", dst="lib", keep_path=False )
            self.copy( "*.dll*", src="@libdirs", dst="lib", keep_path=False )
            self.copy( "*.aar", src="aar", dst="aar", keep_path=False )
            self.copy( "*", src="docs", dst="docs", keep_path=True )
            # copy deps export resources
            self.copy_deps_exports()
            # write exported package name
            with open( "pkg_manifest.json", "w" ) as file:
                json.dump( self.create_manifest(), file, indent=3 )
        if self.options.thirdparty_includes:
            for inc in str(self.options.thirdparty_includes).split(","):
                self.copy( inc )

    def create_manifest(self):
        pkgname = "aac-%s-%s_%s-%s-%s" % (str(self.options.aac_version).lower(), 
            (f"{str(self.options.build_name)}-{str(self.settings.os)}" if self.options.build_name else str(self.settings.os)).lower(), 
            str(self.settings.arch).lower(), 
            str(self.settings.build_type).lower(), 
            datetime.now().strftime("%y%m%d%H%M%S"))
        # create the manifest
        manifest = { "pkgname": pkgname }
        return manifest

    def copy_deps_exports(self):
        for next_dep in self.deps_user_info:
            user_info = self.deps_user_info[next_dep]
            if hasattr(user_info,"aac_export_includes"):
                export_includes = json.loads( user_info.aac_export_includes )
                for next_include in export_includes:
                    self.copy( next_include["pattern"],src=next_include["src"],dst="include", keep_path=True )
            if hasattr(user_info,"aac_export_bins"):
                export_includes = json.loads( user_info.aac_export_bins )
                for next_include in export_includes:
                    self.copy( next_include["pattern"],src=next_include["src"],dst="bin", keep_path=True )
            if hasattr(user_info,"aac_export_shared"):
                export_shared = json.loads( user_info.aac_export_shared )
                for next_shared in export_shared:
                    self.copy( next_shared["pattern"],src=next_shared["src"],dst=os.path.join("share",next_shared["dst"]), keep_path=True )

class AacSdkToolsLogFormatter( logging.Formatter ):
    def format( self, log ):
        t = self.formatTime( log, "%Y-%m-%d %H:%M:%S" )
        return f"[{t}] {log.levelname[0]}: {log.getMessage()}"

class BaseSdkModule(object):

    name = "aac-sdk-module"
    url = "https://github.com/alexa/alexa-auto-sdk"
    license = "Apache-2.0"

    module_name = None
    module_requires = []
    module_system_libs = []
    module_options = {}
    module_default_options = {}
    module_overrides_cmake = False
    module_overrides_gradle = False

    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    exports_sources = "*"

    supported_os = []
    supported_arch = []

    # expose pylib to sub-modules
    _utils = utils

    options = {
        "shared": [True, False],
        "with_engine": [True, False],
        "with_platform": [True, False],
        "with_aasb": [True, False],
        "with_messages": [True,False],
        "with_unit_tests": [True, False],
        "with_jni": [True, False],
        "with_android_libs": [True, False],
        "with_sensitive_logs": [True, False],
        "with_latency_logs": [True, False],
        "with_coverage_tests": [True, False],
        "with_address_sanitizer": [True, False],
        "with_docs": [True, False],
        "message_version": "ANY",
    }
    default_options = {
        "shared": True,
        "with_engine": True,
        "with_platform": True,
        "with_aasb": True,
        "with_messages": True,
        "with_unit_tests": False,
        "with_jni": True,
        "with_android_libs": True,
        "with_sensitive_logs": False,
        "with_latency_logs": False,
        "with_coverage_tests": False,
        "with_address_sanitizer": False,
        "with_docs": True,
        "message_version": "4.0"
    }

    def validate(self):
        if not self.module_name:
            raise ConanInvalidConfiguration("module_name not defined")
        if not f"aac-module-{self.module_name}" == self.name:
            raise ConanInvalidConfiguration(f"invalid module name... must be 'aac-module-{self.module_name}'")

    def set_version(self):
        if self.version == None:
            self.version = "dev"

    def init(self):
        enable_verbose_logging = os.getenv( "ENABLE_VERBOSE_LOGGING", "false" ).upper()
        self.verbose = enable_verbose_logging == "TRUE" or enable_verbose_logging == "1" or enable_verbose_logging == "ON"
        self.options = utils.merge_values(self.options,self.module_options)
        self.default_options = utils.merge_values(self.default_options,self.module_default_options)
        if not self.description:
            self.description = f"Auto SDK module: {self.module_name}"
        # build info used by builder tools during package discovery
        self.builder_info = {
            "name": self.name,
            "module": self.module_name,
            "description": self.description
        }
        # optional builder_info
        if self.supported_os:
            self.builder_info["os"] = self.supported_os
        if self.supported_arch:
            self.builder_info["arch"] = self.supported_arch

    @property 
    def _required_modules(self):
        modules = []
        for req in self.module_requires:
            modules.append(f"aac-module-{req}")
        return modules

    @property
    def _required_system_libs(self):
        return self.module_system_libs

    @property
    def _module_export_name(self):
        name = ""
        for part in self.module_name.split("-"):
            name += part.capitalize()
        return name

    @property
    def _module_library_name(self):
        return "AACE" + self._module_export_name

    @property
    def _module_include_directories(self):
        includeDirectoryList = []
        if self.options.get_safe("with_engine",default=False):
            includeDirectoryList.append( "engine/include" )
        if self.options.get_safe("with_platform",default=False):
            includeDirectoryList.append( "platform/include" )
        if self.options.get_safe("with_aasb",default=False):
            includeDirectoryList.append( "aasb/include" )
        if self.options.get_safe("with_jni",default=False):
            includeDirectoryList.append( "android/src/main/cpp/include" )
        if self.options.get_safe("with_messages",default=False):
            includeDirectoryList.append( os.path.join( self.install_folder, "aasb-messages", "include" ) )
        return includeDirectoryList

    @property
    def _module_sources(self):
        return []
    
    @property
    def _module_message_directories(self):
        messageDirectoryList = []
        if self.options.get_safe("with_messages",default=False):
            messageDirectoryList.append( "aasb/messages" )
        return messageDirectoryList

    def get_include_directories( self, root_folder, use_relpath = True ):
        includeDirectoryList = []
        for next in self._module_include_directories:
            includeDirectoryList.append( next if os.path.isabs(next) or use_relpath else os.path.join( root_folder, next ) )
        return includeDirectoryList
        
    def get_public_include_directories( self, root_folder, use_relpath = True ):
        includeDirectoryList = []
        if self.options.get_safe("with_platform",default=False):
            includeDirectoryList.append( "platform/include" if use_relpath else os.path.join( root_folder, "platform/include" ) )
        if self.options.get_safe("with_messages",default=False):
            includeDirectoryList.append( os.path.join( self.install_folder, "aasb-messages", "include" ) )
        return includeDirectoryList
        
    @property
    def _source_path(self):
        return self.build_folder if self.in_local_cache else self.recipe_folder

    def add_requirement(self,req):
        try:
            user = self.user
            channel = self.channel
        except ConanException:
            self.requires(f"{req}/{self.version}")
        else:
            self.requires(f"{req}/{self.version}@{user}/{channel}")

    def requirements(self):
        for req in self._required_modules:
            self.add_requirement(req)
        # add gtest requirement if building with unit tests
        if self.options.get_safe("with_unit_tests", default=False):
            self.requires("gtest/1.8.1")
            self.options["gtest"].no_main = False

    def build_requirements(self):
        if self.options.with_messages:
            self.build_requires(f"aac-tool-a2ml/{self.version}")
        if self.options.get_safe("with_android_libs", default=False):
            self.build_requires(f"gradle/7.0@aac-sdk/{self.version}")

    def config_options(self):
        if not self.settings.os == "Android":
            del self.options.with_jni
            del self.options.with_android_libs

    def configure(self):
        self.settings.compiler.cppstd = "11"
        if self.options.with_aasb:
            self.options.with_engine = True
            self.options.with_platform = True
            self.options.with_messages = True
        for req in self._required_modules:
            self.options[req].with_engine = self.options.with_engine
            self.options[req].with_platform = self.options.with_platform
            self.options[req].with_aasb = self.options.with_aasb
            self.options[req].with_messages = self.options.with_messages
            self.options[req].with_unit_tests = self.options.with_unit_tests
        # can only have unit tests with engine
        if self.options.with_unit_tests and not self.options.with_engine:
            raise ConanInvalidConfiguration("Unit tests are not supported unless building with engine!")

    def get_cmake_definitions(self):
        return {
            "AAC_EMIT_SENSITIVE_LOGS": utils.bool_value(self.options.with_sensitive_logs,"1","0"),
            "AAC_EMIT_LATENCY_LOGS": utils.bool_value(self.options.with_latency_logs,"1","0"),
            "AAC_ENABLE_COVERAGE": utils.bool_value(self.options.with_coverage_tests,"1","0"),
            "AAC_ENABLE_ADDRESS_SANITIZER": utils.bool_value(self.options.with_address_sanitizer,"1","0"),
            "AAC_ENABLE_UNIT_TESTS": utils.bool_value(self.options.get_safe("with_unit_tests", default=False),"1","0"),
            "CONAN_SYSTEM_INCLUDES": "OFF"
        }

    def imports(self):
        # generate the message headers in the install folder - we do this in the
        # imports phase so that the generated headers will be available for local
        # build development when using `conan intstall`.
        if self.options.with_messages:
            a2ml_build.imports(self)

        cmake_build.imports(self,self.install_folder)

        # import gradle files for android library build
        if self.options.get_safe("with_android_libs", default=False):
            gradle_build.imports(self,self.install_folder)

    def build(self):
        # build the cmake project
        if self.options.with_engine or self.options.with_platform:
            cmake_build.build(self)
        # build the android packages if necessary
        if self.options.get_safe("with_android_libs", default=False):
            gradle_build.build(self)

    def package(self):
        cmake_build.package(self)
        if self.options.with_messages:
            a2ml_build.package(self)
            self.copy("*", src="aasb-docs", dst=f"docs/{self.module_name}/messages", keep_path=True)
        if self.options.get_safe("with_android_libs", default=False):
            gradle_build.package(self)
        if self.options.with_docs:
            self.copy("README.md", dst=f"docs/{self.module_name}", keep_path=False)
            self.copy("*.png", src="assets", dst=f"docs/{self.module_name}/assets", keep_path=False)

    def package_info(self):
        self.cpp_info.defines = ["AAC_" + self.module_name.replace("-","_").upper()]
        self.cpp_info.libs = tools.collect_libs(self)
        self.user_info.module_deps = ",".join(self.deps_cpp_info.deps)
        self.user_info.aac_export_includes = json.dumps([{
            "pattern": "*",
            "src": os.path.abspath("public")
        }])
        if self.options.with_messages:
            self.user_info.message_dir = os.path.abspath("messages")
        if not self.in_local_cache:
            self.cpp_info.includedirs = self._module_include_directories
            self.cpp_info.libdirs = ["build/lib"]

class BaseSdkDependency(object):
    user = "aac-sdk"
    default_channel = "dev"

    # expose pylib to sub-modules
    _utils = utils

    @property
    def _source_path(self):
        return self.build_folder if self.in_local_cache else self.recipe_folder

    def init(self):
        enable_verbose_logging = os.getenv( "ENABLE_VERBOSE_LOGGING", "false" ).upper()
        self.verbose = enable_verbose_logging == "TRUE" or enable_verbose_logging == "1" or enable_verbose_logging == "ON"
        # build info used by builder tools during package discovery
        self.builder_info = {
            "name": self.name
        }
        # optional pkg info
        if self.description:
            self.builder_info["description"] = self.description
