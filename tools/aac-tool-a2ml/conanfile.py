from conans import ConanFile
from six import StringIO
import os, logging

class AacToolA2MLConanFile(ConanFile):
    name = "aac-tool-a2ml"
    exports = "*"

    def set_version(self):
        if self.version == None:
            self.version = "dev"

    @property
    def package_folder_name(self):
        return f"{self.name}-{self.version}"

    def requirements(self):
        self.requires(f"cheetah/3.2.6@aac-sdk/{self.version}")
        self.requires(f"pyyaml/5.4.1.1@aac-sdk/{self.version}")

    def package(self):
        self.copy( "*", dst=self.package_folder_name )

    def package_info(self):
        self.env_info.PYTHONPATH.append( os.path.join( self.package_folder, self.package_folder_name, "src" ) )
