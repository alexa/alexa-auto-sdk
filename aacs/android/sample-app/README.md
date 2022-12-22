# Alexa Auto App

The Alexa Auto App is an Android application that runs on your vehicle's head unit. The app enables the user to select Alexa as an alternative to another voice assistant and configure the way the user interacts with Alexa. The app also allows Alexa and another application to run simultaneously.

This document provides conceptual information about the purpose and features of the Alexa Auto App. It provides the steps for building the app and setting up Alexa. It also describes the user experience when the user interacts with Alexa when the app is running.

## Requirements

* Alexa Auto SDK - Alexa Auto App depends on Auto SDK, and the source code shares the `alexa-auto-sdk` Github project. Clone [`alexa-auto-sdk`](https://github.com/alexa/alexa-auto-sdk) from Github on your development machine
* Conan package manager version 1.44 or higher. - The Alexa Auto App build system integrates Conan into the Android Gradle system as the underlying package manager to build the Auto SDK native libraries
* Python: 3.7 - Alexa Auto app uses Python's standard library support JSON and other protocols.
* Java: 11
* Kotlin Programming language: Version 1.7.20
* Android Build SDK:
    * MinSDK: 27
    * CompileSDK: latest (33)
    * TargetSDK: 27
* Android Tooling:
    * Android Studio (Optional)
    * Android Gradle Plugin: Version (7.3.0)
    * Gradle: latest for the respective AGP (7.5)
* CLI Tools:
    * `zipalign` - `zipalign` is a zip archive alignment tool that helps ensure that all uncompressed files in the archive are aligned relative to the start of the file. 
    * `adb` - Android Debug Bridge (`adb`) is a versatile command-line tool that lets you communicate with a device.
    * `apksigner` - The `apksigner` tool lets you sign APKs and confirm that an APK signature will be verified successfully on all versions of the Android platform supported by that APK.
    * `sdkmanager` - Manages the installation of all other Android CLI tools and SDK versions

> The Alexa Auto App requires hardware-accelerated encryption on your target device. Almost all hardware-based security concepts contain this acceleration. If you see a performance issue running the Alexa Auto app, your device might may not have all mandatory security features.

### Auto SDK extensions

>**Note:** Several private Auto SDK extensions are required to build Alexa Auto App. Work with your Amazon Solution Architect or Partner Manager to access the content at the links.

* [Geolocation](https://developer.amazon.com/alexa/console/avs/preview/resources/details/df3a5c0f-8a81-486e-803b-f4e168afb24e) - The Auto SDK Geolocation enables Alexa Auto App to include consent for geolocation access in the first time user experience (FTUE) flow rather than through the Code-Based Linking (CBL) sign-in flow.
* [Amazonlite](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Amazonlite%20Extension) - The Auto SDK AmazonLite Wake Word extension enables the Alexa wake word in Alexa Auto App.
* [Alexa Comms](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Alexa%20Comms%20Extension) - The Auto SDK Alexa Communication extension enables Alexa-to-Alexa calling capabilities in Alexa Auto App. 

If `{$AUTO_SDK_HOME}` is the root directory of the cloned Auto SDK Github project, unzip the downloaded extensions into the `{$AUTO_SDK_HOME}/extensions/extras/` directory. The file structure of the Auto SDK source tree on your development machine should look similar to the following example:

```
alexa-auto-sdk/
├─ aacs/
│  ├─ android/
│  │  ├─ service/
│  │  ├─ sample-app/
│  │  ├─ app-components/
├─ extensions/
│  ├─ extras/
│  │  ├─ alexacomms/
│  │  ├─ amazonlite/
│  │  ├─ geolocation/
```

### Additional components

>**Note:** The following resources are required but not public. Work with your Amazon Solution Architect or Partner Manager to access the content at the links.

* [APL Resources](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Alexa%20Auto%20SDK%20Alexa%20Presentation%20Language%20module)
    * Move `aplRelease.aar` and `ttsPlayerRelease.aar` to `alexa-auto-apl-renderer/modules/apl-render/src/main/libs/`
    * Unzip `fonts.zip` into `alexa-auto-apl-renderer/modules/apl-render/src/main`
* [Voice Chrome](https://developer.amazon.com/alexa/console/avs/preview/resources/details/Auto%20SDK%20Voice%20Chrome%20Extension)
    * Move `autovoicechrome.aar` to `alexa-auto-voice-interaction/libs`

### AAOS vs AOSP

The following Alexa Auto App [components](https://alexa.github.io/alexa-auto-sdk/docs/android/aacs/app-components/) depend on Android Automotive OS (AAOS) APIs that are not available in standard, non-automotive Android OS variants (AOSP):

* APL
* Car control
* UX Restrictions
  
If the AOSP build variant used in your head unit does not include the AAOS car libraries used by these app components, you are responsible to replace usage of relevant AAOS car libraries with your own solution.

## Build and installation

### Update Alexa Auto App configuration

The Alexa Auto SDK repository comes with a template Alexa Auto App configuration file at `{$AUTO_SDK_HOME}/aacs/android/sample-app/alexa-auto-app/src/main/assets/config/aacs_config.json`. There are several fields to modify to suit your specific platform’s needs prior to building and installing Alexa Auto App.

The following list describes the required information for `aacs.alexa.deviceInfo`:

* For `clientId`, specify the Client ID that you generated when you set up your security profile for your development device.
* For `productId`, specify the Product ID that you entered when you filled in the product information for your development device.

> **Note**: `clientId` and `productId` must correspond to a development device profile that you created as an automotive product by selecting the Automotive product category when you filled in the product information.

* For `deviceSerialNumber`, specify a serial number for the vehicle, unique amongst all vehicles.
* For `manufacturerName`, specify a friendly name for the vehicle manufacturer.
* For `description`, specify a short description of the particular vehicle using this configuration.

See the [AACS Configuration Reference](https://alexa.github.io/alexa-auto-sdk/docs/android/aacs/service/) for details of additional fields.

### Build Alexa Auto App

1. Build the unsigned APK by executing `sample-app/gradlew assembleLocal${Debug|Release}`. All of the core app components and extensions will automatically be included and built into the APK.

2. Align the generated unsigned APK with `zipalign`. For example,

    ```
    zipalign -v -p 4 ${original_unsigned}.apk ${new_aligned}.apk
    ```
3. Sign the aligned APK with `apksigner`. For example,

    ```
    apksigner sign --ks ${keystore} --ks-pass pass:${keystore_pw} --ks-key-alias ${alias} -out ${final_signed}.apk ${new_aligned}.apk
    ```

### Install Alexa Auto App

We strongly recommend installing the Alexa Auto App as a system app.

The following privileged permissions are required to run the app:

```xml 
<privapp-permissions package="com.amazon.alexa.auto.app">
  <permission name="com.amazon.aacscontacts" />
  <permission name="com.amazon.aacstelephony" />
  <permission name="com.amazon.alexaautoclientservice" />
  
  <permission name="android.permission.SET_TIME_ZONE" />
  <permission name="android.permission.READ_PRIVILEGED_PHONE_STATE" />
  <permission name="android.permission.PACKAGE_USAGE_STATS" />
  <permission name="android.permission.CONTROL_INCALL_EXPERIENCE" />
  <permission name="android.permission.BLUETOOTH_PRIVILEGED" />
  <permission name="android.permission.INTERACT_ACROSS_USERS" />
  <permission name="android.permission.CAPTURE_AUDIO_OUTPUT" />
  <permission name="android.permission.WRITE_SECURE_SETTINGS" />
  <permission name="android.permission.MEDIA_CONTENT_CONTROL" />
  
  <permission name="android.car.permission.CONTROL_CAR_EXTERIOR_LIGHTS" />
  <permission name="android.car.permission.CONTROL_CAR_DOORS" />
  <permission name="android.car.permission.CONTROL_CAR_CLIMATE" />
  <permission name="android.car.permission.CONTROL_CAR_SEATS" />
  <permission name="android.car.permission.CAR_ENERGY_PORTS" />
  <permission name="android.car.permission.CONTROL_CAR_MIRRORS" />
  <permission name="android.car.permission.CONTROL_CAR_WINDOWS" />
  <permission name="android.car.permission.CAR_CONTROL_AUDIO_VOLUME" />
  <permission name="android.car.permission.CAR_DISPLAY_IN_CLUSTER" />
  <permission name="android.car.permission.CAR_INSTRUMENT_CLUSTER_CONTROL" />
  <permission name="android.car.permission.CAR_EXTERIOR_LIGHTS" />
  <permission name="android.car.permission.CONTROL_CAR_INTERIOR_LIGHTS" />
  <permission name="android.car.permission.READ_CAR_INTERIOR_LIGHTS" />
  <permission name="android.car.permission.CAR_DYNAMICS_STATE" />
  <permission name="android.car.permission.CAR_VENDOR_EXTENSION" />
  <permission name="android.car.permission.CAR_POWER" />
  <permission name="android.car.permission.CONTROL_CAR_ENERGY_PORTS" />
</privapp-permissions>
```

Include these permissions in a [privileged permission allowlist file](https://source.android.com/docs/core/permissions/perms-allowlist) in the same partition in which you install Alexa Auto App. These permissions are required to be allowlisted prior to installing Alexa Auto App. If not allowlisted, the device will not boot.

Alexa Auto App also requires you to pre-grant the dangerous runtime permissions at install time. The app uses alternative screens in the FTUE flow to request consent from the user for the dangerous permissions, and pre-granting the permissions ensures that the user consent flows are not duplicated. If installing the app with `adb install`, use the `-g` option to automatically grant all manifest-declared permissions at install time.

> **Note**: If the device appears unresponsive after rebooting, check the Logcat to see if the device is still outputting logs. If so, most likely what is locking your device is an unprovided permission that is missing from the priv-app file you push in step 2. Simply check what is missing, provide it in the XML file, and re-push to the appropriate directory and reboot. 

## Usage

This section describes the user experience after you deploy the Alexa Auto App on an Android device. The Alexa Auto App provides an Alexa menu where users can change any settings configured during the setup process.

The menu provides a toggle button for options with an On or Off value. The Alexa menu is different for users of Preview Mode and users who signed in.

> **Note:** Always set Alexa Auto App as default assistant before app launch.

#### Setup (FTUE)

This section describes what the user must do to set up Alexa, which determines the user experience when the user interacts with Alexa. The information here supersedes the information in the [Setup documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-auto/setup.html).

**Note:** The Alexa setup process is different for users of Preview Mode and users who signed in.

The setup procedure includes the following steps:

1. Select Alexa's language (if the language used for IVI is not supported by Alexa).
2. Accept the use of Alexa in Preview Mode or perform a user login.
3. Configure initial Alexa settings (for logged-in users).

> **Note:** To complete Alexa setup, the user must have internet connection.

> **Note:** CBL code pair display and authentication process might take up to 20 seconds in Alexa Auto App debug builds during first time login.

#### Landing Page

The landing page is the home for the Alexa Auto App after the user finishes authorizing with their account. It has three main features: 

* Top Carousel: A roundabout list of permission issues and information to educate the user on features to try. Certain list items additionally have a CTA button to resolve those issues when they tap on it.

* Wake word Toggle: For ease of access to control when Alexa is listening 

* Bottom Utterances: A box that displays two random utterances from the "Getting Started" Domain.

#### Settings

The Alexa Auto App provides an Alexa menu through which the user can change any settings configured during the setup process. For options with an On or Off value, the menu provides a toggle button. The Alexa menu is different for users of Preview Mode and users who signed in.

### Navigation

The Alexa `Naviagtion `module can help the customer browse local businesses, find a point-of-interest (POI), or navigate to a specific address. Use the Navigation Module to configure the following actions:

* Starting Navigation
* Stopping Navigation
* Adding a Waypoint
* Canceling a Waypoint
* Showing Previous Waypoints
* Navigating to a Previous Waypoint
* Getting Turn and Lane Guidance Information
* Getting Road Regulation Information
* Controlling the Display
* Showing Alternative Routes

### Communication

Using the Alexa Comm Modules you can configure your app to place calls and send and read SMS messages via voice using the mobile phone paired to the head unit. Alexa can also drop-in, make announcements and send voice messages to other Alexa devices, including the Alexa App.

### Entertainment

Users can stream their favorite music providers like Amazon Music and Pandora through Alexa Auto App, which publishes the media data and supported actions into a `MediaSession` for display in your head unit's native media app, media widget, or instrument cluster. Users can additionally use Alexa to control Spotify played back directly through the Spotify app and voice control local media sources with radio station and frequency tuning, preset selection, and transport controls. Alexa Auto App supports the following local sources when configured in the Alexa Auto App configuration file: FM radio, AM radio, Sirius XM satellite radio, CD player, USB, and bluetooth.

### Car Control

Using the Alexa Auto SDK `Car Control` module, you can to build a custom experience that allows users to use Alexa to voice-control in their vehicle. To configure the car control functionality for their specific vehicle, you will need to provide configuration details inside the Alexa Auto App configuration file.

## Additional topics

### Testing

Android API levels:

* AAOS: Versions 10, 11, 12
* AOSP: Version 9, 10, 11, 12

### Build options

There are several optional parameters the builder can provide in the `gradlew` command.

#### Specify path to extensions

By default, the builder picks up all the extensions present under the `alexa-auto-sdk/extensions/extras` directory. To override with an alternative path to the extensions, use `-Pextensions`. For example,

```
./gradlew assembleLocalRelease -Pextensions=~/your/custom/path/to/extension1,~/your/custom/path/to/extension2,...
```

#### Clean cache and rebuild

* To clean the AACS and AACS Sample App build cache, run `./gradlew clean` first before running the build command.
* To rebuild only outdated dependencies, append `-Pforce` to the build command. This option is required when you make a change on Alexa Auto SDK. It forces the builder to re-export all the package recipes and triggers rebuild for the packages on which changes are detected.
* To clean all the Alexa Auto SDK cache and rebuild all the dependencies: use `-PcleanDeps`.

#### Skip dependencies

To skip the step of building dependencies, use `-PskipDeps`. Make sure the Alexa Auto SDK dependency AARs are already present under `alexa-auto-sdk/aacs/android/service/core-service/libs` directory before you use this option. This will greatly reduce the build time especially if you have made no changes in the core Auto SDK itself.

#### Specify architecture

Specify the build target by appending the option `-Parch=<your_arch>`, for example: `-Parch=x86_64`. The supported architectures are `x86`,`x86_64`,`armv7`,`armv8`. If not specified, the builder by default builds for the armv8 target.

#### Enable sensitive logs

Use the option `-PsensitiveLogs` to enable senstive logs. Note that sensitive logs are allowed in debug builds only.

#### Change the dependency cache location

Change the location of the Alexa Auto SDK build cache by using this option: `-PconanHome=your/custom/home`. The default location is set to `alexa-auto-sdk/builder/.builder`.

#### Automatically accept licenses

You need to manually accept the Android SDK licenses from command line when you build the first time. Use `-PacceptLicenses` to automatically accept the licenses.

#### Specify options for dependencies

Use `-PconanOptions=<recipe_name>:<option>=<custom_option>` to pass any options for dependencies to the builder. Alexa Auto App uses Conan to build all the Auto SDK dependencies. You can use this option to change the Conan options for any dependency recipe.
