from conans import ConanFile, tools, CMake
import os

class WebvttConan(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"

    name = "webvtt"
    version = "1.0"
    exports = "*"

    _source_subfolder = "source_subfolder"

    def source(self):
        tools.get( f"https://github.com/alexa/webvtt/archive/refs/tags/v{self.version}.zip" )
        os.rename(f"webvtt-{self.version}", self._source_subfolder)

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure(source_folder=self._source_subfolder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

        # Copy the resources required by AVS
        self.copy("*", src=os.path.join(self._source_subfolder, "include"), dst="include")
        self.copy("libwebvtt.a", src="src/webvtt", dst="lib")
