from pylib.common import BaseHandler

class BuilderHandler(BaseHandler):

    def init( self ):
        self.log_info("Initializing builder configuration!")
        self.builder_configuration.initialize()
        self.builder_configuration.configure()
        self.builder_configuration.save()

    def export( self ):
        pattern = self.get_arg("pattern")
        self.builder_configuration.export( [pattern], False )
