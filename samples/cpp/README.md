# C++ Sample App on Linux

This document provides instructions for building and running the C++ Sample App on Linux.

**Table of Contents**:

* [Prerequisites](#prerequisites)
* [Setup](#setup)
* [Building the C++ Sample App](#build)
* [Running the C++ Sample App](#run)
* [AudioFile Menu](#audio-menu)
* [Troubleshooting](#troubleshooting)
* [Release Notes](#releasenotes)

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

Clone the `alexa-auto-sdk` repository into your project directory:

```shell
$ git clone https://github.com/alexa/alexa-auto-sdk.git
$ cd alexa-auto-sdk
$ export AAC_SDK_HOME=$(pwd)
```

>**Note:** Most of the commands that follow are meant to be run from this `alexa-auto-sdk` directory.

You can pass one or more configuration files to the Sample App using the `--config <config-file-path>` flag. When you build additional modules with the sample app, you may need to pass module specific configuration. Please refer to the `README` file within each extra module to get this configuration information. For convenience, a [config file template](./assets/config.json.in) has been included for the core Auto SDK modules. An example configuration file is shown below:

```json
{
    "aace.alexa": {
        "avsDeviceSDK": {
            "deviceInfo": {
                "clientId": "${YOUR_CLIENT_ID}",
                "deviceSerialNumber": "${YOUR_DEVICE_SERIAL_NUMBER}",
                "productId": "${YOUR_PRODUCT_ID}"
            },
            "libcurlUtils": {
                "CURLOPT_CAPATH": "/opt/AAC/etc/certs"
            },
            "miscDatabase": {
                "databaseFilePath": "/opt/AAC/data/miscDatabase.db"
            },
            "certifiedSender": {
                "databaseFilePath": "/opt/AAC/data/certifiedSender.db"
            },
            "alertsCapabilityAgent": {
                "databaseFilePath": "/opt/AAC/data/alertsCapabilityAgent.db"
            },
            "notifications": {
                "databaseFilePath": "/opt/AAC/data/notifications.db"
            },
            "settings": {
                "databaseFilePath": "/opt/AAC/data/settings.db",
                "defaultAVSClientSettings": {
                    "locale": "en-US"
                }
            }
        }
    },
    "aace.storage": {
        "localStoragePath": "/opt/AAC/data/aace-storage.db",
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
    },
    "aace.cbl" : {
        "enableUserProfile": false
    }
}
```

You must customize this template with values specific to your implementation. To do this:

1. Edit the [config file template](./assets/config.json.in) and save it as `samples/cpp/assets/config.json`.
2. Replace the `${YOUR_CLIENT_ID}`, `${YOUR_DEVICE_SERIAL_NUMBER}`, and `${YOUR_PRODUCT_ID}` placeholders with your values, replacing `${YOUR_CLIENT_ID}` and `${YOUR_PRODUCT_ID}` with the Client ID and Product ID you noted when you [created your Amazon Developer account](#amazon-developer-account). You can also find the Client ID in your device's Security Profile under the **Other devices and platforms** tab. The `${YOUR_DEVICE_SERIAL_NUMBER}` can be arbitrary but should not contain spaces and must be unique.

3. Additionally, you may customize the value of the database file paths that are included in the template. For example, you could change the `databaseFilePath` for the `miscDatabase` field to `/my/db/folder/miscDB.db` instead of `/opt/AAC/data/miscDatabase.db`. However, if you modify the database directory path, you must ensure that the directory exists and has write permissions. Therefore, for the previous example, you must ensure that `/my/db/folder` exists and has write permissions.

 >**Note:** The Auto SDK engine will fail to start if the database directory path does not exist or does not have write permissions.

4. Modify the vehicle information (`aace.vehicle`) your vehicle specifics.

After you have customized the configuration template, install dependencies:

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

>**Note:** The command sequence above installs the minimum set of GStreamer libraries and plugins required by the [GStreamer extension](../../extensions/experimental/gstreamer/README.md). See [Recommended Media Support](https://developer.amazon.com/docs/alexa-voice-service/recommended-media-support.html) for a list of codecs, containers, streaming formats, and playlists that your product should support to provide a familiar Alexa experience to your customers.

## Building the C++ Sample App<a id="build"></a>

Follow the instructions in the [Alexa Auto SDK Builder](../../builder/README.md) documentation to set up your development environment, then from `${AAC_SDK_HOME}`, build the C++ Sample App with debug symbols using the following command:

```shell
$ builder/build.sh native --debug
```
This command builds the Sample App for all supported Linux platforms. To restrict the build to one or more specific platforms, use the `-t,--target <target>` option. See the [Builder readme](../../builder/README.md) for instructions.

>**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.

Install the generated C++ Sample App package into `/opt/AAC`:

```shell
$ sudo tar xf builder/deploy/native/aac-sdk-build-native.tar.gz -C /
```
Copy the edited config file from `samples/cpp/assets/config.json` to `/opt/AAC/etc/config.json`:

`cp ${AAC_SDK_HOME}/samples/cpp/assets/config.json /opt/AAC/etc/`

## Running the C++ Sample App<a id="run"></a>

The C++ Sample App requires one or more configuration files and other parameters to be specified at the command line. For convenience all needed configuration files and assets (such as certificates, sample data, menu) are automatically copied to `/opt/AAC/etc/`. To run the C++ Sample App using default installation files:

```shell
$ /opt/AAC/bin/SampleApp --cbreak --config /opt/AAC/etc/config.json \
                                  --menu   /opt/AAC/etc/menu.json > SampleApp.log
```

>**Note:** The `config.json` file (which you must create as described under [Setup](#setup)) contains your device information and Alexa settings. The [menu file](./assets/menu.json) drives the hierarchical interactive text based menu system. Refer to the [menu documentation](./assets/MENU.md) for more information.

If everything works as expected, you should see the Main Menu. Press `A` to authenticate with [CBL](https://developer.amazon.com/docs/alexa-voice-service/code-based-linking-other-platforms.html).

Open a new terminal and tail the SampleApp.log file:

```shell
$ tail -f SampleApp.log
```
## AudioFile Menu<a id="audio-menu"></a>

The C++ Sample App provides an AudioFile menu to send pre-recorded utterances. Responses are saved as MP3 audio files within the current directory where the app was run. Refer to the [C++ Sample App Menu System documentation](./assets/MENU.md#audiofile) for information on how to extend the AudioFile menu with custom audio files. However, this menu is only available if there is no default audio provider specified during the build. By default the Auto SDK build will include the [GStreamer extension](../../extensions/experimental/gstreamer) as the default audio provider if the target platform supports it.

>**Note:** The AudioFile menu appears on platforms that do not provide built-in audio support (such as QNX). On platforms that provide built-in audio support, the AudioFile menu does not appear. 

## Troubleshooting<a id="troubleshooting"></a>

* When interacting with Alexa, if the Dialog State goes from `LISTENING` immediately to `IDLE`, you might not be logged in. Try logging into your account via CBL by tapping `A` from the Main Menu.

 >**Note:** For security reasons, authentication is not persisted if you quit the Sample App.  Upon relaunch, you must re-authenticate via CBL.  Restarting the app using the menu system, however, preserves authentication.

* If the device serial number is not unique, the authentication state bounces between `PENDING` and `CONNECTED` states:

    ```
    Auth state changed: REFRESHED ( NO_ERROR )
    Connection status changed: PENDING ( ACL_CLIENT_REQUEST )
    Connection status changed: CONNECTED ( ACL_CLIENT_REQUEST )
    Connection status changed: PENDING ( SERVER_SIDE_DISCONNECT )
    status changed: CONNECTED ( ACL_CLIENT_REQUEST )
    Connection status changed: PENDING ( SERVER_SIDE_DISCONNECT )
    ...
    ```
    
    To resolve this, edit the `samples/cpp/assets/config.json` file and choose a unique serial number.

<a id="releasenotes"></a>

## v2.0.0 Release Notes<a id = "releasenotes"></a>

### Resolved Issues

* Fixed an issue where switching to LocalMediaSource did not set focus, which caused the current audio to continue playing.

### Known Issues

* Content from media streaming services (e.g., TuneIn) that deliver ASHX URLs currently not supported.
* Unable to play Audible books on Poky Linux for iMX8 (ARM 64-bit) due to stream errors encountered by the GStreamer media player.
* Increasing/decreasing speaker volume in quick succession causes lag and delayed responses from Alexa.
* Refer to the [GStreamer extension documentation](../../extensions/experimental/gstreamer/README.md) for known issues related to audio.
* If you are using the C++ Sample App with the optional Local Voice Control (LVC) extension, the LVC extension may be delayed in response for up to a few minutes if both the Sample App and the LVC processes are initiated in quick succession of each other.

### Limitations

* QNX is supported with file audio only. The following are additional limitations on QNX:
  * There is no direct microphone or speaker support.
  * Use the Audio File Input menu of the C++ Sample App to test utterances.
  * Response audio will be saved as MP3 files in the current working directory.
  * Content from streaming services is not supported.
