import os, subprocess, tempfile, tarfile, json, platform
from pylib.common import BaseHandler

class BuilderHandler(BaseHandler):

    def run( self ):
        self.initialize_configuration()
        self.initialize_environment()

        # update the the builder configuration
        if self.get_arg( "skip_config", False ):
            self.log_info("Skipping configuration")
        else:
            self.builder_configuration.configure()

        # get forced packages
        forced_packages = self.join_arg_list( "force" )

        # create the includes path from the builder args - convert to absolute
        # path reference from the pwd...
        include_paths = [os.path.abspath(next) for next in self.join_arg_list( "include" )]

        # add the sample app to the includes path if needed
        if self.get_arg("with_sampleapp", False):
            include_paths.append( os.path.join( self.sdk_root, "samples/cpp" ) )

        # export the recipes from the builder configuration
        forced_packages = self.builder_configuration.export( forced_packages, True, include_paths )

        # build the specified modules
        self.build( forced_packages )

        # package the archive
        if not self.get_arg( "no_output", False ):
            self.package()
        else:
            self.log_info("Skipping output")
            if self.verbose:
                manifest = self.load_pkg_manifest()
                self.log_info( f"Package archive manifest: {json.dumps(manifest,indent=3)}" )

    def initialize_configuration( self ):

        # validate output file argument
        arg_output = self.get_arg( "output" )
        if arg_output:
            self.output_file = os.path.abspath( arg_output )
            parent_dir = os.path.dirname( self.output_file )
            if not os.path.exists( parent_dir ):
                raise Exception( f"Output file path doesn't exist! {parent_dir}" )
            if not (self.output_file.endswith( ".tgz" ) or self.output_file.endswith( ".tar.gz" )):
                self.output_file = f"{self.output_file}.tgz"
        else:
            self.output_file = None

        # validate sensitive latentcy logs options with debug
        if self.get_arg("with_sensitive_logs") and not self.get_arg("debug"):
            raise Exception( "Invalid sensitive logs option with release build (must specify debug option)" )

    def initialize_environment( self ):
        self.build_env = os.environ
        self.build_env.update({
            "BUILDER_ROOT": self.builder_root,
            "BUILDER_HOME": self.builder_home,
            "BUILDER_ACCEPT_LICENSES": "true" if self.get_arg( "accept_licenses", False ) else "false",
            "ENABLE_VERBOSE_LOGGING": "true" if self.verbose else "false",
            "CONAN_USER_HOME": self.conan_home,
            "GRADLE_USER_HOME": self.gradle_home
        })

    def build( self, forced_packages = [] ):

        build_modules = []

        # get the target config
        target_platform = self.get_arg("platform")
        target_arch = self.get_arg("arch")

        # create the module list from the builder args
        module_list = self.join_arg_list( "modules" )

        # validate the module list
        if module_list:
            for next in module_list:
                pkg_name = f"aac-module-{next.casefold()}"
                if self.builder_configuration.has_package( pkg_name ):
                    build_modules.append( pkg_name )
                else:
                    raise Exception( f"Invalid module package: {pkg_name}" )
        else:
            build_modules.extend( self.builder_configuration.find_packages( "aac-module-*" ) )

        # get the target platform to be used for checking module compatibility
        check_platform = subprocess.run(
            ["conan", "profile", "get", "settings.os", f"aac-{target_platform}" if target_platform else "default"],
            stdout=subprocess.PIPE,
        ).stdout.decode('utf-8').strip()

        # remove modules that are not supported for the target platform
        if check_platform:
            for next in build_modules.copy():
                pkg_entry = self.builder_configuration.get_package( next )
                if "info" in pkg_entry and "os" in pkg_entry["info"] and not check_platform in pkg_entry["info"]["os"]:
                    self.log_info(f"Ignoring module '{next}' on unsupported platform: {check_platform}. Expecting {pkg_entry['info']['os']}",True)
                    build_modules.remove( next )

        if not build_modules:
            raise Exception( "No modules to build!" )

        thirdparty_includes = ["include/nlohmann/json_fwd.hpp"]

        # create the temp directory
        self.install_folder = tempfile.TemporaryDirectory().name

        # create the conan installed command options
        build_args = [
            "conan",
            "install",
            os.path.join( self.sdk_root, "conan", "recipes", "aac-sdk-tools" ),
            f"-if={self.install_folder}",
            "-b", "outdated",
            "-o", f"pkg_modules={','.join(build_modules)}",
            "-o", f"thirdparty_includes={','.join(thirdparty_includes)}",
            "-o", f"aac_version={self.get_arg('version','dev')}",
            "-o", f"with_aasb={self.get_arg('with_aasb',False)}",
            "-o", f"with_unit_tests={self.get_arg('with_unit_tests',False)}",
            "-o", f"with_sensitive_logs={self.get_arg('with_sensitive_logs',False)}",
            "-o", f"with_latency_logs={self.get_arg('with_latency_logs',False)}",
            "-o", f"with_sampleapp={self.get_arg('with_sampleapp',False)}",
            "-o", f"with_docs={self.get_arg('with_docs',True)}",
            "-s", f"build_type={'Debug' if self.get_arg('debug') else 'Release'}"
        ]

        # set the build_name option if specified
        build_name = self.get_arg( "name" )
        if build_name:
            build_args.extend( ["-o", f"build_name={build_name.replace(' ','_')}"] )

        # set the target arch options if specified
        if target_arch:
            build_args.extend( ["-s:h", "arch=%s" % target_arch] )
            if not target_platform == "android" or target_platform.startswith("qnx"):
                build_args.extend( ["-s:b", "arch=x86_64"] )

        # set the target platform options if specified
        if target_platform:
            build_args.append( f"-pr:h=aac-{target_platform}" )
            build_args.append( "-pr:b=default" )

        # set the conan options if specified
        option_list = self.get_arg( "conan_option" )
        if option_list:
            for next in option_list:
                build_args.extend( ["-o", f"{next}"] )

        # set the conan settings if specified
        setting_list = self.get_arg( "conan_setting" )
        if setting_list:
            for next in setting_list:
                build_args.extend( ["-s", f"{next}"] )

        # force build the specified packages
        for next in forced_packages:
            build_args.extend( ["-b", next] )

        # build the auto sdk modules
        subprocess.run( build_args, env=self.build_env, check=True )

    def load_pkg_manifest( self ):
        pkg_manifest_file = os.path.join( self.install_folder, "pkg_manifest.json" )
        if os.path.exists( pkg_manifest_file ):
            with open( pkg_manifest_file, "r" ) as file:
                return json.load( file )
        else:
            return {}

    def package( self ):
        export_name = None

        # load the package manifest
        manifest = self.load_pkg_manifest()

        # get the pkgname version info
        export_name = manifest["pkgname"]

        if not self.output_file:
            deploy_path = os.path.join( self.builder_root, "deploy" )
            if not os.path.exists( deploy_path ):
                os.makedirs( deploy_path )
            self.output_file = os.path.join( deploy_path, f"{export_name}.tgz")

        # create the archive file
        if self.output_file.endswith( ".tgz" ) or self.output_file.endswith( ".tar.gz" ):
            with tarfile.open( self.output_file, "w:gz") as tar:
                tar.add( os.path.join( self.install_folder, "conaninfo.txt" ), arcname=f"{export_name}/aac-buildinfo.txt" )
                for next in ["lib","bin","include","aar","share","docs"]:
                    install_src = os.path.join( self.install_folder, next )
                    if os.path.exists( install_src ):
                        tar.add( install_src, arcname=f"{export_name}/{next}" )
                tar.close()
        else:
            raise Exception( f"Unsupported archive type: {self.output_file}" )

        # log the archive file
        self.log_info( f"Created output archive: \033[1;37m{self.output_file}" )
