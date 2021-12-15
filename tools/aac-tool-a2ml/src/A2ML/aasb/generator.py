import os, logging
from Cheetah.Template import Template

class Generator:

    type_map =  {
        "string": {
            "type": "std::string",
            "includes": ["<string>"]
        },
        "int": {
            "type": "int",
        },
        "long": {
            "type": "long",
        },
        "int32": {
            "type": "int32_t",
        },
        "int64": {
            "type": "int64_t",
        },
        "float": {
            "type": "float",
        },
        "double": {
            "type": "double",
        },
        "bool": {
            "type": "bool",
        },
        "dict": {
            "type": "std::unordered_map<std::string,std::string>",
            "includes": ["<string>","<unordered_map>"]
        },
        "list": {
            "type": "std::vector<std::string>",
            "includes": ["<string>","<vector>"]
        }
    }

    message_include_path_root = "AASB/Message/"

    def __init__( self, model ):
        self.model = model
        self.template_path = os.path.abspath( os.path.join( __file__, "..", "templates" ) )

    def generate( self, output_folder ):
        self.output_folder = output_folder
        for next in self.model.get_exported_interfaces():
            self.generate_interface( next )

    def generate_interface( self, interface ):
        # generate all of the message headers
        for next in interface.get_message_names():
            message = interface.get_message( next )
            if not message:
                raise Exception( "Unknown message type: %s" % next )
            self.generate_type( message )
        #generate all of the type headers
        for next in list(interface.get_type_names( "enum" )) + list(interface.get_type_names( "struct" )):
            type_def = interface.get_type( next )
            if not type_def:
                raise Exception( "Unknown type: %s" % next )
            self.generate_type( type_def )

    def get_message_relative_path( self, type_def ):
        if type_def.interface.path.startswith( self.message_include_path_root ):
            # strip the message include root path if defined
            relative_interface_path = type_def.interface.path[len(self.message_include_path_root):]
        else:
            relative_interface_path = type_def.interface.path
        return relative_interface_path

    def get_message_include_path( self, type_def ):
        if type_def.interface.path.startswith( self.message_include_path_root ):
            include_path = type_def.interface.path
        else:
            include_path = f"{self.message_include_path_root}{type_def.interface.path}"
        return include_path

    def generate_type( self, type_def ):
        args = {
            "generator": self,
            "type": type_def,
        }
        header_template = Template( file=os.path.join( self.template_path, "header.tmpl" ), searchList=[args] )
        footer_template = Template( file=os.path.join( self.template_path, "footer.tmpl" ), searchList=[args] )
        type_header_template = Template( file=os.path.join( self.template_path, "%s.tmpl" % type_def.type ), 
            searchList=[args,{ "header": header_template, "footer": footer_template }] )
        # write the output file
        relative_path = self.get_message_relative_path( type_def )
        # message versioning not supported yet!
        if type_def.interface.version != self.model.version:
            raise Exception( f"Interface version does not match expected message version: specified={type_def.interface.version}, expected={self.model.version}" )
            #output_file_path = os.path.join( self.output_folder, self.message_include_path_root, type_def.interface.version, relative_path )
        else:
            output_file_path = os.path.join( self.output_folder, self.message_include_path_root, relative_path )

        os.makedirs( output_file_path, exist_ok=True )
        with open( os.path.join( output_file_path, "%s.h" % type_def.name ), "w" ) as file:
            file.write( str( type_header_template ) )

    def get_header_guard( self, message ):
        return ("%s_H" % message.symbol.replace(".","_")).upper()

    def split_namespace_name( self, name ):
        components = name.split( "." )
        if len( components ) == 1:
            return (None,name)
        else:
            namespace = ".".join( components[0:-1] )
            name = components[-1]
            return (namespace, name)

    def get_type( self, value_def ):
        if value_def.type in self.type_map:
            return self.type_map[value_def.type]["type"]
        # check for list type
        elif value_def.type.startswith( "list:" ):
            iface_type = value_def.type.split(":",1)[1]
            return "std::vector<%s>" % self.get_inteface_type( iface_type, value_def.interface )
        # check if type is specifed in the model
        else:
            return self.get_inteface_type( value_def.type, value_def.interface )

    def get_value( self, value_def, value ):
        if value_def.type == "string":
            return "\"%s\"" % value
        else:
            return value

    def get_inteface_type( self, name, interface ):
        iface_ns, iface_name = self.split_namespace_name( name )
        return iface_name if not iface_ns or iface_ns == interface.namespace else name.replace(".","::")

    def get_includes( self, type_def, full_path = False ):
        includes = { "<string>", "<nlohmann/json.hpp>" }
        # specifal includes for specifi types
        if type_def.type == "message":
            includes.add("<AASB/Utils/UUID.h>")
        elif type_def.type == "enum":
            includes.add("<AASB/Utils/StringUtils.h>")
        # create the type list from the values using comprehension
        type_list = [next.type for next in type_def.values]
        # iterate over the payload arguments to check if type is defined
        # as an class or enumeration type
        while type_list:
            next = type_list.pop()
            # if this is a constructed data type then separate the components
            # and add them to the values
            if ":" in next:
                type_list.extend( next.split(":",1) )
            elif next in self.type_map:
                if "includes" in self.type_map[next]:
                    includes.update( self.type_map[next]["includes"] )
            # check if the type is actually an alias
            elif type_def.interface.has_type( next, "alias" ):
                alias_type = type_def.interface.get_type( next, "alias" )
                type_list.append( alias_type.alias )
            else:
                namespace, name = self.split_namespace_name( next )
                if not namespace or namespace == type_def.namespace and not full_path:
                    includes.add( "\"%s.h\"" % name )
                else:
                    for next_iface in self.model.interfaces.values():
                        if next_iface.has_type( next ):
                            includes.add( "<%s/%s.h>" % (self.get_message_include_path(next_iface),name))
                            #includes.add( "<%s/%s.h>" % (next_iface.path,name))
        # sort the includes list
        include_list = list(includes)
        include_list.sort()
        return include_list

    def get_aliases( self, type_def ):
        alias_map = {}
        # create the type list from the values using comprehension
        type_list = [next.type for next in type_def.values]
        # iterate over the payload arguments to check if type is defined
        # as an class or enumeration type
        while type_list:
            next = type_list.pop()
            # if this is a constructed data type then separate the components
            # and add them to the values
            if ":" in next:
                type_list.extend( next.split(":",1) )
            elif next in self.type_map:
                pass
            # check if the type is actually an alias
            elif not next in alias_map and type_def.interface.has_type( next, "alias" ):
                alias_map[next] = type_def.interface.get_type( next, "alias" )
        # sort the includes list
        alias_list = list(alias_map.values())
        return alias_list
