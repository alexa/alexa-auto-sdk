import os, logging
import yaml

class Parser:

    def __init__( self, model ):
        self.model = model

    def parse( self, input_folder, exports = False ):
        for (path, dirnames, filenames) in os.walk( input_folder ):
            for name in filenames:
                if name.endswith( ".a2ml" ) or name.endswith( ".a2m" ) or name.endswith( ".yaml" ) or name.endswith( ".yml" ):
                    self.parse_file( os.path.join( path, name ), exports )

    def parse_file( self, filename, exports ):
        logging.debug( "Parsing file: %s" % filename )
        with open( filename, "r" ) as file:
            data = yaml.safe_load( file )
            # validate that we were able to parse the yaml document
            if not data:
                raise Exception( "Invalid A2ML source file: %s" % filename )
            # get the data topic
            if not "topic" in data:
                raise Exception( "No topic defined for A2ML source file: %s" % filename )
            # get the data namespace
            if not "namespace" in data:
                raise Exception( "No namespace defined for A2ML source file: %s" % filename )
            if not "path" in data:
                raise Exception( "No path defined for A2ML source file: %s" % filename )
            # get or create the interface model
            name = data["topic"]
            version = str(data["version"]) if "version" in data else self.model.version
            key = f"{name}:{version}"
            interface = self.model.get_interface( key )
            if interface:
                raise Exception( f"Invalid re-definition of interface ({key}) in A2ML source file: {filename}" )
            interface = self.model.add_interface(data, exports )

            # iterate over root level messages if there are any
            if "messages" in data:
                for next in data["messages"]:
                    # create the model element and add it to the data definition map
                    interface.add_message( next )
            # iterate over root level enums if there are any
            if "types" in data:
                for next in data["types"]:
                    # create the model element and add it to the data definition map
                    interface.add_type( next )
