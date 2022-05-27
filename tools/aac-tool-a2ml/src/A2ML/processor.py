from genericpath import exists
import os, platform, logging, tempfile, shutil, time, importlib, re
from .model import A2MLModel

class A2MLProcessor:

    def __init__( self, args ):
        self.start_time = time.time() #time.process_time()
        self.args = args
        self.generator_root = os.path.abspath( os.path.join( __file__, "../..") )
        self.sdk_root = os.path.abspath( os.path.join( self.generator_root, ".." ) )
        self.verbose = self.get_arg( "verbose", False )
        self.input_folders = None
        self.dependencies = None
        self.output_folder = None
        self.generator = None
        self.message_version = None
        self.model = None

        # initialize logger
        logging.basicConfig( level=logging.DEBUG if self.verbose else logging.INFO,format="\033[0;37m[A2DL] %(levelname)s: %(message)s" )
        logging.info( "Python version: %s" % platform.python_version() )

    def run( self ):
        # initialize the builder
        self.initialize_configuration()
        self.initialize_environment()
        # parse the input files
        self.parse()
        # build the specifed modules
        self.generate()
        # package the archive
        self.export()
        # log completion time
        logging.info( "SUCCESS! elapsed time: %s" % self.get_duration_str( time.time() - self.start_time ) )

    def get_arg( self, name, default = None ):
        return self.args[name] if name in self.args else default

    def initialize_configuration( self ):
        arg_input = self.get_arg( "input" )
        if not arg_input:
            raise Exception("Input argument rquired!")
        # set the input folder list
        self.input_folders = [os.path.abspath(next) for next in arg_input]
        for next in self.input_folders:
            if not (os.path.exists( next ) and os.path.isdir( next )):
                raise Exception("Invalid input argument: %s" % next)
        # set the dependency folder list
        arg_dependencies = self.get_arg( "dependencies" )
        self.dependencies = [os.path.abspath(next) for next in arg_dependencies]
        for next in self.dependencies:
            if not (os.path.exists( next ) and os.path.isdir( next )):
                raise Exception("Invalid dependencies argument: %s" % next)

        # dynamically load parser module
        arg_parser = self.get_arg( "parser" )
        if not arg_parser:
            raise Exception("Invalid parser argument: %s" % arg_parser)
        self.parser_module = importlib.import_module( "A2ML.%s.parser" % arg_parser.lower() )

        #dynamically load generator module
        arg_generator = self.get_arg( "generator" )
        if not arg_generator:
            raise Exception("Invalid generator argument: %s" % arg_generator)
        self.generator_module = importlib.import_module( "A2ML.%s.generator" % arg_generator.lower() )

        # aasb version
        arg_message_version = self.get_arg( "message_version" )
        if not arg_message_version:
            raise Exception("Message version not specified.")
        elif not re.match("^\d+\.\d+(\.\d+)?(-.+)?$",arg_message_version):
            raise Exception(f"Message version is not valid: {arg_message_version}")
        self.message_version = arg_message_version

        # if we are creating the output files then validate the output folder
        if not self.get_arg( "no_output", False ):
            arg_output = self.get_arg( "output", None )
            if not arg_output:
                raise Exception("Output directory must be specified!")
            output_path = os.path.abspath( arg_output )
            if os.path.exists( output_path ):
                if not os.path.isdir( output_path ):
                    raise Exception("Output path must be a directory!")
            else:
                parent_path = os.path.abspath( os.path.join( output_path, ".." ) )
                if not (os.path.exists( parent_path ) and os.path.isdir( parent_path )):
                    raise Exception("Output directory must exist!")
            self.output_folder = output_path

        # create the temporary output folder
        self.temp_output_folder = tempfile.TemporaryDirectory().name

    def initialize_environment( self ):
        pass

    def parse( self ):
        # create the model
        self.model = A2MLModel( self.message_version )
        # create the parser
        parser = self.parser_module.Parser( self.model )
        # load data definitions from all of the specified input folders
        for next in self.dependencies:
            parser.parse( next )
        for next in self.input_folders:
            parser.parse( next, True )

    def generate( self ):
        # create the generator
        generator = self.generator_module.Generator( self.model )
        generator.generate( self.temp_output_folder )

    def export( self ):
        if self.output_folder:
            for (path, dirnames, filenames) in os.walk( self.temp_output_folder ):
                for name in filenames:
                    src_path = os.path.join( path, name )
                    rel_path = os.path.relpath( src_path, self.temp_output_folder )
                    dest_path = os.path.join( self.output_folder, rel_path )
                    dest_parent = os.path.abspath( os.path.join( dest_path, ".." ) )
                    os.makedirs( dest_parent, exist_ok=True )
                    shutil.copy( src_path, dest_path )

    def get_duration_str( self, dur_secs ):
        hours, remainder = divmod( dur_secs, 3600)
        minutes, seconds = divmod( remainder, 60 )
        if minutes < 1:
            return "%ss" % int(seconds)
        elif hours < 1:
            return "%sm %ss" % (int(minutes), int(seconds))
        else:
            return "%sh %sm %ss" % (int(hours), int(minutes), int(seconds))
