# Alexa Auto SDK C++ Sample App

## Overview

The purpose of the C++ Sample App is to provide useful example code to help you integrate your implementation with the Alexa Auto SDK. The C++ Sample App provides an example of creating and configuring an instance of the Engine, and using the MessageBroker API to subscribe to messages from the Engine. It also provides examples of handling audio and stream based interfaces with the MessageStream API, and replying to messages from the Engine. The C++ Sample App also includes detailed logs for interactions with the Alexa Auto SDK, as well as UI elements relevant to the implementation.

## Prerequisites

### Amazon developer account

To use the C++ Sample App, you need an [Amazon Developer](https://developer.amazon.com/docs/app-submission/manage-account-and-permissions.html#create-a-developer-account/) account.

### Register product and security profile

After creating an Amazon developer account, you'll need to [register a product and create a security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html) on the AVS developer portal.

When you follow the instructions to [fill in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information):

* Use your own custom information, taking note of the **Product ID**, as this information is required to confingure the Sample App.
* Be sure to select **Automotive** from the **Product category** pull-down.

When you follow the instructions to [set up your security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#set-up-your-security-profile), generate a **Client ID** and take note of it, as this information is required to configure the Sample App.

### Optional device capabilities

In order to use certain optional Alexa Auto SDK functionality (for example, AmazonLite Wake Word, Alexa Communications, Local Voice Control (LVC), and Device Client Metrics (DCM)) with the Sample App, your product must be placed on the allow list by Amazon. Copy the product's **Amazon ID** from the Developer Console and follow the directions on the [Need Help?](https://alexa.github.io/alexa-auto-sdk/docs/help) page.

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

**`aac_modules`** - Specify default Auto SDK modules to build with the Sample App. This is a comma seperated list of modules that must be installed in the Conan local cache before building. If this option is not overridden, the default value will be `core, alexa, cbl, system-audio`.

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

## Troubleshooting

* When interacting with Alexa, if the Dialog State goes from `LISTENING` immediately to `IDLE`, you might not be logged in. Try [logging into your account via CBL](#authenticate-with-avs-using-code-based-linking-cbl) by tapping `A` from the Main Menu.

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