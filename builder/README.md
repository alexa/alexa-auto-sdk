# Build the Alexa Auto SDK

The `builder` directory contains a collection of software which is required to build the Alexa Auto SDK software components for various target platforms.

**Table of Contents**

* [Overview](#overview)
* [General Build Requirements](#general-build-requirements)
* [Using the Auto SDK Builder](#using-the-auto-sdk-builder)
* [Using the Auto SDK OE Layer](#using-the-auto-sdk-oe-layer)

## Overview<a id ="overview"></a>

You can build the Alexa Auto SDK software components using either the Auto SDK Builder or using the Auto SDK OE layer. 

* The Alexa Auto SDK Builder is based on [OpenEmbedded](https://www.openembedded.org/) (OE), which provides a simple way to cross compile all the Alexa Auto SDK software components for various target platforms and is recommended if you want to use platforms such as Android and QNX. To get started using the Auto SDK builder,  see [Using the Auto SDK Builder](#using-the-auto-sdk-builder).

* For target platforms already based on OpenEmbedded infrastructure, such as [Yocto/Poky](https://www.yoctoproject.org/), you can use an OE-generated SDK or alternatively you can use `meta-aac` layer to build and install the Alexa Auto SDK into your system. For details, see [Using the Auto SDK OE Layer](#using-the-auto-sdk-oe-layer).

## General Build Requirements <a id="general-build-requirements"></a>

You can build on a Linux, Unix, or macOS host of your choice. 

However, we recommend and support running a Docker environment with the following configuration:

* macOS Sierra or Ubuntu 16.04 LTS
* Processor: 2.5 GHz
* Memory: 16 Gb
* Storage: 1 Gb+ available to use.

### Build Dependencies and License Information

During the build time, the following dependencies are fetched and built for the target platform by the Alexa Auto SDK Builder. Please refer to each of the individual entities for the particular licenses.

* [AVS Device SDK v1.17](https://github.com/alexa/avs-device-sdk/)
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

> **Note**: that *OpenEmbedded-Core* will fetch and build additional components for preparing the dedicated toolchain for your environment (Such as *GNU Binutils*). Please refer to the [Yocto project](https://www.yoctoproject.org/software-overview/) to understand how it works.

### Supported Target Platforms

The Alexa Auto SDK is supported on the following platforms:

* Android 5.1 Lollipop API Level 22 or higher.
    * ARM 32-bit
    * ARM 64-bit
    * x86 64-bit
* QNX 7.0
    * ARM 64-bit
    * x86 64-bit
* AGL
    * ARM 64-bit
* Generic Linux
    * x86 64-bit
* Poky Linux
    * ARMv7a (+NEON)
    * AArch64

> **Note**: For Android targets, pre-built platform AARs for the default Auto SDK modules are available in the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/). Read the instructions about downloading and using the AARs in the [Android Sample App README](../samples/android/README.md).

## Using the Auto SDK Builder<a id="using-the-auto-sdk-builder"></a>

You can run the Alexa Auto SDK Builder either natively on a Linux host or in a Docker environment. If you are using macOS, you can run the Alexa Auto SDK Builder using [Docker for Mac](https://www.docker.com/docker-mac).

>**Note:** For QNX targets, you must install the [QNX 7.0 SDP](http://blackberry.qnx.com/en/sdp7/sdp70_download) within your host.

Follow these steps to use the Alexa Auto SDK Builder OE-based building system to build the complete Alexa Auto SDK software for various cross targets:

1. **Perform any setup necessary for your environment**.
  * [Builder Setup on a Desktop Linux Host](#builder-setup-on-a-desktop-linux-host)
  * [Builder Setup in a Docker Environment](#builder-setup-in-a-docker-environment)
  * [Additional setup for Poky Linux targets](#additional-setup-for-poky-linux-targets)
  * [Additional setup for Generic Linux ARM targets](#additional-setup-for=generic-linux-arm-targets)
  * [Additional Setup for Android Targets](#additional-setup-for-android-targets)
  
2. **Issue the following command to run the Auto SDK Builder and generate a complete target installation package** (where `AAC_SDK_HOME` is the location into which you've installed the Alexa Auto SDK). See [Builder Command Arguments](#builder-command-arguments) for details about the `platform`, `target` and `options` arguments.

  ```
$ ${AAC_SDK_HOME}/builder/build.sh <platform> -t <target> [options]
```
  After you successfully build the Auto SDK, the output directory `deploy` will be generated under the `${AAC_SDK_HOME}/builder` directory.

  >**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.

3. **Install the built package for your target platform**.

  * **For Android targets**
     
    Within the output directory, you will find the .aar file (AAR) for each module as well as an sample-core.aar file that is required to generate the Android Sample App. Pre-built default platform AARs for the default Auto SDK modules and the sample-core AAR are also available from the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/). You can add these AARs as dependencies of your Android project instead of building the AARs yourself with the Auto SDK Builder.

    >**Note:** If you want to implement any optional modules (such as wake word support, Alexa Communications, Local Voice Control (LVC), Device Client Metrics (DCM), or Voice Chrome), you must use the AARs generated by the Alexa Auto SDK Builder. The prebuilt platform AARs and sample-core AAR available in JCenter are for the default Auto SDK modules only.

  * **For Linux/QNX targets**

    Within the output directory, you will find the tar.gz archive `aac-sdk-build-<target>.tar.gz`. You can upload this package to your actual target hardware.

    `aac-sdk-build` contains the following build artifacts:

     * `/opt/AAC/bin/`: *cURL* binaries with *ngHTTP2* enabled
     * `/opt/AAC/include/`: All dev headers for all dependencies.
     * `/opt/AAC/lib/`: All shared libraries, including *AVS Device SDK*, *cURL*, *ngHTTP2*.
     * `/opt/AAC/share/`: CMake files for building external Alexa Auto SDK modules.
      
    If you've built the `native` target, then you can install these build artifacts directly on your Linux PC.

    >**Note**: If you've built Auto SDK with the `-g` option, you will find an extra tar.gz archive `aac-sdk-build-<target>-dbg.tar.gz`, which contains debug symbols for later GDB use.


### Builder Setup on a Desktop Linux Host <a id ="builder-setup-on-a-desktop-linux-host"></a>
To run the Alexa Auto SDK Builder natively, follow the guide below. **Ubuntu 18.04 LTS** and **Ubuntu 16.04 LTS** were tested and are recommended as the Linux host environment.

#### Set up OpenEmbedded/BitBake
First you will need to set up [OpenEmbedded-Core](https://www.openembedded.org/wiki/OpenEmbedded-Core).

The following example installs OpenEmbedded-Core and BitBake under your home directory. Note that the variable `OE_CORE_PATH` must be pointed at the OpenEmbedded-Core source directory.

```
$ cd ~
$ git clone git://git.openembedded.org/openembedded-core oe-core -b rocko
$ cd oe-core
$ git clone git://git.openembedded.org/bitbake -b 1.36
$ export OE_CORE_PATH=$(pwd)
```
The minimum requirements to run OpenEmbedded on an Ubuntu Linux host are as follows:

```
$ apt-get install chrpath diffstat gawk texinfo \
python python3 wget unzip build-essential cpio \
git-core libssl-dev quilt cmake \
libsqlite3-dev libarchive-dev python3-dev \
libdb-dev libpopt-dev zlib1g-dev
```

>**Note:** For Linux targets, you must install libssl-dev as well:

```
$ apt-get install libssl-dev
```
### Builder Setup in a Docker Environment <a name ="builder-setup-in-a-docker-environment"></a>

To use Builder on macOS hosts, you must install [Docker Community Edition (CE) for Mac](https://www.docker.com/docker-mac) according to its official guide.

Upon first run, Builder builds the Docker image `aac/ubuntu-base:<revision>` and creates a dedicated Docker volume `buildervolume` to run the Alexa Auto SDK Builder in your Docker environment. This might take up to an hour to complete.

>**Note:** If you are upgrading from Auto SDK v1.6.0 or earlier to Auto SDK v2.0.0 or later, be sure to clean the `buildervolume` Docker volume before performing the upgrade.

>**IMPORTANT NOTE for macOS:** If you are trying to build for QNX targets on a macOS host, you must install QNX 7.0.0 SDP within a **case-sensitive** file system, using additional Linux installation tools. You may need to use an external drive for installation since your system file system is NOT case-sensitive by default.

### Additional setup for Poky Linux targets <a id ="additional-setup-for-poky-linux-targets"></a>
If you are building a Poky Linux ARM target, make sure you have the appropriate toolchain for your target platform prior to running the Alexa Auto SDK Builder. For example, if you are building a Poky Linux ARM target `pokyarm64` on an Ubuntu system you can download and run the [Poky ARM toolchain](http://downloads.yoctoproject.org/releases/yocto/yocto-2.6.1/toolchain/x86_64/poky-glibc-x86_64-core-image-sato-aarch64-toolchain-2.6.1.sh).

Auto SDK Builder will use `/opt/poky/2.6.1` as a root SDK directory by default. You can change this behavior with the `--poky-sdk` option.

### Additional setup for Generic Linux ARM targets <a id ="additional-setup-for=generic-linux-arm-targets"></a>

#### Linaro Toolchain

>**Note:** The Linaro Linux targets are available as previews only and have not been tested fully.

Make sure to install the following prerequisites on your host:

* [Linaro Linux targeted binary toolchain](https://www.linaro.org/downloads/) (Version `gcc-linaro-7.4.1-2019.02` is recommended)

Linaro toolchains are typically named in the following format: `<version>-<build>-<host>` (e.g. `gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf`)

The Auto SDK Builder will try to find the Linaro toolchain in the above format, under the `${HOME}` directory by default. For ARMv7A HF targets, you should install the toolchain in `${HOME}/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf`.

To change this behavior, you should use the `--linaro-prefix` option to specify a prefix in `<path>/<version>-` format. (Defaults to `${HOME}/gcc-linaro-7.4.1-2019.02-`, with `<build>-<host>` determined by the Auto SDK Builder.)

#### Cross sysroots

Additionally, you need the cross sysroot directory for your cross targets. The Auto SDK Builder will try to find sysroot under the `${HOME}/sysroots` directory by default. The Sysroot directory name must match the `<host>` value from the toolchain. For ARMv7A HF targets, you should install your copy of sysroot in `${HOME}/sysroots/arm-linux-gnueabihf`.

To change this behavior, you should specify the search path via the `--linaro-sysroots` option. (Defaults to `${HOME}/sysroots`).

### Additional Setup for Android Targets <a id = "additional-setup-for-android-targets"></a>
* Make sure to install the following prerequisites on your host:

  * Android Studio 3.4.1+
  * Gradle 4.10.1 - 5.6.2 (5.1.1 - 5.6.2 if you are using Android Studio)

  >**Note:**  You must ensure that the Gradle version you are using is compatible with the Android Studio version you are using. See the [Android Gradle Plugin Release Notes](https://developer.android.com/studio/releases/gradle-plugin#updating-gradle) for information about matching Android Studio versions to Gradle versions.

  * *(macOS host only)* `gsed`, `gfind`, and `coreutils`</p><p>
    You can use Homebrew to install the required macOS prerequisites:
  
      ```
      brew install gnu-sed findutils coreutils
      ```

* Set the `ANDROID_HOME`to Android SDK Path. For example:

  ```
  $ export ANDROID_HOME=~/User/<user>/Android/sdk
  ```

* Make sure to accept the licenses in the SDK Manager.

### Builder Command Arguments <a id = "builder-command-arguments"></a>

#### `platform`
The following `platform` values are available:

* `linux` for Linux targets
* `android` for Android targets
* `qnx7` for QNX7 targets
* `agl` for Automotive Grade Linux targets

#### `target`
The table below lists the `target` values available to specify the cross compilation target.

You must specify at least one `target`. For multiple targets, use a comma-separated list; for example:

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
| AGL AArch64                | `agl`      | `aglarm64`      |

>**Note:** The `linaroarmel` and `linaroarmhf` targets are available as previews only and have not been tested fully.

For all other targets/toolchains, please refer to the files `meta-aac-builder/conf/machine/*.conf`. Those targets are provided by default for Poky-based Linux systems.

#### `options`
The availabe `options` include:

* `-h,--help` to show full available options.
* `-g,--debug` option to build with debugging options.
* *(Android targets only)* `--android-api <integer>` option to explicitly specify Android API level. The default is `22`.
* *(QNX7 targets only)* `--qnx7sdp-path <path>` option to specify QNX 7.0.0 SDP installation (in host). If you run Builder within Docker environment, host QNX SDP tools are always used. So make sure you have installed Linux tools within SDP even if your host is macOS.
* *(Poky Linux & AGL targets only)* `--pokysdk-path <path>` option to specify Poky SDK installation root path.
* *(Generic Linux ARM targets only)* `--linaro-prefix <prefix>` option to specify the path where the Linaro toolchain is located. See the section *Additional setup for Generic Linux ARM targets* above for the details.
* *(Generic Linux ARM targets only)* `--linaro-sysroots <path>` option to specify the path where the cross sysroot directories are located. See the section *Additional setup for Generic Linux ARM targets* above for the details.
* `--default-logger-enabled <enabled>` option to enable/disable the default engine logger ( `On` | `Off` ). This default value is `On`. If you enable the default Engine logger, you must also set the `--default-logger-level <level>` and `--default-logger-sink <sink>` options, either explicitly or by accepting the default values.
* `--default-logger-level <level>` option to set the logger level for the default engine logger ( `Verbose` | `Info` | `Metric` | `Warn` | `Error` | `Critical` ). The default value is `Info` for release builds, and `Verbose` for debug builds.
* `--default-logger-sink <sink>` option to set the logger sink for the default engine logger ( `Console` | `Syslog` ). The default value is `Syslog` for Android build targets, and `Console` for all other build targets.
* `--enable-sensitive-logs <Off | On>` option to enable or disable the inclusion of sensitive data in debugging logs. The  default value is `Off`. If you enable sensitive logs, you must also build with debugging options (`--debug`).

  >**Important:** If you enable sensitive logs, make sure you redact any sensitive data if posting logs publicly.

### Clean build

To build cleanly, use the following command to remove all caches.

```
$ ./build.sh clean
```

>**Tip**: The `build.sh` script typically performs a clean build. In most situations there is no need for a more thorough clean. When you use the `clean` option, the OpenEmbedded build cache used for building is purged. As a result, the next build may take up to an hour to complete.

### Building with mbedTLS

On some Android Samsung devices, OpenSSL causes the Alexa Auto Sample App to terminate. You can specify an additional argument `--use-mbedtls` to build the Auto SDK with mbedTLS.

## Using the Auto SDK OE Layer <a name ="using-the-auto-sdk-oe-layer"></a>

If you want to integrate the Alexa Auto SDK software into an existing OpenEmbedded-based system, you can use the *Alexa Auto SDK OE Layer* a.k.a `meta-aac`, without using the *Alexa Auto SDK Builder*.

>**Note**: For Android and QNX targets, you should use the Alexa Auto SDK Builder since the Alexa Auto SDK OE Layer method may require advanced OpenEmbedded system administration skills.

The recommended and tested platform is **Poky Linux 2.4 (rocko)**.

### Adding layers and module recipes

You need to add the following OE layers into your setup.

* Alexa Auto SDK OE layer: `${AAC_SDK_HOME}/builder/meta-aac`

Additionally, you may need to add the individual Alexa Auto SDK module recipes by adding them to `BBFILES`. To add all SDK modules, you can simply add the following line to your `bblayers.conf`:


```
BBFILES += "${AAC_SDK_HOME}/modules/*/*.bb"
```
>**Note: `${AAC_SDK_HOME}` needs to be interpreted as the actual full path.

### cURL with ngHTTP2

You must configure the `curl` package with the `nghttp2` feature enabled. The `meta-aac` layer defines a default `PACKAGECONFIG` for `curl` but if your system has its own definition, you need to modify `PACKAGECONFIG` to include `nghttp2`.

>**Note:** We provide the default `nghttp2` recipe within the `meta-aac` layer, but you may use other alternatives.
