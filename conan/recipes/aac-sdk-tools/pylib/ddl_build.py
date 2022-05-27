import os, logging

def build(obj):
    ddl_source = os.path.join( obj.source_folder, "aasb", "messages" )
    if os.path.exists( ddl_source ):
        from buildconfig import run
        logging.info( "Building AASB message headers..." )
        run.start( ddl_source, os.path.join( obj.build_folder, "aasb-messages", "include" ) )
