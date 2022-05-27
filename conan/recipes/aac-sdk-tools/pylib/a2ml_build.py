import os, logging

def get_module_deps( obj ):
    module_deps = list(obj.deps_cpp_info.deps)
    # iterate through any transient module deps and add them if needed
    for next_dep in obj.deps_cpp_info.deps:
        if "module_deps" in obj.deps_user_info[next_dep].vars:
            transient_deps = obj.deps_user_info[next_dep].module_deps.split(",")
            for next_transient_dep in transient_deps:
                if not next_transient_dep in module_deps:
                    module_deps.append( next_transient_dep )
    # add required modules if needed
    for next_module in [f"aac-module-{next_module}" for next_module in obj.module_requires]:
        if not next_module in  module_deps:
            module_deps.append( next_module )
    return module_deps

def imports(obj):
    # generate the message headers
    logging.info( "Generating AASB message headers..." )
    from A2ML.processor import A2MLProcessor
    message_dependencies = []
    for next in get_module_deps(obj):
        if "message_dir" in obj.deps_user_info[next].vars:
            if os.path.exists( obj.deps_user_info[next].message_dir ):
                message_dependencies.append( obj.deps_user_info[next].message_dir )

    message_input_dirs = []
    message_source_folder = obj.build_folder if obj.in_local_cache else obj.recipe_folder
    for next in [os.path.join(message_source_folder,next) for next in obj._module_message_directories]:
        if os.path.exists( next ):
            message_input_dirs.append( next )
    if message_input_dirs:
        builder = A2MLProcessor({
            "input": message_input_dirs,
            "dependencies": message_dependencies,
            "output": os.path.join( obj.install_folder, "aasb-messages" ),
            "parser": "a2ml",
            "generator": "aasb",
            "message_version": str(obj.options.message_version)
        })
        builder.run()
        # generate the message documentation
        if obj.options.get_safe("with_docs", default=False):
            logging.info( "Generating AASB message docs..." )
            builder = A2MLProcessor({
                "input": message_input_dirs,
                "dependencies": message_dependencies,
                "output": os.path.join( obj.install_folder, "aasb-docs" ),
                "parser": "a2ml",
                "generator": "markdown",
                "message_version": str(obj.options.message_version)
            })
            builder.run()

def package(obj):
    logging.info( "Packaging AASB messages..." )
    for next in obj._module_message_directories:
        obj.copy("*.yml", src=next, dst="messages", keep_path=False)
        obj.copy("*.yaml", src=next, dst="messages", keep_path=False)
        obj.copy("*.a2m", src=next, dst="messages", keep_path=False)
        obj.copy("*.a2ml", src=next, dst="messages", keep_path=False)
