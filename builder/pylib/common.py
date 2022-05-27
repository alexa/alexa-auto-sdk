import logging, platform, os, time, json, subprocess, sys, hashlib, re, ast

class BaseHandler:

    def __init__( self, args ):
        self._start_time = time.time()
        self._args = args
        self._builder_root = os.path.abspath( os.path.join( __file__, "../..") )
        self._sdk_root = os.path.abspath( os.path.join( self._builder_root, ".." ) )
        self._verbose = self.get_arg( "verbose", False )

        # initialize logger
        logging.basicConfig(level=logging.DEBUG if self._verbose else logging.INFO,format="\033[0;37m[BUILDER] %(message)s" )
        self.log_info( f"Python version: {platform.python_version()}" )

        # get the builder home directory
        arg_home = self.get_arg( "home" )
        if arg_home:
            self._builder_home = os.path.abspath( arg_home )
            # check if the specified home directory exists, and it is a valid directory
            if not (os.path.exists( self._builder_home ) and os.path.isdir( self._builder_home )):
                raise Exception( f"The specified builder home must exist: {arg_home}" )
        else:
            self._builder_home = os.path.join( self._builder_root, ".builder" )
        self.log_info( f"Builder home: {self._builder_home}" )

        # create the conan env
        self._conan_env = os.environ
        self._conan_env.update({
            "CONAN_USER_HOME": self.conan_home,
        })

        # read existing config file
        self._builder_configuration = BuilderConfiguration( self, os.path.join( self.builder_home, "builder_configuration.json" ) )
        self._builder_imports = BuilderImports( self, os.path.join( self.builder_home, "builder_imports.json" ) )

    @property
    def start_time(self):
        return self._start_time

    @property
    def elapsed_time(self):
        return time.time() - self._start_time

    @property
    def verbose(self):
        return self._verbose

    @property
    def sdk_root(self):
        return self._sdk_root

    @property
    def builder_root(self):
        return self._builder_root

    @property
    def builder_home(self):
        return self._builder_home

    @property
    def conan_home(self):
        return f"{self._builder_home}"

    @property
    def conan_env(self):
        return self._conan_env

    @property
    def gradle_home(self):
        return f"{self._builder_home}/.gradle"

    @property
    def builder_configuration(self):
        return self._builder_configuration

    @property
    def builder_imports(self):
        return self._builder_imports

    def execute_command( self, command ):
        handler_method = getattr( self, command if command else "run" )
        if not handler_method:
            raise Exception(f"Invalid command method: {command}")
        handler_method()
        # log completion time
        self.log_success( f"elapsed time: {self.get_duration_str( self.elapsed_time )}" )

    def run(self):
        self.log_warning("nothing to execute.")

    #
    # argument functions
    #
    def get_arg( self, name, def_val = None ):
        val = getattr( self._args, name, None )
        return val if val else def_val

    def join_arg_list( self, arg ):
        value = []
        for next in self.get_arg(arg,[]):
            for item in next:
                value.extend( [it for it in item.split(",") if it and not it in value] )
        return value

    #
    # time functions
    #
    def get_duration_str( self, dur_secs ):
        hours, remainder = divmod( dur_secs, 3600)
        minutes, seconds = divmod( remainder, 60 )
        if minutes < 1:
            return f"{int(seconds)}s"
        elif hours < 1:
            return f"{int(minutes)}m {int(seconds)}s"
        else:
            return f"{int(hours)}h {int(minutes)}m {int(seconds)}s"

    #
    # logging functions
    #
    def log_direct( self, msg ):
        logging.info( msg )

    def log_debug( self, msg, strong = False ):
        logging.debug( f"\033[{1 if strong else 0};37mDEBUG: {msg}" )

    def log_info( self, msg, strong = False ):
        logging.info( f"\033[{1 if strong else 0};37mINFO: {msg}" )

    def log_warning( self, msg, strong = False ):
        logging.warning( f"\033[{1 if strong else 0};35mWARNING: {msg}" )

    def log_error( self, msg, strong = True ):
        logging.error( f"\033[{1 if strong else 0};31mERROR: {msg}" )

    def log_success( self, msg, strong = True ):
        logging.info( f"\033[{1 if strong else 0};32mSUCCESS: {msg}" )

    def log_exception( self, msg, strong = True ):
        logging.exception( f"\033[{1 if strong else 0};31mERROR: {msg}" )

