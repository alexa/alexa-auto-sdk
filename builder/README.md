# Build the Alexa Auto SDK

The `builder` directory contains a collection of software which is required to build the Alexa Auto SDK components for various target platforms.

<!-- omit in toc -->
## Table of Contents
- [Overview](#overview)
- [General Build Requirements and Recommendations](#general-build-requirements-and-recommendations)
- [Supported Platforms and Targets](#supported-platforms-and-targets)
- [Build Dependencies and License Information](#build-dependencies-and-license-information)
- [Using the Auto SDK Builder](#using-the-auto-sdk-builder)
  - [Preparing the Host](#preparing-the-host)
  - [Using the Auto SDK Builder](#using-the-auto-sdk-builder)
  - [Building AACS Using the Auto SDK Builder (Local Build Flavor)](#building-aacs-using-the-auto-sdk-builder-local-build-flavor)
  - [Building AACS Using the Pre-built AARs (Remote Build Flavor)](#building-aacs-using-the-pre-built-aars-remote-build-flavor)
  - [Builder Setup on a Desktop Linux Host](#builder-setup-on-a-desktop-linux-host)
  - [Builder Setup in a Docker Environment](#builder-setup-in-a-docker-environment)
  - [Additional Setup for Poky Linux Targets](#additional-setup-for-poky-linux-targets)
  - [Additional Setup for Generic Linux ARM Targets](#additional-setup-for-generic-linux-arm-targets)
  - [Additional Setup for Android Targets](#additional-setup-for-android-targets)
  - [Builder Command Arguments](#builder-command-arguments)
  - [Clean build](#clean-build)
  - [Building with mbedTLS](#building-with-mbedtls)
- [Using the Auto SDK OE Layer](#using-the-auto-sdk-oe-layer)
  - [Adding layers and module recipes](#adding-layers-and-module-recipes)
  - [cURL with ngHTTP2](#curl-with-nghttp2)

## Overview

There are two methods to build the Alexa Auto SDK: 

* [Auto SDK Builder](#using-the-auto-sdk-builder) (Recommended): The builder is based on [OpenEmbedded (OE)](https://www.openembedded.org/), which provides a simple way to cross compile all the Auto SDK components for various target platforms. If you want to build the Auto SDK with Alexa Auto Client Service (AACS), you must use this method. 

* [Auto SDK OE layer](#using-the-auto-sdk-oe-layer): For target platforms already based on the OpenEmbedded infrastructure, such as [Yocto/Poky](https://www.yoctoproject.org/), you can use an OE-generated SDK or a `meta-aac` layer to build and install the Auto SDK.

>**Note**: For Android and QNX targets, use the Auto SDK Builder because the Auto SDK OE Layer method may require advanced OpenEmbedded system administration skills.
  
The Auto SDK home directory is the directory to which you clone the `alexa-auto-sdk` repository and is represented as `${AAC_SDK_HOME}` in this document.

## General Build Requirements and Recommendations

You can build the Auto SDK on a Linux, Unix, or macOS host. The builder can run either natively on a Linux host or in a Docker environment (recommended). For information about Docker, see the [Docker documentation](https://docs.docker.com/get-docker).

The following list describes the required host configuration:

* Operating system:
  * macOS Sierra 
  * Ubuntu 16.04 LTS or Ubuntu 18.04 LTS
* Processor: 2.5 GHz
* Memory: 16 Gb
* Storage: 1 Gb+ available to use

For QNX targets, you must install the [QNX 7.0 SDP](http://blackberry.qnx.com/en/sdp7/sdp70_download) on your host.

## Supported Platforms and Targets

You can build the Auto SDK for the following operating systems (platforms) and hardware architectures (targets):

* Android 5.1 Lollipop API Level 22 or higher.
    * ARM 32-bit
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

## Build Dependencies and License Information

During build time, the Auto SDK Builder fetches and builds the dependencies appropriate for the  platform. For license information about the dependencies, go to the following websites:

* [AVS Device SDK v1.19.1](https://github.com/alexa/avs-device-sdk/)
  * [cURL 7.65.3](https://curl.haxx.se/)
  * [ngHTTP2 1.39.1](https://github.com/nghttp2/nghttp2)
  * [SQLite3 3.20.0](https://www.sqlite.org/) or later
  * [OpenSSL 1.1.0](https://www.openssl.org/) or later
* [Google Test v1.8.0](https://github.com/google/googletest)
* [libopus 1.3.1](https://opus-codec.org/)
* [OpenEmbedded-Core Rocko](https://www.openembedded.org/wiki/OpenEmbedded-Core)
* [BitBake](https://www.yoctoproject.org/software-overview/)
* [Android Toolchain](https://www.android.com/)
  * [NDK r20](https://developer.android.com/ndk/)
  * [SDK Tools 26.0.1](https://developer.android.com/studio/releases/sdk-tools)

> **Note**: OpenEmbedded-Core fetches and builds additional components to prepare the dedicated toolchain for your environment (for example, GNU Binutils). For information about how OpenEmbedded-Core works, see the [Yocto project](https://www.yoctoproject.org/software-overview/).

## Using the Auto SDK Builder
This section describes how to build the Auto SDK with the builder. 

### Preparing the Host
Depending on your host, you might need to perform builder setup before using the builder, as described in the following sections:

  * [Builder Setup on a Desktop Linux Host](#builder-setup-on-a-desktop-linux-host)
  * [Builder Setup in a Docker Environment](#builder-setup-in-a-docker-environment)
  * [Additional setup for Poky Linux targets](#additional-setup-for-poky-linux-targets)
  * [Additional setup for Generic Linux ARM targets](#additional-setup-for=generic-linux-arm-targets)
  * [Additional Setup for Android Targets](#additional-setup-for-android-targets)
  
### Using the Auto SDK Builder
This section describes how to build the Auto SDK and install the built package.

#### Running the Build Script
Enter the following command to build the Auto SDK and generate an installation package:

`$ ${AAC_SDK_HOME}/builder/build.sh <platform> -t <target> [options]`
    
See [Builder Command Arguments](#builder-command-arguments) for details about the `platform`, `target` and `options` arguments. This command creates the `deploy` directory in `${AAC_SDK_HOME}/builder`. With the `-g` option, the builder creates an archive named `aac-sdk-build-<target>-dbg.tar.gz`, which contains debug symbols for later GDB use.

>**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.
#### Installing the Built Package on Android Targets
The output directory contains the .aar file (AAR) for each module and a sample-core.aar file required to generate the Android Sample App. Pre-built default platform AARs for the default Auto SDK modules and the sample-core AAR are also available from the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/). You can add these AARs as dependencies of your Android project instead of building the AARs with the Auto SDK Builder.

>**Note:** If you want to implement any optional modules (such as wake word support, Alexa Communications, Local Voice Control (LVC), Device Client Metrics (DCM), or Voice Chrome), you must use the AARs generated by the Auto SDK Builder. The prebuilt platform AARs and sample-core AAR available in JCenter are for the default modules only. 

#### Installing the Built Package on Linux or QNX Targets

The output directory contains the tar.gz archive, `aac-sdk-build-<target>.tar.gz`. You can upload this package to your target.

The `aac-sdk-build` directory contains the following build artifacts:

  * `/opt/AAC/bin/`: *cURL* binaries with *ngHTTP2* enabled.
  * `/opt/AAC/include/`: All dev headers for all dependencies.
  * `/opt/AAC/lib/`: All shared libraries, including *AVS Device SDK*, *cURL*, *ngHTTP2*.
  * `/opt/AAC/share/`: CMake files for building external Alexa Auto SDK modules.
      
If you build for the `native` target, you can install these build artifacts directly on your Linux host.

### Building AACS Using the Auto SDK Builder (Local Build Flavor)
If your platform is Android, you can build the Auto SDK with AACS. When running the builder script, you must specify the `--aacs-android` option. The following example shows how to build the Auto SDK for the `androidarm` target with AACS and the optional Alexa Communications and Local Voice Control modules:

`$ ${AAC_SDK_HOME}/builder/build.sh android -t androidarm --aacs-android extensions/extras/alexacomms extensions/extras/local-voice-control`

See [Builder Command Arguments](#builder-command-arguments) for details about the `platform`, `target` and `options` arguments. The builder creates the `builder/deploy/aar`, which contains the AARs, and `builder/deploy/apk`, which contains the AACS APK. With the `-g` option, the command also creates an archive named `aac-sdk-build-<target>-dbg.tar.gz`, which contains debug symbols for later GDB use.

>**Note:** You do not need to specify the AASB extension when building the Auto SDK with AACS. The AASB extension is included by default.

>**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.

The unsigned version of the AACS APK is in `builder/deploy/apk`. Go to [Signing the AACS APK](#signing-the-aacs-apk) for information about signing the APK. 

### Building AACS Using the Pre-built AARs (Remote Build Flavor)
The AACS Gradle build is configured to use JCenter to always pull the latest release artifacts during compilation. The pre-built platform AARs for the default modules and the AARs required to build AACS are available in the JCenter repo. To run the build, enter the following commands:

```
    $ cd ${AAC_SDK_HOME}/platforms/android/alexa-auto-client-service/android-service
    $ gradle assembleRemoteRelease
```

#### Signing the AACS APK
To sign the AACS APK, follow these steps:

1. Create a custom keystore using the following command, or skip to the next step and use an existing keystore:
    
    `keytool -genkey -v -keystore <keystore_name>.keystore -alias <alias> -keyalg RSA -keysize 2048 -validity 10000`

2. Enter the following command to change to the directory where the APK is:
   
   `cd ${AAC_SDK_HOME}/builder/deploy/apk`

3. Enter one of the following commands to optimize the APK files, depending on whether you have the local or remote build flavor:
   
    `zipalign -v -p 4 service-local-release-unsigned.apk service-local-release-unsigned-aligned.apk`

    `zipalign -v -p 4 service-local-release-unsigned.apk service-remote-release-unsigned-aligned.apk`

    `zipalign` is included in the Android SDK Build Tools. On a Mac, it is usually located in this directory:
    
     ~/Library/Android/sdk/build-tools/<Android_SDK_Build_Tools_version>/zipalign

4. Enter one of the following commands to sign the APK by using your keystore, depending on whether you have the local or remote build flavor:
   
    `apksigner sign --ks <path_to_keystore>/<keystore_name>.keystore  --ks-pass pass:<passphrase> --out service-app-release.apk service-local-release-unsigned-aligned.apk`
    `apksigner sign --ks <path_to_keystore>/<keystore_name>.keystore  --ks-pass pass:<passphrase> --out service-app-release.apk service-remote-release-unsigned-aligned.apk`

    When prompted, enter the passphrase that you set when you created the keystore. The `apksigner` tool is in Android SDK Build Tools 24.0.3 or higher. On a Mac, it is usually in the following directory:
    
     ~/Library/Android/sdk/build-tools/<Android_SDK_Build_Tools_version>/apksigner

### Builder Setup on a Desktop Linux Host
Follow these steps to prepare for building the Auto SDK natively on the Linux host: 

1. Set up [OpenEmbedded-Core](https://www.openembedded.org/wiki/OpenEmbedded-Core). The following example installs `OpenEmbedded-Core` and `BitBake` under your home directory. The variable `OE_CORE_PATH` must point to the `OpenEmbedded-Core` source directory.

```
    $ cd ~
    $ git clone git://git.openembedded.org/openembedded-core oe-core -b rocko
    $ cd oe-core
    $ git clone git://git.openembedded.org/bitbake -b 1.36
    $ export OE_CORE_PATH=$(pwd)
```
2. Enter the following command to ensure that the Ubuntu host meets the minimum requirements to run `OpenEmbedded`:

```
    $ apt-get install chrpath diffstat gawk texinfo \
    python python3 wget unzip build-essential cpio \
    git-core libssl-dev quilt cmake \
    libsqlite3-dev libarchive-dev python3-dev \
    libdb-dev libpopt-dev zlib1g-dev
```

3. For a Linux target, enter the following command to install `libssl-dev`:

```
    $ apt-get install libssl-dev
```
### Builder Setup in a Docker Environment

To use the builder on a macOS host, install [Docker Community Edition (CE) for Mac](https://www.docker.com/docker-mac) according to its official guide.

Follow these important guidelines:

* If you are upgrading from Auto SDK v1.6.0 or earlier to Auto SDK v2.0.0 or later, be sure to clean the `buildervolume` Docker volume before performing the upgrade.

* To build for QNX targets on a macOS host, you must install QNX 7.0.0 SDP within a **case-sensitive** file system, using additional Linux installation tools. You may need to use an external drive for installation because your system file system is NOT case-sensitive by default.

* When compiling the Auto SDK for the first time, the builder instantiates the Docker environment and starts building. On macOS, the builder might crash with this error message:
   
  `g++: internal compiler error: Killed (program cc1plus)`
  
   This error occurs when Docker runs out of memory. By default, Docker is set to use 2 GB of memory, which is sufficient on a host with 2 cores. If your host has more than 2 cores, allocate 6 GB to Docker to resolve this issue.

Upon first run, the builder builds the Docker image `aac/ubuntu-base:<revision>` and creates a dedicated Docker volume `buildervolume` to run the Auto SDK Builder in your Docker environment. This might take up to an hour to complete.

### Additional Setup for Poky Linux Targets
To build a Poky Linux ARM target, make sure you have the appropriate toolchain for your target platform prior to running the Auto SDK Builder. For example, if you are building a Poky Linux ARM target `pokyarm64` on an Ubuntu system, you can download and run the [Poky ARM toolchain](http://downloads.yoctoproject.org/releases/yocto/yocto-2.6.1/toolchain/x86_64/poky-glibc-x86_64-core-image-sato-aarch64-toolchain-2.6.1.sh).

Auto SDK Builder uses `/opt/poky/2.6.1` as a root SDK directory by default. You can change this behavior with the `--poky-sdk` option.

### Additional Setup for Generic Linux ARM Targets

#### Linaro Toolchain

>**Note:** The Linaro Linux targets are available as previews only and have not been tested fully.

A Linaro Linux target requires the [Linaro Linux targeted binary toolchain](https://www.linaro.org/downloads/). Version `gcc-linaro-7.4.1-2019.02` is recommended.

Linaro toolchains are typically named in the following format:

`<version>-<build>-<host>`

The following example shows a Linaro toolchain:

`gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf`)`

The Auto SDK Builder tries to find the Linaro toolchain in this format, under the `${HOME}` directory, by default. For ARMv7A HF targets, install the toolchain in `${HOME}/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf`.

To change this behavior, use the `--linaro-prefix` option to specify a prefix in the following format:

`<path>/<version>-`

The default prefix is `${HOME}/gcc-linaro-7.4.1-2019.02-`. The `<build>-<host>` portion following the prefix is determined by the Auto SDK Builder.

#### Cross sysroots

Additionally, you need the cross `sysroot` directory for your cross targets. The Auto SDK Builder tries to find `sysroot` under the `${HOME}/sysroots` directory by default. The `sysroot` directory name must match the `<host>` value from the toolchain. For ARMv7A HF targets, install your copy of `sysroot` in `${HOME}/sysroots/arm-linux-gnueabihf`.

To change this behavior, specify the search path via the `--linaro-sysroots` option. The default search path is `${HOME}/sysroots`.

  >**Note:**  The sysroot from the Linaro website is extremely minimal and is not sufficient for building the Auto SDK. For example, it does not include the required connectivity, multimedia, and other support packages. Linaro Linux target is intended for generic Linux ARM targets, which have their own build or vendor provided sysroots. Make sure to use the sysroot for your target root filesystem, not the Linaro sysroot.

### Additional Setup for Android Targets
* Install the following required software on your host:

  * Android Studio 3.4.1+
  * Gradle 5.6.4 or above

  >**Note:**  You must ensure that your Gradle version is compatible with the Android Studio version that you use for building the Auto SDK. See the [Android Gradle Plugin Release Notes](https://developer.android.com/studio/releases/gradle-plugin#updating-gradle) for information about matching Android Studio versions to Gradle versions.

  * *(macOS host only)* `gsed`, `gfind`, and `coreutils`</p><p>
    You can use Homebrew to install the required macOS prerequisites:
  
      ```
      brew install gnu-sed findutils coreutils
      ```

* Set the `ANDROID_HOME`to Android SDK path. For example:

  ```
  $ export ANDROID_HOME=~/User/<user>/Android/sdk
  ```

* Make sure to accept the licenses in the SDK Manager.

### Builder Command Arguments

#### `platform`
The following `platform` values are available:

* `linux` for Linux targets
* `android` for Android targets
* `qnx7` for QNX7 targets

#### `target`
The table below lists the `target` values available to specify the cross compilation target.

You must specify at least one `target`. For multiple targets, use a comma-separated list as in the following example:

```
$ ${AAC_SDK_HOME}/builder/build.sh linux -t native,pokyarm,pokyarm64
```

| Platform Name              | `platform` | `target`      |
| -------------------------- | -----------| --------------- |
| Generic Linux (x86-64)     | `linux`    | `native`        |
| Poky Linux Cortex-A8 HF    | `linux`    | `pokyarm`       |
| Poky Linux AArch64         | `linux`    | `pokyarm64`     |
| Generic Linux ARMv7a       | `linux`    | `linaroarmel`   |
| Generic Linux ARMv7a HF    | `linux`    | `linaroarmhf`   |
| Android ARMv7a             | `android`  | `androidarm`    |
| Android ARMv8a             | `android`  | `androidarm64`  |
| Android x86-64             | `android`  | `androidx86-64` |
| QNX AArch64                | `qnx7`     | `qnx7arm64`     |
| QNX x86-64                 | `qnx7`     | `qnx7x86-64`    |
|

>**Note:** The `linaroarmel` and `linaroarmhf` targets are available as previews only and have not been tested fully.

For all other targets or toolchains, see the files `meta-aac-builder/conf/machine/*.conf`. Those targets are provided by default for Poky-based Linux systems.

#### `options`
The following list describes the available `options`:

* `-h,--help`: Show full available options.
* `-g,--debug`: Build with debugging options.
* *(Android targets only)* `--android-api <integer>`: Explicitly specify the Android API level. The default is `22`.
* *(QNX7 targets only)* `--qnx7sdp-path <path>`: Specify QNX 7.0.0 SDP installation (in host). If you run the builder within the Docker environment, host QNX SDP tools are always used. So make sure you have installed Linux tools within SDP even if your host is macOS.
* *(Poky Linux target only)* `--pokysdk-path <path>`: Specify Poky SDK installation root path.
* *(Generic Linux ARM targets only)* `--linaro-prefix <prefix>`: Specify the path where the Linaro toolchain is located. See the section *Additional setup for Generic Linux ARM targets* for details.
* *(Generic Linux ARM targets only)* `--linaro-sysroots <path>`: Specify the path where the cross `sysroot` directories are located. See the section *Additional setup for Generic Linux ARM targets* for details.
* `--default-logger-enabled <enabled>`: Enable/disable the default engine logger ( `On` | `Off` ). The default value is `On`. If you enable the default Engine logger, you must also set the `--default-logger-level <level>` and `--default-logger-sink <sink>` options, either explicitly or by accepting the default values.
* `--default-logger-level <level>`: Set the logger level for the default engine logger ( `Verbose` | `Info` | `Metric` | `Warn` | `Error` | `Critical` ). The default value is `Info` for release builds, and `Verbose` for debug builds.
* `--default-logger-sink <sink>`: Set the logger sink for the default engine logger ( `Console` | `Syslog` ). The default value is `Syslog` for Android build targets, and `Console` for all other build targets.
* `--enable-sensitive-logs <Off | On>`: Enable or disable the inclusion of sensitive data in debugging logs. The default value is `Off`. If you enable sensitive logs, you must also build with debugging options (`--debug`).
    >**Important:** If you enable sensitive logs, make sure you redact any sensitive data if posting logs publicly.

* `--aacs-android`: Build AACS including the required components AASB, IPC, and constants. 

### Clean build

To build cleanly, use the following command to remove all caches.

```
$ ./build.sh clean
```

>**Tip**: The `build.sh` script typically performs a clean build. In most situations there is no need for a more thorough clean. Using the `clean` option purges the `OpenEmbedded` build cache. As a result, the next build may take up to an hour to complete.

### Building with mbedTLS

On some Android Samsung devices, OpenSSL causes the Alexa Auto Sample App to terminate. The alternative is to use mbedTLS. To build the SDK with mbedTLS, specify the `--use-mbedtls` argument.

## Using the Auto SDK OE Layer

If you want to integrate the Auto SDK into an existing OpenEmbedded-based system, you can use the Alexa Auto SDK OE Layer, which is also known as `meta-aac`, instead of the Auto SDK Builder.

The recommended and tested platform is Poky Linux 2.4 (rocko).

### Adding layers and module recipes

Add the following OE layers to your setup:

* Alexa Auto SDK OE layer, which is at the following location:
  
  `${AAC_SDK_HOME}/builder/meta-aac`

* You may need to add the individual Auto SDK module recipes to `BBFILES`. To add all modules, simply add the following line to the `bblayers.conf` file:

```
BBFILES += "${AAC_SDK_HOME}/modules/*/*.bb"
```
>**Note: `${AAC_SDK_HOME}` needs to be interpreted as the actual full path.

### cURL with ngHTTP2

You must configure the `curl` package with the `nghttp2` feature enabled. The `meta-aac` layer defines a default `PACKAGECONFIG` for `curl`. However, if your system has its own definition, you must modify `PACKAGECONFIG` to include `nghttp2`.

>**Note:** The default `nghttp2` recipe is included in the `meta-aac` layer, but you may use other alternatives.
