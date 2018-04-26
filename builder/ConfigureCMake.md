# Building Alexa Auto Core with CMake

>**Note:** This method is recommended for advanced developers who are familiar with CMake.

Developers who want to build Alexa Auto Core modules individually without using the [Alexa Auto Core Builder](README.md) can use CMake to generate the build files.

## Prerequisites

The following external software components are required.

* [CMake 3.6+](https://cmake.org/)
* [AVS Device SDK v1.4](https://github.com/alexa/avs-device-sdk/releases/tag/v1.4)
* [Google Test](https://github.com/google/googletest)
* [RapidJSON v1.1.0](https://github.com/Tencent/rapidjson)

>**Note:** The version of *RapidJSON* that is included with AVS Device SDK source tree can be used here.

### AVS Device SDK

First you need to have built the AVS Device SDK (and its dependencies) manually.

1. Follow the *Step-by-Step Guides* from the offical [AVS Device SDK Wiki](https://github.com/alexa/avs-device-sdk/wiki) (hereafter called "the AVS instructions").
2. In the interest of simplicity and clarity, set some environment variables. (These assume that you are following the AVS instructions for building for macOS.)

   ```
   $ AVS_ROOT_PATH=/usr/local
   $ AVS_SDK_PATH=${HOME}/sdk-folder/sdk-source/avs-device-sdk
   ```

3. Before building the binaries, check out SDK v1.4:

   ```
   $ cd ${AVS_SDK_PATH}
   $ git checkout tags/v1.4 -b avs
   ```

4. Continue to build the AVS Device SDK binaries, following the AVS instructions, until the binaries have been built.
5. Install the AVS Device SDK binaries.

   ```
   $ cd ${AVS_SDK_PATH}
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

## Run CMake for Alexa Auto Core modules

After building the AVS Device SDK, invoke the configuration script (where `AAC_SDK_HOME` is the location you've installed the AAC SDK.):

```
$ ${AAC_SDK_HOME}/builder/configure-cmake.sh <options> <modules>
```

The following `options` are available:

* `-r <avs-search-path>` option for searching dependencies (i.e. AVS Device SDK binaries). Defaults to `/usr/local`.
* `-i <install-prefix>` option for where to install AAC modules. (i.e.  CMake's `CMAKE_INSTALL_PREFIX`) Defaults to `/opt/AAC`.
* `-g <generator>` option to generate a [variety](https://cmake.org/cmake/help/v3.0/manual/cmake-generators.7.html#cmake-generators) of output types for command-line build tools or IDE project files (e.g. XCode). Defaults to `Unix Makefiles`.
* `-m <modules-path>` option for searching module projects. Default to `${AAC_SDK_HOME}/modules`.
* `-t` option to enable unit tests.
* `-x` option to specify any extra options passes to CMake invocation.
* `-b` option to specify the working directory for build. Default to `${AAC_SDK_HOME}/builder/build-cmake`.

For `<modules>`, specify a sequence of AAC modules to build. Defaults to `core alexa navigation`. *(Note that the module name order is observed. In the default case, `core` will be built first, then `alexa`, then `navigation`.)*

After you've configured CMake, you may invoke the build tool specified in the `generator` setting. By default, it is standard Unix Makefiles, so `make` and `make install` can be used.

```
$ cd build-cmake/tmp
$ make
$ make install
```
