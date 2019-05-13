**Table of Contents**

- [C++ Sample App on Linux](#c-sample-app-on-linux)
    - [Prerequisites](#prerequisites)
        - [Amazon Developer Account](#amazon-developer-account)
        - [Linux Ubuntu 16.04 LTS](#linux-ubuntu-1604-lts)
        - [Configuration File](#configuration-file)
    - [Setup](#setup)
    - [Build](#build)
    - [Run](#run)
    - [Troubleshooting](#troubleshooting)
- [Release Notes](#release-notes)

# C++ Sample App on Linux<a id="c-sample-app-on-linux"></a>

This document provides instructions for building and running the C++ Sample App on Linux.

## Prerequisites<a id="prerequisites"></a>

### Amazon Developer Account<a id="amazon-developer-account"></a>

You need an [Amazon Developer](https://developer.amazon.com/docs/app-submission/manage-account-and-permissions.html#create-a-developer-account/) account. After registering for an Amazon Developer account, you must [register a product](https://developer.amazon.com/docs/alexa-voice-service/register-a-product.html) in the Automotive product category and generate a client ID for code-based linking (CBL). Make note of the following parameters as you work through the registration: Product ID, and Client ID. This information is required for your [configuration file](#configuration-file) below.

### Linux Ubuntu 16.04 LTS<a id="linux-ubuntu-1604-lts"></a>

All development and testing of the C++ Sample App has been performed on Linux Ubuntu 16.04 LTS. You need around 15GB of free space for building the Alexa Auto SDK and C++ Sample App for one target. Working with multiple targets can significantly increase the storage footprint.

### Configuration File<a id="configuration-file"></a>

The C++ Sample App requires one or more configuration files for device information, module configuration, and Alexa settings. See details in [Setup](#setup).

## Setup<a id="setup"></a>

Create your project directory (if you do not already have one):

```shell
$ mkdir ~/Projects
$ cd ~/Projects
```

Clone the `aac-sdk` repository into your project directory:

```shell
$ git clone https://github.com/alexa/aac-sdk.git
$ cd aac-sdk
$ export AAC_SDK_HOME=$(pwd)
```

>**Note:** Most of the commands that follow are meant to be run from this `aac-sdk` directory.

One or more configuration files can be passed to the sample app using the `--config <config-file-path>` flag. When additional modules are built with the sample app, it may be necessary to pass module specific configuration. Please refer to the `README` file within each extra module to get configuration information. For convenience, a [config file template](./assets/config.json.in) has been included for the core Auto SDK modules. The template must be customized with customer specific values.

Edit the [config file template](./assets/config.json.in) and save as `samples/cpp/assets/config.json`. Replace the `${YOUR_CLIENT_ID}`, `${YOUR_DEVICE_SERIAL_NUMBER}`, and `${YOUR_PRODUCT_ID}` placeholders with [your own values](#amazon-developer-account). Additionally, you may customize the value of the database file paths that are included in the template. For example, the `databaseFilePath` for the `miscDatabase` field could be changed to `/my/db/folder/miscDB.db` instead of `./miscDatabase.db`. However, if the database directory path is modified, you must ensure that the directory exists and has write permissions. Therefore, for the previous example, you must ensure that `/my/db/folder` exists and has write permissions.

>**Note:** The Auto SDK engine will fail to start if the database directory path does not exist or does not have write permissions.

```json
{
    "deviceInfo": {
        "clientId": "${YOUR_CLIENT_ID}",
        "deviceSerialNumber": "${YOUR_DEVICE_SERIAL_NUMBER}",
        "productId": "${YOUR_PRODUCT_ID}"
    },
    "libcurlUtils": {
        "CURLOPT_CAPATH": "/etc/ssl/certs"
    },
    "miscDatabase": {
        "databaseFilePath": "./miscDatabase.db"
    },
    "certifiedSender": {
        "databaseFilePath": "./certifiedSender.db"
    },
    "alertsCapabilityAgent": {
        "databaseFilePath": "./alertsCapabilityAgent.db"
    },
    "notifications": {
        "databaseFilePath": "./notifications.db"
    },
    "settings": {
        "databaseFilePath": "./settings.db",
        "defaultAVSClientSettings": {
            "locale": "en-US"
        }
    },
    "aace.storage": {
        "localStoragePath": "./sample-aace.db",
        "storageType": "sqlite"
    },
    "aace.vehicle": {
        "info": {
            "make": "Amazon",
            "model": "AACE",
            "year": "2019",
            "trim": "aac",
            "geography": "US",
            "version": "1.2.3",
            "os": "Sample OS 1.0",
            "arch": "Sample Arch 1.0",
            "language": "en-US",
            "microphone": "SingleArray",
            "countries": "US,GB,IE,CA,DE,AT,IN,JP,AU,NZ,FR",
            "vehicleIdentifier": "Sample Identifier ABC"
        },
        "operatingCountry": "US"
    }
}
```

You must replace placeholders with [your own values](#amazon-developer-account). The `clientId` is the one found in your device's Security Profile under the "Other devices and platforms" tab. The `deviceSerialNumber` can be arbitrary but should not contain spaces and must be unique.

Vehicle information (`aace.vehicle`) should also be modified to match the customer vehicle specifics.

Install dependencies:

```shell
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install -y gstreamer1.0-libav \
                        gstreamer1.0-plugins-bad \
                        gstreamer1.0-plugins-good \
                        libgstreamer-plugins-bad1.0-dev \
                        libgstreamer-plugins-base1.0-dev \
                        libgstreamer-plugins-good1.0-dev \
                        libgstreamer1.0-dev
```

>**Note:** Installs a minimal set of GStreamer libraries and plugins required by the [Sample Audio Implementation](../audio/README.md). See [Recommended Media Support](https://developer.amazon.com/docs/alexa-voice-service/recommended-media-support.html) for a list of codecs, containers, streaming formats, and playlists, that your product should support to provide a familiar Alexa experience to your customers.

## Build<a id="build"></a>

Follow the instructions in the [Alexa Auto SDK Builder](../../builder/README.md) documentation to setup your development environment, then from `${AAC_SDK_HOME}`, build the C++ Sample App with the following command:

```shell
$ builder/build.sh oe --debug --target native samples/audio samples/cpp/aac-sample-cpp.bb
```

>**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.

Install the generated C++ Sample App package into `/opt/AAC`:

```shell
$ sudo tar xf builder/deploy/native/aac-image-minimal-native.tar.gz -C /
```

## Run<a id="run"></a>

From `${AAC_SDK_HOME}`, launch the C++ Sample App, specifying your config and menu files and the log output. For example:

```shell
$ /opt/AAC/bin/SampleApp --cbreak --config samples/cpp/assets/config.json --menu samples/cpp/assets/menu.json > SampleApp.log
```

>**Note:** The `config.json` file *(you must create this file)* contains your device information and Alexa settings. The [menu file](./assets/menu.json) drives the hierarchical interactive text based menu system. Refer to the [menu documentation](./assets/MENU.md) for more information.

If everything works as expected, you should see the Main Menu. Press `A` to authenticate with [CBL](https://developer.amazon.com/docs/alexa-voice-service/code-based-linking-other-platforms.html).

Open a new terminal and tail the SampleApp.log file:

```shell
$ tail -f SampleApp.log
```
## Audio Menu

The C++ Sample App provides an [audio file menu](./assets/MENU.md#audiofile) to send pre-recorded utterances. Responses are saves as MP3 audio files within the current directory where the app was run. Refer to the [audio file menu](./assets/MENU.md#audiofile) documentation for information on how to extend this menu with custom audio files.

## Troubleshooting<a id="troubleshooting"></a>

When interacting with Alexa, if the Dialog State goes from `LISTENING` immediately to `IDLE`, you might not be logged in. Try logging into your account via CBL by tapping `A` from the Main Menu.

>**Note:** For security reasons, authentication is not persisted once the app has been quit.  Upon relaunch, authentication via CBL must be used again.  Restarting the app using the menu system, however, preserves authentication.

If device serial number is not unique, the authentication state bounces between `PENDING` and `CONNECTED` states, like so:

```shell
Auth state changed: REFRESHED ( NO_ERROR )
Connection status changed: PENDING ( ACL_CLIENT_REQUEST )
Connection status changed: CONNECTED ( ACL_CLIENT_REQUEST )
Connection status changed: PENDING ( SERVER_SIDE_DISCONNECT )
Connection status changed: CONNECTED ( ACL_CLIENT_REQUEST )
Connection status changed: PENDING ( SERVER_SIDE_DISCONNECT )
...
```

To resolve this, edit the `samples/cpp/assets/config.json` file and choose a unique serial number.

<a id="releasenotes"></a>

## v1.6.0 released on 2019-05-16:<a id="v160-released-on-2019-05-16"></a>

### Enhancements<a id="enhancements"></a>

* Phone Call Controller support
* Alexa Communications support
* Local Voice Control support
* QNX platform support
* Can now specify the ALSA device used for audio input

### Known Issues<a id="known-issues"></a>

* Content from media streaming services (e.g., TuneIn) that deliver ASHX URLs is not currently supported.
* Unable to play Audible books on Poky Linux for iMX8 (ARM 64-bit) due to stream errors encountered by GStreamer media player.
* Increasing/decreasing speaker volume in quick succession causes lag and delayed responses from Alexa.
* Refer to the [Sample Audio Implementation documentation](../audio/README.md) for known issues related to audio.
* Switching to LocalMediaSource does not set focus, causing the current audio to continue playing.

### Limitations<a id="limitations"></a>

* QNX is supported with file audio only. The following are additional limitations on QNX: 
  * There is no direct microphone or speaker support.
  * Use the Audio File Input menu of the C++ Sample App to test utterances.
  * Response audio will be saved as MP3 files in the current working directory.
  * Content from streaming services is not supported.
