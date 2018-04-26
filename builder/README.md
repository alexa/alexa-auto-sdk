# Alexa Auto Core Builder

This directory contains a collection of software which is required to build Alexa Auto Core software components for various target platforms.

## Getting Started

Alexa Auto Core Builder can be run either natively on a Linux host or in a Docker environment. 

macOS users can run Alexa Auto Core Builder using [Docker for Mac](https://www.docker.com/docker-mac); see [Builder setup in a Docker environment](#docker) below.

### Builder setup on a desktop Linux host

To run Alexa Auto Core Builder natively, please follow the guide below. **Ubuntu 16.04 LTS** was tested and is recommended as the Linux host environment.

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
$ apt-get install chrpath diffstat gawk texinfo
```
>**Note:** For QNX targets, you must install the [QNX 7.0 SDP](http://blackberry.qnx.com/en/sdp7/sdp70_download).

### Builder setup in a Docker environment <a name = "docker"></a>

To use Builder on macOS hosts, [Docker CE for Mac](https://www.docker.com/docker-mac) must be installed according to its official guide.

Upon first run, Builder will build the Docker image `aac/ubuntu-base:latest` and create a dedicated Docker volume `buildervolume` for running Alexa Auto Core Builder in your Docker environment. This might take up to an hour to complete.

## Running Builder

To run Alexa Auto Core Builder, invoke the following command. This will generate a complete target installation image. (where `AAC_SDK_HOME` is the location you've installed the AAC SDK.)

```
$ ${AAC_SDK_HOME}/builder/build.sh <options>
```

>**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.

The following `options` are available:

* `-t <target>` to specify cross compilation target. Defaults to `native`. See the table below for the details.
* `-c` option to clean build. If the OE recipe name is specified with `-p` option, the only specified recipe will be cleaned.
* `-p <recipe-name>` to specify OE recipe explicitly. Defaults to `aac-image-minimal`.
* `-b` option to specify the working directory for a build. Defaults to `${AAC_SDK_HOME}/builder/build`. This option will be ignored when running inside the Docker environment.
* `-d` option to run *Alexa Auto Core Builder* inside the Docker environment. On macOS, this option will be enabled by default.
* `-g` option to build with debugging options.
* *(Android target only)* `-a` option to explicitly specify Android API level. Defaults to `21`.

The following build targets are available:

| Platform Name  |  `-t` value  |
| -------------- | ------------ |
| Generic Linux  | `native`     |
| Android ARMv7a | `androidarm` |
| Android x86    | `androidx86` |
| QNX AArch64    | `qnx7arm64`  |
| QNX x86-64     | `qnx7x86-64` |


>*Note: Using Builder to build Alexa Auto Core for macOS targets is not currently supported. [Building Alexa Auto Core with CMake](ConfigureCMake.md) provides instructions for building for Mac.*

For example, to build all the Alexa Auto Core modules and their dependencies for an *Android ARMv7a* target, run the following command:

```
$ ${AAC_SDK_HOME}/builder/build.sh -t androidarm
```

## Install image

After successfully building the image, the output directory `deploy/` will be generated under the `${AAC_SDK_HOME}/builder/` directory.

Within the output directory, you will find the tar.gz archive `aac-image-minimal-<target>.tar.gz`. You may upload this image to your actual target hardware.

`aac-image-minimal` contains the following build artifacts:

* `/opt/AAC/bin/`: *cURL* binaries with *ngHTTP2* enabled
* `/opt/AAC/include/`: All dev headers for all dependencies.
* `/opt/AAC/lib/`: All shared libraries, including *AVS Device SDK*, *cURL*, *ngHTTP2*.
* `/opt/AAC/share/`: CMake files for building external AAC modules.

If you've built the `native` target image, then you may install them directly on your Linux PC.

>Note: If you've built the image with the `-g` option, you will find an extra tar.gz archive `aac-image-minimal-<target>-dbg.tar.gz`, which contains debug symbols for later GDB use.

>Note: These binaries use the Alexa Auto Core C++ API, and cannot be used for Android targets without the [Alexa Auto Core Engine for Android](../platforms/android/README.md) package (JNI API).

## Clean build

To build cleanly, remove the working directory. 

```
$ rm -rf <working-directory>
```

The default location for the working directory is `${AAC_SDK_HOME}/builder/build`.