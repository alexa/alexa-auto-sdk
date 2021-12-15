import logging

class A2MLModel:

    def __init__( self, version ):
        self.interfaces = {}
        self.version = version

    def add_interface( self, data, exports = False ):
        interface = A2MLInterface( data, self.version, exports )
        if A2MLModel.exists( interface.key, self.interfaces ):
            raise Exception( f"Interface already defined: {interface.key}" )
        # add the data definition to the map
        logging.debug( f"Adding interface: {interface.key}" )
        self.interfaces[interface.key] = interface
        return interface

    def get_exported_interfaces( self ):
        return [next for next in self.interfaces.values() if next.exports]

    def get_interface_names( self ):
        return self.interfaces.keys()

    def has_interface( self, name ):
        return A2MLModel.exists( name, self.interfaces )

    def get_interface( self, name ):
        return A2MLModel.get( name, self.interfaces )

    @classmethod
    def exists( cls, name, obj, check_type = None ):
        return name in obj and (check_type == None or type(obj[name]) is check_type)

    @classmethod
    def get( cls, name, obj, default = None, check_type = None, throw_if_fail = False ):
        value = obj[name] if cls.exists( name, obj, check_type ) else default
        if not value and throw_if_fail:
            raise Exception( "Get object attribute failed: %s" % name )
        return value

class A2MLNode:
    def __init__( self, data, parent = None ):
        if parent and not isinstance( parent, A2MLNode ):
            raise Exception( "Invalid parent type: was %s but expecting %s" % (type(parent),A2MLNode) )
        self.data = data
        self.parent = parent

    def exists( self, name, check_type = None ):
        return A2MLModel.exists( name, self.data, check_type )

    def get( self, name, default = None, check_type = None, throw_if_fail = False ):
        return A2MLModel.get( name, self.data, default, check_type, throw_if_fail )

    @property
    def interface( self ):
        if isinstance( self, A2MLInterface ):
            return self
        else:
            return self.parent.interface if self.parent else None

    @property
    def namespace( self ):
        return self.interface.namespace

    @property
    def version( self ):
        return self.interface.version

    @property 
    def name( self ):
        return self.get( "name", check_type=str )

    @property 
    def description( self ):
        return self.get( "desc", check_type=str )

class A2MLInterface( A2MLNode ):
    def __init__( self, data, version, exports = False):
        super( A2MLInterface, self ).__init__( data )
        self.messages = {}
        self.types = {}
        self.default_version = version
        self.exports = exports
        # versioned interfaces not currently supported
        if self.exists( "version" ):
            iface_ver = str(self.get("version",throw_if_fail=True))
            if iface_ver != self.default_version:
                logging.warn( f"Interface version does not match expected message version: interface={self.name}, specified={iface_ver}, expected={self.default_version} (forcing version={self.default_version})" )

    @property
    def topic( self ):
        return self.get( "topic", check_type=str, throw_if_fail=True )

    @property
    def name( self ):
        return self.topic

    @property
    def namespace( self ):
        return self.get( "namespace", check_type=str, throw_if_fail=True )

    @property
    def path( self ):
        return self.get( "path", check_type=str, throw_if_fail=True )

    @property
    def version( self ):
        # versioned interfaces not currently supported
        return self.default_version
        # if self.exists( "version" ):
        #     return str( self.get( "version", throw_if_fail=True ) )
        # else:
        #     return self.default_version

    @property
    def key( self ):
        return f"{self.name}:{self.version}"

    # messages
    def add_message( self, data ):
        message = A2MLMessage( data, self )
        if A2MLModel.exists( message.symbol, self.messages ):
            raise Exception( "Message already defined: %s" % message.symbol )
        # add the data definition to the map
        logging.debug( "Adding message: %s" % message.symbol )
        self.messages[message.symbol] = message
        return message

    def get_message_names( self, direction = None ):
        if direction:
            return [key for key in self.messages.keys() if self.messages[key].direction == direction]
        else:
            return self.messages.keys()

    def has_message( self, symbol ):
        return A2MLModel.exists( symbol, self.messages ) or A2MLModel.exists( "%s.%s" % (self.namespace,symbol), self.messages )

    def get_message( self, symbol ):
        message = A2MLModel.get( symbol, self.messages )
        return message if message else A2MLModel.get( "%s.%s" % (self.namespace,symbol), self.messages )

    # types
    def add_type( self, data ):
        type_def = A2MLType( data, self )
        if self.has_type( type_def.symbol ):
            raise Exception( "Type already defined: %s" % type_def.symbol )
        # add the type to the map
        logging.debug( "Adding %s type: %s" % (type_def.type,type_def.symbol) )
        if not type_def.type in self.types:
            type_map = {}
            type_map[type_def.symbol] = type_def
            self.types[type_def.type] = type_map
        else:
            self.types[type_def.type][type_def.symbol] = type_def
        return type_def

    def get_type_names( self, type = None ):
        if type:
            return self.types[type].keys() if type in self.types else []
        else:
            type_names = []
            for next in self.types:
                type_names.extend( self.get_type_names( next ) )
            return type_names

    def has_type( self, symbol, type = None ):
        norm_sym = symbol if "." in symbol else "%s.%s" % (self.namespace,symbol)
        if type:
            return A2MLModel.exists( norm_sym, self.types[type] ) if type in self.types else False
        else:
            for next in self.types:
                if self.has_type( norm_sym, next ):
                    return True
            return False

    def get_type( self, symbol, type = None ):
        norm_sym = symbol if "." in symbol else "%s.%s" % (self.namespace,symbol)
        if type:
            return A2MLModel.get( norm_sym, self.types[type] ) if type in self.types else None
        else:
            for next in self.types:
                type_def = self.get_type( norm_sym, next )
                if type_def:
                    return type_def
            return None
        