##
## BuilderSettingsFile class
##
class BuilderSettingsFile:
    def __init__( self, handler, file ):
        self._handler = handler
        self._file = file
        self._parent = os.path.abspath( os.path.join( self._file, os.path.pardir ) )
        self._data = None
        self.load()

    @property
    def version(self):
        return 1

    @property
    def name(self):
        return "settings"

    @property
    def verbose(self):
        return self._handler.verbose

    @property
    def sdk_root(self):
        return self._handler.sdk_root

    @property
    def data(self):
        return self._data

    def relpath( self, path ):
        return os.path.relpath( path, self._parent )

    def abspath( self, path ):
        return path if os.path.isabs( path ) else os.path.abspath( os.path.join( self._parent, path ) )

    def initialize(self):
        self._data = {
            "version": self.version
        }

    def load(self):
        # validate the setting file exists and is the correct version
        if os.path.exists( self._file ):
            with open( self._file, "r" ) as file:
                self._data = json.load( file )
            # validate the configuration
            if not ("version" in self._data and self._data["version"] == self.version):
                self._handler.log_info(f"Invalid builder {self.name} version...creating!")
                self._data = None
        else:
            self._handler.log_info(f"Builder {self.name} file doesn't exist...creating!")
        # initialize the settings data if needed
        if not self._data:
            self.initialize()
            self.save()
        
    def save(self):
        os.makedirs( os.path.dirname( self._file ), exist_ok=True )
        with open( self._file, "w" ) as file:
            json.dump( self._data, file, indent=3 )

