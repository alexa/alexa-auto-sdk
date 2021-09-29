# AACS Sample App

The AACS Sample App is an Android application that runs on your vehicle's head unit. It demonstrates how an application uses Alexa Auto Client Service (AACS), an Alexa Auto SDK feature that speeds up Alexa integration for in-vehicle infotainment (IVI). The app enables the user to select Alexa as an alternative to another voice assistant and configure the way the user interacts with Alexa. The app also allows Alexa and another application to run simultaneously.

This document provides conceptual information about the purpose and features of the AACS Sample App. It provides the steps for building the app and setting up Alexa. It also describes the user experience when the user interacts with Alexa when the app is running.

**Note:** The out-of-box experience and permission screens in the AACS Sample App are based on preliminary drafts of the automotive HMI guidelines. These screens will be updated when the HMI guidelines are finalized. 

<!-- omit in toc -->
## Table of Contents
- [AACS Sample App Features](#aacs-sample-app-features)
- [AACS Sample App Architecture](#aacs-sample-app-architecture)
- [Prerequisites](#prerequisites)
  - [Requirements for Using AACS Sample App](#requirements-for-using-aacs-sample-app)
  - [Requirements for Using AACS Sample App with Preview Mode](#requirements-for-using-aacs-sample-app-with-preview-mode)
  - [Requirements for Using AACS Sample App with APL](#requirements-for-using-aacs-sample-app-with-apl)
  - [Requirements for Building AACS Sample App](#requirements-for-building-aacs-sample-app)
  - [Requirements for Using Optional Features](#requirements-for-using-optional-features)
  - [Build AACS Sample App using AACS AAR](#build-aacs-sample-app-using-aacs-aar)
- [About App Components](#about-app-components)
- [Building the AACS Sample App](#building-the-aacs-sample-app)
  - [Cloning the Auto SDK Repository](#cloning-the-auto-sdk-repository)
  - [Editing the Configuration File](#editing-the-configuration-file)
  - [Including Build Dependency (AAR)](#including-build-dependency-aar)
  - [Building and Signing the AACS Sample App APK](#building-and-signing-the-aacs-sample-app-apk)
- [Alexa Setup](#alexa-setup)
  - [Language Selection](#language-selection)
  - [Starting Alexa on the Introduction Screen](#starting-alexa-on-the-introduction-screen)
  - [Starting Authorization](#starting-authorization)
  - [Alexa Configuration for Logged-in Users](#alexa-configuration-for-logged-in-users)
- [Using the Alexa Menu](#using-the-alexa-menu)
  - [Alexa Menu for Preview Mode Users](#alexa-menu-for-preview-mode-users)
  - [Alexa Menu for Signed-in Users](#alexa-menu-for-signed-in-users)
  - [Alexa Menu Options](#alexa-menu-options)
- [Using the AACS Sample App](#using-the-aacs-sample-app)
  - [Selecting Alexa as the Assistant](#selecting-alexa-as-the-assistant)
  - [Using Alexa Custom Assistant Module Library for Animation](#using-alexa-custom-assistant-module-library-for-animation)
  - [Using the AACS Sample App for Media Player](#using-the-aacs-sample-app-for-media-player)
- [Known Issues](#known-issues)

## AACS Sample App Features
The following list describes the AACS Sample App features:
* The app is built on all default Auto SDK modules. All core capabilities are enabled, and the app has implemented full-stack support for the Media Player, Voice Chrome, and AmazonLite Wake Word capabilities. The app also supports single waypoint navigation, canceling existing navigation, and showing local search views.
* On the Android Automotive platform, the app provides multimodal support when displaying the media player. For example, the app accepts both voice input and touch-screen input when the user tries to control the player.
* The app supports template runtime views. In addition to a voice response, a display card containing relevant information based on the user's utterance is temporarily overlaid on the screen. This feature is currently supported for utterances related to local search and weather. 

>**Note:** Amazon recommends that you familiarize yourself with AACS by reading the [AACS README](../../../platforms/android/alexa-auto-client-service/README.md).
  
## AACS Sample App Architecture
The following diagram illustrates the AACS Sample App Architecture.

<p align="center">
<img src="./assets/AACSSampleAppArch.png"/>
</p>

## Prerequisites
You must meet the prerequisites described in this section before you can run the AACS Sample App.

### Requirements for Using AACS Sample App
The following list describes the requirements for the AACS Sample App:

* The app can only run on an Android device.
* The app requires AACS to be running. You can obtain the AACS APK according to the instructions in the [AACS README](../../../platforms/android/alexa-auto-client-service/README.md).
* The app requires the Voice Chrome extension.

The app is optimized for and tested with the Android Automotive operating system. It is tested with Android API level 28.

### Requirements for Using AACS Sample App with Preview Mode
Preview Mode is an Alexa feature that gives users a restricted set of Alexa features without requiring a login to the Amazon account. To allow users to use the app with Preview Mode, obtain an app component `alexa-auto-preview-mode-util` from Amazon with the help of your Solutions Architect (SA) or Partner Manager. Follow the instructions included to build the app component before you proceed.

### Requirements for Using AACS Sample App with APL
Alexa has a visual design framework called Alexa Presentation Language (APL), which allows you to build interactive voice and visual experiences across the device landscape. To allow users to use the app with APL, follow the instructions in [Alexa Auto APL Renderer README](../../../platforms/android/app-components/alexa-auto-apl-renderer/README.md) to configure and build the app.

  >**Note:** If you do not want to use APL, please do not include Alexa Auto APL Renderer component in [app-component](../../../platforms/android/app-components/) folder.

### Requirements for Building AACS Sample App
The requirements for building the app depends on whether you use the command line interface (CLI) or Android Studio:

* CLI: You need Gradle to build the AACS Sample App. The tested Gradle version is 6.5.

* Android Studio: The Android Studio version must be 4.0 or later. Make sure that your Gradle version and Android Studio are compatible. See the [Android Gradle Plugin Release Notes](https://developer.android.com/studio/releases/gradle-plugin#updating-gradle) for information about matching Android Studio versions to Gradle versions.

### Requirements for Using Optional Features
To use optional features delivered by Auto SDK extensions, contact your Solutions Architect or Partner Manager. The following list describes the extensions that you can use with the AACS Sample App:

* Alexa Custom Assistant extension gives the user the option of using a custom voice assistant when running the AACS Sample App.

* Bluetooth extension and Mobile Authorization extension enable the user to log in to Amazon through the Alexa mobile app on the user's phone, without requiring the user to enter the CBL code.

### Build AACS Sample App using AACS AAR
You can run your application and AACS as a single application. By default, the AACS Sample App is built to use AACS as a separate APK. To include AACS as an AAR in the AACS Sample App, follow these steps:

1. Follow the instructions in [Builder README](../../../builder/README.md) to generate the AACS AAR and place the AAR in the `builder/deploy/aacs-aar/` folder.
2. Create a 'libs' directory in `samples/android-aacs-sample-app/alexa-auto-app`.
3. Copy all the built AAR dependencies (excluding aacscommonutils-release.aar, aacsipc-release.aar and aacsconstants-release.aar) from `builder/deploy/aar/` to the `samples/android-aacs-sample-app/alexa-auto-app/libs` directory. If You have received the libraries from your Amazon Solutions Architect or Partner Manager, copy all the libraries in the `samples/android-aacs-sample-app/alexa-auto-app/libs` directory.
  
  >**Note:** Do not copy the AACS AAR file.
  
4. Open Android Studio.
5. Click the `Open an Existing Project` option and select `samples/android-aacs-sample-app` folder.
6. Click `Open Module Settings` for `alexa-auto-app`.
7. Add a new module by importing AACS AAR from `builder/deploy/aacs-aar/` or the AAR downloaded from your Amazon Solutions Architect or Partner Manager.
8. For `alexa-auto-app`, add a module dependency on the newly created AACS AAR module.
9. In `samples/android-aacs-sample-app/alexa-auto-app/build.gradle`, add the following lines in `android {}` to avoid duplication of native libraries after generating the build.

```
// Prevent duplicate .so libraries
packagingOptions {
    pickFirst 'lib/armeabi-v7a/libc++_shared.so'
    pickFirst 'lib/arm64-v8a/libc++_shared.so'
    pickFirst 'lib/x86/libc++_shared.so'
    pickFirst 'lib/x86_64/libc++_shared.so'
    doNotStrip '**/*'
}
```
  >**Note:** `AACSConstants.AACS_PACKAGE_NAME` is deprecated and will be removed from future Alexa Auto SDK versions. Use `AACSConstants.getAACSPackageName(Context)` instead.
  
  >**Note:** Avoid hardcoding `com.amazon.alexaautoclientservice` in the code. It may result in AACS failures.

  >**Note:** To build the app with the `gradle` command, use `export ANDROID_NDK_HOME=<PATH TO android-ndk-r20b>` to avoid NDK mismatch issues.
  
  >**Note:** If you use the AACS Sample App with AACS as an AAR and install it as a system privileged app, in order to work with offline cases, you need to compose an [SELinux policy] (https://source.android.com/security/selinux/device-policy) to allow LVC permissive for outside socket file changes. For development, you can set the device to be under permissive mode using `adb shell setenforce 0`.


## About App Components

The AACS Sample App APK contains several app components, each of which consists of the compiled source code or resources used by the app to provide the UI layout, communicate with AACS, and so on. The following list shows the app components used by the AACS Sample App: 

* alexa-auto-apis
* alexa-auto-apl-renderer
* alexa-auto-apps-common-ui
* alexa-auto-apps-common-util
* alexa-auto-comms-ui
* alexa-auto-lwa-auth
* alexa-auto-media-player
* alexa-auto-navigation
* alexa-auto-settings
* alexa-auto-setup
* alexa-auto-templateruntime-renderer
* alexa-auto-voice-interaction
   
See the respective README file about the purpose of each component.

## Building the AACS Sample App
To build the AACS Sample App, follow these major steps:
1) Clone the Auto SDK repository.
2) Edit the configuration information for your device.
3) Include the build dependencies.
4) Build the AACS Sample App.

### Cloning the Auto SDK Repository
Follow these steps to clone the Auto SDK repository:
1) Create your project directory (if you do not already have one):

```shell
    mkdir ~/Projects
    cd ~/Projects
```

2) Clone the alexa-auto-sdk repository into your project directory:

```shell
    git clone https://github.com/alexa/alexa-auto-sdk.git
    cd alexa-auto-sdk
```

The Projects directory contains the Auto SDK directory structure with the `android-aacs-sample-app` directory and `app-components` directory, as shown in the following Auto SDK directory structure:

~~~
alexa-auto-sdk
  ├── ...
  ├── assets
  ├── builder
  ├── extensions
  ├── modules
  ├── platforms
  │       └── android       
  │             ├── alexa-auto-client-service
  │             ├── app-components
  |             |     └── alexa-auto-apis
  │             |     └── alexa-auto-apl-renderer
  │             |     └── alexa-auto-apps-common-ui
  │             |     └── alexa-auto-apps-common-util
  │             |     └── alexa-auto-comms-ui
  |             |     └── alexa-auto-lwa-auth
  |             |     └── alexa-auto-media-player
  |             |     └── alexa-auto-navigation
  │             |     └── alexa-auto-settings
  │             |     └── alexa-auto-setup
  │             |     └── alexa-auto-templateruntime-renderer
  │             |     └── alexa-auto-voice-interaction
  │             └── modules
  ├── samples
  │   ├── android
  │   ├── android-aacs-sample-app
  │   │    ├── alexa-auto-app
~~~

### Editing the Configuration File
For Alexa Voice Service (AVS) to authenticate your device profile, specify the configuration information in this file:

[alexa-auto-sdk/samples/android-aacs-sample-app/alexa-auto-app/src/main/assets/config/aacs_config.json](src/main/assets/config/aacs_config.json)

The following list describes the required information for `aacs.alexa.deviceInfo`:

* For `clientId`, specify the Client ID that you generated when you [set up your security profile](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#set-up-your-security-profile) for your development device.
* For `productId`, specify the Product ID that you entered when you [filled in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information) for your development device.

    >**Note:** `clientId` and `productId` must correspond to a development device profile that you created as an **automotive** product by selecting the `Automotive` product category when you [filled in the product information](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product.html#fill-in-product-information).  
* For `deviceSerialNumber`, specify the serial number of your device.
* For `manufacturerName`, specify the name of the device manufacturer.
* For `description`, specify a description of your device.

### Including Build Dependency (AAR)
The AACS Sample App APK requires the Auto SDK Voice Chrome extension (autovoicechrome.aar) as a dependency. Follow these steps to include the AAR:

1. Create the following directory:

   `alexa-auto-sdk/platforms/android/app-components/alexa-auto-voice-interaction/libs`
2. Copy the AAR into the directory.
  
After including the dependency, you can build the AACS Sample App APK either on the CLI or by using Android Studio.

### Building and Signing the AACS Sample App APK 
You can use the command line interface (CLI) or the Android Studio to build and sign the AACS Sample App APK.

#### Using the CLI  
Follow these steps to build the AACS Sample App APK:
1) Enter the following command to change the directory:

~~~
    cd ~/Projects/alexa-auto-sdk/samples/android-aacs-sample-app/
~~~   

2) Enter the following command to start the build:

>**Note:** If you do not want to use APL, please remove alexa-auto-apl-renderer component in [app-component](../../../platforms/android/app-components/) folder, before running the build command.

~~~
    ./gradlew assembleRelease
~~~

>**Note:** To enable the debug log during the build, use `assembleDebug` in the `gradlew` command.

The `gradlew` command creates the unsigned APK, which is located in the following directory:

alexa-auto-app/build/outputs/apk/release/alexa-auto-app_release_1.0.apk

The `gradlew` command also creates each app component's AAR, which is located in each component's build output directory. For example, the Alexa Auto Media Player AAR is in the following directory:

../../platforms/android/app-components/alexa-auto-media-player/build/outputs/aar/alexa-auto-media-player_release.aar

#### Using the CLI to Sign the APK

The procedure for signing the APK requires these commands:
* `zipalign` is included in the Android SDK Build Tools. On a Mac, it is usually located in this directory:
    
  ~/Library/Android/sdk/build-tools/<Android_SDK_Build_Tools_version>/zipalign
* `apksigner` is in Android SDK Build Tools 24.0.3 or higher. On a Mac, it is usually in the following directory:
    
  ~/Library/Android/sdk/build-tools/<Android_SDK_Build_Tools_version>/apksigner

You can include the build tools in your path so that you can run the commands from any directory. The following example is for Android SDK Build Tools version 29.0.2:
  
   `echo "export PATH=\$PATH:~/Library/Android/sdk/build-tools/29.0.2/" ~/.bash_profile && . ~/.bash_profile`

>**Important!** Sign the AACS Sample App APK with the same signing key as the one for the AACS APK.

To sign the APK, follow these steps:

1. Create a custom keystore using the following command, or skip to the next step and use an existing keystore:
    
    `keytool -genkey -v -keystore <keystore_name>.keystore -alias <alias> -keyalg RSA -keysize 2048 -validity 10000`

2. Enter the following command to change to the directory where the APK is:
   
   `cd alexa-auto-app/build/outputs/apk/release`

3. Enter the following command to optimize the APK files:
   
    `zipalign -v -p 4 alexa-auto-app_release_1.0.apk alexa-auto-app_unsigned_release_1.0-aligned.apk`

4. Enter the following commands to sign the APK by using your keystore:
   
    `apksigner sign --ks <path_to_keystore>/<keystore_name>.keystore  --ks-pass pass:<passphrase> --out alexa-auto-app_signed_release_1.0.apk alexa-auto-app_unsigned_release_1.0-aligned.apk`

5. When prompted, enter the passphrase that you set when you created the keystore. 

#### Using Android Studio

> **Note:** These instructions assume that you have edited the [configuration files](#editing-the-configuration-file).

1. Launch <a href=https://developer.android.com/studio/index.html>Android Studio</a> and select Open an existing Android Studio project.
2. Open the folder containing the APK. For example, open the `alexa-auto-app/samples/android-aacs-sample-app` folder.
3. Click the <strong>Open</strong> button.
4. Under **Build Variants**, select **release**.

> **Note**: Android Studio builds and signs the APK.

## Alexa Setup
This section describes what the user must do to set up Alexa, which determines the user experience when the user interacts with Alexa. The information here supersedes the information in the [Setup documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/setup.html).

>**Note:** The Alexa setup process is different for users of Preview Mode and users who signed in.

The setup procedure includes the following steps:
1) Select Alexa's language (if the language used for IVI is not supported by Alexa).
2) Accept the use of Alexa in Preview Mode or perform a user login.
3) Configure initial Alexa settings (for logged-in users).

>**Note:** To complete Alexa setup, the user must have internet connection. 

### Language Selection
The Alexa setup starts with selecting a language to be used by Alexa. The workflow for language selection depends on the language used by the IVI:

* If the language used for the IVI is supported by Alexa, there is no need to select a language because the app sets Alexa's language to match the IVI language. If the user prefers to use a different language for Alexa, the user can change it at a later time through the Alexa menu.

* If the language used for the IVI is not supported by Alexa, a menu is displayed for language selection. After the selection, the locale is changed according to the language selected. However, the setup workflow displays text in en-US.

  For a list of languages supported by Alexa, see the [Alexa Voice Service documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/system.html#locales).

### Starting Alexa on the Sign in Screen
The Sign in screen is displayed after the user selects the language. It displays the following contents depending on whether the AACS Sample App is used with Preview Mode:

* With Preview Mode: The screen shows two buttons. The user can click on the `TRY ALEXA` button to connect to Alexa in Preview mode, or on the `SIGN-IN` button to sign in using their Amazon account credentials.
* Without Preview Mode: The screen shows the `SIGN-IN` button only.

### Starting Authorization

The exact authorization workflow depends on whether the AACS Sample App is used with Preview Mode. If so, the Alexa Voice Service (AVS) access token is retrieved without a user login. Otherwise, the user uses Login With Amazon (LWA) to gain access to Alexa. The Auto SDK can use various authorization methods, such as Code-Based Linking (CBL) and Mobile Authorization, to retrieve the access token. 

#### Authorization With Preview Mode
On the Sign-in screen, the user can click `TRY ALEXA`, which means that the user will access Alexa through Preview Mode. The Preview Mode consent screen is then displayed wherein the user can review Amazon’s Conditions of Use, Alexa & Alexa Device terms and Privacy policy. After clicking on the `AGREE & ENABLE ALEXA` button, the Success screen is displayed providing sample utterances for the user to try.

#### Authorization Without Preview Mode
The user can log in to Amazon to access the complete set of Alexa features. How the user logs in depends on whether the Auto SDK is running with the Bluetooth extension and Mobile Authorization extension. 

Without the LWP feature, the user sees the Sign-in screen showing the URL and a code (in the form of a string and a QR code), which the user uses to finish the CBL authorization. After authorization, the user's Amazon account is linked to the vehicle.

With the LWP feature, when the user's phone is connected to the head unit via Bluetooth, the user sees a notification on the phone. After the user accepts the notification, the user is authenticated with the Amazon account information that the Alexa mobile app uses for Amazon login. For more information about LWP, see the LWP extension README.

### Alexa Configuration for Logged-in Users
If the user is logged in, the user is prompted to further configure Alexa. The user is shown the location consent screen wherein they have the option to either enable location sharing or skip it. Next, if a Bluetooth-connected phone is detected, the app prompts the user to give permission for Alexa to access contacts.

After this configuration step, the app displays the Success screen as confirmation that the user can start using Alexa. If your device is allow-listed by Amazon for supporting device setup, the user hears a first-time conversation from Alexa, which provides the on-boarding experience.

## Using the Alexa Menu
The AACS Sample App provides an Alexa menu through which the user can change any settings configured during the setup process. For options with an On or Off value, the menu provides a toggle button. The Alexa menu is different for users of Preview Mode and users who signed in.

>**Note:** When developing your Android app, you may add options to the Alexa menu. Add such option at the end of the Alexa menu.

### Alexa Menu for Preview Mode Users
In Preview Mode, the Alexa menu is organized as follows:

* Sign in
* Alexa Hands-Free
* Location sharing
* Alexa's language
* Disable Alexa

### Alexa Menu for Signed-in Users
For signed-in users, the Alexa menu is organized as follows:

* Alexa Hands-Free
* Location sharing
* Communication
    * Device name
        * Contacts
* Alexa's language
* Sign out
  
### Alexa Menu Options
The following table describes each Alexa menu option: 

| Option | Description | Possible values (Default) 
|-|-|-|
|Sign in | (For Preview Mode users only) It allows the user to sign in with CBL or LWP. |
|Alexa Hands-Free | If it is enabled, the user can say "Alexa" to invoke Alexa. If it is disabled, the user can still use PTT or TTT to invoke Alexa. | On, Off (On)
|Location sharing | If it is enabled, the user can use location-based utterances like "Alexa, show me some coffee shops near me". If it is disabled, Alexa will not have access to the device's location and will expect the user to explicitly provide their location as a follow up voice utterance in order to respond to location-based utterances. | On, Off (value specified during Alexa setup)
|Communication | Name of the device on which the app runs (e.g., Sam's iPhone). |  
|Communication > Contacts | If it is enabled, contacts are uploaded from the phone to Alexa, and the user can use Alexa to call or receive a call from a contact. | On, Off (value specified during Alexa setup)
|Alexa's language | The language used by Alexa when responding to your request. | Languages supported by Alexa (language specified during Alexa setup)
|Sign out | (For signed-in users only) Button for the user to sign out of Alexa. It displays a confirmation message before the user is signed out. |
|Disable Alexa | (For Preview Mode users only) It displays a screen with a DISABLE button, which stops Alexa from being available in the vehicle. |

## Using the AACS Sample App
This section describes the user experience after you deploy the AACS Sample App on an Android device.

### Selecting Alexa as the Assistant
While Google Assistant is usually the default voice assistant on an Android device, when the AACS Sample App is running, the user has the option of selecting Alexa as the assistant. The app, by running Alexa Voice Interaction as an Android service, demonstrates how a user can use the Android Automotive System UI to select Alexa as the assistant as follows:

1) Go to the device's Settings.
2) Go to `Apps & notifications` > `Default apps` > `Assist & voice input`. Then select Alexa.

>**Note:** After selecting Alexa as the assistant, if the Auto SDK is built with the Alexa Custom Assistant extension, the user can invoke either Alexa or a custom voice assistant (e.g., Brandon).

### Using Alexa Custom Assistant Module Library for Animation
If the Alexa Custom Assistant extension is installed, you can use the Alexa Custom Assistant Module Library to display custom animation when user is interacting with the custom assistant. For more information about how to enable custom animation, see the Alexa Custom Assistant extension README.

### Using the AACS Sample App for Media Player
In addition to letting the user choose Alexa as the voice assistant, the AACS Sample app enables the user to start and control the media player.

Alexa integrates with most music providers in cloud. After the user launches the app and selects Alexa as the assistant, the user can say, for example, "Alexa, play the station [station call sign] on iHeartRadio."

The app displays the user interface (UI) that comes with Android Automotive. The same UI is presented for all media providers, such as iHeartRadio and Amazon Music. However, the exact UI elements depend on the provider. For example, when the app plays music from iHeartRadio, it does not include a rewind button to go back. When the app plays music from Amazon Music, the rewind button is present. In addition, the user can say, "Go back 30 seconds," to go back.

The user can also use the app to listen to audio books. For example, if the user says, "Alexa, play the start of lord of the rings book one," Alexa starts playing the audio book from Audible. The UI displayed by the app includes the buttons for skipping 30 seconds backward or forward.

## Known Issues
* Android Emulator on macOS has poor audio quality, which would cause the Alexa Text-to-Speech (TTS) output to be unusable.
* In the Android Automotive Emulator, the Push-to-Talk button on the system navigation bar does not integrate with the Android Voice Interaction module properly. It is hard coded to invoke Google Assistant instead. Therefore, even if the user tries to switch from the default voice assistant to Alexa, the Push-to-Talk button on the system navigation bar still invokes the Google Assistant.
* AACS Sample App does not handle the phone account query intent for dialing hence it does not initiate actual phone calls with AACS Telephony library, when a mobile phone connects. To make it work, your application needs to handle the intent (see [AACS Telephony README](../../../platforms/android/app-components/alexa-auto-telephony/README.md#dial)) to specify which phone account to use. 
* Sometimes, when the user disables preview mode and tries enabling preview mode again, the "Something went wrong" screen is displayed. This is because the logout process is currently asynchronous and may not complete before the user tries re-enabling preview mode again.
* AACS Sample App uses Android symbol as the app's logo, which is not an approved icon, to get Alexa logo, contact your Solutions Architect or Partner Manager.

Note: Please refer to the component level README files for more information/known issues that relate to the component 