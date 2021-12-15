from conans import ConanFile, CMake, tools, AutoToolsBuildEnvironment
import os


class Faad2Conan(ConanFile):
    python_requires = "aac-sdk-tools/1.0"
    python_requires_extend = "aac-sdk-tools.BaseSdkDependency"

    name = "faad2"
    version = "2.10.0"
    description = "FAAD2 is an open source MPEG-4 and MPEG-2 AAC decoder"
    url = "https://github.com/conan-multimedia/faad2"
    homepage = "https://www.audiocoding.com/faad2.html"
    license = "GPLv2Plus"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=False"
    generators = "cmake"
    source_subfolder = "source_subfolder"

    def build_requirements(self):
        if hasattr(self, 'settings_build') and tools.cross_building(self, skip_x64_x86=True):
            pass # use libtool provided by the toolchain
        else:
            self.build_requires("libtool/2.4.6")

    def source(self):
        tarball = '_'.join(self.version.split('.'))
        tools.get('https://github.com/knik0/faad2/archive/refs/tags/{file}.tar.gz'.format(file=tarball))
        extracted_dir = self.name + "-" + tarball
        os.rename(extracted_dir, self.source_subfolder)

    def build(self):
        with tools.chdir(self.source_subfolder):
            self.run("autoreconf -f -i")
            autotools = AutoToolsBuildEnvironment(self)
            _args = ["--prefix=%s/builddir" % (os.getcwd()), "CFLAGS=-fPIC", "CXXFLAGS=-fPIC"]
            if self.options.shared:
                _args.extend(['--enable-shared=yes', '--enable-static=no'])
            else:
                _args.extend(['--enable-shared=no', '--enable-static=yes'])
            autotools.configure(args=_args)
            autotools.make(args=["-j4"])
            autotools.install()

    def package(self):
        with tools.chdir(self.source_subfolder):
            self.copy("*", src="%s/builddir" % (os.getcwd()))

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
