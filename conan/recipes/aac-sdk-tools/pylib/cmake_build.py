import os, shutil, logging
from conans import tools, RunEnvironment, CMake
from conans.tools import Version
from pylib import utils

def imports( obj, dest_folder ):
    aac_sdk_tools_path = obj.python_requires["aac-sdk-tools"].path
    source_path = obj._source_path

    # copy cmake files from tools package
    dest_cmake_path = os.path.join( dest_folder, "cmake" )

    # create the cmake directory
    os.makedirs( dest_cmake_path, exist_ok=True )

    # get cmake files from tool package
    for next in utils.list_files( aac_sdk_tools_path, "cmake", "cmake", False ):
        shutil.copy( next, dest_cmake_path )

    # copy any cmake scripts from the current module and any
    # of the module dependencies source directories
    obj.copy( "*", src=os.path.join( source_path, "cmake" ), dst=dest_cmake_path, keep_path=False )

    if not obj.module_overrides_cmake:
        cmake_file = os.path.join( dest_folder, "CMakeLists.txt" )
        shutil.copyfile( os.path.join( aac_sdk_tools_path, "CMakeLists.txt"), cmake_file )
        # replace the library name in the top level cmake
        tools.replace_in_file( cmake_file, "AutoSdkModule", obj._module_library_name )

    # create the aacbuildinfo.cmake file
    aac_build_info_path = os.path.join( dest_folder, "aacbuildinfo.cmake" )

    with open( aac_build_info_path, "w" ) as out:
        cmake_file_path_sep = "\n    "
        system_libs = []
        # write engine sources cmake variable list
        if obj.options.get_safe( "with_engine", default=False ):
            out.write( f"set(AAC_ENGINE_SOURCES{cmake_file_path_sep}" )
            out.writelines( cmake_file_path_sep.join( utils.list_files( source_path, "engine/src", "cpp", False ) ) )
            out.write( ")\n" )
        # write platform sources cmake variable list
        if obj.options.get_safe( "with_platform", default=False ):
            out.write( f"set(AAC_PLATFORM_SOURCES{cmake_file_path_sep}" )
            out.writelines( cmake_file_path_sep.join( utils.list_files( source_path, "platform/src", "cpp", False ) ) )
            out.write( ")\n" )
        # write aasb sources cmake variable list
        if obj.options.get_safe( "with_aasb", default=False ):
            out.write( f"set(AAC_AASB_SOURCES{cmake_file_path_sep}" )
            aasb_sources = utils.list_files( source_path, "aasb/src", "cpp", False )
            logging.info( f"Listing AASB message sources in {dest_folder}" )
            aasb_message_sources = utils.list_files( dest_folder, "aasb-messages/src", "cpp", False )
            logging.info( f"AASB message sources: {aasb_message_sources}" )
            out.writelines( cmake_file_path_sep.join( aasb_sources + aasb_message_sources ) )
            out.write( ")\n" )
        # write messages cmake definitions
        if obj.options.get_safe( "with_messages", default=False ):
            out.write( "add_definitions(-DHAS_AASB_MESSAGES)\n" )
        # write aasb version cmake definitions
        out.write( f"add_definitions(-DMESSAGE_VERSION=\"{obj.options.message_version}\")\n" )
        # write jni sources cmake variable list
        if obj.options.get_safe( "with_jni", default=False ):
            out.write( f"set(AAC_JNI_SOURCES{cmake_file_path_sep}" )
            out.writelines( cmake_file_path_sep.join( utils.list_files( source_path, "android/src/main/cpp/src", "cpp", False ) ) )
            out.write( ")\n" )
            system_libs.extend( ["android", "log"] )

        # Write system libs
        system_libs.extend( obj._required_system_libs )
        if system_libs:
            out.write( f"set(AAC_SYSTEM_LIBS {';'.join(system_libs)})\n" )

        # write extra sources from recipe
        module_sources = obj._module_sources
        if module_sources:
            out.write( f"set(AAC_MODULE_SOURCES{cmake_file_path_sep}" )
            out.writelines( cmake_file_path_sep.join( module_sources ) )
            out.write( ")\n" )
        # write unit test sources cmake variable list
        if obj.options.get_safe( "with_unit_tests", default=False ):
            out.write( f"set(AAC_UNIT_TEST_FRAMEWORK_SOURCES{cmake_file_path_sep}" )
            out.writelines( cmake_file_path_sep.join( utils.list_files( source_path, "testing/unit/framework/src", "cpp", False ) ) )
            out.write( ")\n" )
            out.write( f"set(AAC_UNIT_TEST_FRAMEWORK_INCLUDES{cmake_file_path_sep}" )
            out.writelines( os.path.join( source_path, "testing/unit/framework/include" ) )
            out.write( ")\n" )
            out.write( f"set(AAC_UNIT_TESTS{cmake_file_path_sep}" )
            out.writelines( cmake_file_path_sep.join( utils.list_files( source_path, "testing/unit/tests", "cpp", False, False ) ) )
            out.write( ")\n" )

        # write includes cmake variable list
        out.write( f"set(AAC_INCLUDES{cmake_file_path_sep}" )
        out.writelines( cmake_file_path_sep.join( obj.get_include_directories( source_path, False ) ) )
        out.write( ")\n" )

        # write all sources cmake variable list
        out.write( f"set(AAC_SOURCES{cmake_file_path_sep}" )
        out.writelines( cmake_file_path_sep.join( [
            "${AAC_ENGINE_SOURCES}",
            "${AAC_PLATFORM_SOURCES}",
            "${AAC_AASB_SOURCES}",
            "${AAC_JNI_SOURCES}",
            "${AAC_MODULE_SOURCES}"
        ]))
        out.write( ")\n" )

        # write module specific cmake variables
        cmake_definitions = obj.get_cmake_definitions()
        out.writelines( [f"set({next} {cmake_definitions[next]})\n" for next in cmake_definitions] )

        # local source only cmake variables
        if not obj.in_local_cache:
            out.write( "# local source only cmake variables\n" )
            out.write( f"set(CMAKE_BUILD_TYPE {obj.settings.build_type})\n" )
            out.write( f"set(BUILD_SHARED_LIBS {utils.bool_value(obj.options.shared,'ON','OFF')})\n" )
            out.write( f"set(CMAKE_LIBRARY_OUTPUT_DIRECTORY {os.path.join( source_path,'build','lib')})\n" )
            out.write( "set(CONAN_SETUP_OPTIONS NO_OUTPUT_DIRS)\n" )
            out.write( "set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})\n" )
            out.write( "set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})\n" )
            out.write( "set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})\n" )
            out.write( "set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})\n" )
            out.write( "# ide source groups\n" )
            out.write( "source_group(\"Engine\" FILES ${AAC_ENGINE_SOURCES})\n" )
            out.write( "source_group(\"Platform\" FILES ${AAC_PLATFORM_SOURCES})\n" )
            out.write( "source_group(\"Messages\" FILES ${AAC_AASB_SOURCES})\n" )
            out.write( "source_group(\"JNI\" FILES ${AAC_JNI_SOURCES})\n" )
            # add headers source group
            out.write( "# ide headers\n" )
            module_headers = []
            for next in obj.get_include_directories( source_path ):
                module_headers.extend( utils.list_files( source_path, next, "h", False ) )
            # write the headers
            out.write( f"set(AAC_HEADERS{cmake_file_path_sep}" )
            out.writelines( cmake_file_path_sep.join( module_headers ) )
            out.write( ")\n" )
            out.write( "source_group(\"Headers\" FILES ${AAC_HEADERS})\n" )
            # add headers to AAC_SOURCES
            out.write( "list(APPEND AAC_SOURCES ${AAC_HEADERS})" )

        # windows specific cmake configuration
        if obj.settings.os == "Windows":
            out.write( "# windows specific configuration\n" )
            out.write( "add_definitions(-DNOGDI -DNO_SIGPIPE -DNO_SYSLOG)\n" )

        if obj.settings.compiler == "gcc" and obj.settings.compiler.version >= Version("8.0"):
            out.write( "# extra compiler flags\n" )
            out.write( "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -Wno-class-memaccess\")\n" )

