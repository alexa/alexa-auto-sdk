from pylib.common import BaseHandler

class BuilderHandler(BaseHandler):

    def init(self):
        self.log_info("Initializing imports!")
        self.builder_imports.initialize()
        self.builder_imports.save()

    def list( self ):
        import_logs = []
        for next in self.builder_imports.find_imports():
            entry = self.builder_imports.get( next )
            import_logs.append( "\n"
                f"name: {next}\n" 
                f"path: {self.builder_imports.abspath(entry['path'])}\n"
                f"enabled: {entry['enabled']}")
        if import_logs:
            msg = "\n".join( import_logs )
            self.log_direct( f"Imports:\n{msg}\n" )
        else:
            self.log_info("No imports to display")

    def add( self ):
        self.builder_imports.add( self.get_arg("name"), self.get_arg("path") )

    def remove( self ):
        pattern = self.get_arg("pattern")
        import_list = self.builder_imports.find_imports( pattern )
        if not import_list:
            raise Exception(f"No matching imports found: {pattern}")
        for next in import_list:
            self.builder_imports.remove( next )

    def enable( self ):
        pattern = self.get_arg("pattern")
        import_list = self.builder_imports.find_imports( pattern )
        if not import_list:
            raise Exception(f"No matching imports found: {pattern}")
        for next in import_list:
            self.builder_imports.set_enabled( next, True )

    def disable( self ):
        pattern = self.get_arg("pattern")
        import_list = self.builder_imports.find_imports( pattern )
        if not import_list:
            raise Exception(f"No matching imports found: {pattern}")
        for next in import_list:
            self.builder_imports.set_enabled( next, False )
