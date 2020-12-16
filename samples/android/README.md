# Alexa Auto SDK Android Sample App

The purpose of the Android Sample App is to provide useful example code to help you integrate your platform implementation with the Alexa Auto SDK. The Android Sample App provides an example of creating and configuring an instance of the Engine, overriding the default implementation of each Alexa Auto Platform Interface, and registering those custom interface handlers with the Engine. It includes one default example implementation of authorizing with Alexa Voice Service (AVS) via Code Based Linking (CBL). The Android Sample App also includes detailed logs for interactions with the Alexa Auto SDK and convenience features for viewing those logs in the application, as well as UI elements relevant to each Platform Interface implementation. 
<!-- omit in toc -->
## Table of Contents

- [Prerequisites](#prerequisites)
- [Enabling Optional Device Capabilities](#enabling-optional-device-capabilities)
- [Setting up the Android Sample App](#setting-up-the-android-sample-app)
- [Running the Android Sample App](#running-the-android-sample-app)
- [Using the Android Sample App](#using-the-android-sample-app)
- [Debugging Notes](#debugging-notes)
- [Known Issues](#known-issues)

## Prerequisites

### Amazon Developer Account

To use the Android Sample App, you need an [Amazon Developer](https://developer.amazon.com/docs/app-submission/manage-account-and-permissions.html#create-a-developer-account/) account.

### Registered Product and Security Profile

After creating an Amazon developer account, you'll need to [register a product and create a security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html) on the AVS developer portal.

When you follow the instructions to [fill in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information):

* Use your own custom information, taking note of the **Product ID**, as this information is required for your [configuration file](#configuration-file) .
* Be sure to select **Automotive** from the **Product category** pull-down.

When you follow the instructions to [set up your security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#set-up-your-security-profile), generate a **Client ID** and take note of it, as this information is required for your [configuration file](#configuration-file).

### Configuration File

The Android Sample App requires a [configuration file](#editing-the-configuration-file) that contains information device client information required for authorization with AVS. 

## Enabling Optional Device Capabilities

In order to use certain optional Alexa Auto SDK functionality (for example, AmazonLite Wake Word, Alexa Communications, Local Voice Control (LVC), Device Client Metrics (DCM), or Voice Chrome for Android) with the Sample App, your product must be placed on the allow list by Amazon. Copy the product's **Amazon ID** from the Developer Console and follow the directions on the [Need Help?](../../NEED_HELP.md#requesting-additional-functionality) page.

## Setting up the Android Sample App

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
### Editing the Configuration File

You must populate the [`app_config.json`](./app/src/main/assets/app_config.json) file with the configuration information required to authorize your device profile with AVS:

1. Open the [`app_config.json`](./app/src/main/assets/app_config.json) file in your favorite editor.

2. For the the `"clientId"` parameter, replace `"<YOUR DEVICE'S CLIENT ID>"` with the Client ID that you generated when you [set up your security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#set-up-your-security-profile) for your development device.
3. For the `"productId"` parameter, replace `"<YOUR DEVICE'S PRODUCT ID>"` with the Product ID that you entered when you [filled in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information) for your development device.

    >**Note:** The `"clientId"` and `"productId"` must correspond to a development device profile that you created as an **automotive** product by selecting the `Automotive` product category when you [filled in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information).  
    
>**Note:**You can leave `"amazonId"` set to its placeholder value. This parameter is not required unless you are using the optional Device Client Metrics (DCM) extension.

### Including Alexa Auto SDK Build Dependencies (AAR files)

Choose one of the following two options to include the Alexa Auto SDK build dependencies. These options correspond to two build flavors: remote and local.

> **Note:** If you want to implement any optional modules (such as wake word support, Alexa Communications, Local Voice Control (LVC), Device Client Metrics (DCM), or Voice Chrome for Android), you must choose option 2 and build the platform AARs and the core-sample AAR using the Auto SDK Builder. The prebuilt AARs available in JCenter are for the default Auto SDK modules only.

> **Note:** The Auto SDK requires Gradle version 4.10.1 - 5.6.2 (5.1.1 - 5.6.2 if you are using Android Studio).

#### Option 1: Use the pre-built platform AARs (remote build flavor)

The pre-built platform AARs for the default Auto SDK modules and the core-sample AAR required to run the Android Sample App are available in the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/).

The Sample App builder scripts are configured to use JCenter to always pull the latest release artifacts during compilation. To run the builder scripts, issue the following command:

```
$ ./gradlew assembleRemoteRelease
```

Alternatively, you can manually download the platform AARs and core-sample AAR from the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/) to

 `${AAC_SDK_HOME}/samples/android/app/src/main/libs` 

> **Important!** If you choose to download the AAR files manually, you may need to create a libs directory, and you must make sure to download the platform AARs and core-sample AAR corresponding to the version of Alexa Auto SDK that you are using.


Once you have downloaded the AARs, follow the steps to [configure the project in Android Studio](#configure-the-project-in-android-studio).

#### Option 2: Use the Alexa Auto SDK Builder to generate the AARs (local build flavor)

If you do not use the pre-built platform AARs and core-sample AAR, you must build the Android-specific binaries for the Alexa Auto SDK library project to link to and build the Android platform AARs and the core-sample AAR. See the [Alexa Auto SDK Builder](../../builder/README.md#for-android-targets) instructions for details about how to build the Alexa Auto SDK dependencies for Android.

> **Important!** The Android Sample App always compiles the platform AARs and core-sample AAR from `${AAC_SDK_HOME}/samples/android/app/src/main/libs` and `${AAC_SDK_HOME}/builder/deploy/aar` when available.

> If you decide to use the Alexa Auto SDK Builder to generate the platform AARs and the core-sample AAR, make sure to remove all Alexa Auto SDK platform AARs from `${AAC_SDK_HOME}/samples/android/app/src/main/libs`. You can keep any third party libraries.

Once you have generated the platform AARs and the core-sample AAR, issue the following command to run the builder scripts:

``` 
$ ./gradlew assembleLocalRelease
```
Alternatively, you can follow the steps to [configure the project in Android Studio](#configure-the-project-in-android-studio).

>**Note:** If you get gradle-related errors (such as `Could not open settings remapped class cache...`, `Could not open settings generic class cache...`, or `BUG! exception in phase 'semantic analysis' in source unit 'BuildScript' Unsupported class file major version 57`) when attempting to build the Android Sample App using the Alexa Auto SDK Builder, install Java 8 and point the java_home directory to 1.8:
`export JAVA_HOME=$(/usr/libexec/java_home -v 1.8)`.

### Configure the Project in Android Studio (optional)

> **Note:** The Auto SDK requires Android Studio version 3.4.1+. In addition, you must ensure that the Gradle version you are using is compatible with the Android Studio version you are using. See the [Android Gradle Plugin Release Notes](https://developer.android.com/studio/releases/gradle-plugin#updating-gradle) for information about matching Android Studio versions to Gradle versions.

> **Note:** These instructions assume that you have [populated the `app/assets/app_config.json` file](#editing-the-configuration-file) with information specific to your device and [included the Alexa Auto SDK build dependencies (AAR files)](#including-alexa-auto-sdk-build-dependencies-aar-files).

1. Launch <a href=https://developer.android.com/studio/index.html>Android Studio</a> and select Open an existing Android Studio project.
2. Open the `${AAC_SDK_HOME}/samples/android` folder and click the <strong>Open</strong> button. (Tested with Android Studio version 3.x)
3. Under **Build Variants**, select the appropriate build flavor (**localRelease** or **remoteRelease**).

> **Note**: Android Studio builds and signs the Android Package File.

## Running the Android Sample App

Use Android Studio to install and run the Sample App on your target device.

* Minimum tested Android API level 22
* Minimum tested and recommended Android NDK Revision 16b

### Authenticating with AVS using Code-Based Linking (CBL)

When the Sample App launches, it displays a code and a URL in a box. Follow the on-screen prompt to authenticate with AVS using CBL:

1. Open a browser and navigate to the URL displayed in the Sample App.
2. In the browser, enter the code displayed in the Sample App.
3. Click **Continue** and follow the onscreen instructions in the browser to complete the authentication.

## Using the Android Sample App
The Sample App provides an example of how to create and configure an instance of the Engine, extend the Alexa Auto SDK Platform Interfaces, and register the interface implementations with the Engine. The Platform Interface implementations are located in the `impl/` folder of the `com.amazon.sampleapp` directory with the `Handler` postfix. These classes extend the JNI wrapper classes, which mirror the Alexa Auto C++ API. You can read more about these interfaces in the following documentation:
 
 * [Core module README](../../platforms/android/modules/core/README.md)
 * [Alexa module README](../../platforms/android/modules/alexa/README.md)
 * [Navigation module README](../../platforms/android/modules/navigation/README.md)
 * [Phone Call Controller module README](../../platforms/android/modules/phonecontrol/README.md)
 
    >**Note:** The Android Sample App includes a simulated local phone that leverages the Phone Call Controller module. However, it cannot currently use cellular voice connections paired with or installed in the host device. You can use this simulator as an example of how to implement the Phone Control interface on the host platform to perform actions such as dialing, hanging up, etc.
 * [Code-based Linking (CBL) module README](../../platforms/android/modules/cbl/README.md)
 * [Alexa Presentation (APL) module README](../../platforms/android/modules/apl/README.md)
    >**Note:** APL rendering on the Android Sample App requires a component that is available by [request from your Amazon Solutions Architect (SA) or Partner Manager](../../NEED_HELP.md#requesting-additional-functionality).

The Sample App GUI consists of a menu bar and a log console. The expandable menu icon in the menu bar opens an options menu to the right of the screen that contains GUI elements relevant to the Platform Interface implementations as well as the authentication UI. Interacting with Alexa and the Engine requires successful authentication with AVS. You can log out using the **Log Out** button in the options menu, which will clear the saved refresh token.

You can use the microphone icon on the menu bar to initiate a speech interaction with Alexa, either via tap-to-talk (press and release) or hold-to-talk (press and hold). You can also initiate an interaction with the Alexa wake word.

Initiate various interactions with Alexa and explore the options menu features and the log console to better understand the Alexa Auto SDK functionality. The logger implementation tags log messages in the following way:

- `[AVS]` refers to log messages from the AVS Device SDK.
- `[AAC]` refers to log messages from the Alexa Auto SDK.
- `[CLI]` refers to log messages from the Sample App itself.

> **Note**:
Some Alexa interactions will return data for rendering a [Display Card](https://alexa.design/dev-display-cards) for visual feedback. Card rendering in the Sample App is an example of parsing the payload of rendering calls to the TemplateRuntime Platform Interface. The Sample App implementation of these cards is not meant as a UI design guideline or requirement.

### Authorization with AVS in the Sample App

Every request to Amazon Voice Service (AVS) requires an Login with Amazon (LWA) access token. Code-Based Linking (CBL) is the recommended method to acquire access tokens and is demonstrated by the Android Sample App. See the [CBL module README](../../platforms/android/modules/cbl/README.md) for details about the Auto SDK's implementation of CBL.

### Handling Unknown Locations in Navigation Use Cases
Your platform implementation should handle cases where a GPS location cannot be obtained by returning the `UNDEFINED` value provided by the Auto SDK. In these cases, the Auto SDK does not report the location in the context, and your platform implementation should return a localization object initialized with `UNDEFINED` values for latitude and longitude ((latitude,longitude) = (`UNDEFINED`,`UNDEFINED`)) in the context object of every SpeechRecognizer event.

### Enabling Google Maps for Navigation view (optional)

In order to view Google Map locations on navigation requests via the Sample App, you must input your own Google Maps API key.

1. For details about how to get the API key, see the [Google Maps Documentation](https://developers.google.com/maps/documentation/android-sdk/signup).
2. Put the API key into the `AndroidManifest.xml` file under the **value** section of the meta data tag `com.google.android.geo.API_KEY`.

### Enabling SiriusXM as a Local Media Source
The Sample App does not configure SiriusXM as a local media source by default. If you need the SiriusXM local media source, you must enable and build it. To do this, uncomment the Mock SIRIUSXM platform handler registration in the [`MainActivity.java`](./app/src/main/java/com/amazon/sampleapp/MainActivity.java) class, then rebuild the Sample App.

>**Note:** When SiriusXM is present as a local media source, the cloud defaults to local SiriusXM only and blocks any use of the cloud SiriusXM service even if the local implementation/service is unavailable or not enabled.  

### SpeakerManager Configuration

`SpeakerManager` is now a configurable option, enabled by default. When not enabled, user requests to change the volume or mute now have an appropriate Alexa response, e.g. "Sorry, I can't control the volume on your device".

You can programmatically generate speaker manager configuration using the `createSpeakerManagerConfig()` factory method, or provide the equivalent JSON values in a configuration file.
```
{
    "aace.alexa": {
        "speakerManager": {
            "enabled": false
        }
    }
}
```

See the the Android Sample App MainActivity.java for an example of programmatically generating your speaker manager configuration.
```
ArrayList<EngineConfiguration> configuration = new ArrayList<EngineConfiguration>(Arrays.asList(
    ...
    AlexaConfiguration.createSpeakerManagerConfig( false ),
    ...
));
```

## Debugging Notes

### Debugging the Sample App
Use Android Studio and press the **Debug 'app'** button on toolbar. This will launch the app on target device and attach the app with Android Studio debugger.

### Debugging Java Code
Debugging Java code is straightforward. Open the file and scroll to the method which you wish to debug. Click on to the vertical gray bar on the left of the source editor. This will set the breakpoint on the source line. Use the Android Studio UI to navigate the stack trace, watch variables etc., when breakpoint is hit.

### Debugging C++ Code
Debugging instructions for the Alexa Auto SDK C++ library are same as the instructions for debugging Java code. The Alexa Auto SDK Android library is compiled and linked by Android Studio, so no special instructions are required.

### Debugging External Libraries

To debug all external libraries which are not built by Android Studio, follow these instructions (at present, all the modules except the platform/android and sample/android are built outside Android Studio.)

Prerequisite: Install the Low Level Debugger (LLDB) package in Android Studio. You can find the LLDB component on the **SDK Manager** -> **SDK Tools** tab.

1. Use the builder script to generate a build with debug symbols. See the [Builder README](../../builder/README.md#builder-command-arguments) for documentation of builder options. For example, to build an Android x86-64 build with debugging symbols use:

	`builder/build.sh android -t androidx86-64 -g`
2. Debug symbols are located at:

	`builder/deploy/android<arch>/aac-sdk-build-<arch>-android-<apilevel>-dbg.tar.gz`
3. Extract the symbol tar: (`tar -xvf aac-sdk-build-<arch>-android-<apilevel>-dbg.tar.gz`). Extracted symbols are located here: `builder/deploy/androidx86-64/opt/AAC/lib/.debug`
4. Rebuild the platform AARs using: `builder/build.sh gradle -g`
5. Rebuild the Sample App in Android Studio.
6. Launch the Sample App in Android studio by pressing the **Debug 'app'** button on the toolbar
7. Select **View** -> **Tool Windows** -> **Debug**
8. In the debug window, switch to the **lldb** tab.
9. To debug each external library, provide LLDB the location of the library's symbols using `add-dsym`. For example:

	`(lldb) add-dsym ${AAC_SDK_HOME}/builder/deploy/androidx86-64/opt/AAC/lib/.debug/libAACECoreEngine.so`
10. Use LLDB `image lookup` to locate the function to debug. For example: `image lookup -vn EngineImpl::start`
11. If you are debugging on macOS, then the `CompileUnit` (source file) output of LLDB `image lookup` points to the file system in the docker image. Use LLDB `settings set target.source-map` to direct LLDB to locate the source on the local file system instead. For example:

 ```
 (lldb) settings set target.source-map /workdir/build/tmp-android-22/work/core2-32-linux-android/
aac-module-core/0.99.0-r0/src/engine/src/ ${AAC_SDK_HOME}/modules/core/engine/src/
```

 >**Note:**  Remember to do this for each module you wish to debug.

12. From the LLDB `image lookup` output, locate the fully qualified unmangled name of the function ("Function: name = "). Set the breakpoint on the function with LLDB `breakpoint set`. For example:

	 `breakpoint set --name aace::engine::core::EngineImpl::start`
13. When the breakpoint is hit, use the Android Studio debugger window to find call stacks, watch variables, etc. You can also set a further breakpoint using Android Studio editor, with either method described under [Debugging Java Code](#java-debugging), and use other features of LLDB as you need.

> **Note**: If the LLDB window isn't visible in the debug tab, select **Run** -> **Edit Configurations** -> **Debugger** and change the `Debug Type` to `Native`.

## Known Issues

* In the Alexa companion app, if you change the timezone of the device to a value already listed in the sample app timezone drop down, the timezone field of the sample app is not updated to the new value until the app is restarted.
* Alexa Presentation Language (APL) rendering does not support ssmlToSpeech and ssmlToText transformers. Therefore, some skills, like Jeopardy, will not provide the expected user experience.
* The sample app goes into listening mode when calling 911, as Alexa prompts the user to say "Alexa, Cancel" to stop. This is due to the Android platform not providing audio loopback to cancel out self triggers. See the [Loopback Detector README](../../extensions/loopback-detector/README.md) for an example of using the Auto SDK with the AmazonLite wake word to cancel out self references.
* The `AudioOutput.prepare()` method implementation in `AudioOutputHandler` blocks returning until it reads the full audio attachment from the Engine on the caller's thread. If working properly, the implementation returns quickly and reads the attachment on a separate thread.
* A generic error TTS is returned when the user initiates offline contact calling before uploading contacts to the cloud on the sample app with the optional Local Voice Control extension.
* Particular sections of the Flash Briefing do not resume properly from a paused state. Attempting to play after pause in some cases may restart the media playback.
* Display card rendering is not adaptable to a variety of screen sizes.
* The sample app does not implement managing inter-app audio focus. As a result, other apps do not recognize its audio playback appropriately.
* Alexa dialog playback may stop abruptly when switching between Wi-Fi and mobile data.
* The sample app disconnects from AVS after remaining idle for some time and takes a while to reconnect.
* Music service provider logos in the SVG format are not rendered during the music playback.
