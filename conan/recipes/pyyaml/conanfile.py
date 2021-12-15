from conans import ConanFile, tools
import os

class PyYAMLConanFile(ConanFile):

    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"

    name = "pyyaml"
    version = "5.4.1.1"
    no_copy_source = True

    def source(self):
        tools.get( f"https://github.com/yaml/pyyaml/archive/refs/tags/{self.version}.tar.gz" )

    def package(self):
        pyyaml_path = f"pyyaml-{self.version}" 
        self.copy( "*", src=os.path.join( pyyaml_path, "lib3" ), dst=os.path.join( pyyaml_path, "lib" ) )

    def package_info(self):
        pyyaml_path = f"pyyaml-{self.version}"
        self.env_info.PYTHONPATH.append( os.path.join( self.package_folder, pyyaml_path, "lib" ) )
