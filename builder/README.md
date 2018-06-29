# Building Alexa Automotive Core

This directory contains a collection of software which is required to build Alexa Automotive Core software components for various target platforms.

## Overview

Alexa Automotive Core can be built in 2 methods we provide.

* Using **Alexa Automotive Core Builder**: The build system based on [OpenEmbedded](https://www.openembedded.org/) that provides a simple way to cross compile all Alexa Automotive Core software components for various target platforms. Recommended for developers who wants to use platforms such as **Android** and **QNX**.
* Using **CMake**: The industry standard build processing software. Recommended for advanced developers who wants to work on active development on PC or macOS.

## Quick Start

Run the build process as follows (where `AAC_SDK_HOME` is the location you've installed the AAC SDK.):

```
$ ${AAC_SDK_HOME}/builder/build.sh (oe|cmake) <args>
```

* To use **Alexa Automotive Core Builder** specify `oe` as a first argument. Please refer to [the document](BuildWithOE.md) for further options.
* To use **CMake** specify `cmake` as a first argument. Please refer to [the document](BuildWithCMake.md) for further options.