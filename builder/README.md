# Building the Alexa Auto SDK

This directory contains a collection of software which is required to build the Alexa Auto SDK software components for various target platforms.

## Overview

The Alexa Auto SDK can be built in 2 methods we provide.

* Using the **Alexa Auto SDK Builder**: The build system based on [OpenEmbedded](https://www.openembedded.org/) that provides a simple way to cross compile all the Alexa Auto SDK software components for various target platforms. Recommended for developers who wants to use platforms such as **Android** and **QNX**.
* Using **CMake**: The industry standard build processing software. Recommended for advanced developers who wants to work on active development on PC or macOS.

## Build Dependencies

During the build time, the following dependencies shall be fetched and built for target by **Alexa Auto SDK Builder** and/or **CMake**. Please refer to each individual entities for the particular licenses.

* [AVS Device SDK v1.9](https://github.com/alexa/avs-device-sdk/)
  * [cURL 7.56.0](https://curl.haxx.se/)
  * [ngHTTP2 1.31.1](https://github.com/nghttp2/nghttp2)
  * [SQLite3 3.20.0](https://www.sqlite.org/)
  * *Android target only* [OpenSSL 1.1.0h](https://www.openssl.org/)
* [Google Test v1.8.0](https://github.com/google/googletest)
* [libopus 1.2.1](https://opus-codec.org/)

Additionally the following dependencies will be required for **Alexa Auto SDK Builder**.

* [OpenEmbedded-Core Rocko](https://www.openembedded.org/wiki/OpenEmbedded-Core)
* [BitBake](https://www.yoctoproject.org/software-overview/)
* *Android target only* [Android Toolchain](https://www.android.com/)
  * [NDK r16b](https://developer.android.com/ndk/)
  * [SDK Tools 26.0.1](https://developer.android.com/studio/releases/sdk-tools)

Note that *OpenEmbedded-Core* will fetch and build additional components for preparing the dedicated toolchain for your environment (Such as *GNU Binutils*). Please refer to the [Yocto project](https://www.yoctoproject.org/software-overview/) for how it works.

## Quick Start

Run the build process as follows (where `AAC_SDK_HOME` is the location you've installed the AAC SDK.):

```
$ ${AAC_SDK_HOME}/builder/build.sh (oe|cmake) <args>
```

* To use the **Alexa Auto SDK Builder**, specify `oe` as a first argument. Please refer to [the document](BuildWithOE.md) for further options.
* To use **CMake**, specify `cmake` as a first argument. Please refer to [the document](BuildWithCMake.md) for further options.
