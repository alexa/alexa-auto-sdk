import os, logging
from conans import ConanFile, tools

class GradleConanFile(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"
    name = "gradle"
    version = "7.0"
    no_copy_source = True
    settings = "os", "arch", "compiler", "build_type"
    requires = ["zulu-openjdk/11.0.8"]

    @staticmethod
    def chmod_plus_x(filename):
        if os.name == "posix":
            os.chmod(filename, os.stat(filename).st_mode | 0o111)

    def fix_permissions(self,root_folder):
        if os.name != "posix":
            return
        for root, _, files in os.walk(root_folder):
            for filename in files:
                filename = os.path.join(root, filename)
                with open(filename, 'rb') as f:
                    sig = f.read(4)
                    if type(sig) is str:
                        sig = [ord(s) for s in sig]
                    else:
                        sig = [s for s in sig]
                    if len(sig) > 2 and sig[0] == 0x23 and sig[1] == 0x21:
                        logging.info(f"chmod on script file: {filename}")
                        self.chmod_plus_x(filename)
                    elif sig == [0x7F, 0x45, 0x4C, 0x46]:
                        logging.info(f"chmod on ELF file: {filename}")
                        self.chmod_plus_x(filename)
                    elif sig == [0xCA, 0xFE, 0xBA, 0xBE] or \
                         sig == [0xBE, 0xBA, 0xFE, 0xCA] or \
                         sig == [0xFE, 0xED, 0xFA, 0xCF] or \
                         sig == [0xCF, 0xFA, 0xED, 0xFE] or \
                         sig == [0xFE, 0xEF, 0xFA, 0xCE] or \
                         sig == [0xCE, 0xFA, 0xED, 0xFE]:
                        logging.info(f"chmod on Mach-O file: {filename}")
                        self.chmod_plus_x(filename)

    def source(self):
        tools.get( f"https://services.gradle.org/distributions/gradle-{self.version}-bin.zip" )

    def package(self):
        self.copy( "*", src=f"gradle-{self.version}" )
        # fix executable permisions for command line tools
        self.fix_permissions(self.package_folder)

    def package_info(self):
        self.env_info.GRADLE_PROGRAM = os.path.join( self.package_folder, "bin", "gradle" )
        self.env_info.PATH.append( os.path.join( self.package_folder, "bin" ) )
