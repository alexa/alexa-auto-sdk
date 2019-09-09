# Android Sample App

The purpose of the Android Sample App is to provide useful example code to help you integrate your platform implementation with the Alexa Auto SDK. The Android Sample App provides an example of creating and configuring an instance of the Engine, overriding the default implementation of each Alexa Auto Platform Interface, and registering those custom interface handlers with the Engine. It includes one default example implementation of authorizing with AVS via Code Based Linking (CBL). The Android Sample App also includes detailed logs for interactions with the Alexa Auto SDK and convenience features for viewing those logs in the application, as well as UI elements relevant to each Platform Interface implementation. 

**Table of Contents**:

* [Setting up the Sample App](#setup)
* [Running the Sample App](#run)
* [Authorization with AVS in the Sample App](#authorization)
* [Enabling Google Maps for Navigation View (optional)](#enablegooglemaps)
* [Debugging Notes](#debuggingnotes)
* [Release Notes](#releasenotes)

## Setting up the Sample App<a id="setup"></a>

### Register a Product

After registering for an Amazon developer account, you'll need to create an Alexa device and security profile. Make note of the following parameters as you go through setup: Product ID and  Client ID.

1. Log in to the [Amazon Developer portal](https://developer.amazon.com) and navigate to the [My Products](https://developer.amazon.com/avs/home.html#/avs/home) view of the AVS Console.
2. Click **Create Product** and fill in the registration form for your application. (The device registration details can be useful in certain development workflows but will not actually be used for this example.)
3. Make note of the **Product ID**.
4. Click **Next**.
5. Click **Create New Profile**.
6. Enter a name and description for your security profile.
7. Click **Next**.
8. Click **Other devices and platforms**.
9. Enter a client ID name and click **Generate ID**.
10. Make note of the **Client ID**.
11. Agree to the license terms in the Developer Portal and click **Finished**.

### Include Alexa Auto SDK Build Dependencies (AAR files)<a id="include-build-dependencies"></a>

Choose one of the following two options to include the Alexa Auto SDK build dependencies. These options correspond to two build flavors: remote and local.

> **Note:** If you want to implement any optional modules (such as wake word support, Alexa Communications, Local Voice Control (LVC), or Device Client Metrics (DCM)), you must choose option 2 and build the AARs using the Auto SDK Builder. The prebuilt AARs available in JCenter are the default Auto SDK modules only.

#### Option 1: Use the pre-built platform AARs (remote build flavor)

The pre-built platform AARs for the default Auto SDK modules are available in the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/).

The Sample App builder scripts are configured to use JCenter to always pull the latest release artifacts during compilation. To run the builder scripts, issue the following command:

```
$ gradle assembleRemoteRelease
```

Alternatively, you can manually download the platform AARs from the [JCenter repo](https://jcenter.bintray.com/com/amazon/alexa/aace/) to

 `${AAC_SDK_HOME}/samples/android/app/src/main/libs` 

> **Important!** If you choose to download the AAR files manually, you may need to create a libs directory, and you must make sure to download the platform AARs corresponding to the version of Alexa Auto SDK that you are using.


Once you have downloaded the AARs, follow the steps to [configure the project in Android Studio](#configprojectinAS).

#### Option 2: Use the Alexa Auto SDK Builder to generate the AARs (local build flavor)

If you do not use the pre-built platform AARs, you must build the Android-specific binaries for the Alexa Auto SDK library project to link to and build the Android platform AARs. See the [Alexa Auto SDK Builder](../../builder/README.md) instructions for details about how to build the Alexa Auto SDK dependencies for Android.

> **Important!** The Android Sample App always compiles the platform AARs from `${AAC_SDK_HOME}/samples/android/app/src/main/libs` and `${AAC_SDK_HOME}/builder/deploy/aar` when available.
> 
> If you decide to use the Alexa Auto SDK Builder to generate the platform AARs, make sure to remove all Alexa Auto SDK platform AARs from `${AAC_SDK_HOME}/samples/android/app/src/main/libs`. You can keep any third party libraries.

Once you have generated the platform AARs, issue the following command to run the builder scripts:

```
$ gradle assembleLocalRelease
```
Alternatively, you can follow the steps to [configure the project in Android Studio](#configprojectinAS).

### Enable Device Capabilities<a id ="enable-device-capabilities"></a>

In order to use the certain Alexa Auto SDK functionality, your product needs to be whitelisted by Amazon. Copy the product's **Amazon ID** from the Developer Console and follow the whitelisting directions on the [Need Help?](../../NEED_HELP.md) page.

### Configure the Project in Android Studio<a id="configprojectinAS"></a> (optional)

> **Note:** These instructions assume that you have [included the Alexa Auto SDK build dependencies (AAR files)](#include-build-dependencies).

1. Launch <a href=https://developer.android.com/studio/index.html>Android Studio</a> and select Open an existing Android Studio project.
1. Open the `${AAC_SDK_HOME}/samples/android` folder and click the <strong>Open</strong> button. (Tested with Android Studio version 3.x)
1. Read about [authorization with AVS in the Sample App](#authorization-with-avs-in-the-sample-app) and follow further setup instructions, if necessary.
1. Populate the `app\_config.json` file in the assets directory with the Client ID and Product ID for your product. The Product DSN may be any unique identifier, and you can add it with the key "productDsn". By default this is the device specific  ANDROID\_ID. The contents of this file are required for authorization.
2. Under **Build Variants**, select the appropriate build flavor (**localRelease** or **remoteRelease**).

> **Note**: Android Studio builds and signs the Android Package File.

## Running the Sample App<a id="run"></a>

Use Android Studio to install and run the Sample App on your target device.

* Minimum tested Android API level 22
* Minimum tested and recommended Android NDK Revision 16b

### Understanding the Sample App
The Sample App provides an example of how to create and configure an instance of the Engine, extend the Alexa Auto SDK Platform Interfaces, and register the interface implementations with the Engine. The Platform Interface implementations are located in the `impl/` folder of the `com.amazon.sampleapp` directory with the `Handler` postfix. These classes extend the JNI wrapper classes, which mirror the Alexa Auto C++ API. You can read more about these interfaces in the following documentation:
 
 * [Core module README](./modules/core/README.md)
 * [Alexa module README](./modules/alexa/README.md)
 * [Navigation module README](./modules/navigation/README.md)
 * [Phone Call Controller module README](./modules/phone-control/README.md)

>**Note:** The Android Sample App includes a simulated local phone that leverages the Phone Call Controller module. However, it cannot currently use cellular voice connections paired with or installed in the host device. This simulator can be used as an example of how to implement the Phone Control interface on the host platform to perform actions such as dialing, hanging up, etc.

The Sample App GUI consists of a menu bar and a log console. The expandable menu icon in the menu bar opens an options menu to the right of the screen that contains GUI elements relevant to the Platform Interface implementations as well as the authentication UI. Interacting with Alexa and the Engine requires successful authentication with AVS. You can log out using the **Log Out** button in the options menu, which will clear the saved refresh token.

You can use the microphone icon on the menu bar to initiate a speech interaction with Alexa, either via tap-to-talk (press and release) or hold-to-talk (press and hold). You may also initiate an interaction with the Alexa wake word, and the Sample App provides a switch in the options menu to enable or disable wake word when supported.

Initiate various interactions with Alexa and explore the options menu features and the log console to better understand the Alexa Auto SDK functionality. The logger implementation tags log messages in the following way:

- `[AVS]` refers to log messages from the AVS Device SDK.
- `[AAC]` refers to log messages from the Alexa Auto SDK.
- `[CLI]` refers to log messages from the Sample App itself.

> **Note**:
Some Alexa interactions will return data for rendering a [Display Card](https://alexa.design/dev-display-cards) for visual feedback. Card rendering in the Sample App is an example of parsing the payload of rendering calls to the TemplateRuntime Platform Interface. The Sample App implementation of these cards is not meant as a UI design guideline or requirement.

## Authorization with AVS in the Sample App<a id="authorization"></a>

To access the Alexa Voice Service (AVS) your product must acquire valid access tokens. Code-Based Linking (CBL) is the recommended method to acquire such tokens and is demonstrated by the Android Sample App.

### Register Security Profile with Login With Amazon

In the [Amazon Developer portal](https://developer.amazon.com), go to **Login With Amazon**.
Here you will be able to register the security profile you associated with your product. You will need to provide your company privacy policy URL and a logo image.

## Enabling Google Maps for Navigation view (optional)<a id = "enablegooglemaps"></a>

In order to view Google Map locations on navigation requests via the Sample App, you must input your own Google Maps API key.

1. Read about how to get the API key in the [Google Maps Documentation](https://developers.google.com/maps/documentation/android-sdk/signup).
2. Put the API key into the `AndroidManifest.xml` under the **value** section of the meta data tag `com.google.android.geo.API_KEY`.

## Debugging notes<a id = "debuggingnotes"></a>

### Debugging the Sample App
Use Android Studio and press the **Debug 'app'** button on toolbar. This will launch the app on target device and attach the app with Android Studio debugger.

### Debugging Java Code<a id="java-debugging"></a>
Debugging Java code is straightforward. Open the file and scroll to the method which you wish to debug. Click on to the vertical gray bar on the left of the source editor. This will set the breakpoint on the source line. Use the Android Studio UI to navigate the stack trace, watch variables etc., when breakpoint is hit.

### Debugging C++ Code
Debugging instructions for the Alexa Auto SDK C++ library are same as the instructions for debugging Java code. The Alexa Auto SDK Android library is compiled and linked by Android Studio, so no special instructions are required.

### Debugging External Libraries

To debug all external libraries which are not built by Android Studio, follow these instructions (At present, All the modules except the platform/android and sample/android are built outside Android Studio.)

Prerequisite: Install the Low Level Debugger (LLDB) package in Android Studio. You can find the LLDB component on the **SDK Manager** -> **SDK Tools** tab.

1. Use the builder script to generate a build with debug symbols. See "Running Builder" in the [Builder README](../../builder/README.md) for documentation of builder options. For example, to build an Android x86 build with debugging symbols use:

	`builder/build.sh oe -t androidx86 -g`
2. Debug symbols are located at:

	`builder/deploy/android<arch>/aac-sdk-build-<arch>-android-<apilevel>-dbg.tar.gz`
3. Extract the symbol tar: (`tar -xvf aac-sdk-build-<arch>-android-<apilevel>-dbg.tar.gz`). Extracted symbols are located here: `builder/deploy/androidx86/opt/AAC/lib/.debug`
4. Rebuild the platform AARs using: `builder/build.sh gradle -g`
5. Rebuild the Sample App in Android Studio.
6. Launch the Sample App in Android studio by pressing the **Debug 'app'** button on the toolbar
7. Select **View** -> **Tool Windows** -> **Debug**
8. In the debug window, switch to the **lldb** tab.
9. To debug each external library, provide LLDB the location of the library's symbols using `add-dsym`. For example:

	`(lldb) add-dsym ${AAC_SDK_HOME}/builder/deploy/androidx86/opt/AAC/lib/.debug/libAACECoreEngine.so`
10. Use LLDB `image lookup` to locate the function to debug. For example: `image lookup -vn EngineImpl::start`
11. If you are debugging on MacOS, then the `CompileUnit` (source file) output of LLDB `image lookup` points to the file system in the docker image. Use LLDB `settings set target.source-map` to direct LLDB to locate the source on the local file system instead. For example:

 ```
 (lldb) settings set target.source-map /workdir/build/tmp-android-22/work/core2-32-linux-android/
aac-module-core/0.99.0-r0/src/engine/src/ ${AAC_SDK_HOME}/modules/core/engine/src/
```

 >**Note:**  Remember to do this for each module you wish to debug.

12. From the LLDB `image lookup` output, locate the fully qualified unmangled name of the function ("Function: name = "). Set the breakpoint on the function with LLDB `breakpoint set`. For example:

	 `breakpoint set --name aace::engine::core::EngineImpl::start`
13. When the breakpoint is hit, use the Android Studio debugger window to find call stacks, watch variables, etc. You can also set a further breakpoint using Android Studio editor, with either method described under [Debugging Java Cpde](#java-debugging), and use other features of LLDB as you need.

> **Note**: If the LLDB window isn't visible in the debug tab, select **Run** -> **Edit Configurations** -> **Debugger** and change the `Debug Type` to `Native`.

## v2.0.0 Release Notes<a id="releasenotes"></a>

#### Resolved Issues

* Fixed an issue where music playback continued after logging out from the Sample App. 
* Fixed an issue where the display card and book title were not displayed for Kindle content.
* Fixed some broken links in the documentation.
* Fixed an issue where the MACCAndroidClient did not rediscover Media App Command and Control (MACC) compliant apps if they were unresponsive after being idle for a long period (around 30 minutes).
  
#### Known Issues
* Pressing the Pause button immediately after pressing the Next button does not advance or pause the music. Instead the currently-playing song continues playing. In addition, the play controller dialog advances to the next song details but then jumps back to the current song details.
* If the microphone is in use, the Android Sample App does not disable Alexa triggers such as tap-to-talk.
* The Android Sample App goes into listening mode when calling 911, as Alexa prompts users to say "Alexa, Cancel" to stop. This is due to the fact that the Android platform does not provide audio loopback to cancel out self triggers. See the [Loopback Detector README](../../extensions/loopback-detector/README.md) for an example of how to use the Auto SDK with the AmazonLite wakeword to cancel out self references.
* The `AudioOutput.prepare` method implementation in `AudioOutputHandler` blocks returning until it reads the full audio attachment from the Engine on the caller's thread. It should not do this and instead should return quickly and read the attachment on a separate thread.
* When authenticated with CBL, if network connectivity is lost while refreshing the access token, the Sample App does not automatically attempt to refresh the token when connectivity is restored, and the GUI incorrectly displays the "log in" button even though a refresh token is present. Restoring an authorized state requires either restarting the app or following the log in steps again with network connectivity.
* Display card rendering is not adaptable to a variety of screen sizes.
* Particular sections of the Flash Briefing do not resume from a paused state properly. Attempting to play after pause in some cases may restart the media playback.
* The app does not implement managing inter-app audio focus, so other apps will not recognize its audio playback appropriately.
* Alexa dialog playback may stop abruptly when switching between Wi-Fi and mobile data.
* The app disconnects from AVS after remaining idle for some time and takes a while to reconnect.
* After enabling location services, you must restart the Sample App in order to use the location services.