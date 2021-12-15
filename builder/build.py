#!/usr/bin/python3
import logging, argparse, importlib, sys

def parse_arguments():

    parser = argparse.ArgumentParser( description="AutoSDK Builder" )
    subparsers = parser.add_subparsers(title="commands")
    
    add_build_arguments( subparsers.add_parser("build", help="builds auto sdk components") )
    add_clean_arguments( subparsers.add_parser("clean", help="cleans builder cache") )
    add_configure_arguments( subparsers.add_parser("configure", help="builder configuration") )
    add_imports_arguments( subparsers.add_parser("imports", help="manages external search paths for packages") )

    # set the default command
    set_default_command( parser, "build" )

    try:
        args = parser.parse_args()
        # get the handler module
        if not "handler" in args:
            raise Exception("Invalid command!")
        handler = args.handler
        handler_module = importlib.import_module( f"pylib.{handler}_cmd" )
        if not handler_module:
            raise Exception(f"Invalid command handler: {handler}")
        # get the command method from the handler class
        command = getattr( args, "command", None )
        # create the command handler
        handler_inst = handler_module.BuilderHandler( args )
        # execute the handler method
        handler_inst.execute_command( command )

    except Exception as ex:
        if check_arg( ["-v","--verbose"] ):
            logging.exception( f"\033[1;31mERROR: {ex}" )
        else:
            logging.error( f"\033[1;31mERROR: {ex}" )

    #reset the terminal font
    print("\033[0m")

def add_common_arguments( parser ):

    # common.home
    parser.add_argument( "--home", "--builder-home",
        metavar="PATH",
        default=None,
        help="override builder home path"
    )
    # common.verbose
    parser.add_argument( "-v", "--verbose",
        action="store_true",
        default=False,
        help="enable verbose logging"
    )

def add_build_arguments(parser):
    parser.set_defaults( handler="build" )
    add_common_arguments(parser)
    # build.platform
    parser.add_argument( "-p", "--platform",
        metavar="PLATFORM",
        default=None,
        help="target platform - android,qnx,etc."
    )
    # build.arch
    parser.add_argument( "-a", "--arch",
        metavar="ARCH",
        choices=["x86","x86_64","armv7hf","armv8"],
        help="target architecture"
    )
    # build.debug
    parser.add_argument( "-g","--debug",
        action="store_true",
        default=False,
        help="specify debug build type"
    )
    # build.modules
    parser.add_argument( "-m","--modules",
        metavar="MODULE",
        action="append",
        nargs="+",
        help="list of modules to build"
    )
    # build.name
    parser.add_argument( "-n", "--name",
        help="optional package identifier"
    )
    # build.accept_licenses
    parser.add_argument( "-y", "--accept-licenses",
        action="store_true",
        default=False,
        help="auto-accept licenses"
    )
    # build.force
    parser.add_argument( "-f", "--force",
        metavar="PACKAGE",
        action="append",
        nargs="+",
        default=None,
        help="force export and build package"
    )
    # build.include
    parser.add_argument( "-i", "--include",
        metavar="PATH",
        action="append",
        nargs="+",
        help="add include path to conan configuration"
    )
    # build.option
    parser.add_argument( "-o", "--conan-option",
        metavar="OPTION",
        action="append",
        help="specify a conan build option"
    )
    # build.settings
    parser.add_argument( "-s", "--conan-setting",
        metavar="SETTING",
        action="append",
        help="specify a conan build setting"
    )
    # build.with_aasb
    parser.add_argument( "--with-aasb", "--aasb", 
        action="store_true",
        default=True,
        help="include aasb messages (default: True)"
    )
    parser.add_argument( "--no-aasb", 
        dest="with_aasb", 
        action="store_false",
        # help="don't include aasb messages"
    )
    # build.with_docs
    parser.add_argument( "--with-docs", "--docs", 
        action="store_true",
        default=True,
        help="include docs (default: True)"
    )
    parser.add_argument( "--no-docs", 
        dest="with_docs", 
        action="store_false",
        # help="don't include docs"
    )
    # build.with_unit_tests 
    parser.add_argument( "--with-unit-tests", "--unit-tests", 
        action="store_true",
        default=False,
        help="include unit tests (default: False)"
    )
    parser.add_argument( "--no-unit-tests", 
        dest="with_unit_tests", 
        action="store_false",
        # help="don't include unit tests"
    )
    # build.with_samplepp
    parser.add_argument( "--with-sampleapp", "--sampleapp", 
        action="store_true",
        default=False,
        help="include sample app (default: False)"
    )
    parser.add_argument( "--no-sampleapp", 
        dest="with_sampleapp", 
        action="store_false",
        # help="don't include sample app"
    )
    # build.sensitive_logs
    parser.add_argument( "--with-sensitive-logs", "--sensitive-logs", 
        action="store_true",
        default=False,
        help="emit sensitive data in debugging logs (default: False)"
    )
    parser.add_argument( "--no-sensitive-logs", 
        dest="with_sensitive_logs", 
        action="store_false",
        # help="don't emit sensitive data in debugging logs"
    )
    # build.latency_logs
    parser.add_argument( "--with-latency-logs", "--latency-logs", 
        action="store_true",
        default=False,
        help="emit latency data in debugging logs (default: False)"
    )
    parser.add_argument( "--no-latency-logs", 
        dest="with_latency_logs", 
        action="store_false",
        # help="don't emit latency data in debugging logs"
    )
    # build.output
    parser.add_argument( "--output",
        metavar="FILE",
        help="filename for output build archive"
    )
    # build.no_output
    parser.add_argument( "--no-output",
        action="store_true",
        default=False,
        help="don't create output package"
    )
    # build.skip_config
    parser.add_argument( "--skip-config",
        action="store_true",
        default=False,
        help="skip build configuration"
    )
    
