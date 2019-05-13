## Android Sample Application

### Overview

The Android Sample App demonstrates a basic platform implementation of Alexa Auto SDK integration. It provides an example of creating and configuring an instance of the Engine, overriding the default implementation of each Alexa Auto Platform Interface, and registering those custom interface handlers with the Engine. It includes one default example implementation of authorizing with AVS via Code Based Linking (CBL), and one additional authorization method via Login With Amazon (LWA) browser which requires some additional setup. It also includes detailed logs for interactions with the Alexa Auto SDK and developer convenience features for viewing those logs in the application, and UI elements relevant to each Platform Interface implementation. The purpose of the Android Sample App is to provide developers with useful example code to facilitate integration with the Alexa Auto SDK.

Click [here](#androidsampleapprelnote) to read the release notes for this sample app.

### Setup

#### Register a Product

After registering for an Amazon developer account, you'll need to create an Alexa device and security profile. Make note of the following parameters as you go through setup: Product ID, Client ID, and Client Secret.

1. Log in to the [Amazon Developer portal](https://developer.amazon.com) and navigate to the [My Products](https://developer.amazon.com/avs/home.html#/avs/home) view of the AVS Console.
2. Click **Create Product** and fill in the registration form for your application. (The device registration details can be useful in certain development workflows but will not actually be used for this example.)
3. Make note of the **Product ID**.
4. Click **Next**.
5. Click **Create New Profile**.
6. Enter a name and description for your security profile.
7. Click **Next**.
8. Make note of the **Client ID** and **Client Secret**.
9. Agree to the license terms in the Developer Portal and click **Finished**.

#### Alexa Auto SDK Build Dependencies

There are two options for including the Alexa Auto SDK dependencies:

##### Option 1: Use the pre-built aace-release.aar

To use the pre-built aace-release.aar, first download aace-release.aar from [Alexa Auto SDK Releases](https://github.com/alexa/aac-sdk/releases) to `${AAC_SDK_HOME}/samples/android/app/src/main/libs` *(you may need to create a libs directory)*, then follow steps to [configure the project in Android Studio](#configure-the-project-in-android-studio).

> **Note**: Make sure to download the aace-release.aar corresponding to the version of Alexa Auto SDK that you are using.

##### Option 2: Use Alexa Auto SDK Builder

If you do not use the pre-built aace-release.aar, you must build the Android specific binaries for the Alexa Auto SDK library project to link to. See the [Alexa Auto SDK Builder](../../builder/README.md) instructions to build the Alexa Auto SDK binaries for your Android target.

> **Note**: The Android Sample App always compiles with pre-built aace-release.aar when available. You must remove aace-release.aar from `${AAC_SDK_HOME}/samples/android/app/src/main/libs` if you decide to use Alexa Auto SDK Builder instead.

#### Enabling Device Capabilities

In order to use the certain Alexa Auto SDK functionality, your product needs to be whitelisted by Amazon. Copy the product's **Amazon ID** from the Developer Console and follow the whitelisting directions on the [Need Help?](../../NEED_HELP.md) page.

#### Configure the Project in Android Studio

1. Launch <a href=https://developer.android.com/studio/index.html>Android Studio</a> and select Open an existing Android Studio project."
1. Open the `${AAC_SDK_HOME}/samples/android` folder and click the <strong>Open</strong> button. (Tested with Android Studio version 3.x)
1. Include the [Alexa Auto SDK Build Dependencies](#alexa-auto-sdk-build-dependencies).
If you do not use the pre-built aace-release.aar, you must select **Build > Refresh Linked C++ Projects** in Android Studio to update the dependencies.
1. Read about [authorization with AVS in the Sample App](#authorization-with-avs-in-the-sample-app) and follow further setup instructions, if necessary.
1. Populate the app\_config.json file in the assets directory with the Client ID, Client Secret, and Product ID for your product. Product DSN may be any unique identifier, and can be added with the key "productDsn". By default this is the device specific  ANDROID\_ID. The contents of this file are required for authorization.
> ***Note***: Android Studio builds and signs the Android Package File.


### Running the Sample App

Use Android Studio to install and run the Sample App on your target device.

* Minimum tested Android API level 22
* Minimum tested and recommended Android NDK Revision 16b

### Understanding the Sample App
The Sample App provides an example of how to create and configure an instance of the Engine, extend the Alexa Auto SDK Platform Interfaces, and register the interface implementations with the Engine. The Platform Interface implementations are located in the `impl/` folder of the `com.amazon.sampleapp` directory with the `Handler` postfix. These classes extend the JNI wrapper classes, which mirror the Alexa Auto C++ API. You can read more about these interfaces in the following documentation:

- [Core Module](../../platforms/android/CORE.md)
- [Alexa Module](../../platforms/android/ALEXA.md)
- [Navigation Module](../../platforms/android/NAVIGATION.md)
- [Phone Control Module](../../platforms/android/PHONECONTROL.md)

>**Note:** The Android Sample App includes a simulated local phone that leverages the Phone Control Module. However, it cannot currently use cellular voice connections paired with or installed in the host device. This simulator can be used as an example of how to implement the Phone Control interface on the host platform to perform actions such as dialing, hanging up, etc.

The Sample App GUI consists of a menu bar and a log console. The expandable menu icon in the menu bar opens an options menu to the right of the screen that contains GUI elements relevant to the Platform Interface implementations as well as the authentication UI. Interacting with Alexa and the Engine requires successful authentication with AVS. You can log out using the **Log Out** button in the options menu, which will clear the saved refresh token.

The microphone icon on the menu bar may be used to initiate a speech interaction with Alexa, either via tap-to-talk (press and release) or hold-to-talk (press and hold). You may also initiate an interaction with the Alexa wake word, and the Sample App provides a switch in the options menu to enable or disable wake word when supported.

Initiate various interactions with Alexa and explore the options menu features and the log console to better understand the Alexa Auto SDK functionality. The logger implementation tags log messages in the following way:

- `[AVS]` refers to log messages from the AVS Device SDK.
- `[AAC]` refers to log messages from the Alexa Auto SDK.
- `[CLI]` refers to log messages from the Sample App itself.

> **Note**:
Some Alexa interactions will return data for rendering a [Display Card](https://alexa.design/dev-display-cards) for visual feedback. Card rendering in the Sample App is an example of parsing the payload of rendering calls to the TemplateRuntime Platform Interface. The Sample App implementation of these cards is not meant as a UI design guideline or requirement.


### Authorization with AVS in the Sample App

To access the Alexa Voice Service (AVS) it is required that your product acquire valid access tokens. By default the Android Sample App demonstrates the CBL method to acquire such tokens:

- **Code-Based Linking (CBL)**: The user is provided with a short alphanumeric code and a URL in which to enter the code on any web browser-enabled device. [Read more about CBL](https://developer.amazon.com/docs/alexa-voice-service/code-based-linking-other-platforms.html), if desired. The Sample App implementation of CBL requires no further setup. It includes an example of optionally fetching user profile information, which requires you [add your company privacy policy URL and a logo image to your security profile](#register-security-profile-with-login-with-amazon).
- **Login With Amazon Browser**: The app launches a browser window where the user can login with their Amazon credentials. This can be used instead of the CBL implementation, but requires [additional setup](#login-with-amazon-browser-setup). 

#### Register Security Profile with Login With Amazon

In the [Amazon Developer portal](https://developer.amazon.com), go to **Login With Amazon**.
Here you will be able to register the security profile you associated with your product. You will need to provide your company privacy policy URL and a logo image.

#### Login with Amazon Browser Setup

In order to use LWA with browser instead of CBL, the following steps must be completed: 

- [Add the LWA library to the project](#getting-the-lwa-library)
- [Get an API Key](#generating-the-api-key)
- [Change Auth Handler registration code](#LWA-code-changes)

##### Getting the LWA library

Download and expand the [Amazon Developer SDK for Android](https://developer.amazon.com/sdk-download) and add the library to your Android Studio project:

1. Navigate to the `Login With Amazon` folder
2. Copy the `login-with-amazon-sdk.jar` library to the Android Sample App libs folder ( In the `AAC_SDK_PATH/samples/android/app/libs` directory)


##### Generating the API key

You will need an [API key](https://developer.amazon.com/docs/login-with-amazon/create-android-project.html#add-api-key)
 to get started. The API key can be generated when you associate your Android Sample App with a Security Profile in the Developer Portal. You need to add the Sample App package identifier and Android Studio's debug runtime signature to the Security Profile for your product according to the following instructions:

1. Select the **Security Profile > Android/Kindle** settings for your product in the Developer Portal.
2. Fill in the fields as follows:
	* The **API key name** is an arbitrary name
	* The **Package ID** must match the app's package identifier (i.e. `com.amazon.sampleapp` by default)
	* The **MD5** and **SHA256** signatures can be extracted from the Android Studio keystore. By default, these can be found by using Java's `keytool` (`${JAVA_HOME}/bin/keytool`) such as in the following example:
		```
		keytool -list -v -keystore ~/.android/debug.keystore
		```
		There is no password by default.

		If you are not using the default keystore, view more keystore instructions under [Determining an Android App Signature](https://alexa.design/dev-lwa-register#determining-an-android-app-signature).
		If you do not see the MD5, you may need to temporarily switch Java version to 1.8.
		You can check the java home version as follows:

		```
		/usr/libexec/java_home -V
		```
		If you don't see Java version 1.8, you need to download it, then
		```
		export JAVA_HOME=`/usr/libexec/java_home -v 1.8`
		```

3. Return to the Developer portal and finish filling in the form.
4. Click the **Generate Key** button.
5. Copy the API key and paste it into the `api_key.txt` file in the Android Studio project according to [Add Your API Key to Your Project](https://alexa.design/dev-create-lwa-project).

See [Register for Login With Amazon](http://alexa.design/dev-lwa-register) for more details about registering with LWA.

##### LWA Code Changes

A few minor code changes must be made to switch to using LWA with browser over CBL. 

1. In the MainActivity.java, uncomment the import statement below the `// LWA Browser Sample Code` comment. 
2. In the MainActivity.java, comment out the `// CBL Auth Handler`, uncomment the `// LWA Auth Handler`. 

##### Local Voice Engine (LVE) Code Changes

In order to communicate with the LVE app, the Alexa Auto app should share the same user ID as the LVE app. To achieve that, you need to update the Android manifest file of your Alexa Auto app to have the `android:sharedUserId` set to `com.amazon.AlexaAutoUserId`
For example:
```
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    android:sharedUserId="com.amazon.AlexaAutoUserId"
    package="<YOUR_ALEXA_AUTO_APP_PACKAGE_NAME>">
```

### Enable Google Maps for Navigation view ( Optional )

In order to view Google Map locations on navigation requests via the sample app, you must input your own Google Maps API key.

1. Read about how to get the API key in the [Google Maps Documentation](https://developers.google.com/maps/documentation/android-sdk/signup).
2. Put the API key into the AndroidManifest.xml under the value section of the meta data tag "com.google.android.geo.API_KEY".

### Debugging notes

#### Debugging the app
Use Android Studio and hit the "Debug 'app'" button on toolbar. This will launch the app on target device and attach the app with Android Studio debugger.

#### Debugging Java code
Debugging Java code is straight forward. Open the file and scroll to the method which you wish to debug. Click on to the vertical gray bar on the left of the source editor. This will set the breakpoint on the source line. Use Android Studio UI interface to navigate the stack trace, watch variables etc., when breakpoint is hit

#### Debugging C++ Code
Debugging instructions for the Alexa Auto SDK C++ library are same as the instruction for Java code. The Alexa Auto SDK Android library is compiled and linked by Android Studio so no special instructions are required. Similarly for more C++ projects in Android Studio, debugging is seamless.

For debugging all external libraries which are not built by Android Studio, follow these instructions (At present, All the modules except the platform/android and sample/android are built outside Android Studio.)

PreRequisite: Install the Low Level Debugger (LLDB) package in Android Studio. The LLDB component can be found on the SDK Manager -> SDK Tools tab.

1. Use builder script to generate a build with debug symbols (Reference: builder/README.md, look for Running Builder -> `options`). For example, to build Android x86 build with debugging symbols use: `builder/build.sh -t androidx86 -g`
2. Debug symbols (For android, symbols are packaged in so file) can be located at: `builder/deploy/android<arch>/aac-image-minimal-<arch>-android-<apilevel>-dbg.tar.gz`
3. Extract the symbol tar: (`tar -xvf aac-image-minimal-<arch>-android-<apilevel>-dbg.tar.gz`). Extracted symbols will be located here: `builder/deploy/androidx86/opt/AAC/lib/.debug`
3. Rebuild the sample app in Android Studio
4. Launch the app in Android studio by hitting the "Debug 'app'" button on toolbar
5. Select Menu item View -> Tool Windows -> Debug
6. In debug window, switch to "lldb" tab.
7. For debugging each external library, tell lldb where it's symbols are using add-dsym. Example: `(lldb) add-dsym ${AAC_SDK_HOME}/builder/deploy/androidx86/opt/AAC/lib/.debug/libAACECoreEngine.so`
8. Locate the function to debug, with lldb "image lookup." Example: `image lookup -vn EngineImpl::start`
9. If you are debugging on MacOS, then "CompileUnit" (source file) output of lldb "image lookup" will point to file system in docker image. Tell lldb to locate the source on local file system instead with lldb "settings set target.source-map". Example: `(lldb) settings set target.source-map /workdir/build/tmp-android-22/work/core2-32-linux-android/aac-module-core/0.99.0-r0/src/engine/src/ ${AAC_SDK_HOME}/modules/core/engine/src/`. Remember to do this for each module you wish to debug.
10. From lldb "image lookup" output, locate the fully qualified unmangled name of the function ("Function: name = "). Set the breakpoint on the function with lldb "breakpoint set". Example: `breakpoint set --name aace::engine::core::EngineImpl::start`
11. When breakpoint is hit, use Android Studio debugger window to find call stacks, watch variables etc. Also further breakpoint in same file can be set using Android Studio editor method described in "Java Debugging". Of course all the power of lldb is also available for your assistance.

> **Note**: If the LLDB window isn't visible in debug tab, change the debug configuration to dual mode debugging.

## Release Notes<a id="releasenotes"></a>

### v1.6.0 released on 2019-05-16:

#### Enhancements

* The sample app can now generate a default device serial number (DSN) if one is not specified through the configuration file at build time.
  
#### Known Issues

* Music playback may continue after logging out from the app.
* The `MediaPlayer.prepare` method implementation in `MediaPlayerHandler` blocks returning until it reads the full audio attachment from the Engine on the caller's thread. It should not do this and instead should return quickly and read the attachment on a separate thread.
* The MACCAndroidClient does not rediscover Media App Command and Control (MACC) compliant apps if they are unresponsive after being idle for a long period (around 30 minutes).
* Display card rendering is not adaptable to a variety of screen sizes.
* When authenticated with CBL, if network connectivity is lost while refreshing the access token, the Sample App does not automatically attempt to refresh the token when connectivity is restored, and the GUI incorrectly displays the "log in" button even though a refresh token is present. Restoring an authorized state requires either restarting the app or following the log in steps again with network connectivity.
* Particular sections of the Flash Briefing do not resume from a paused state properly. Attempting to play after pause in some cases may restart the media playback.
* The app does not implement managing inter-app audio focus, so other apps will not recognize its audio playback appropriately.
* Alexa dialog playback may stop abruptly when switching between Wi-Fi and mobile data.
* The app disconnects from AVS after remaining idle for some time and takes a while to reconnect.
