# Building the Alexa Auto SDK with CMake

>**Note:** This method is recommended for advanced developers who are familiar with CMake.

Developers who want to build Alexa Auto SDK modules individually without using the [Alexa Auto SDK Builder](README.md) can use CMake to generate the build files.

## Prerequisites

The following external software components are required.

* [CMake 3.6+](https://cmake.org/)
* [AVS Device SDK v1.7.1](https://github.com/alexa/avs-device-sdk/releases/tag/v1.7.1)
* [Google Test](https://github.com/google/googletest)
* [RapidJSON v1.1.0](https://github.com/Tencent/rapidjson)

>**Note:** The version of *RapidJSON* that is included with AVS Device SDK source tree can be used here.

### AVS Device SDK

First you need to have built the AVS Device SDK (and its dependencies) manually.

1. Follow the *Step-by-Step Guides* from the official [AVS Device SDK Wiki](https://github.com/alexa/avs-device-sdk/wiki/macOS-Quick-Start-Guide) (hereafter called "the AVS instructions") for building macOS.
2. In the interest of simplicity and clarity, set some environment variables. (These assume that you are following the AVS instructions for building for macOS.)

   ```
   $ AVS_ROOT_PATH=/usr/local
   $ AVS_SDK_PATH=${HOME}/sdk-folder/sdk-source/avs-device-sdk
   $ AVS_SDK_BUILD=${HOME}/sdk-folder/sdk-build
   $ AAC_SDK_HOME=${HOME}/Projects/aac
   ```

3. Before building the binaries, check out SDK v1.7.1:

   ```
   $ cd ${AVS_SDK_PATH}
   $ git checkout tags/v1.7.1 -b avs
   ```

4. Continue to build the AVS Device SDK binaries, following the AVS instructions, until the binaries have been built.
5. Install the AVS Device SDK binaries. Default installation to `/usr/local`.

   ```
   $ cd ${AVS_SDK_BUILD}
   $ sudo make install
   ```

### RapidJSON

Copy a `rapidjson` directory that contains RapidJSON header files into the same `include` path in which the AVS Device SDK was installed (`${AVS_SDK_PATH}/ThirdParty/` if following the AVS instructions). The following command will use the RapidJSON that comes with the AVS Device SDK:

```
$ cp -R ${AVS_SDK_PATH}/ThirdParty/rapidjson/rapidjson-1.1.0/include/rapidjson ${AVS_ROOT_PATH}/include
```

### Google Test

Use CMake to build and install Google Test on your PC. Remember to specify the same target install path where the AVS Device SDK was installed. The following example will download the sources under your home directory.

1. Clone the [Google Test Git repository](https://github.com/google/googletest)

   ```
   $ cd ~
   $ git clone https://github.com/google/googletest
   ```

2. Build it with CMake, specifying the same install prefix as you've installed the AVS Device SDK in.

   ```
   $ cd ~/googletest
   $ rm -rf build/*
   $ mkdir -p build && cd build
   $ cmake .. -DCMAKE_INSTALL_PREFIX=${AVS_ROOT_PATH}
   $ make
   $ sudo make install
   ```

## Run CMake for Alexa Auto SDK modules

After building the AVS Device SDK, invoke the configuration script (where `AAC_SDK_HOME` is the location you've installed the Alexa Auto SDK.):

```
$ ${AAC_SDK_HOME}/builder/build.sh cmake [options] [<module>...]
```

The following `options` are available:

* `-h,--help` to show full available options.
* `-t,--enable-tests` option to enable unit tests.
* `-m,--modules-path <path>` option for searching module projects. Default to `${AAC_SDK_HOME}/modules`.
* `-b,--build-dir <directory>` option to specify the working directory for build. CMake binary directory `build` will be generated under this directory. Default to `${AAC_SDK_HOME}/builder/cmake`.
* `-r,--search-path <path>` option for searching dependencies (i.e. AVS Device SDK binaries). Defaults to `/usr/local`.
* `-i,--install-prefix <path>` option for where to install Alexa Auto SDK modules. (i.e. CMake's `CMAKE_INSTALL_PREFIX`) Defaults to `/opt/AAC`.
* `-g,--generator <generator-name>` option to generate a [variety](https://cmake.org/cmake/help/v3.0/manual/cmake-generators.7.html#cmake-generators) of output types for command-line build tools or IDE project files (e.g. XCode). Defaults to `Unix Makefiles`.
* `-x <args>` option to specify any extra options passes to CMake invocation.


For `[<module>...]`, specify a sequence of Alexa Auto SDK modules to build. Defaults to `core alexa navigation`. *(Note that the module name order is observed. In the default case, `core` will be built first, then `alexa`, then `navigation`.)*

Example:
```
$ ${AAC_SDK_HOME}/builder/build.sh cmake --search-path /usr/local core alexa navigation
```

After you've configured CMake, you may invoke the build tool specified in the `generator` setting. By default, it is standard Unix Makefiles, so `make` and `make install` can be used.

```
$ cd cmake/build
$ make
$ make install
```
