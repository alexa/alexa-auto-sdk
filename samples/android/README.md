## Android Sample Application

### Overview

The AAC Android Sample App shows how to do a basic platform implementation of the AAC SDK APIs. It provides logs and other relevant information when interacting with Alexa on an Android device. The intention is to provide developers with useful example code and show data received from AVS so they can quickly integrate the AAC SDK into their application code.

### Setup

The AAC Android Sample App requires some authentication to authorize an AVS connection.

- Launch [Android Studio](https://developer.android.com/studio/index.html) and choose "Open an existing Android Studio project".
- Open the `${AAC_SDK_HOME}/samples/android` folder and click the **Open** button. (Tested with Android Studio version 3.x)

[Login with Amazon](https://developer.amazon.com/login-with-amazon) (LWA) is a service which allows the user to authenticate with AVS via an Amazon account (and link their account to the app in order to access enabled Alexa skills, purchased media, etc.). The LWA library needs to be set up according to the instructions provided below.

In order to set up LWA, you will need to:

- [Add the LWA library to your project](#getting-the-lwa-library)
- [Create a product and security profile in the AVS Developer Console](#creating-a-product-and-security-profile)
- [Link the AAC Android Sample App to it and get an API key](#generating-the-api-key)
- [Add the API key to the Android Studio project](https://alexa.design/dev-create-lwa-project)

If you have not already done so, you must then build the Android specific binaries for the AAC SDK library project to link to. 

See the [Alexa Auto Core Builder](../../builder/README.md) instructions to build the AAC SDK binaries for your Android target.

After building, the AAC Android Sample App should build successfully in Android Studio. If not, try the steps below in Android Studio. 

1. Select **Build > Refresh Linked C++ Projects**
2. Select **Build > Clean Project**.

#### Getting the LWA library

Start by downloading and expanding the [Amazon Developer SDK for Android](https://developer.amazon.com/sdk-download). Then, install the library into your Android Studio project:

1. Navigate to the Login With Amazon folder.
2. Add library `login-with-amazon-sdk.jar` to the Android Studio project(In the `AAC_SDK_PATH/samples/android/` folder).
3. Make sure it is added as a dependency to the `app` module in the build.gradle.
4. (If it is not being automatically linked) Right-click and select **"Add As Library..."**

#### Registering the app

You will need an API key to get started. The API key is generated when you associate your AAC Android Sample App with a Security Profile from the Developer Portal. Start by registering a new security profile and then add the AAC Sample App package identifier and Android Studio's debug runtime signature to the Security Profile.

> **NOTE**: Security Profiles created according to the LWA instructions currently do not enable access to all available AVS features. In order to explore the maximum available capabilities, create your Security Profile as part of registering an AVS product.

##### Creating a product and security profile

1. Go to the AVS Developer Console, and display the [My Products](https://developer.amazon.com/avs/home.html#/avs/home) view. ( If you already have a product with a security profile, skip to [Generating the API key](#generating-the-api-key))
2. Click **Create Product** and fill in the form as an application.  (This device registration can be useful in certain development workflows, but will not actually be used for this example.)
3. Make note of the **Product ID**.
4. Click **Next**.
5. Click **Create New Profile**.
6. Give it a name. 
7. Agree to the license terms in the Developer Portal and click **Finished**.

>**Note:** The security profile name will be visible to end-users if you implement the Code-Based Linking authentication provision method.

##### Enabling device capabilities

In order to use the AAC Navigation API, your device currently needs to be whitelisted by Amazon.  Copy the product's **Amazon ID** in this view and follow the whitelisting directions on the [Need Help?](../../NEED_HELP.md) page.

1. After agreeing to the license terms, a dialog box will appear allowing you to add capabilities.
2. Click the **capabilities** link.  (If you dismissed the dialog without clicking **capabilities**, you can set them in the [AVS Developer Console's "My Devices" view](https://developer.amazon.com/avs/home.html#/avs/home) by clicking **Manage** for the device you just created and choosing **Capabilities** at the left.)
3. Enable **Named Timers and Reminders** and **Display Cards with Media**.
4. Click Update.

##### Generating the API key

1. Select the **Security Profile > Android/Kindle** settings.
2. Fill in the fields that appear as follows.
	* The **API key name** is any arbitrary name.
	* The **Package ID** must match the app's package identifier (i.e. `com.amazon.sampleapp` by default).
	* The **MD5** and **SHA256** signatures are in this case those of the Android Studio debug runtime, and can be extracted from the Android Studio keystore. By default, these can be found by using Java's `keytool`. See [Using keytool](#using-keytool) below for details.
3. Return to the Developer portal and finish filling in the form.
4. Click the **Generate Key** button.
5. Copy the resulting API key.
6. Paste this into the `api_key.txt` file in the Android Studio project according to [Add Your API Key to Your Project](https://alexa.design/dev-create-lwa-project).

See [Register for Login With Amazon](http://alexa.design/dev-lwa-register) for further details.

### Usage

Use Android Studio to launch the app on your target device. (It must be a physical device; the emulators in Android Studio are not suitable as they do not currently support microphone input.)

> **NOTE**: The Android `minSdkVersion` is 22. The AAC Sample App has only been tested on hardware of API level 23+.

#### Running the app

The AAC Android Sample App UI consists of a menu bar and a log console. The microphone icon at the right end of the menu bar invokes AVS using the tap-to-talk method. The log console provides logs that will be useful for a developer to quickly understand the AAC SDK functionality. The options menu opened from the right, contains LWA functionality, log filter options, and the playback controller.

##### LWA

Under the Options menu (drawer opened from the right), there are settings and a  **Log in with Amazon** button. In order to log in with Amazon, fill in the settings as follows:

1. Set the `PRODUCT_ID` with your **Product ID** from [before](#create-a-product-and-security-profile). (Do not confuse it with the Amazon ID.)
2. Set the `PRODUCT_DSN` as a numeric string of arbitrary value. (i.e. "123456")

The LWA button will open a browser and allow the user to sign in, after which the app will resume. If successful, the token should be saved to the app's memory storage. On the next startup the previous token will be refreshed automatically.

You can log out using the **Log Out** button in the options menu. This will clear the saved refresh token. Alternatively you can clear the app's storage data.

##### Log tags

There should be 3 separate log tags that are shown in this sample.

- `[AVS]` refers to logs being passed from the AVS Device SDK.
- `[AAC]` refers to logs being passed from the AAC SDK.
- `[CLI]` refers to logs coming from the AAC Sample App itself.

##### Display cards

Some directives will return a [Display Card](https://alexa.design/dev-display-cards) for visual feedback, in addition to the logged JSON data. The rendering of the cards is to show an example of how the data can be rendered. It is not meant as a UI design guideline or requirement.

#### AAC Interfaces

Since the AAC Android Sample App uses LWA, an `authDelegate` configuration is not passed (as would be used for file-based configuration and authentication) to the `configure()` method.

The Engine is then started with `start()`. If the LWA token is present it will connect, otherwise it will wait for the `AuthProvider` state to be updated. The `AuthProvider` state can only be updated after the Engine has started with a call to `authStateChanged()`. Activating the tap-to-talk without proper authorization will just log a warning from the app.

Developers who wish to understand how to use the AAC SDK on Android can find examples of all of the interface implementations in the `impl/` folder of the `com.amazon.sampleapp` directory with the `Handler` postfix. These classes extend the JNI wrapper classes, which mirror the AAC C++ API. 

You can read more about the Android platform interfaces in the Android Module API pages:

- [Core Module](../../platforms/android/CORE.md)
- [Alexa Module](../../platforms/android/ALEXA.md)
- [Navigation Module](../../platforms/android/NAVIGATION.md)

#### Other authorization methods

Although only web-based LWA is currently implemented in the AAC Android Sample App, you can implement [Code Based Linking (CBL)](https://developer.amazon.com/docs/alexa-voice-service/code-based-linking-other-platforms.html) for your project. Native Android LWA flow is currently not supported..

#### Using keytool
 Use `keytool` (`${JAVA_HOME}/bin/keytool`) to determine the Android debug runtime signature as in the following example.

```
keytool -list -v -keystore ~/.android/debug.keystore
```
There is no password by default.	

If you are not using the default keystore, view more keystore instructions under [Determining an Android App Signature](https://alexa.design/dev-lwa-register#determining-an-android-app-signature).
> **NOTE**: If you do not see the MD5, you may need to temporarily switch Java versions (set Java home to 1.8) 

