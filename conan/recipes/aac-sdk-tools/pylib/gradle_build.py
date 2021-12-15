import os, shutil, logging
from conans import tools
from pylib import utils

def imports( obj, dest_folder ):
    aac_sdk_tools_path = obj.python_requires["aac-sdk-tools"].path
    if not obj.module_overrides_gradle:
        gradle_file = os.path.join( dest_folder, "build.gradle" )
        shutil.copyfile( os.path.join( aac_sdk_tools_path, "build.gradle" ), gradle_file )
        shutil.copyfile( os.path.join( aac_sdk_tools_path, "gradle", "gradle.properties" ), os.path.join( dest_folder, "gradle.properties" ) )
        # replace the library name in the top level build.gradle file
        tools.replace_in_file( gradle_file, "aac-module", obj.module_name )

def build(obj):
    # copy the native libraries into the android source
    abi = utils.get_android_abi(str(obj.settings.arch))
    dst_nativelib_folder = os.path.join(obj.build_folder,"android","src","main","jniLibs",abi)
    tools.mkdir( dst_nativelib_folder )
    native_libs = get_native_libraries(obj)
    for next_lib in native_libs:
        shutil.copy( next_lib, dst_nativelib_folder )
    # copy the module android libraries
    dst_androidlib_folder = os.path.join(obj.build_folder,"android","libs")
    tools.mkdir( dst_androidlib_folder )
    android_libs = get_android_libraries(obj)
    for next_lib in android_libs:
        shutil.copy( next_lib, dst_androidlib_folder )
    # run the gradle build
    logging.info( "Building Android libraries..." )
    gradle_home = os.getenv("GRADLE_USER_HOME")
    with tools.environment_append( { "GRADLE_USER_HOME": gradle_home } if gradle_home else {} ):
        obj.run( f"gradle assemble{obj.settings.build_type}", run_environment=True, cwd=obj.build_folder )

def package(obj):
    logging.info( "Packaging Android libraries..." )
    obj.copy("*", src=os.path.join(obj.build_folder,"build","outputs","aar"), dst="aar", keep_path=False)

def resolve_native_library(obj,dep_name,lib_name):
    for next_path in obj.deps_cpp_info[dep_name].lib_paths:
        path = os.path.join( next_path, utils.get_os_lib_name(obj.settings.os,lib_name) )
        if os.path.exists( path ):
            return str(path)

def get_module_deps(obj):
    module_deps = set(obj.deps_cpp_info.deps)
    # iterate through any transient module deps and remove them
    for next_dep in obj.deps_cpp_info.deps:
        if "module_deps" in obj.deps_user_info[next_dep].vars:
            transient_deps = obj.deps_user_info[next_dep].module_deps.split(",")
            for next_transient_dep in transient_deps:
                module_deps.discard( next_transient_dep )
    # remove any transient modules
    for next_module in obj.module_requires:
        module_deps.discard( f"aac-module-{next_module}" )
    return module_deps

def get_native_libraries(obj):
    module_deps = get_module_deps(obj)
    native_libs = { 
        os.path.join( obj.build_folder, "lib", f"{utils.get_os_lib_prefix(obj.settings.os)}{obj._module_library_name}{utils.get_os_lib_ext(obj.settings.os)}" )
    }
    # collect all of the libraries that defined for this modules dependencies
    for next_dep in module_deps:
        for next_lib in obj.deps_cpp_info[next_dep].libs:
            resolved_lib = resolve_native_library( obj, next_dep, next_lib )
            if resolved_lib:
                native_libs.add( resolved_lib )
    # add any runtime libraries defined in the toolchain
    android_stl = os.getenv("ANDROID_STL_SHARED_LIB")
    if android_stl:
        native_libs.add(android_stl)
    return native_libs

def get_android_libraries(obj):
    libs = set()
    # iterate through all of the package dependencies 
    for next_dep in obj.deps_cpp_info.deps:
        android_lib_dir = os.path.join(obj.deps_cpp_info[next_dep].rootpath,"aar")
        if os.path.exists(android_lib_dir):
            for next_file in os.listdir(android_lib_dir):
                resolved_path = os.path.join(android_lib_dir,next_file)
                if os.path.isfile(resolved_path) and next_file.endswith('.aar'):
                    libs.add(resolved_path)
    return libs