# Alexa Auto SDK C++ Sample App on Linux

The purpose of the C++ Sample App is to provide useful example code to help you integrate your platform implementation with the Alexa Auto SDK. The C++ Sample App provides an example of creating and configuring an instance of the Engine, overriding the default implementation of each Alexa Auto platform interface, and registering those custom interface handlers with the Engine. It includes one default example implementation of authorizing with Alexa Voice Service (AVS) via Code Based Linking (CBL). The C++ Sample App also includes detailed logs for interactions with the Alexa Auto SDK and convenience features for viewing those logs in the application, as well as UI elements relevant to each platform interface implementation.
<!-- omit in toc -->
## Table of Contents

- [Prerequisites](#prerequisites)
- [Enabling Optional Device Capabilities](#enabling-optional-device-capabilities)
- [Setting up the C++ Sample App](#setting-up-the-c-sample-app)
- [Building the C++ Sample App](#building-the-c-sample-app)
- [Running the C++ Sample App](#running-the-c-sample-app)
- [Using the C++ Sample App](#using-the-c-sample-app)
- [Troubleshooting](#troubleshooting)
- [Known Issues](#known-issues)

## Prerequisites

### Amazon Developer Account

To use the C++ Sample App, you need an [Amazon Developer](https://developer.amazon.com/docs/app-submission/manage-account-and-permissions.html#create-a-developer-account/) account.

### Registered Product and Security Profile

After creating an Amazon developer account, you'll need to [register a product and create a security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html) on the AVS developer portal.

When you follow the instructions to [fill in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information):

* Use your own custom information, taking note of the **Product ID**, as this information is required for your [configuration file](#configuration-file) .
* Be sure to select **Automotive** from the **Product category** pull-down.

When you follow the instructions to [set up your security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#set-up-your-security-profile), generate a **Client ID** and take note of it, as this information is required for your [configuration file](#configuration-file).

### Linux Ubuntu 16.04 LTS

All development and testing of the C++ Sample App has been performed on Linux Ubuntu 16.04 LTS. You need around 15GB of free space for building the Alexa Auto SDK and C++ Sample App for one target. Working with multiple targets can significantly increase the storage footprint.

### Configuration File

The C++ Sample App requires one or more [configuration files](#editing-the-configuration-file) for device information, module configuration, and Alexa settings.

## Enabling Optional Device Capabilities

In order to use certain optional Alexa Auto SDK functionality (for example, AmazonLite Wake Word, Alexa Communications, Local Voice Control (LVC), and Device Client Metrics (DCM)) with the Sample App, your product must be placed on the allow list by Amazon. Copy the product's **Amazon ID** from the Developer Console and follow the directions on the [Need Help?](../../NEED_HELP.md#requesting-additional-functionality) page.

## Setting up the C++ Sample App

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

### Editing the Configuration File

You can pass one or more configuration files to the Sample App using the `--config <config-file-path>` flag. When you build additional modules with the sample app, you may need to pass module-specific configuration. Please refer to the `README` file within each module to get this configuration information. For convenience, a [config file template](./assets/config.json.in) has been included for the core Auto SDK modules. 
You must customize this template with values specific to your implementation. To do this:

1. Edit the [config file template](./assets/config.json.in) and save it as `samples/cpp/assets/config.json`.

2. Replace the `${YOUR_CLIENT_ID}`, `${YOUR_PRODUCT_ID}`, and `${YOUR_DEVICE_SERIAL_NUMBER}` placeholders with your values as follows:
   * Replace `${YOUR_CLIENT_ID}` with the Client ID, which you can find in your device's Security Profile under the **Other devices and platforms** tab.
   * Replace `${YOUR_PRODUCT_ID}` with the Product ID, which you can find under the **Products** tab on the AVS Developer Console. (It is different from the Amazon ID.)
   * Replace `${YOUR_DEVICE_SERIAL_NUMBER}` with an arbitrary value that must not contain spaces and must be unique.

 >**Note:** The Client ID and Product ID must correspond to a development device profile that you created as an **automotive** product by selecting the `Automotive` product category when you [filled in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information).  

3. Additionally, you may customize the value of the database file paths that are included in the template. For example, you could change the `databaseFilePath` for the `miscDatabase` field to `/my/db/folder/miscDB.db` instead of `/opt/AAC/data/miscDatabase.db`. However, if you modify the database directory path, you must ensure that the directory exists and has write permissions. Therefore, for the previous example, you must ensure that `/my/db/folder` exists and has write permissions.

 >**Note:** The Auto SDK engine will fail to start if the database directory path does not exist or does not have write permissions.

4. Modify the vehicle information (`aace.vehicle`) to match your vehicle specifics.

### Installing Dependencies

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

>**Note:** The command sequence above installs the minimum set of GStreamer libraries and plugins required by the [System Audio extension](../../extensions/experimental/system-audio/README.md) for audio capture/playback on Linux and Mac. See [Recommended Media Support](https://developer.amazon.com/docs/alexa-voice-service/recommended-media-support.html) for a list of codecs, containers, streaming formats, and playlists that your product should support to provide a familiar Alexa experience to your customers.

## Building the C++ Sample App

Follow the instructions in the [Alexa Auto SDK Builder](../../builder/README.md) documentation to set up your development environment, then from `${AAC_SDK_HOME}`, build the C++ Sample App with debug symbols:

```shell
$ builder/build.sh <platform> -t <target> --debug
```

For example, to build for just native Linux, use:

```shell
$ builder/build.sh linux -t native --debug
```

To build for multiple targets, include a comma-separated list of target values. For example:

```shell
$ builder/build.sh linux -t native,pokyarm,pokyarm64 --debug
```

See the [Alexa Auto SDK Builder documentation](../../builder/README.md#running-builder) for a list of supported platforms and targets.

>**Note:** The first run might take up to an hour to complete while OpenEmbedded generates all necessary toolchains internally.

Install the generated C++ Sample App package into `/opt/AAC`:

```shell
$ sudo tar xf builder/deploy/native/aac-sdk-build-native.tar.gz -C /
```
Copy the edited config file from `samples/cpp/assets/config.json` to `/opt/AAC/etc/config.json`:

`cp ${AAC_SDK_HOME}/samples/cpp/assets/config.json /opt/AAC/etc/`

## Running the C++ Sample App

The C++ Sample App requires that you specify one or more configuration files and other parameters at the command line. For convenience all needed configuration files and assets (such as certificates, sample data, and menu) are automatically copied to `/opt/AAC/etc/`. To run the C++ Sample App using default installation files:

```shell
$ LD_LIBRARY_PATH=/opt/AAC/lib \
  /opt/AAC/bin/SampleApp --cbreak --config /opt/AAC/etc/config.json \
                                  --menu   /opt/AAC/etc/menu.json > SampleApp.log
```

>**Note:** The `config.json` file contains your device information and Alexa settings. The [menu file](./assets/menu.json) drives the hierarchical interactive text based menu system. Refer to the [menu documentation](./assets/MENU.md) for more information.
>
>When you run the C++ Sample App, a `config-system-audio.json` file with default audio settings for Linux platforms is copied to `/opt/AAC/etc`. For information about how to modify these settings for QNX, see the [System Audio extension README](../../extensions/experimental/system-audio/README.md#running-the-c-sampleapp-with-the-audio-configuration).

### Authenticating with AVS using Code-Based Linking (CBL)

After the Sample App launches, you will see the Main Menu. Follow these steps to authorize your device with AVS using CBL.

#### Starting the CBL authorization

1. Press `A`, the Sample App displays the below message:
    ```shell
    ################################################################################
    #                                                                              #
    #                              Authorization Menu                              #
    #                                                                              #
    ################################################################################

    [ 1 ]    Start CBL Authorization
    [ esc ]  Go back
    ```
2. Press `1` to start the CBL authorization. The Sample App displays messages, including a code and a URL in a format similar to the following:

    ```
    ###########################
              123456     
    ############################################
         url: http://www.amazon.com/us/code 
    ############################################
    ```    
>**Note:** You may have to scroll up to see the code and URL.

3. Open a browser and navigate to the URL displayed in the Sample App.
4. In the browser, enter the code displayed in the Sample App.
5. Click **Continue** and follow the instructions in the browser to complete the authentication.

#### Canceling the authorization

After you start the authorization, the `Authorization` menu displays option [1] for you to cancel the authorization that is in progress. Press `1` to cancel the authorization.

```shell
################################################################################
#                                                                              #
#                              Authorization Menu                              #
#                                                                              #
################################################################################

 [ 1 ]    Cancel CBL Authorization
 [ esc ]  Go back
```

#### Logging out of the CBL authorization

After the device is registered successfully, the Sample App displays option [1] in the `Authorization` menu for you to log out of CBL authorization. Press `1` to log out from the authorization.

```shell
################################################################################
#                                                                              #
#                              Authorization Menu                              #
#                                                                              #
################################################################################

 [ 1 ]    Logout CBL Authorization
 [ esc ]  Go back

```

### Tailing the Log File

Open a new terminal and tail the SampleApp.log file:

```shell
$ tail -f SampleApp.log
```

## Using the C++ Sample App
### Authorization with AVS in the Sample App

Every request to AVS requires an Login with Amazon (LWA) access token. Code-Based Linking (CBL) is the recommended method to acquire access tokens and is demonstrated by the C++ Sample App. See the [CBL module README](../../modules/cbl/README.md) for details about the Auto SDK's implementation of CBL.

### Multimedia Support for QNX

The C++ Sample App supports the [BlackBerry QNX Multimedia Suite](https://blackberry.qnx.com/content/dam/qnx/products/qnxcar/QNX_MultimediaSuite_ProductBrief_Online_FINAL.pdf) for live audio input and output on QNX platforms. 

>**Note:** The SHOUTcast/lcecast streaming format is not supported.

See the [System Audio extension README](../../extensions/experimental/system-audio/README.md) for details about configuring audio input and output on QNX platforms.

### AudioFile Menu

The C++ Sample App provides an AudioFile menu to send pre-recorded utterances. Responses are saved as MP3 audio files within the current directory where the app was run. Refer to the [C++ Sample App Menu System documentation](./assets/MENU.md#audiofile) for information on how to extend the AudioFile menu with custom audio files. However, this menu is only available if there is no default audio provider specified during the build. By default the Auto SDK Builder will build the C++ Sample App with the [System Audio](../../extensions/experimental/system-audio/README.md) configuration defined in the config-system-audio.json file.

>**Note:** The AudioFile menu appears on platforms that do not provide built-in audio support (such as platforms that are under development). On platforms that provide built-in audio support, the AudioFile menu does not appear. 

### Handling Unknown Locations for Navigation Use Cases
Your platform implementation should handle cases where a GPS location cannot be obtained by returning the `UNDEFINED` value provided by the Auto SDK. In these cases, the Auto SDK does not report the location in the context, and your platform implementation should return a localization object initialized with `UNDEFINED` values for latitude and longitude ((latitude,longitude) = (`UNDEFINED`,`UNDEFINED`)) in the context object of every SpeechRecognizer event. 

### Enabling SiriusXM as a Local Media Source
The Sample App does not configure SiriusXM as a local media source by default. If you need the SiriusXM local media source, you must enable and build it. To do this, uncomment the following line in the [`Application.cpp`](./SampleApp/src/Application.cpp) class, then rebuild the Sample App:

`{ aace::alexa::LocalMediaSource::Source::SIRIUS_XM, nullptr }`

>**Note:** When SiriusXM is present as a local media source, the cloud defaults to local SiriusXM only and blocks any use of the cloud SiriusXM service even if the local implementation/service is unavailable or not enabled. 

## Troubleshooting

* When interacting with Alexa, if the Dialog State goes from `LISTENING` immediately to `IDLE`, you might not be logged in. Try [logging into your account via CBL](#authenticating-with-avs-using-code-based-linking-cbl) by tapping `A` from the Main Menu.

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
    
## Known Issues
None.