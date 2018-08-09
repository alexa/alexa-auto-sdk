## Android Sample Application

### Overview

The Android Sample App demonstrates a basic platform implementation of Alexa Auto SDK integration. It provides an example of creating and configuring an instance of the Engine, overriding the default implementation of each Alexa Auto Platform Interface, and registering those custom interface handlers with the Engine. It includes two example implementations of authorizing with AVS via Login with Amazon (LWA),
detailed logs for interactions with the Alexa Auto SDK and developer convenience features for viewing those logs in the application, and UI elements relevant to each Platform Interface implementation. The purpose of the Android Sample App is to provide developers with useful example code to facilitate integration with the Alexa Auto SDK.

Click [here](#androidsampleapprelnote) to read the release notes for this sample app.

### Setup

#### Register a product

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

#### Include Alexa Auto SDK dependencies
If you have not already done so, you must build the Android specific binaries for the Alexa Auto SDK library project to link to.

See the [Alexa Auto SDK Builder](../../builder/README.md) instructions to build the Alexa Auto SDK binaries for your Android target.

#### Enabling device capabilities

In order to use the certain Alexa Auto SDK functionality, your product needs to be whitelisted by Amazon. Copy the product's **Amazon ID** from the Developer Console and follow the whitelisting directions on the [Need Help?](../../NEED_HELP.md) page.

#### Configure the project in Android Studio

1. Launch [Android Studio](https://developer.android.com/studio/index.html) and select "Open an existing Android Studio project".
2. Open the `${AAC_SDK_HOME}/samples/android` folder and click the **Open** button. (Tested with Android Studio version 3.x)
3. [Include the Alexa Auto SDK dependencies](#include-Alexa-Auto-SDK-dependencies) and select **Build > Refresh Linked C++ Projects** in Android Studio.
4. Read about [authorization with AVS in the Sample App](#authorization-with-avs-in-the-sample-app) and follow further setup instructions, if necessary.
5. Populate the app_config.json file in the assets directory with the Client ID, Client Secret, and Product ID for your product. Product DSN may be any unique identifier. The contents of this file are required for configuring the Engine as well as authorizing with Code-Based Linking.

### Running the Sample App
Use Android Studio to install and run the Sample App on your target device.
> **NOTE**: The Alexa Auto SDK supports Android API level 22+.

### Understanding the Sample App
The Sample App provides an example of how to create and configure an instance of the Engine, extend the Alexa Auto SDK Platform Interfaces, and register the interface implementations with the Engine. The Platform Interface implementations are located in the `impl/` folder of the `com.amazon.sampleapp` directory with the `Handler` postfix. These classes extend the JNI wrapper classes, which mirror the Alexa Auto C++ API. You can read more about these interfaces in the following documentation:

- [Core Module](../../platforms/android/CORE.md)
- [Alexa Module](../../platforms/android/ALEXA.md)
- [Navigation Module](../../platforms/android/NAVIGATION.md)
- [Phone Control Module](../../platforms/android/PHONECONTROL.md)

The Sample App GUI consists of a menu bar and a log console. The expandable menu icon in the menu bar opens an options menu to the right of the screen that contains GUI elements relevant to the Platform Interface implementations as well as the authentication UI. Toggle the login method selector to switch between authentication with LWA Code-Based Linking and browser Login with Amazon. Interacting with Alexa and the Engine requires successful authentication with AVS. You can log out using the **Log Out** button in the options menu, which will clear the saved refresh token.

The microphone icon on the menu bar may be used to initiate a speech interaction with Alexa, either via tap-to-talk (press and release) or hold-to-talk (press and hold). You may also initiate an interaction with the Alexa wake word, and the Sample App provides a switch in the options menu to enable or disable wake word when supported.

Initiate various interactions with Alexa and explore the options menu features and the log console to better understand the Alexa Auto SDK functionality. The logger implementation tags log messages in the following way:

- `[AVS]` refers to log messages from the AVS Device SDK.
- `[AAC]` refers to log messages from the Alexa Auto SDK.
- `[CLI]` refers to log messages from the Sample App itself.

> **NOTE**:
Some Alexa interactions will return data for rendering a [Display Card](https://alexa.design/dev-display-cards) for visual feedback. Card rendering in the Sample App is an example of parsing the payload of rendering calls to the TemplateRuntime Platform Interface. The Sample App implementation of these cards is not meant as a UI design guideline or requirement.


### Authorization with AVS in the Sample App

To access the Alexa Voice Service (AVS) it is required that your product acquire [Login with Amazon (LWA)](https://developer.amazon.com/login-with-amazon) access tokens. The Android Sample App demonstrates two methods to acquire such tokens:

- **Code-Based Linking (CBL)**: The user is provided with a short alphanumeric code and a URL in which to enter the code on any web browser-enabled device. [Read more about CBL](https://developer.amazon.com/docs/alexa-voice-service/code-based-linking-other-platforms.html), if desired. The Sample App implementation of CBL requires no further setup. It includes an example of optionally fetching user profile information, which requires you[ add your company privacy policy URL and a logo image to your security profile](#register-security-profile-with-login-with-amazon).

- Using the **Login with Amazon SDK**: The Sample App implements a usage of the Login with Amazon SDK for Android that launches a web brower on the device and prompts the user to log in with their Amazon account. Using this method to authorize in the Sample App requires additional setup:
	- [Add the LWA library to the project](#getting-the-lwa-library)
	- [Register Security Profile with Login With Amazon](#register-security-profile-with-login-with-amazon)
	- [Get an API Key](#generating-the-api-key)

> **NOTE**: If you do not wish to use the LWA library method for authentication in the Sample App, no further reading is required for this section.

#### Getting the LWA library

Download and expand the [Amazon Developer SDK for Android](https://developer.amazon.com/sdk-download) and add the library to your Android Studio project:

1. Navigate to the `Login With Amazon` folder
2. Add library `login-with-amazon-sdk.jar` to the Android Studio project (In the `AAC_SDK_PATH/samples/android/` folder)
3. Ensure the library is added as a dependency to the `app` module in the build.gradle. (If it is not automatically linked,  right-click and select **"Add As Library..."**)

#### Register Security Profile with Login With Amazon

In the [Amazon Developer portal](https://developer.amazon.com), go to **APPS & SERVICES > Login With Amazon**.
Here you will be able to register the security profile you associated with your product. You will need to provide your company privacy policy URL and a logo image.

#### Generating the API key

You will need an API key to get started. The API key is generated when you associate your Android Sample App with a Security Profile in the Developer Portal. You need to add the Sample App package identifier and Android Studio's debug runtime signature to the Security Profile for your product according to the following instructions:

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
		export JAVA_HOME=`/usr/libexec/java_home -v 1.8
		```

3. Return to the Developer portal and finish filling in the form.
4. Click the **Generate Key** button.
5. Copy the API key and paste it into the `api_key.txt` file in the Android Studio project according to [Add Your API Key to Your Project](https://alexa.design/dev-create-lwa-project).

See [Register for Login With Amazon](http://alexa.design/dev-lwa-register) for more details about registering with LWA.

### Debugging notes

#### Debugging the app
Use Android Studio and hit the "Debug 'app'" button on toolbar. This will launch the app on target device and attach the app with Android Studio debugger.

#### Debugging Java code
Debugging Java code is straight forward. Open the file and scroll to the method which you wish to debug. Click on to the vertical gray bar on the left of the source editor. This will set the breakpoint on the source line. Use Android Studio UI interface to navigate the stack trace, watch variables etc., when breakpoint is hit

#### Debugging C++ code
For debugging the Alexa Auto SDK C++ library, which is part of the Android Studio sample project, debugging instructions are same as Java code. The Alexa Auto Android SDK platform library is compiled and linked by Android Studio so no special instructions are required. Similarly for more C++ projects in Android Studio, debugging is seamless.

For debugging all external libraries which are not built by Android Studio, follow these instructions (At present, All the modules except the platform/android and sample/android are built outside Android Studio)

PreRequisite: Install LLDB package in Android Studio (LLDB component can be found in SDK Manager -> SDK Tools tab)

1. Use builder script to generate a build with debug symbols (Reference: builder/README.md, look for Running Builder -> `options`). For example, to build Android x86 build with debugging symbols use: `builder/build.sh -t androidx86 -g`
2. Debug symbols (For android, symbols are packaged in so file) can be located at: `builder/deploy/android<arch>/aac-image-minimal-<arch>-android-<apilevel>-dbg.tar.gz`
3. Extract the symbol tar: (`tar -xvf aac-image-minimal-<arch>-android-<apilevel>-dbg.tar.gz`). Extracted symbols will be located here: `builder/deploy/androidx86/opt/AAC/lib/.debug`
3. Rebuild the sample app in Android Studio
4. Launch the app in Android studio by hitting the "Debug 'app'" button on toolbar
5. Select Menu item View -> Tool Windows -> Debug
6. In debug window, switch to "lldb" tab. Cheers to Android Studio for integrating lldb debugger!
7. For debugging each external library, tell lldb where it's symbols are using add-dsym. Example: `(lldb) add-dsym ${AAC_SDK_HOME}/builder/deploy/androidx86/opt/AAC/lib/.debug/libAACECoreEngine.so`
8. Locate the function to debug, with lldb "image lookup". Example: `image lookup -vn EngineImpl::start`
9. If you are debugging on MacOS, then "CompileUnit" (source file) output of lldb "image lookup" will point to file system in docker image. Tell lldb to locate the source on local file system instead with lldb "settings set target.source-map". Example: `(lldb) settings set target.source-map /workdir/build/tmp-android-22/work/core2-32-linux-android/aac-module-core/0.99.0-r0/src/engine/src/ ${AAC_SDK_HOME}/modules/core/engine/src/`. Remember to do this for each module you wish to debug.
10. From lldb "image lookup" output, locate the fully qualified unmangled name of the function ("Function: name = "). Set the breakpoint on the function with lldb "breakpoint set". Example: `breakpoint set --name aace::engine::core::EngineImpl::start`
11. When breakpoint is hit, use Android Studio debugger window to find call stacks, watch variables etc. Also further breakpoint in same file can be set using Android Studio editor method described in "Java Debugging". Of course all the power of lldb is also available for your assistance.

Note: If LLDB window isn't visible in debug tab, change the debug configuration to pick dual mode debugging (in place of default auto)

## Android Sample App Release Notes<a id="androidsampleapprelnote"></a>

### Resolved Issues

* The issue related to music playing after saying "stop" for music playback then saying "flash briefing" is fixed. The flash briefing response plays as expected.
* The Login with Amazon browser implementation now refreshes the auth token after one hour.
* Examples were added about how to fetch user profiles for LWA with browser and CBL.

### Known Issues

* Display card rendering is not adaptable to a variety of screen sizes.
* When authenticated with CBL, if network connectivity is lost while refreshing the access token, the Sample App does not automatically attempt to refresh the token when connectivity is restored, and the GUI incorrectly displays the "log in" button even though a refresh token is present. Restoring an authorized state requires either restarting the app or following the log in steps again with network connectivity.
* Particular sections of the Flash Briefing do not resume from a paused state properly. Attempting to play after pause in some cases may restart the media playback.
* The Sample App does not implement managing inter-app audio focus, so other apps will not recognize its audio playback appropriately.
* Alexa dialog playback may stop abruptly when switching between wifi and mobile data.
* The `PhoneCallController` UI is limited and does not provide a way to enter a phone number and generate a unique call identifier to initiate a call outside the scope of an Alexa interaction.
* The Sample App may not run on some emulators, such as Nexus 6 API 25 and ARM 64-bit.
