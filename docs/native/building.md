# Build Alexa Auto SDK

## Supported platforms and architectures

Auto SDK can be built for the following supported target platforms and hardware architectures:

* Android 5.1 Lollipop API Level 22 or higher.
    * ARM 64-bit
    * x86 64-bit
* QNX 7.0
    * ARM 64-bit
    * x86 64-bit
* Generic Linux
    * x86 64-bit
* Poky Linux
    * ARMv7a (+NEON)
    * AArch64
* macOS
    * x86 64-bit

## General build requirements

You can build the Alexa Auto SDK natively on a Linux or macOS host, or you can use Docker. For specific information about Docker, see [Build in a Docker container](#build-in-a-docker-container).

The following list describes the supported host configurations:

* Operating system:
    * macOS Sierra
    * Ubuntu 18.04 LTS (Bionic) or Ubuntu 20.04 LTS (Focal)
* Processor: 2.5 GHz
* Memory: 16 Gb
* Storage: 1 Gb+ available to use

## Build dependencies

To build Auto SDK, you must install the following dependencies on your host machine:

### General

- [Python 3.7](https://www.python.org/downloads)
- [Conan 1.45](https://conan.io/downloads.html)
- [CMake 3.12](https://cmake.org/install)

### Linux

- GCC
- GStreamer (see [Install GStreamer](#install-gstreamer))

### macOS

- Xcode

## Understand the build system

Building software for multiple platforms can be complex because specific toolchains might vary depending on the build system and target platform. In general, there are two flavors of builds: native and cross-compiled. In a native build, the build system uses its own toolchain and libraries to build the software, so the compiled software can run on the platform that built it. In cross-compilation, the build system typically uses an installed toolchain to compile the software for a different target platform. It's possible that more than one toolchain is installed on a system, so extra steps are typically needed to cross-compile to those targets. Auto SDK uses [Conan](https://conan.io), along with other tools and scripts described in this section, to manage the complexities required to implement a complete build system.

### Conan

The Auto SDK build system uses Conan as its underlying package manager and build configuration tool. For every Conan package, there is a recipe that defines the dependencies of the package and specifies how to download and build the package source code. After building a package, Conan copies the binaries and other artifacts into a cache directory so other recipes that depend on that package can use the prebuilt binaries without rebuilding them. When a Conan recipe defines a dependency, Conan finds and builds the dependency as required, taking care of complexities such as transitive requirements, package version conflicts, and managing multiple versions of a package built with different configurations.

Before using a package, Conan must export or download the package into the local cache. When a package recipe exists in the same local repository as the source code it builds, as is the case when you download Auto SDK, you must run `conan create` or `conan export` before other recipes can build the package. Community servers such as [Conan Center](https://conan.io/center) host some popular third party libraries, however, so Conan automatically downloads them to the local cache as needed. Auto SDK requires a combination of Conan packages including local recipes for Auto SDK modules and tools, local recipes for third party packages, and third party packages hosted on the Conan Center server.

Once Conan copies a package into the local cache, a recipe can build or consume the package based on the specified build configuration. Conan will build a new package version if the package version is required and missing from the cache. For example, if you build Auto SDK for Linux, Conan will build all of the required packages for the specified Linux target. If you then build for an Android target, Conan will rebuild all of the required packages for the Android target and cache both the Linux and Android versions. In addition to target platform, any option or setting that you specify when building a recipe affects the package version.

#### Auto SDK modules

Auto SDK includes a base Conan recipe class that all Auto SDK modules extend. This is defined in `conan/recipes/aac-sdk-tools`, and must be exported before other modules since it is required by each module's recipe definition. The base recipe defines common build options, and relies on specific conventions in the module's directory structure to find source files and headers, and to generate other artifacts that are needed at build time. A simple Conan recipe is required for each module to override abstract values in the base class (such as the module name), and to define any module specific dependencies or options that are required. A module can also define it's own CMake files, unique configuration, or even custom build steps as needed.

For each module, the base recipe defines common options that are used to specify which components are included in the library. The default values provided in the base recipe should be used in most cases when you are building release libraries for production. For some cases, however, you may want to enabled features such as `with_sensitive_logs` or `with_unit_tests`, to add additional information when debugging issues with the libraries. To find out which options are defined for a specific module, you can use the `conan inspect` command to display information about any Conan recipe. This command will display all of options and default values for a recipe, including any options that are inherited from the base module recipe. See the [Specify build settings and options](#specify-build-settings-and-options) section in this guide, for more information.

Applications integrate with Auto SDK using the `MessageBroker` API by publishing and subscribing to specific message topics and actions (see [Understand the Auto SDK API](../explore/concepts/core-api-overview.md)). Most modules provide interfaces that require these messages to be defined, in which case they will include one or more message definition files in the `aasb/messages` directory of the module. The model created by the message definitions are used when building Auto SDK to generate message headers that are required to build the module, and are also used to create documentation for each message interface. 

#### Third party dependencies

Auto SDK has dependencies on several third party packages (libraries and build tools for example), which may themselves have dependencies on other packages. In general, managing these types of build requirements can be very complex for a large project. Conan helps by providing community hosted recipes for many common packages, as well as by allowing developers to create there own package recipes. It is important to understand that some of the packages used by Auto SDK are pulled from the Conan Center remote server, while others are defined locally in the `conan/recipes` directory of Auto SDK. Local recipes are typically required when the package does not already exist on Conan Center, or there are specific patches or changes to the recipe that are needed for Auto SDK. 

### Builder Tool

The Builder Tool is a script that can be used to build Auto SDK libraries for supported platforms. It improves the build process by wrapping underlying Conan commands and options with a simple command line interface. Although it is possible to use Conan by itself to build Auto SDK—see [Build with Conan directly](#build-with-conan-directly)—it is recommended to use the Builder Tool for common build tasks.

### Alexa Auto Client Service

Alexa Auto Client Service (AACS) is an Android service library that simplifies the process of integrating Auto SDK on Android-based devices. AACS has a dependency on Auto SDK native Android libraries, but can be built independently using standard Android development tools. For more information about building AACS, see the [Android developer documentation](../android/index.md).

## Build with Builder Tool

The Builder Tool script, `build.py` is located in the `builder` directory of the SDK. It wraps underlying Conan commands, and simplifies building libraries for Auto SDK modules on supported platforms. Individual modules, components, and dependencies in the SDK are described as packages in the builder. Each package has a corresponding Conan recipe that is used to build and deploy the package to the cache located in the builder's home directory. An archive containing all of the specified build artifacts is created from the cache, and written to the `deploy` directory of the builder, after the build has completed. This section describes the most common commands used to build Auto SDK. For a complete reference to the Builder Tool command line interface, see [Builder Tool command reference](https://alexa.github.io/alexa-auto-sdk/docs/builder).

Auto SDK supports native builds for Ubuntu Linux (x86_64) and MacOS, and building for each platform follows the same steps. After cloning the Auto SDK git repository on your system, the following examples should be run with `alexa-auto-sdk` as the working directory.

The following command will build all of the modules that are included in the Auto SDK repository, along with any dependencies that are required for the target platform:

```shell
$ ./builder/build.py
```

When you run the build command, the builder tool will export and configure any new build artifacts, such as package recipes or configuration files, that are discovered in the search path. The first time you run (or after cleaning the build cache), you'll see several log messages indicating that the build recipes are being exported to the local cache:

```shell
[BUILDER] INFO: Python version: 3.7.3
[BUILDER] INFO: Cleaning cached builder data
[BUILDER] INFO: Builder home: ../aac-sdk/builder/.builder
[BUILDER] INFO: Conan home: ../aac-sdk/builder/.builder/.conan
[BUILDER] INFO: Gradle home: ../aac-sdk/builder/.builder/.gradle
[BUILDER] INFO: Configuring Conan...
[BUILDER] INFO: Installing Conan configuration: ../aac-sdk/conan/config
[BUILDER] INFO: Exporting recipe: aac-sdk-tools
[BUILDER] INFO: Exporting recipe: aac-module-core
[BUILDER] INFO: Exporting recipe: aac-module-alexa
[BUILDER] INFO: Exporting recipe: aac-module-cbl
[BUILDER] INFO: Exporting recipe: android-sdk-tools
[BUILDER] INFO: Exporting recipe: avs-device-sdk
...
```

The builder keeps track of which recipes have already been added to the cache, so that the next time you run the build command only new recipes will be exported. It is possible, however, to tell the builder to force re-exporting a recipe (using the `-f` or `--force` option), and build it if necessary. The following command will force all Auto SDK module recipes to be re-exported:

```shell
$ ./builder/build.py -f "aac-module-*"
```

To explicitly force one or more recipes to be exported, you can specify the name of the module (or explicit package name) that you want. The following example will force the builder to re-export and build only the `alexa` and `cbl` modules.

```shell
$ ./builder/build.py -f alexa cbl
```

Each time the builder is run, it will also attempt to re-configure Conan settings by initializing the Conan configuration and installing any config files found in the search path. This happens every time because it is possible, using Docker for example, to re-use the Conan home path when building with a different build system configuration. This step ensures that the Conan configuration will match the build system currently being used. In the case that you want to skip the configuration step for some reason (maybe you have overridden configuration settings in the Conan home manually), you can tell the builder to skip the configuration step using the `--skip-config` option:

```shell
$ ./builder/build.py --skip-config
```

### Specify the build target

Auto SDK can be cross-compiled for supported target systems by specifying the platform and architecture with the build command. Android and QNX targets can be built on either Linux or macOS, and Poky must be built using Linux. For information about specific build target requirements, see the [Platform-specific build information](#platform-specific-build-information) section of this guide. To set the target platform using the Builder Tool, specify the `--platform,-p <platform>` option when doing a build:

```shell
$ ./builder/build.py -p android
```

You can also set the target architecture by specifying the  `--arch,-a <architecture>` option:

```shell
$ ./builder/build.py -p android -a x86_64
```

The following table defines the supported platforms and architectures.

|platform |arch                       |
|---------|---------------------------|
|android  |armv8, x86_64              |
|qnx      |armv8, x86_64              |
|poky     |armv8, armv7hf, x86_64, x86|

### Specify which modules to build

If you are using a subset of modules in Auto SDK, you can specify which modules to build on the command line using the `-m` or `--modules` option followed by a list of modules names. Dependent modules and libraries will be included transitively when specifying which modules to build. The following example will build the `core`, `alexa`, and `cbl` modules, and package them into the output archive:

```shell
$ ./builder/build.py -m core alexa cbl
```

You can verify which modules were specified in the build by looking at the `[requires]` section or `pkg_modules` option value in the `aac-buildinfo.txt` file:

```
[requires]
    aac-module-alexa/dev
    aac-module-cbl/dev
    aac-module-core/dev

[options]
    ...
    pkg_modules=aac-module-core/dev,aac-module-alexa/dev,aac-module-cbl/dev
```

You could also build the same modules by specifying the following on the command line:

```shell
$ ./builder/build.py -m cbl
```

This works because the `cbl` module depends on the `alexa` module, which depends on the `core` module, so even though they are not specified on the command line, `core` and `alexa` are transitively included. The `aac-buildinfo.txt` file will only show the `cbl` module under the `[requires]` section, however, the full list of included dependencies can be found under the `[full_requires]` section in the build info:

```
[full_requires]
    aac-module-alexa/dev:1de4d8ddd6d19b16b05d95052195f9556361e7b5
    aac-module-cbl/dev:8b2bd324ad68ca44682ed4ed11f0845ef8df1a5c
    aac-module-core/dev:fe4587e72f3350cdb9dab53b293dfee0d5575a0a
    ...
```

### Clean build artifacts

Conan caches binaries and artifacts for each package after it is built, so they can be used as dependencies by other packages without having to be re-built each time. If you make changes to the source code in the SDK, however, you must either explicitly force the builder to re-export and build the package (using the `--force,-f <pattern>` option of the builder), or remove the package entirely from the cache. To remove packages from the cache using the Builder Tool, you can use the `clean` command:

```shell
$ ./builder/build.py clean <pattern>
```

You must specify the package name or regex-style pattern to clean. For example, to remove all of the packages from the cache, you can use the following command:

```shell
$ ./builder/build.py clean "*"
```

To remove a specific module, you can either specify the package name or just the module's name:

```shell
$ ./builder/build.py clean alexa
```

Since the convention used by Auto SDK is to specify the module's package name as `aac-module-<name>`, you can also use the full package name as part of the pattern. One way to remove all Auto SDK modules from the cache would be to use the following command:

```shell
$ ./builder/build.py clean "aac-module-*"
```

If a package has been removed from the cache, the Builder Tool will automatically detect that it needs to be re-exported and built the next time you do a build, and it is not necessary to specify the package using the `--force` option.

### Build debug libraries

Building debug libraries for Auto SDK can be specified by using the `--debug` or `-g` option when doing a build:

```shell
$ ./builder/build.py -g
```

When this option is used, debug libraries for all of the Auto SDK modules and dependencies will be built if required, and exported to the build archive. If you want more specific control over which debug libraries to use, you can specify the `build_type` option as a Conan setting instead, using the `--conan-setting,-s <name>=<value>` build option. For example, to use debug libraries only for Auto SDK modules, you can use the following build command:

```shell
$ ./builder/build.py -s "aac-module-*":build_type=Debug
```

This is a less common use case, however, that requires you to be familiar with some of the underlying Conan build architecture. To learn more about some of the Conan specific options for building Auto SDK, see the [Build with Conan directly](#build-with-conan-directly) section of this guide.


### Locate the build output

When you run the builder tool, all of the shared libraries and dependencies will be saved in an archive file in the `builder/deploy` directory by default. The name of the archive file is displayed in the console when the build is completed:

```shell
[BUILDER] INFO: Created output archive: ../aac-dev-macos_x86_64-release-210706140415.tgz
```

The default name of the archive indicates the following information that is used to build the SDK: 

```
aac-<version>-<os>_<arch>-<build-type>-<datetime>.tgz
```

Sometimes it is helpful to tag a build with an identifier, for example, if you want to indicate a build was made for a specific purpose. If you want to add an additional identifier to the archive name, you can use `--name` option when running the build tool:

```shell
$ ./builder/build.py --name test
...
[BUILDER] INFO: Created output archive: ../aac-dev-test-macos_x86_64-release-210706142403.tgz
```

It is also possible to completely override the output file name and path by specifying the `-o` or `--output` option on the command line:

```shell
$ ./builder/build.py --output /mypath/custom-output.tgz
...
[BUILDER] INFO: Created output archive: /mypath/custom-output.tgz
```

If you don't want the builder to generate an output archive at all, you can specify the `--no-output` option on the command line. This is helpful if you just want to re-build one or more module, for example, to run unit tests or inspect the package libraries:

```shell
$ ./builder/build.py --no-output
```

#### Archive contents

The output archive created by the Builder Tool includes all of the build artifacts from the modules and dependencies specified by the build command. You can extract the archive with the following command (the exact filename will be slightly different for your build):

```shell
$ tar -xvzf builder/deploy/aac-dev-linux_x86_64-release.tgz
```

After you can extract the contents of the archive, there should be a directory with contents similar to the following file structure:

```
aac-dev-linux_x86_64-release/
  ├─ docs/
  ├─ include/
  ├─ lib/
  |  ├─ libAACECore.so
  |  └─ ...
  ├─ share/
  └─ aac-buildinfo.txt
```

You can get additional information about the archive contents from a description file in the archive named `aac-buildinfo.txt`. The build description file can be used to identify which modules, settings, and options were used to generate the libraries by the build. The following is an example of the information found in the build description file:

```
[settings]
    arch=x86_64
    build_type=Release
    compiler=apple-clang
    compiler.libcxx=libc++
    compiler.version=11.0
    os=Macos

[requires]
    aac-module-aasb/dev
    aac-module-address-book/dev
    aac-module-alexa/dev
    aac-module-car-control/dev
    aac-module-cbl/dev
    aac-module-connectivity/dev
    aac-module-core/dev
    aac-module-messaging/dev
    aac-module-navigation/dev
    aac-module-phone-control/dev
    aac-module-text-to-speech/dev

[options]
    aac_version=dev
    with_sensitive_logs=False
    pkg_modules=aac-module-aasb/dev,aac-module-address-book/dev,aac-module-alexa/dev,...    
    with_aasb=False

[full_settings]
    arch=x86_64
    build_type=Release
    compiler=apple-clang
    compiler.libcxx=libc++
    compiler.version=11.0
    os=Macos

[full_requires]
    aac-module-aasb/dev:4990d7e4c95bbcae311c6d13cb0e71a09ecd2f43
    aac-module-address-book/dev:8b2bd324ad68ca44682ed4ed11f0845ef8df1a5c
    aac-module-alexa/dev:1de4d8ddd6d19b16b05d95052195f9556361e7b5
    ...
```

## Build with Conan directly

Conan can be used directly to build Auto SDK components and other package dependencies, or to use Auto SDK libraries in other Conan recipes. It's helpful to have a good general understanding of how Conan works first, and also to understand the basic Auto SDK build system. The examples in this section should be run with `aac-sdk` as the working directory.

### Export Conan recipes

The following script will find all of the Conan recipes in Auto SDK and export them to the local cache. Package binaries won't actually be built until they are required by another recipe during a build operation, or explicitly built by running the `conan create` command. This is a convenience script and is not required if you want to export or create packages individually.

```shell
$ ./conan/setup.py
```

If you want to export a single package individually, you can run the `conan export` command. For example, to export the alexa module to the local cache:

```shell
$ conan export modules/alexa
```

It is important to understand that exporting a module using the `conan export` command does not automatically find and export any of the dependent packages specified in the recipe. Attempting to build the alexa module would fail, unless all of the requirements can be resolved in the local cache. Running the `conan/setup.py` script is usually the safest option to ensure all required packages are copied to the cache, however, exporting a package individually can save time after you make changes, if you have previously exported all of the packages. 

### Build modules

In most cases it shouldn't be necessary to manually build Auto SDK modules, since Conan can build missing dependencies when required by another recipe. It is possible, however, to create/build a package independently using Conan if needed. The following example shows how to create the Alexa module package from the command line:

```shell
$ conan create modules/alexa --build missing
```

The `conan create` command tells conan to create a new binary package from a recipe file and install it in the local cache. In the example above, `modules/alexa` refers to the parent directory in Auto SDK (`aac-sdk/modules/alexa`), where the `conanfile.py` recipe is located for the Alexa module.

By specifying the `--build missing` option, Conan will automatically build dependencies where a binary package is missing for the specified build configuration. If the dependency has already been created it will not be built again.

Using the `--build` flag without any additional options will force all of the dependencies to be rebuilt, even if the binary for the specified configuration already exists.

### Specify build settings and options

When you build a Conan package, you can specify settings and options that result in different binaries when the source code is built. Conan `settings` are project-wide configurations, such as `os`, `compiler`, `build_type`, and `arch`. These settings should be applied to each package when selecting the correct binary. Most of the time, settings will be applied based on the selected (or default) profile. To view or modify a profile, you can use the `conan profile` command. To show the default profile values, you can enter the following command:

```shell
$ conan profile show default

Configuration for profile default:

[settings]
os=Macos
os_build=Macos
arch=x86_64
arch_build=x86_64
compiler=apple-clang
compiler.version=11.0
compiler.libcxx=libc++
build_type=Release
[options]
[build_requires]
[env]
```

You usually don't need to change settings specified in the profile, but if needed, you can override any setting value when running a Conan command. For example, to build a debug version of the alexa module, you can add `-s build_type=Debug` to the `conan create` command:

```shell
$ conan create modules/alexa -b missing -s build_type=Debug
```

Individual packages can also define `options` which are specific to it's own build requirements. One common option that most packages define is `shared`, which is used to build either the static or dynamic library. Options can also be used to specify conditional features which should be included in the build, for example, `libcurl` defines an option called `with_nghttp2` to specify that the build should include support for `http2`. 

#### Inspect package recipes

To see which options a recipe has defined, you can use the `conan inspect` command:

```shell
$ conan inspect modules/alexa/conanfile.py 

name: aac-module-alexa
version: dev
url: https://github.com/alexa/alexa-auto-sdk
homepage: None
license: Apache-2.0
author: None
description: Auto SDK module: alexa
topics: None
generators: cmake
exports: None
exports_sources: *
short_paths: False
apply_env: True
build_policy: None
revision_mode: hash
settings: ('os', 'compiler', 'build_type', 'arch')
options:
    message_version: ANY
    shared: [True, False]
    with_aasb: [True, False]
    with_address_sanitizer: [True, False]
    with_android_libs: [True, False]
    with_coverage_tests: [True, False]
    with_docs: [True, False]
    with_engine: [True, False]
    with_jni: [True, False]
    with_latency_logs: [True, False]
    with_messages: [True, False]
    with_platform: [True, False]
    with_sensitive_logs: [True, False]
    with_unit_tests: [True, False]
default_options:
    message_version: 4.0
    shared: True
    with_aasb: True
    with_address_sanitizer: False
    with_android_libs: True
    with_coverage_tests: False
    with_docs: True
    with_engine: True
    with_jni: True
    with_latency_logs: False
    with_messages: True
    with_platform: True
    with_sensitive_logs: False
    with_unit_tests: False
deprecated: None
```

This command shows different attributes of the package, including its `options` and the default values for each option specified in `default_options`. To override a default option when building a package, you can add `-o [option]=[value]`. If you want to override an option for a specific package, then you can specify the package name as well, `-o [pkg]:[option]=[value]`. For example, to build and run unit tests for the alexa module, you can add `-o with_unit_tests=True` to the `conan create` command:

```shell
$ conan create modules/alexa -b missing -o with_unit_tests=True
```

### Remove packages from the cache

Packages can be removed from the local cache if needed by using the `conan remove` command. For example, the following command can be used to remove the alexa module from the cache:

```shell
$ conan remove aac-module-alexa -f
```

The `-f` option is used to remove the package without confirmation. To remove all Auto SDK modules from the cache, you can specify the following pattern `aac-module-*` in place of a package name, or specify `*` to remove all packages:

```shell
$ conan remove "aac-module-*" -f
$ conan remove "*"
```

> **Note:** when specifying a wildcard in the package name, you must surround the pattern with quotes.

### Use Auto SDK in other recipes

If you have your own project that uses Conan, to build an application or library for example, you can include Auto SDK packages in the requirements section of your Conan recipe. The following example shows how you can include Auto SDK modules that are built on the same development machine, in a `conanfile.txt` recipe:

```
[requires]
aac-module-core/dev
aac-module-alexa/dev
aac-module-cbl/dev
aac-module-system-audio/dev
...
```

When you build your package, as long as the Auto SDK packages have been exported to the local cache, Conan will include the specified modules when building your project. It is important to note the convention used by Auto SDK, where all module packages are named `aac-module-<module_name>`, and the default package version when building locally will be `dev` unless overridden at build time.

You can add Auto SDK modules as a requirement to `conanfile.py` recipes as well, by specifying them using the `requires` attribute in the recipe:

```python
class ConanRecipe(ConanFile):
    requires = 
        ["aac-module-core/dev","aac-module-alexa","aac-module-cbl/dev","aac-module-system-audio/dev"]
    ...
```

## Platform-specific build information

### Android

Android can be cross-compiled on either MacOS or Linux, using the NDK toolchain build requirement specified in the `aac-android` profile. To build Android compatible binaries with the Builder Tool, simply use the `--platform` or `-p` option to specify the `android` platform.

```shell
$ ./builder/build.py -p android
```

By default the android configuration used to build the SDK is defined in the `aac-android` Conan profile:

```
[settings]
os=Android
os.api_level=26
arch=armv8
build_type=Release
compiler=clang
compiler.libcxx=libc++
compiler.version=8

[build_requires]
android-sdk-tools/4.0@aac-sdk/stable
```

You can override default target architecture to build either the `armv8`, or `x86_64` version of the binaries by specifying the `--arch` or `-a` option on the command line:

```shell
$ ./builder/build.py -p android --arch=x86_64
```

> The first time you build Auto SDK for Android, the Android SDK must be downloaded and installed. This is handled by the `android-sdk-tools` recipe in Auto SDK when you build, however, several license agreements must be manually accepted before any of the Android tools can be used. These agreements will need to be accepted anytime you change the builder home directory, or clean the builder cache as well. You can optionally accept all of the license agreements by specifying `-y` or `--accept-licenses` when running the builder from the command line.

If you are using Conan directly to build Auto SDK libraries, you must specify the `--profile:host,-pr:b` and `--profile:build,-pr:b` options as part of the build command. In this case for Android, you would specify `aac-android` as the host (target) profile in your build command, in addition to explicitly specifying `default` as the build profile:


```shell
$ conan create modules/alexa -pr:h aac-android -pr:b default -b missing
```

You can override any setting for the target platform on the command line, for example, to build the `x86_64` version of the Android libraries you can specify `-s:h arch=x86_64` as an option:

```shell
$ conan create modules/alexa -pr:h aac-android -pr:b default -b missing -s:h arch=x86_64
```

### Ubuntu

Building Auto SDK for Linux on Ubuntu requires installing some additional dependencies, such as GStreamer if you are using the `system-audio` module.

#### Install GStreamer

The `system-audio` module uses GStreamer to implement the core audio interfaces, and must be installed prior to building. The following command will install the dependencies required to build with GStreamer:

```shell
$ apt install -y \
    pkg-config libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc \
    gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl \
    gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
```

#### Update the default Conan profile

You might run into an issue on Ubuntu where Conan does not detect the default `libstdc++11` setting properly, so it is recommended to check this when setting up your host environment. You can run the following command to update the default Conan profile to use the `libstdc++11` compiler option:

```shell
$ conan profile new default --detect
$ conan profile update settings.compiler.libcxx=libstdc++11 default
```

### Poky

Poky can be cross compiled on Linux using the host Poky SDK toolchain. To build Poky compatible binaries with the Builder Tool, simple use the `--platform` or `-p` option to specify the `poky` platform.

```shell
$ ./builder/build.py -p poky
```

By default the poky configuration used to build the Auto SDK is defined in the aac-poky Conan profile:

```
[settings]
compiler.version=8.2
arch=armv7hf
build_type=Release
os=Linux
compiler.libcxx=libstdc++11

[build_requires]
poky-sdk/2.6.1
```

You can override default target architecture to build either the `armv7hf`, or `armv8` version of the binaries by specifying the `--arch` or `-a` option on the command line:

```shell
$ ./builder/build.py -p poky --arch=armv8
```

> The first time you build Auto SDK for Poky, the Poky SDK must be downloaded and installed. This is handled by the `poky-sdk` recipe in Auto SDK when you build, however, several license agreements must be manually accepted before the Poky SDK can be used. These agreements will need to be accepted anytime you change the builder home directory, or clean the builder cache as well. You can optionally accept all of the license agreements by specifying `-y` or `--accept-licenses` when running the builder from the command line.

If you are using Conan directly to build Auto SDK libraries, you must specify the `--profile:host,-pr:h` and `--profile:build,-pr:b` options as part of the build command. In this case for Poky, you would specify `aac-poky` as the host (target) profile in your build command, in addition to explicitly specifying default as the build profile:

```shell
$ conan create modules/alexa -pr:h aac-poky -pr:b default -b missing
```

You can override any setting for the target platform on the command line, for example, to build the `armv8` version of the Poky libraries you can specify `-s:h arch=armv8` as an option:

```shell
$ conan create modules/alexa -pr:h aac-poky -pr:h default -b missing -s:h arch=armv8
```

### QNX

QNX can be cross compiled on Linux or MacOS using the host QNX SDP tools. To build QNX, you must install the [QNX 7.0 SDP](http://blackberry.qnx.com/en/sdp7/sdp70_download) on your host as a prerequisite. To build QNX compatible binaries with the Builder Tool, simply use the `--platform` or `-p` option to specify the `qnx` platform:

```shell
$ ./builder/build.py -p qnx
```

By default the QNX configuration used to build the Alexa Auto SDK is defined in the `aac-qnx` Conan profile:

```
[settings]
os=Neutrino
os.version=7.0
arch=armv8
compiler=qcc
compiler.version=5.4
compiler.libcxx=cxx
compiler.cppstd=None

[build_requires]
qnx-cross-compiling/7.0.0
[options]
[env]
```

You can override default target architecture to build either the `armv8`, or `x86_64` version of the binaries by specifying the `--arch` or `-a` option on the command line:

```shell
$ ./builder/build.py -p qnx --arch=x86_64
```

The Conan recipe assumes that the QNX SDP is installed in your home director: `~/qnx700`, but you can override this by setting the `qnx7sdp_path` option using the `--conan-option` or `-o` argument on the command line: 

```shell
$ ./builder/build.py -p qnx -o qnx7-sdp:qnx7sdp_path=/path/to/qnx7sdp
```

### macOS

macOS can be used as a build host for cross-compiled Android and QNX targets, as well and target for native development and testing.

### Windows

> Windows is not currently supported as a build host or target.

## Build in a Docker container

You can use Docker for native Linux builds, or any cross-compiler target that is supported with Linux, as long as the Docker container has the required build dependencies installed. For convenience, you can use the `aac-ubuntu-bionic` or `aac-ubuntu-focal` containers provided in the `conan/docker` directory of the SDK. The following commands should be run with `aac-sdk` as the working directory.

Create the `aac-ubuntu-bionic` docker image:

```shell
$ docker build -t aac/ubuntu-bionic conan/docker/aac-ubuntu-bionic
```

Build Auto SDK using the Builder Tool:

```shell
$ docker run -it -v$(pwd):/home/conan/aac-sdk --rm \
    aac/ubuntu-bionic /bin/bash -c "aac-sdk/builder/build.py"
```

The option `-v$(pwd):/home/conan/aac-sdk` specifies that we want to mount the current directory on the host machine (which should be the Auto SDK root), to `/home/conan/aac-sdk` in the Docker container file system. After starting the container, you will be able to build Auto SDK using Conan with the same commands used on your host machine.

When the build is complete, the output archive file will be saved to the mounted `aac-sdk/builder/deploy` directory of your host machine. If you inspect `aac-buildinfo.txt` in the archive, you should see that the libraries were built for `os=Linux, arch=x86_64`:

```
[settings]
    arch=x86_64
    build_type=Release
    compiler=gcc
    compiler.libcxx=libstdc++11
    compiler.version=7
    os=Linux
``` 

### Optimize build performance

When you build Auto SDK using a Docker container it can take much longer to build than it would natively on your host computer. This is because the Builder Tool home directory is specified as `aac-sdk/builder` by default, which is a directory on the host file system. File operations in general are much slower when running on a mounted volume, so this will impact the build performance. One option is to specify a different home directory on the container's volume when running the build command instead. This will greatly improve the build time, however, you should be aware that when you remove the container the cached build artifacts may be lost. The Builder Tool will still write the output archive to `aac-sdk/builder/deploy` on the mounted volume by default, even if the home directory is changed.

The following example shows how you can set the home directory using the `--home` option, when doing a build using Docker:

```shell
$ docker run -it -v$(pwd):/home/conan/aac-sdk --rm \
    aac/ubuntu-bionic /bin/bash -c "aac-sdk/builder/build.py --home /home/conan"
```
