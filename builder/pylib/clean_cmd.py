import subprocess
from pylib.common import BaseHandler

class BuilderHandler(BaseHandler):

    def run( self ):
        clean_packages = self.builder_configuration.find_packages( self.get_arg("pattern") )
        self.log_info( "Cleaning cached builder data..." )
        for next in clean_packages:
            # remove package from builder settings
            self.builder_configuration.remove_package( next )
            # remove package from conan
            if not self.get_arg( "skip_conan", False ):
                subprocess.run( ["conan", "remove", next, "-f"], env=self.conan_env, capture_output=not self.verbose, check=True )
            # save the settings state
            self.builder_configuration.save()
