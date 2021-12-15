import os, logging
from Cheetah.Template import Template

class Generator:

    def __init__( self, model ):
        self.model = model
        self.template_path = os.path.abspath( os.path.join( __file__, "..", "templates" ) )

    def generate( self, output_folder ):
        self.output_folder = output_folder
        for next in self.model.get_exported_interfaces():
            self.generate_interface( next )

    def generate_interface( self, interface ):
        template_file = os.path.join( self.template_path, "interface.tmpl" )
        a2ml_template = Template( file=template_file, searchList=[{
            "generator": self,
            "interface": interface
        }])
        # write the output file
        # message versioning not supported yet!
        if interface.version != self.model.version:
            raise Exception( f"Interface version does not match expected message version: specified={interface.version}, expected={self.model.version}" )
        else:
            output_folder = self.output_folder
        #output_folder = self.output_folder if interface.version == self.model.version else os.path.join( self.output_folder, interface.version )
        output_file_path = os.path.join( output_folder, "%s.md" % interface.name.replace(".","_") )
        os.makedirs( output_folder, exist_ok=True )
        with open( output_file_path, "w" ) as file:
            file.write( str( a2ml_template ) )
