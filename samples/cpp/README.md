# Alexa Auto SDK C++ Sample App

## Overview

The purpose of the C++ Sample App is to provide useful example code to help you integrate your implementation with the Alexa Auto SDK. The C++ Sample App provides an example of creating and configuring an instance of the Engine, and using the MessageBroker API to subscribe to messages from the Engine. It also provides examples of handling audio and stream based interfaces with the MessageStream API, and replying to messages from the Engine. The C++ Sample App also includes detailed logs for interactions with the Alexa Auto SDK, as well as UI elements relevant to the implementation.

## Prerequisites

### Amazon developer account

To use the C++ Sample App, you need an [Amazon Developer](https://developer.amazon.com/docs/app-submission/manage-account-and-permissions.html#create-a-developer-account/) account.

### Register product and security profile

After creating an Amazon developer account, you'll need to [register a product and create a security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html) on the AVS developer portal.

When you follow the instructions to [fill in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information):

* Use your own custom information, taking note of the **Product ID**, as this information is required to configure the Sample App.
* Be sure to select **Automotive** from the **Product category** pull-down.

When you follow the instructions to [set up your security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#set-up-your-security-profile), generate a **Client ID** and take note of it, as this information is required to configure the Sample App.

### Optional device capabilities

In order to use certain optional Alexa Auto SDK functionality (for example, AmazonLite Wake Word, Alexa Communications, and Local Voice Control (LVC)) with the Sample App, your product must be placed on the allow list by Amazon. Copy the product's **Amazon ID** from the Developer Console and follow the directions on the [Need Help?](https://alexa.github.io/alexa-auto-sdk/docs/help) page.

>**Note:** Most of the commands that follow are meant to be run from this `alexa-auto-sdk` directory.

## Build and Run the Sample App

Before you build and run the Sample App, it is recommended that you first review and understand how to [build Auto SDK](https://alexa.github.io/alexa-auto-sdk/docs/native/building). The Sample App can be built by using the Auto SDK Builder Tool, or by using Conan to build the Auto SDK and Sample App packages directly. Each option is described in more detail in this section.

### Build using Builder Tool

The C++ Sample App can be built using the Auto SDK Builder Tool by specifying the `--with-sampleapp` or `--sampleapp` option when doing a build. The following examples should be run with `alexa-auto-sdk` as the working directory.

To build Auto SDK and Sample App with all modules included:

```bash
$ ./builder/build.py --with-sampleapp
```

The build archive is created in the `builder/deploy` directory, and will include the Sample App binary, along with all of the required libs and configuration files needed to run the application. The name of the archive will depend on your build settings, but in general will match the following pattern:

```
aac-<version>-<os>_<arch>-<build-type>-<datetime>.tgz
```

You can extract the contents of the build archive to any location on your target device, with the following command:

```shell
$ tar -xvzf <archive>.tgz
```

After extracting the contents, the directory structure should look something like the following:

```
aac-dev-linux_x86_64-release/
  ├─ bin/
  |  └─ SampleApp 
  ├─ docs/
  ├─ include/
  ├─ lib/
  |  ├─ libAACECore.so 
  |  └─ ... 
  ├─ share/
  |  ├─ sampleapp/
  |  |  ├─ certs/
  |  |  ├─ config/
  |  |  |  └─ config.json 
  |  |  ├─ inputs/
  |  |  ├─ menu/
  |  |  |  └─ menu.json 
  |  |  └─ sampledata/
  |  └─ ... 
  └─ aac-buildinfo.txt
```

#### Run the Sample App

Before running the Sample App, you are required to configure the settings defined in the `share/sampleapp/config/config.json` file, including setting up your unique client ID and product information. This is described in more detail in the [Configure the Sample App](#configure-the-sample-app) section of this document. Once the configuration changes have been made, you can run the Sample App from the root directory of the extracted archive using the following command:

```shell
$ DYLD_LIBRARY_PATH=lib:+:${DYLD_LIBRARY_PATH} \
    LD_LIBRARY_PATH=lib:+:${LD_LIBRARY_PATH} \
    ./bin/SampleApp -c share/sampleapp/config/config.json -m share/sampleapp/menu/menu.json
```

### Build using Conan

The Auto SDK C++ Sample App can be configured using the provided Conan recipe, and then built with CMake. The Conan recipe requires packages that are defined as part of Auto SDK, which must first be installed into the local cache (see [Build Alexa Auto SDK](https://alexa.github.io/alexa-auto-sdk/docs/native/building) for instructions about how to install Auto SDK Conan packages). If you are specifying any additional dependencies, such as extra modules for Auto SDK, those packages must also be installed in the Conan cache before configuring the Sample App.

If the required dependencies are already installed, the following commands can be used to quickly configure, build and run the Sample App.

```bash
# export auto sdk conan dependencies
$ python conan/setup.py

# configure the sample app and install it in a build directory
$ conan install samples/cpp -if=build-sampleapp -b missing

# build the sample app in the build directory
$ conan build samples/cpp -bf=build-sampleapp

# run the sample app from the build directory
$ cd build-sampleapp
$ DYLD_LIBRARY_PATH=lib:+:${DYLD_LIBRARY_PATH} \
  	 LD_LIBRARY_PATH=lib:+:${LD_LIBRARY_PATH} \
     ./bin/SampleApp -c ./config/config.json -m ./menu/menu.json

```
> Note: These examples assume your working directory is set to the root `alexa-auto-sdk` directory.

#### Specify build options

You can use the following command line options with the Conan install command. The options are defined in the Conan recipe:

**`aac_modules`** - Specify default Auto SDK modules to build with the Sample App. This is a comma separated list of modules that must be installed in the Conan local cache before building. If this option is not overridden, the default value will be `core, alexa, cbl, system-audio`.

**`extra_modules`** - Specify additional modules to build with the Sample App. This is a comma separated list of modules that must be installed in the Conan local cache before building. This is a useful option if you want to specify modules to build in addition to the default modules, rather than replacing the default modules entirely.

You can specify the options above using `-o` when running the Conan command. For example, to specify additional modules that are included when building the Sample App, you can us the following option:

```shell
$ conan install samples/cpp -if=build-sampleapp -b missing \ 
	-o extra_modules="navigation,phone-control"
```

#### Specify environment variables for configuration values

For convenience, a [config file template](https://github.com/alexa/alexa-auto-sdk/blob/master/samples/cpp/assets/config/config.json) has been included for the core Auto SDK modules with well-known tokens (e.g. `CLIENT_ID`, `PRODUCT_ID`) for various configuration values. You can set environment variables for these tokens; when building the Sample App, they will be replaced in the configuration file.

For example you can set an environment variable when running `conan install` like this:

```bash
$ CLIENT_ID=xxxx \
  PRODUCT_ID=xxxx \
    conan install ...
```

## Configure the Sample App

You can pass one or more configuration files to the Sample App using the `--config <config-file-path>` flag. When you build additional modules with the sample app, you may need to pass module-specific configuration. Please refer to the `README` file within each module to get this configuration information. 

For convenience, a [config file template](https://github.com/alexa/alexa-auto-sdk/blob/master/samples/cpp/assets/config/config.json) has been included for the core Auto SDK modules. You must customize this template with values specific to your implementation. You can either edit the configuration file manually or specify environment variables that can be used to override the configuration values when building the Sample App with Conan, see how to [specify environment variables for configuration values](#specify-environment-variables-for-configuration-values).

To change the config file manually, follow these steps:

1. Edit the [config file template](https://github.com/alexa/alexa-auto-sdk/blob/master/samples/cpp/assets/config/config.json) and save it.

2. Replace the `${YOUR_CLIENT_ID}`, `${YOUR_PRODUCT_ID}`, and `${YOUR_DEVICE_SERIAL_NUMBER}` placeholders with your values as follows:
    * Replace `${YOUR_CLIENT_ID}` with the Client ID, which you can find in your device's Security Profile under the **Other devices and platforms** tab.
    * Replace `${YOUR_PRODUCT_ID}` with the Product ID, which you can find under the **Products** tab on the AVS Developer Console. (It is different from the Amazon ID.)
    * Replace `${YOUR_DEVICE_SERIAL_NUMBER}` with an arbitrary value that must not contain spaces and must be unique.

    >**Note:** The Client ID and Product ID must correspond to a development device profile that you created as an **automotive** product by selecting the `Automotive` product category when you [filled in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information).  

3. Replace the `${DATA_PATH}` and `${CERTS_PATH}` with paths to your database and certificates, respectively. You must ensure that the directories exist and have write permissions.

    >**Note:** The Auto SDK engine will fail to start if the database directory path does not exist or does not have write permissions.

4. Modify the vehicle information (`aace.vehicle`) to match your vehicle specifics.

## Use the Sample App

### Authenticate with AVS using Code-Based Linking (CBL)

Every request to AVS requires an Login with Amazon (LWA) access token. Code-Based Linking (CBL) is the recommended method to acquire access tokens and is demonstrated by the C++ Sample App. After the Sample App launches, you will see the Main Menu. Follow these steps to authorize your device with AVS using CBL.

#### Start the CBL authorization

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

#### Cancel the authorization

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

#### Log out of the CBL authorization

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

### Multimedia support for QNX

The C++ Sample App supports the [BlackBerry QNX Multimedia Suite](https://blackberry.qnx.com/content/dam/qnx/products/qnxcar/QNX_MultimediaSuite_ProductBrief_Online_FINAL.pdf) for live audio input and output on QNX platforms. 

>**Note:** The SHOUTcast/lcecast streaming format is not supported.

See the System Audio module documentation for details about configuring audio input and output on QNX platforms.

### AudioFile menu

The C++ Sample App provides an AudioFile menu to send pre-recorded utterances. Responses are saved as MP3 audio files within the current directory where the app was run. Refer to the [C++ Sample App Menu System documentation](https://github.com/alexa/alexa-auto-sdk/blob/master/samples/cpp/assets/menu/MENU.md) for information on how to extend the AudioFile menu with custom audio files. However, this menu is only available if there is no default audio provider specified during the build. By default the Auto SDK Builder will build the C++ Sample App with the `System Audio` configuration defined in the `config-system-audio.json` file.

>**Note:** The AudioFile menu appears on platforms that do not provide built-in audio support (such as platforms that are under development). On platforms that provide built-in audio support, the AudioFile menu does not appear. 

### Handle unknown locations for navigation use-cases
Your platform implementation should handle cases where a GPS location cannot be obtained by returning the `UNDEFINED` value provided by the Auto SDK. In these cases, the Auto SDK does not report the location in the context, and your platform implementation should return a localization object initialized with `UNDEFINED` values for latitude and longitude ((latitude,longitude) = (`UNDEFINED`,`UNDEFINED`)) in the context object of every SpeechRecognizer event. 

### Enable SiriusXM as a local media source
The Sample App does not configure SiriusXM as a local media source by default. If you need the SiriusXM local media source, you must enable and build it. To do this, add the following line to the list of local media sources in the [`Application.cpp`](https://github.com/alexa/alexa-auto-sdk/blob/4.0/samples/cpp/src/Application.cpp#L493) class then rebuild the Sample App:

`{ aace::alexa::LocalMediaSource::Source::SIRIUS_XM, nullptr }`

>**Note:** When SiriusXM is present as a local media source, the cloud defaults to local SiriusXM only and blocks any use of the cloud SiriusXM service even if the local implementation/service is unavailable or not enabled. 

## Using Sample App with 3rd Party Wakewords

The C++ Sample App provides a reference implementation to help OEMs integrate with Siri and Apple CarPlay. The app demonstrates how OEMs can use CarPlay in Voice Activity Detection (VAD) and Keyword Detection (KWD) modes. In the VAD mode, the sample app integrates with Amazon's PryonLite VAD APIs to showcase voice activity detection and time stamp calculations

### Building Sample App

To use the sample app with 3P CarPlay capabilities, build with the following flag:
```shell
$ builder/build.py -o aac-sampleapp:with_3pva=True --with-sampleapp
```

The sample app will include the `AgentHandler` and supporting files needed to demonstrate the CarpPlay features. You will also need to include the Amazonlite Extension in your build. 
Contact your Amazon Solutions Architect (SA) or Partner Manager to obtain the correct Pryonlite package for Amazonlite to support VAD and corresponding Amazonlite Extension.



### Features for CarPlay

To access the CarPlay features, select Car Play or **“C”** from the main menu

**[ C ]  Car Play Menu**

The Car Play features allows you to: 

1. Register/Deregister Siri as the 3P agent
2. Trigger Siri interactions
3. Set CarPlay modes (VAD, Keyword detection, Deactivated)

```shell
################################################################################
#                                                                              #
#                              Car Play Menu                                   #
#                                                                              #
################################################################################
[A]    Agent registration
[S]    SIRI Interaction
[M]    Car Play Mode
[esc]  Go back
```

### Agent Registration 

```shell
################################################################################
#                                                                              #
#                              Agent registration Menu                         #
#                                                                              #
################################################################################
 [ 1 ]    Register SIRI Agent
 [ 2 ]    Deregister SIRI Agent
 [ esc ]  Go back

```

### Siri Interaction

Once Siri is registered and in CarPlay Keyword mode (see CarPlay Modes section below), you can invoke Siri with the wake word or use the “Button Down” option to simulate a gesture (Push-to-talk/Tap-to-talk) in this Siri interaction menu

```shell

################################################################################
#                                                                              #
#                            SIRI Interaction Menu                             #
#                                                                              #
################################################################################

 [ 1 ]    Button Down
 [ 2 ]    Set State to SPEAKING
 [ 3 ]    Set State to NONE
 [ esc ]  Go back
 ```
When Siri is granted dialog, you can also use this menu to simulate dialog state changes on behalf of Siri. (*Note*: In an actual integration, the device will send dialog state changes to your application. Your application must publish the messages for setting the dialog states in Auto SDK).

### CarPlay Modes

You can use the following menu to set CarPlay in one of the three modes.
```shell

################################################################################
#                                                                              #
#                              Car Play Mode Menu                              #
#                                                                              #
################################################################################

 [ 1 ]    VAD
 [ 2 ]    Keyword
 [ 3 ]    Deactivated
 [ esc ]  Go back
```

The OEM application integrates with CarPlay APIs, handles the different modes, and communicates with the device (iPhone). The sample app demonstrates the following:

In VAD mode, the sample app demonstrates how to:

1. Create an instance of PryonLite VAD/SRD with parameters for SRD config and register for VAD detection callback.
2. Feed audio to the PryonLite VAD/SRD instance (in 10 ms chunks), keep track of (save) the timestamps and the offsets of the chunks and calculate the Start Of Speech (SoS) timestamp using the offset given by the PryonLite instance and the saved timestamps.
3. Create a requestSiri event with the SoS timestamp to send to the device.

#### VAD Mode - Voice Activity Detection

    There are 2 options to enable VAD feature.
    * Mic Audio 
    * Audio File 
    
```shell

################################################################################
#                                                                              #
#                              VAD Mode Menu                                   #   
#                                                                              #
################################################################################

    [ 1 ]    Mic Audio
    [ 2 ]    Audio File Input
    [ esc ]  Go back
```
In order to enable this feature, a config for PryonLiteVAD which is given below should be added in the SampleApp Config, to provide path to Pryonlite models which are copied over to the `share/amazonlite/models` folder, after untarring the `.tgz` file. In future if in Amazonlite package, model file inside Pryonlite package, update this file name `(.bin)` as well in this config. (name of model file can be found inside `share/amazonlite/models` folder).

```shell
    "pryonLiteVAD": {
        "rootPath": "${PATH_TO_WW_LOCALE_MODELS}",
        "models": "H.ar-SA+de-DE+en-AU+en-CA+en-GB+en-IN+en-US+es-MX+es-US+fr-CA+fr-FR+hi-IN+it-IT+ja-JP+pt-BR.alexa+siri.bin"
    }

```     

If the Keyword Detection mode, the app demonstrates how to:

1. Stop the instance of PryonLite VAD/SRD if previously running.
2. Publish AASB message to Auto SDK to enable "Siri" wakeword.
3. Handle "Siri" keyword detected messages from Auto SDK.
4. Publish message to Auto SDK (Arbitrator) to request dialog.
5. If the dialog is granted, calculate the start of the keyword time stamp with the indices sent in the keyword detected callback.
6. Create a requestSiri event with the start of the keyword timestamp to send to the device.




## Troubleshooting

* When interacting with Alexa, if the Dialog State goes from `LISTENING` immediately to `IDLE`, you might not be logged in. Try [logging into your account via CBL](#authenticate-with-avs-using-code-based-linking-cbl) by tapping `A` from the Main Menu.

    **Note:** For security reasons, authentication is not persisted if you quit the Sample App.  Upon relaunch, you must re-authenticate via CBL.  Restarting the app using the menu system, however, preserves authentication.

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
