import uuid, os
from conans import ConanFile, tools, CMake

class AlexaAutoClientService(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    version = "dev"
    exports = "*", "!*.txt"
    name = "alexaautoclientservice"
    options = {
        "aac_version": "ANY",
        "aac_modules": "ANY",
        "extra_modules": "ANY",
        "with_sensitive_logs" : [True, False]
    }

    default_options = {
        "aac_version": "dev",
        "aac_modules": "core,aasb,alexa,bluetooth,apl,cbl,address-book,car-control,custom-domain,connectivity,messaging,navigation,phone-control,text-to-speech,text-to-speech-provider",
        "extra_modules": None,
        "with_sensitive_logs" : False
    }

    def requirements(self):
        module_list = set( str(self.options.aac_modules).split(",") )
        if self.options.extra_modules:
            module_list.update( str(self.options.extra_modules).split(",") )
        for next in module_list:
            module = next.strip().casefold()
            req = f"aac-module-{module}"
            self.options[req].with_aasb = True
            self.options[req].with_messages = True
            if (self.options.with_sensitive_logs == True and self.settings.build_type == "Debug"):
                self.options[req].with_sensitive_logs = True
            self.requires(f"{req}/{self.options.aac_version}")
        
    def imports(self):
        libs_folder = os.path.abspath( os.path.join( __file__, "..", "service", "core-service", "libs"))
        self.copy("*.aar",dst=libs_folder,keep_path=False)

    def package(self):
        self.copy("*.aar")

    def deploy(self):
        self.copy("*.aar", keep_path=False)
