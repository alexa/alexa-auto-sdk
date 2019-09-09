# Building the Alexa Auto SDK

The `builder` directory contains a collection of software which is required to build the Alexa Auto SDK software components for various target platforms.

**Table of Contents**

* [Overview](#overview)
* [Quick Start](#quick-start)
* [Using the Alexa Auto SDK Builder](#using-the-builder)
* [Using the Alexa Auto SDK OE Layer](#meta-aac)

## Overview<a id ="overview"></a>

The **Alexa Auto SDK Builder** is based on [OpenEmbedded](https://www.openembedded.org/) (OE), which provides a simple way to cross compile all the Alexa Auto SDK software components for various target platforms and is recommended if you want to use platforms such as **Android** and **QNX**.

For target platforms already based on OpenEmbedded infrastructure, such as [Yocto/Poky](https://www.yoctoproject.org/), you can use an OE-generated SDK or alternatively you can use `meta-aac` layer to build and install the Alexa Auto SDK into your system. See [Alexa Auto SDK OE Layer](#meta-aac) for `meta-aac` layer usage.

## Quick Start<a id="quick-start"></a>

Run the build process as follows (where `AAC_SDK_HOME` is the location into which you've installed the AAC SDK.):

```
$ ${AAC_SDK_HOME}/builder/build.sh <args>
```

## Using the Alexa Auto SDK Builder<a id="using-the-builder"></a>

Follow the instructions in this section to use the Alexa Auto SDK Builder OE-based building system to build the complete Alexa Auto SDK software for various cross targets.

### Getting Started

You can run the Alexa Auto SDK Builder either natively on a Linux host or in a Docker environment.

macOS users can run the Alexa Auto SDK Builder using [Docker for Mac](https://www.docker.com/docker-mac); see [Builder setup in a Docker environment](#docker).

>**Note:** For QNX targets, you must install the [QNX 7.0 SDP](http://blackberry.qnx.com/en/sdp7/sdp70_download) within your host.

#### Builder Setup on a Desktop Linux Host

To run the Alexa Auto SDK Builder natively, follow the guide below. **Ubuntu 18.04 LTS** and **Ubuntu 16.04 LTS** were tested and are recommended as the Linux host environment.

##### Set up OpenEmbedded/BitBake

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

#### Builder Setup in a Docker Environment <a name = "docker"></a>

To use Builder on macOS hosts, you must install [Docker Community Edition (CE) for Mac](https://www.docker.com/docker-mac) according to its official guide.

Upon first run, Builder builds the Docker image `aac/ubuntu-base:<revision>` and creates a dedicated Docker volume `buildervolume` to run the Alexa Auto SDK Builder in your Docker environment. This might take up to an hour to complete.

>**IMPORTANT NOTE on macOS:** If you are trying to build for QNX targets with macOS host, you must install QNX 7.0.0 SDP within a **case-sensitive** file system, using additional Linux installation tools. You may need to use an external drive for installation since your system file system is NOT case-sensitive by default.

#### Additional Setup for Android Targets

Make sure to install the following prerequisites on your host.

* Android Studio 3.4.1+
* Gradle 4.10.1+
* *(macOS host only)* `gsed` & `gfind`
  * May be installed through Homebrew: `brew install gnu-sed findutils`

Set the `ANDROID_HOME`to Android SDK Path. For example:

```
$ export ANDROID_HOME=~/User/<user>/Android/sdk
```

Make sure to accept the licenses in the SDK Manager.

### Running the Alexa Auto SDK Builder

To run the Alexa Auto SDK Builder, invoke the following command:

```
$ ${AAC_SDK_HOME}/builder/build.sh <platform> [options]
```

This generates a complete target installation package (where `AAC_SDK_HOME` is the location into which you've installed the Alexa Auto SDK).

>**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.

The following `platforms` are available:

* `android` for Android targets
* `qnx7` for QNX7 targets
* `native` for Generic Linux targets
* `poky` for Poky-based embedded Linux targets
* `agl` for Automotive Grade Linux targets

The following `options` are available:

* `-h,--help` to show full available options.
* `-t,--target <target>` to specify the cross compilation target. This can be a comma-separated list for multiple targets. See the table below for details. Note that if you do not specify the targets, then all possible targets for the platform will be built.
* `-g,--debug` option to build with debugging options.
* `-c,--clean` option to clean build. If you specify the OE recipe name with the `--package` option, only the specified recipe will be cleaned.
* *(Android targets only)* `--android-api <integer>` option to explicitly specify the Android API level. The default is `22`.
* *(QNX7 targets only)* `--qnx7sdp-path <path>` option to specify QNX 7.0.0 SDP installation (in host). If you run Builder within a Docker environment, host QNX SDP tools are always used, so make sure you have installed Linux tools within SDP even if your host is macOS.
* `--default-logger-enabled <enabled>` option to enable/disable the default Engine logger ( `On` | `Off` ). This default value is `On`. If you enable the default Engine logger, you must also set the `--default-logger-level <level>` and `--default-logger-sink <sink>` options, either explicitly or by accepting the default values.
* `--default-logger-level <level>` option to set the logger level for the default Engine logger ( `Verbose` | `Info` | `Metric` | `Warn` | `Error` | `Critical` ). The default value is `Info` for release builds, and `Verbose` for debug builds.
* `--default-logger-sink <sink>` option to set the logger sink for the default Engine logger ( `Console` | `Syslog` ). The default value is `Syslog` for Android build targets, and `Console` for all other build targets.

The following build targets are available:

| Platform Name              | `-t` value      |
| -------------------------- | --------------- |
| Generic Linux              | `native`        |
| Android ARMv7a             | `androidarm`    |
| Android ARMv8a             | `androidarm64`  |
| Android x86                | `androidx86`    |
| Android x86-64             | `androidx86-64` |
| QNX AArch64                | `qnx7arm64`     |
| QNX x86-64                 | `qnx7x86-64`    |

>**Note**: Using The Alexa Auto SDK Builder to build the Alexa Auto SDK for macOS targets is not currently supported.

For all other targets/toolchains, please refer to the files `meta-aac-builder/conf/machine/*.conf`. Those targets are provided by default for Poky-based Linux systems:

| Platform Name                          | `-t` value      |
| -------------------------------------- | --------------- |
| AGL AArch64                            | `aglarm64`      |
| Poky Linux ARMv7a (+NEON)              | `pokyarm`       |
| Poky Linux AArch64                     | `pokyarm64`     |

If you are building a Poky Linux ARM target, make sure you have the appropriate toolchain for your target platform prior to running the Alexa Auto SDK Builder. For example, if you are building a Poky Linux ARM target `pokyarm64` on an Ubuntu system you could download and run the the following script:

> http://downloads.yoctoproject.org/releases/yocto/yocto-2.6.1/toolchain/x86_64/poky-glibc-x86_64-core-image-sato-aarch64-toolchain-2.6.1.sh

To build all the Alexa Auto SDK modules and their dependencies for a *Poky Linux AArch64* target, run the following command:

```
$ ${AAC_SDK_HOME}/builder/build.sh poky -t pokyarm64
```

Similarly, to build an *Android ARMv7a* target, run the following command:

```
$ ${AAC_SDK_HOME}/builder/build.sh android -t androidarm
```

### Install the Built Package

After you successfully build the Auto SDK, the output directory `deploy` will be generated under the `${AAC_SDK_HOME}/builder` directory.

#### For Android targets

Within the output directory, you will find the .aar file for each module. Pre-built default platform AARs for the default Auto SDK modules are also available from the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/). You can add these AARs as dependencies of your Android project instead of building the AARs yourself with the Auto SDK Builder.

> **Note:** If you want to implement any optional modules (such as wake word support, Alexa Communications, Local Voice Control (LVC), or Device Client Metrics (DCM)), you must use the AARs generated by the Alexa Auto SDK Builder. The prebuilt AARs available in JCenter are the default Auto SDK modules only.

#### For Linux/QNX targets

Within the output directory, you will find the tar.gz archive `aac-sdk-build-<target>.tar.gz`. You can upload this package to your actual target hardware.

`aac-sdk-build` contains the following build artifacts:

* `/opt/AAC/bin/`: *cURL* binaries with *ngHTTP2* enabled
* `/opt/AAC/include/`: All dev headers for all dependencies.
* `/opt/AAC/lib/`: All shared libraries, including *AVS Device SDK*, *cURL*, *ngHTTP2*.
* `/opt/AAC/share/`: CMake files for building external Alexa Auto SDK modules.

If you've built the `native` target, then you can install these build artifacts directly on your Linux PC.

>**Note**: If you've built Auto SDK with the `-g` option, you will find an extra tar.gz archive `aac-sdk-build-<target>-dbg.tar.gz`, which contains debug symbols for later GDB use.

### Clean build

To build cleanly, use the following command to remove all caches.

```
$ ./build.sh clean
```

>**Tip**: The `build.sh` script typically performs a clean build. In most situations there is no need for a more thorough clean. When you use the `clean` option, the OpenEmbedded build cache used for building is purged. As a result, the next build may take up to an hour to complete.

### Build with mbedTLS

On some Android Samsung devices, OpenSSL causes the Alexa Auto Sample App to terminate. You can specify an additional argument `--use-mbedtls` to build the Auto SDK with mbedTLS.

## Using the Alexa Auto SDK OE Layer <a name = "meta-aac"></a>

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