class A2MLMessage( A2MLNode ):

    # direction constants
    INCOMING = "incoming"
    OUTGOING = "outgoing"
    # messageType constants
    PUBLISH = "publish"
    REPLY = "reply"

    def __init__( self, data, interface ):
        super( A2MLMessage, self ).__init__( data, interface )
        self.type = "message"
        # create data types
        self.payload = []
        if self.exists( "payload", check_type=list ):
            for next in self.get( "payload" ):
                self.payload.append( A2MLValue( next, self ) )
        self.reply = []
        if self.messageType == A2MLMessage.REPLY:
            self.reply.extend( self.payload )
        elif self.exists( "reply", check_type=list ):
            for next in self.get( "reply" ):
                self.reply.append( A2MLValue( next, self ) )
        self.alt_name = None
        # check if the message data override the message name
        if self.exists( "name", check_type=str ):
            alt_name = self.get( "name" )
            # check if the overridden name is different from the original name
            if alt_name.endswith( "Message" ):
                if not alt_name == "%sMessage" % self.action:
                    self.alt_name = alt_name[:-7]
            elif not alt_name == self.action:
                self.alt_name = alt_name
        
    @property
    def name( self ):
        return "%sMessage" % (self.alt_name if self.alt_name else self.action)

    @property
    def symbol( self ):
        return "%s.%s" % (self.namespace,self.name)

    @property
    def topic( self ):
        return self.interface.topic

    @property
    def action( self ):
        return self.get( "action", check_type=str, throw_if_fail=True )

    @property
    def direction( self ):
        return self.get( "direction", check_type=str, throw_if_fail=True )

    @property
    def messageType( self ):
        return self.get( "messageType", check_type=str, default=A2MLMessage.PUBLISH )

    @property
    def values( self ):
        return self.payload + self.reply

    def set_reply( self, reply ):
        for next in reply:
            self.reply.append( A2MLValue( next, self ) )

class A2MLType( A2MLNode ):

    # type constants
    ENUM = "enum"
    STRUCT = "struct"
    ALIAS = "alias"

    def __init__( self, data, interface ):
        super( A2MLType, self ).__init__( data, interface )
        # check if this an alias type
        if not self.type in self.type_names:
            raise "Invalid interface type: was %s but expecting on of %s." % (self.type,self.type_names)
        # intialize the type values
        if self.exists( "values", check_type=list ):
            self.values = []
            for next in self.get( "values", check_type=list, throw_if_fail=True ):
                self.values.append( A2MLValue( next, self ) )
        else:
            self.values = None

    @property
    def type( self ):
        return self.get( "type", check_type=str, throw_if_fail=True )

    @property
    def symbol( self ):
        return "%s.%s" % (self.namespace,self.name)

    @property
    def alias( self ):
        return self.get( "alias", check_type=str )

    @property
    def type_names( self ):
        return [A2MLType.ENUM, A2MLType.STRUCT, A2MLType.ALIAS]

    def get_value_names( self ):
        return [next.name for next in self.values]  

class A2MLValue( A2MLNode ):

    # type constants
    STRING = "string"
    INTEGER = "int"
    FLOAT = "float"
    BOOLEAN = "bool"
    LIST = "list"
    DICTIONARY = "dict"

    def __init__( self, data, parent ):
        super( A2MLValue, self ).__init__( data, parent )

    @property
    def type( self ):
        return self.get( "type", default=A2MLValue.STRING )

    @property
    def value( self ):
        return self.get( "value" )

    @property
    def default( self ):
        return self.get( "default" )

    @property
    def required( self ):
        return self.default == None

    @property
    def optional( self ):
        return not self.required

    @property
    def example( self ):
        return self.get( "example" )

    def is_primitive( self ):
        return self.type in [self.STRING,self.INTEGER,self.FLOAT,self.BOOLEAN]

    def is_list( self ):
        return self.type == self.LIST or self.type.startswith(f"{self.LIST}:")

    def is_dict( self ):
        return self.type == self.DICTIONARY

    def is_enum( self ):
        type_def = self.resolve_type()
        return type_def and type_def.type == type_def.ENUM

    def is_struct( self ):
        type_def = self.resolve_type()
        return type_def and type_def.type == type_def.STRUCT

    def is_alias( self ):
        type_def = self.resolve_type()
        return type_def and type_def.type == type_def.ALIAS

    def resolve_type( self ):
        return self.interface.get_type( self.type[5:] if self.is_list() else self.type )