def add_clean_arguments(parser):
    parser.set_defaults( handler="clean" )
    add_common_arguments( parser )

    # packages
    parser.add_argument( "pattern",
        help="pattern or package name"
    )
    # clean.skip_conan
    parser.add_argument( "--skip-conan",
        action="store_true",
        default=False,
        help="skips cleaning the conan cache"
    )
    # build.skip_gradle
    parser.add_argument( "--skip-gradle",
        action="store_true",
        default=False,
        help="skips cleaning the gradle cache"
    )

def add_configure_arguments(parser):
    parser.set_defaults( handler="configure" )
    # add configure sub-commands
    subparsers = parser.add_subparsers(title="commands")
    subparsers.required = True
    subparsers.dest = "command"
    # configure.init
    parser_configure_init = subparsers.add_parser("init", help="initializes the builder configuration")
    parser_configure_init.set_defaults( command="init" )
    add_common_arguments(parser_configure_init)
    # configure.export
    parser_configure_export = subparsers.add_parser("export", help="exports packages from the builder configuration")
    parser_configure_export.set_defaults( command="export" )
    add_common_arguments(parser_configure_export)
    # packages
    parser_configure_export.add_argument( "pattern",
        metavar="PATTERN",
        help="pattern or package name"
    )

def add_imports_arguments(parser):
    parser.set_defaults( handler="imports" )
    # add configure sub-commands
    subparsers = parser.add_subparsers(title="commands")
    subparsers.required = True
    subparsers.dest = "command"
    # imports.init
    parser_imports_init = subparsers.add_parser("init", help="initialize the imports configuration")
    parser_imports_init.set_defaults( command="init" )
    add_common_arguments(parser_imports_init)
    # imports.list
    parser_imports_list = subparsers.add_parser("list", help="lists the imported search paths")
    parser_imports_list.set_defaults( command="list" )
    add_common_arguments(parser_imports_list)
    # imports.add
    parser_imports_add = subparsers.add_parser("add", help="adds a new search path to the builder")
    parser_imports_add.set_defaults( command="add" )
    add_common_arguments(parser_imports_add)
    parser_imports_add.add_argument( "name",
        metavar="NAME",
        help="import name"
    )
    parser_imports_add.add_argument( "path",
        metavar="PATH",
        help="import search path"
    )
    # imports.remove
    parser_imports_remove = subparsers.add_parser("remove", help="removes a search path from the builder")
    parser_imports_remove.set_defaults( command="remove" )
    add_common_arguments(parser_imports_remove)
    parser_imports_remove.add_argument( "pattern",
        metavar="PATTERN",
        help="import name or pattern"
    )
    # imports.enable
    parser_imports_enable = subparsers.add_parser("enable", help="enables an import in the builder")
    parser_imports_enable.set_defaults( command="enable" )
    add_common_arguments(parser_imports_enable)
    parser_imports_enable.add_argument( "pattern",
        metavar="PATTERN",
        help="import name or pattern"
    )
    # imports.disable
    parser_imports_disable = subparsers.add_parser("disable", help="disables an import in the builder")
    parser_imports_disable.set_defaults( command="disable" )
    add_common_arguments(parser_imports_disable)
    parser_imports_disable.add_argument( "pattern",
        metavar="PATTERN",
        help="import name or pattern"
    )

def check_arg(args):
    for next in sys.argv[1:]:
        if next in args:
            return True
    return False

def set_default_command(parser, name):
    subparser_found = False
    if not check_arg( ["-h","--help"] ):
        for x in parser._subparsers._actions:
            if not isinstance(x, argparse._SubParsersAction):
                continue
            for sp_name in x._name_parser_map.keys():
                if sp_name in sys.argv[1:]:
                    subparser_found = True
        if not subparser_found:
            sys.argv.insert(1, name)

# check the python version meets minimum requirements
if sys.version_info[0] < 3 or (sys.version_info[0] == 3 and sys.version_info[1] < 7):
    raise Exception(f"Unsupported Python version: {sys.version} (require Python >=3.7)")

# run the argument parser
parse_arguments()