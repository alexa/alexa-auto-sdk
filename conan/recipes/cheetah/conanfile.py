from conans import ConanFile, tools
import os

class CheetahConanFile(ConanFile):

    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"

    name = "cheetah"
    version = "3.2.6"
    exports = "*"
    no_copy_source = True

    def source(self):
        tools.get( f"https://github.com/CheetahTemplate3/cheetah3/archive/refs/tags/{self.version}.tar.gz" )

    def package(self):
        cheetah_path = f"cheetah3-{self.version}"
        self.copy( "*", src=os.path.join( cheetah_path, "Cheetah" ), dst=os.path.join( cheetah_path, "lib", "Cheetah" ) )
        self.copy( "*", src=os.path.join( cheetah_path, "bin" ), dst=os.path.join( cheetah_path, "bin" ) )
        # apply patches
        tools.patch(base_path=os.path.join(self.package_folder,cheetah_path), patch_file=os.path.join(self.recipe_folder,"patches","fix_namemapper_warning.patch") )

    def package_info(self):
        cheetah_path = f"cheetah3-{self.version}"
        self.env_info.PYTHONPATH.append( os.path.join( self.package_folder, cheetah_path, "lib" ) )
        self.env_info.PATH.append( os.path.join( self.package_folder, cheetah_path, "bin" ) )
