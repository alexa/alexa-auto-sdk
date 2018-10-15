# Building the Alexa Auto SDK with CMake

>**Note:** This method is recommended for advanced developers who are familiar with CMake.

Developers who want to build Alexa Auto SDK modules individually without using the [Alexa Auto SDK Builder](README.md) can use CMake to generate the build files.

## Prerequisites

The following external software components are required.

* [CMake 3.8+](https://cmake.org/)
* [AVS Device SDK v1.9](https://github.com/alexa/avs-device-sdk/releases/tag/v1.9)
* [Google Test v1.8.0](https://github.com/google/googletest)
* [RapidJSON v1.1.0](https://github.com/Tencent/rapidjson)

>**Note:** The version of *RapidJSON* that is included with AVS Device SDK source tree can be used here.

### AVS Device SDK

First you need to have built the AVS Device SDK (and its dependencies) manually. Please refer to the official [AVS Device SDK Wiki](https://github.com/alexa/avs-device-sdk/wiki/Dependencies) for build dependencies.

You also need to apply the series of patches for Alexa Auto SDK. Locate the directory `${AAC_SDK_HOME}/builder/meta-aac/recipes-avs/avs-device-sdk/avs-device-sdk-1.9` then apply the all patches within the directory.

***Patch list for Device SDK 1.9***

```
0001-SpeechEncoder-Introduce-SpeechEncoder-with-OPUS-supp.patch
0002-AIP-Only-16kHz-is-supported-for-OPUS-now.patch
0003-AIP-Initial-SpeechEncoder-support.patch
0004-Alerts-stop-delay-fix.patch
0007-Alerts-Detailed-Info-Interface.patch
```

We recommend to install AVS Device SDK with prefix `/opt/AAC` or any other arbitrary directory rather than default `/usr/local` so you can avoid contamination with system.

### RapidJSON

Copy a `include/rapidjson` directory that contains RapidJSON header files into the same `include` path in which the AVS Device SDK was installed.

The following command will use the RapidJSON that comes with the AVS Device SDK, assuming your AVS Device SDK installation path is `/opt/AAC`:

```
$ cp -R <avs-device-sdk-tree>/ThirdParty/rapidjson/rapidjson-1.1.0/include/rapidjson /opt/AAC/include
```

### Google Test

Use CMake to build and install Google Test on your PC. Remember to specify the same target install path where the AVS Device SDK was installed.

The example commands are below, assuming your AVS Device SDK installation path is `/opt/AAC`:

```
$ git clone https://github.com/google/googletest -b release-1.8.0
$ cd googletest
$ mkdir -p build && cd build
$ cmake .. -DCMAKE_INSTALL_PREFIX=/opt/AAC
$ make
$ sudo make install
```

## Run CMake for Alexa Auto SDK modules

After building the AVS Device SDK, invoke the configuration script (where `AAC_SDK_HOME` is the location you've installed the Alexa Auto SDK.):

```
$ ${AAC_SDK_HOME}/builder/build.sh cmake [options] [cmake-options]
```

The following `options` are available:

* `-h,--help` to show full available options.
* `-m,--modules <path>` option to specify which modules to build. You may specify multiple modules as a comma separated string. Default to `core,alexa,navigation,phone-control`.
* `-b,--working-dir <directory>` option to specify the working directory for build. CMake binary directory `build` will be generated under this directory. Default to `${AAC_SDK_HOME}/builder/cmake`.
* `-t,--enable-tests` option to enable unit tests.

Example:
```
$ ${AAC_SDK_HOME}/builder/build.sh cmake -DCMAKE_INSTALL_PREFIX=/opt/AAC
```

After you've configured CMake, you may invoke the build tool specified in the `-G` [CMake option](https://cmake.org/cmake/help/v3.0/manual/cmake-generators.7.html#cmake-generators). By default, it is standard Unix Makefiles, so `make` and `make install` can be used.

```
$ cd cmake/build
$ make
$ sudo make install
```
