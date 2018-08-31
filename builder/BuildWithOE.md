# Alexa Auto SDK Builder

Developers who want to build the complete Alexa Auto SDK software for various cross targets can use OpenEmbedded based building system *Alexa Auto SDK Builder*.

## Getting Started

The Alexa Auto SDK Builder can be run either natively on a Linux host or in a Docker environment.

macOS users can run the Alexa Auto SDK Builder using [Docker for Mac](https://www.docker.com/docker-mac); see [Builder setup in a Docker environment](#docker) below.

>**Note:** For QNX targets, you must install the [QNX 7.0 SDP](http://blackberry.qnx.com/en/sdp7/sdp70_download) within your host.

### Builder setup on a desktop Linux host

To run the Alexa Auto SDK Builder natively, please follow the guide below. **Ubuntu 16.04 LTS** was tested and is recommended as the Linux host environment.

#### Set up OpenEmbedded/BitBake

First you will need to set up [OpenEmbedded-Core](https://www.openembedded.org/wiki/OpenEmbedded-Core).

The following example will install OpenEmbedded-Core and BitBake under your home directory. Note that the variable `OE_CORE_PATH` must be pointed at the OpenEmbedded-Core source directory.

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
git-core libssl-dev
```
>**Note:** For Linux targets, you must install libssl-dev as well.

```
$ apt-get install libssl-dev
```

### Builder setup in a Docker environment <a name = "docker"></a>

To use Builder on macOS hosts, [Docker CE for Mac](https://www.docker.com/docker-mac) must be installed according to its official guide.

Upon first run, Builder will build the Docker image `aac/ubuntu-base:latest` and create a dedicated Docker volume `buildervolume` for running the Alexa Auto SDK Builder in your Docker environment. This might take up to an hour to complete.

>**IMPORTANT NOTE on macOS:** If you are trying to build for QNX targets with macOS host, QNX 7.0.0 SDP must be installed within *Case-Sensitive* file system, with additional Linux tools installation. You may need to use external drive for installation since your system file system is NOT *Case-Sensitive* by default.

## Running Builder

To run the Alexa Auto SDK Builder, invoke the following command. This will generate a complete target installation image. (where `AAC_SDK_HOME` is the location you've installed the Alexa Auto SDK.)

```
$ ${AAC_SDK_HOME}/builder/build.sh oe [options]
```

>**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.

The following `options` are available:

* `-h,--help` to show full available options.
* `-t,--target <target>` to specify cross compilation target. Defaults to `native`. See the table below for the details.
* `-b,--build-dir <directory>` option to specify the working directory for a build. Defaults to `${AAC_SDK_HOME}/builder/build`. This option will be ignored when running inside the Docker environment.
* `-g,--debug` option to build with debugging options.
* `-c,--clean` option to clean build. If the OE recipe name is specified with `--package` option, the only specified recipe will be cleaned.
* *(Android target only)* `--android-api <integer>` option to explicitly specify Android API level. Defaults to `22`.
* *(QNX7 target only)* `--qnx7sdp-path <path>` option to specify QNX 7.0.0 SDP installation (in host). If you run Builder within Docker environment, host QNX SDP tools are always used. So make sure you have installed Linux tools within SDP even if your host is macOS.

The following build targets are available:

| Platform Name              | `-t` value      |
| -------------------------- | --------------- |
| Generic Linux              | `native`        |
| Android ARMv7a             | `androidarm`    |
| Android ARMv8a *(Preview)* | `androidarm64`  |
| Android x86                | `androidx86`    |
| Android x86-64 *(Preview)* | `androidx86-64` |
| QNX AArch64                | `qnx7arm64`     |
| QNX x86-64                 | `qnx7x86-64`    |


>*Note: Using Builder to build the Alexa Auto SDK for macOS targets is not currently supported. [Building the Alexa Auto SDK with CMake](ConfigureCMake.md) provides instructions for building for Mac.*

For example, to build all the Alexa Auto SDK modules and their dependencies for an *Android ARMv7a* target, run the following command:

```
$ ${AAC_SDK_HOME}/builder/build.sh oe -t androidarm
```

## Install image

After successfully building the image, the output directory `deploy/` will be generated under the `${AAC_SDK_HOME}/builder/` directory.

Within the output directory, you will find the tar.gz archive `aac-image-minimal-<target>.tar.gz`. You may upload this image to your actual target hardware.

`aac-image-minimal` contains the following build artifacts:

* `/opt/AAC/bin/`: *cURL* binaries with *ngHTTP2* enabled
* `/opt/AAC/include/`: All dev headers for all dependencies.
* `/opt/AAC/lib/`: All shared libraries, including *AVS Device SDK*, *cURL*, *ngHTTP2*.
* `/opt/AAC/share/`: CMake files for building external Alexa Auto SDK modules.

If you've built the `native` target image, then you may install them directly on your Linux PC.

>Note: If you've built the image with the `-g` option, you will find an extra tar.gz archive `aac-image-minimal-<target>-dbg.tar.gz`, which contains debug symbols for later GDB use.

>Note: These binaries use the Alexa Auto SDK C++ API, and cannot be used for Android targets without the [Alexa Auto SDK Engine for Android](../platforms/android/README.md) package (JNI API).

## Clean build

To build cleanly, following command will remove all caches.

```
$ ./build.sh oe --clean
```
> **Tip**: The `build.sh` script typically performs a clean build. In most situations there is no need for a more thorough clean. When the `--clean` option is used, the Docker image used for building is purged. Purging the Docker image may cause the next build to take up to an hour to complete because the Docker image is reconstructed.

## Build with `mbedtls`

On some Android Samsung devices, OpenSSL caused the Alexa Auto sample app to terminate. Follow the steps below to use `mbedtls`.

```
$ echo "PACKAGECONFIG_pn-curl = \"mbedtls nghttp2\"" >> ${HOME}/aac-extra.conf
$ ${AAC_SDK_HOME}/builder/build.sh oe <options> ${HOME}/aac-extra.conf
```