def configure(obj):
    cmake = CMake(obj)
    cmake.configure()
    return cmake

def build(obj):
    logging.info( "Building native libraries..." )
    cmake = configure(obj)
    cmake.build()
    if obj.options.get_safe("with_unit_tests", default=False):
        env_run = RunEnvironment(obj)
        test_env = env_run.vars
        test_env["DYLD_LIBRARY_PATH"].append( os.path.join( obj.build_folder, "lib" ) )
        test_env["LD_LIBRARY_PATH"].append( os.path.join( obj.build_folder, "lib" ) )
        test_env["GTEST_OUTPUT"] = f"xml:{os.path.join(obj.build_folder,'test_results',obj.module_name)}/"
        logging.info( "Running tests..." )
        ctest_count = tools.get_env("CTEST_REPEAT", 1)
        with tools.environment_append(test_env):
            obj.run( f"ctest --repeat until-fail:{ctest_count} --output-on-failure {'-VV' if obj.verbose else ''}", run_environment=True, cwd=obj.build_folder )

def package(obj):
    logging.info( "Packaging native libraries..." )
    obj.copy("*", src="lib", dst="lib", keep_path=False)
    obj.copy("*", src="bin", dst="bin", keep_path=False)
    for nextIncludeDir in obj.get_include_directories( obj.build_folder ):
        obj.copy("*.h", dst="include", src=nextIncludeDir)
    for nextIncludeDir in obj.get_public_include_directories( obj.build_folder ):
        obj.copy("*.h", dst="public", src=nextIncludeDir)
    if obj.options.get_safe( "with_unit_tests", default=False ):
        obj.copy("*.h", dst="include", src="testing/unit/framework/include")
        obj.copy("*.xml", dst="test_results", src="test_results")