##
## BuilderConfiguration class
##
class BuilderConfiguration(BuilderSettingsFile):

    @property
    def name(self):
        return "configuration"

    @property
    def search_paths(self):
        return [            
            os.path.join( self.sdk_root, "conan", "recipes" ),
            os.path.join( self.sdk_root, "modules" ),
            os.path.join( self.sdk_root, "extensions" ),
            os.path.join( self.sdk_root, "tools" )
        ]

    @property
    def import_search_paths(self):
        return self._handler.builder_imports.search_paths

    @property
    def packages(self):
        return self.data["packages"]

    def initialize(self):
        super().initialize()
        # initial configuration data
        self.data["packages"] = {}

    def configure(self,include_paths = []):
        # initialize the conan configuration data
        self._handler.log_info("Configuring Conan...")
        subprocess.run( ["conan", "config", "init"], env=self._handler.conan_env, capture_output=not self.verbose, check=True )
        # if os is linux then force compiler.libcxx to libstdc++11
        if sys.platform.startswith("linux"):
            subprocess.run( ["conan", "profile", "update", "settings.compiler.libcxx=libstdc++11", "default"], env=self._handler.conan_env, capture_output=not self.verbose, check=True )

        # install configuration data from search path locations
        config_paths = [self._handler.sdk_root]
        config_paths.extend( include_paths )
        for next in config_paths:
            config_profile_path = os.path.abspath( os.path.join( next, "conan", "config" ) )
            if os.path.exists( config_profile_path ):
                self._handler.log_info( f"Installing Conan configuration: {config_profile_path}" )
                subprocess.run( ["conan", "config", "install", "-t", "dir", config_profile_path], env=self._handler.conan_env, capture_output=not self.verbose, check=True )

    def has_package( self, pkg_name ):
        return pkg_name in self.packages

    def get_package( self, pkg_name ):
        if pkg_name in self.packages:
            return self.packages[pkg_name]
        else:
           raise Exception(f"Invalid package: {pkg_name}")

    def get_package_path( self, pkg_name ):
        return self.abspath( self.get_package(pkg_name)["path"] )

    def remove_package( self, pkg_name ):
        if pkg_name in self.packages:
            del self.packages[pkg_name]

    def find_packages( self, pattern ):
        found_packages = []
        if pattern in self.packages:
            found_packages.append( pattern )
        elif f"aac-module-{pattern}" in self.packages:
            found_packages.append( f"aac-module-{pattern}" )
        else:
            found_packages.extend( [pkg_name for pkg_name in self.packages.keys() if re.match(f"^{pattern}$".replace("*",".*"),pkg_name)] )
        return found_packages

    def get_package_name( self, recipe_path ):
        for next in self.packages:
            if self.get_package_path(next) == recipe_path:
                return next
        return None

    def find_recipes_in_path( self, search_path, relative = True, depth = 3 ):
        package_recipes = []
        abs_path = self.abspath( search_path )
        if not os.path.exists(abs_path):
            raise Exception(f"Search path does not exists: {abs_path}")
        for entry in os.scandir( abs_path ):
            if entry.is_file():
                if entry.name == "conanfile.py":
                    package_recipes.append( entry.path )
            elif depth > 0 and entry.is_dir():
                package_recipes.extend( self.find_recipes_in_path( entry.path, depth - 1 ) )
        return package_recipes

    def find_recipes( self, include_paths = [] ):
        search_paths = self.search_paths + self.import_search_paths + include_paths
        # find all of the package recipe files in the search paths
        package_recipes = []
        for next in search_paths:
            package_recipes.extend( [path for path in self.find_recipes_in_path(next) if not path in package_recipes] )
        return package_recipes

    def md5( self, filename ):
        hash_md5 = hashlib.md5()
        with open( filename, "rb" ) as file:
            for chunk in iter(lambda: file.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()        
    
    def create_package( self, recipe_path ):

        pkg_name = None

        # create the package entry
        entry = {
            "path": self.relpath( recipe_path ),
            "checksum": self.md5( recipe_path )
        }

        # get the builder_info from the recipe
        builder_info = subprocess.check_output( ["conan", "inspect", recipe_path, "--raw", "builder_info"], env=self._handler.conan_env ).decode()
        # convert the settings into an object
        if builder_info:
            builder_info_obj = ast.literal_eval( builder_info )
            if type(builder_info_obj) == dict:
                entry["info"] = builder_info_obj
                # get the pkg name
                if "name" in builder_info_obj:
                    pkg_name = builder_info_obj["name"]

        if not pkg_name:
            # get the name from the recipe
            pkg_name = subprocess.check_output( ["conan", "inspect", recipe_path, "--raw", "name"], env=self._handler.conan_env ).decode()
            if not pkg_name: 
                raise Exception( f"Error inspecting conan package: {recipe_path}" )

        return pkg_name, entry

    def prune_packages( self, recipe_list = [] ):
        for pkg_name in self.packages.copy():
            # check to see if configured package is in the discovered conan package list
            # and prune it if not present any more...
            if not self.get_package_path(pkg_name) in recipe_list:
                self._handler.log_info(f"Pruning package: {pkg_name}",True)
                del self.packages[pkg_name]

    def export_package( self, pkg_name ):
        if pkg_name in self.packages:
            self._handler.log_info( f"Exporting package: {pkg_name}" )
            subprocess.run( ["conan", "export", self.get_package_path(pkg_name)], env=self._handler.conan_env, capture_output=not self.verbose, check=True )
        else:
            raise Exception(f"Invalid package: {pkg_name}")

    def export( self, forced_packages = [], prune = True, include_paths = [] ):

        # find all of the package recipes in the search path
        package_recipes = self.find_recipes( include_paths )

        # prune packages from the configuration that are no longer found
        if prune: 
            self.prune_packages( package_recipes )
                    
        # collect all of the packages that need to be exported
        added_recipes = []
        exported_packages = []

        # add packages that are not in the configuration
        for next in package_recipes:
            pkg_name = self.get_package_name( next )
            if pkg_name:
                if not self.packages[pkg_name]["checksum"] == self.md5(next):
                    self._handler.log_warning(f"Package checksums do not match: {pkg_name}... re-exporting!")
                    added_recipes.append( next )
            else:
                added_recipes.append( next )

        if added_recipes:
            # sort the packages by directory location
            added_recipes.sort()
            # aac-sdk-tools has to be processed first!
            aac_sdk_tools_recipe = os.path.join( self._handler.sdk_root, "conan", "recipes", "aac-sdk-tools", "conanfile.py" )
            if aac_sdk_tools_recipe in added_recipes:
                added_recipes.insert( 0, added_recipes.pop( added_recipes.index( aac_sdk_tools_recipe ) ) )
            # proccess each of the added packages
            for next in added_recipes:
                pkg_name, builder_info = self.create_package( next )
                # add the package to the configuration
                self.packages[pkg_name] = builder_info
                # export the package to the conan cache
                self.export_package( pkg_name )
                # append the package to the exported package list
                exported_packages.append( pkg_name )

        # resolve all of the specified forced packages
        forced_packages_resolved = []
        for next in forced_packages:
            forced_packages_resolved.extend( [pkg for pkg in self.find_packages( next ) if not (pkg in forced_packages_resolved or pkg in exported_packages)] )
                
        # export the resolved forced packages
        for next in forced_packages_resolved:
            self.export_package( next )

        # save the configuration
        self.save()

        # return the list of forced packages 
        return forced_packages_resolved

##
## BuilderImports class
##
class BuilderImports(BuilderSettingsFile):

    @property
    def name(self):
        return "imports"

    # @property
    # def imports(self):
    #     return self.data["imports"].items()

    @property
    def search_paths(self):
        enabled_search_paths = []
        for next in self.data["imports"].values():
            if next["enabled"]:
                enabled_search_paths.append( self.abspath(next["path"]) )
        return enabled_search_paths 

    def initialize(self):
        super().initialize()
        # initial imports data
        self.data["imports"] = {}

    def find_imports( self, pattern = "*" ):
        return [name for name in self.data["imports"].keys() if re.match(f"^{pattern}$".replace("*",".*"),name)]

    def get( self, name ):
        if not name in self.data["imports"]:
            raise Exception(f"Import not found: {name}")
        return self.data["imports"][name]

    def add( self, name, path, search_depth = 3 ):
        if name in self.data["imports"]:
            raise Exception(f"Import already exists: {name}")
        if not os.path.exists( path ):
            raise Exception(f"Import path does not exist: {path}")
        if not os.path.isdir( path ):
            raise Exception(f"Import path is not a directory: {path}")
        # create the new import entry
        entry = {
            "path": self.relpath(path),
            "enabled": True,
        }
        # add entry to builder imports
        self._handler.log_info(f"Adding import: {name}")
        self.data["imports"][name] = entry
        # save the imports
        self.save()

    def remove( self, name ):
        # remove the import entry
        if name in self.data["imports"]:
          self._handler.log_info(f"Removing import: {name}")
          del self.data["imports"][name]
        else:
            raise Exception(f"Import not found: {name}")
        # save the imports
        self.save()

    def set_enabled( self, name, enabled ):
        entry = self.get(name)
        entry["enabled"] = enabled
        self._handler.log_info(f"Setting import state ({name}): {'Enabled' if enabled else 'Disabled'}")
        # save the imports
        self.save()
