# APL Render Module <!-- omit in toc -->

The APL Render module is an Android library that enables Alexa Presentation Language (APL) rendering capabilities in an Android application. For detailed information about APL, see the [APL documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/understand-apl.html).

<!-- omit in toc -->
## Table of Contents
- [Overview](#overview)
- [Understanding Android View Host](#understanding-android-view-host)
- [APL Render Module Functionality](#apl-render-module-functionality)
- [How to use the APL Render Module](#how-to-use-the-apl-render-module)
    - [Defining the APL Layout](#defining-the-apl-layout)
    - [Initializing the APL Runtime](#initializing-the-apl-runtime)
    - [Implementing the Event Interface](#implementing-the-event-interface)
    - [Instantiating APLPresenter](#instantiating-aplpresenter)
- [Rendering an APL Document](#rendering-an-apl-document)
- [Overriding Android View Host Options](#overriding-android-view-host-options)
    - [Using APLOptions.Builder](#using-aploptionsbuilder)
- [APL Runtime Properties](#apl-runtime-properties)
    - [Driving State](#driving-state)
    - [Theme](#theme)
- [APL Extensions](#apl-extensions)
    - [Backstack](#backstack)
    - [Local Information Data](#local-information-data)
        - [ILocalInfoDataConsumer](#ilocalinfodataconsumer)
        - [ILocalInfoDataReporter](#ilocalinfodatareporter)
- [Building the APL Render Library](#building-the-apl-render-library)

## Overview
Rendering an APL document on a device requires the implementation of various components and the logic that makes the components work together. To handle APL-related directives and events, the device must support APL interfaces. It needs integration of the [APL Core Library](https://github.com/alexa/apl-core-library) to manage the document parsing and rendering workflow. 

In addition, you must build a view host for the device to render the APL document on the screen, as well as provide components to download layouts, images, and other resources. If the APL document generates multimedia content, such as a video or audio file, you need a media player to play back the content. Lastly, APL rendering needs the orchestration logic to manage the lifecycle of a rendered document, which includes user events, audio focus management, time out management, visual context, command execution, and much more. The Alexa Auto SDK, with the APL Render module and a prebuilt Android view host, simplifies the process of APL rendering because it provides the aforementioned components and logic for you. 

## Understanding Android View Host

The Android view host is the component responsible for rendering APL on the screen. Amazon provides a prebuilt Android view host as an Android Archive Library (AAR) on the developer portal. To download the AAR, contact your Solutions Architect (SA) or Partner Manager.

>**Note:** To use the Android Render module, you must place the Android view host AAR in the [src/main/libs/](./src/main/libs) folder of the APL Render module.

## APL Render Module Functionality

The APL Render module provides all the functionality needed for enabling APL rendering capabilities in an Android application. The APL Render module provides the following capabilities:

* APL runtime initialization
* HTTP Resource downloader
* Android view host integration
* Android audio focus management
* Activity tracking for timeout management
* Audio and media players
* Interfaces to easily override functionality

## How to use the APL Render Module

To use the APL Render module without customization, follow these steps:

1. Define the APL layout.
2. Initialize the APL runtime.
3. Implement the event interface.
4. Instantiate `APLPresenter`.

### Defining the APL Layout

The application must define the layout of the screen on which the APL document is rendered. the width and height of the `APLLayout` must fall in range with one of the three supported [automotive viewport profiles](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-alexa-viewport-profiles-package.html): auto extra small, auto small, and auto medium. Define an `com.amazon.apl.android.APLLayout` object, as shown in the following example, where the object is defined under `res/layout`:

```java
<?xml version="1.0" encoding="utf-8"?>
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    android:orientation="vertical"
    android:layout_width="match_parent"
    xmlns:app="http://schemas.android.com/apk/res-auto"
    android:layout_height="match_parent">

    <com.amazon.apl.android.APLLayout
        android:layout_width="match_parent"
        android:layout_height="match_parent"
        android:background="?attr/colorPrimary"
        android:id="@+id/apl"
        android:theme="@style/Theme.AppCompat"
        app:aplTheme="dark"
        app:isRound="false"
        app:mode="auto"
        />

</LinearLayout>
```

The `app:aplTheme` field corresponds to the default [APL theme](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-viewport-property.html#theme) to be used if one is not specified in the APL document. Typical values are `light` or `dark`. 

The `app:mode` field specifies the operating mode, which is a [viewport property](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-viewport-property.html#viewport_mode_property). For information about the viewport object, see the [viewport documentation](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-viewport-property.html). The value for this field should be `auto` for an Automotive device.

### Initializing the APL Runtime

The application must invoke the `APLPresenter.initialize()` static method before inflating the `APLLayout` UI component. The following code shows how to use the `onCreate` method of `Activity` to initialize the APL runtime:

```java
import com.amazon.apl.android.render.APLPresenter;

//---------------------------------------------------------------------
// Initialize the APL Runtime. This must be called during
// Activity.onCreate() or prior to APLLayout inflation.
//---------------------------------------------------------------------
onCreate() {
    Context context = getApplicationContext(); 
    APLPresenter.initialize(context);
}
```

### Implementing the Event Interface 

The application must implement the `com.amazon.apl.android.render.interfaces.IAPLEventSender` interface. The `IAPLEventSender` interface provides the APIs to allow the APL Render module to report events to Alexa or the capability agent. You can integrate the event interface into the APL handler that implements the Auto SDK `APL` AASB message interface. The following code shows how to do the integration:

```java
import com.amazon.aace.apl.APL;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;

public class APLHandler extends APL implements IAPLEventSender {

    //---------------------------------------------------------------------
    // Override IAPLEventSender methods. 
    // APLHandler will register with APL Render library as the event sender. 
    //---------------------------------------------------------------------
    @Override
    public void sendRenderDocumentResult(String token, boolean result, String error) {
        renderDocumentResult(token, result, error); // APL::renderDocumentResult
    }

    @Override
    public void sendExecuteCommandsResult(String token, boolean result, String error) {
        executeCommandsResult(token, result, error); // APL::executeCommandsResult
    }

    ...
}

```

### Instantiating APLPresenter

The application must instantiate the `APLPresenter` object, which provides the orchestration logic in the APL rendering process.

>**Note:** Create `APLPresenter` *after* the APL platform interface handler is registered with the Auto SDK Engine. 

The following code shows how to instantiate the `APLPresenter` object:

```java
import com.amazon.apl.android.render.APLPresenter;

public class APLHandler extends APL implements IAPLEventSender {

    private APLPresenter mPresenter;

    public void buildAPLPresenter(JSONArray visualCharacteristics, String defaultWindowId) {
        //---------------------------------------------------------------------
        // Retrieve the APLLayout view with id 'apl' defined in apl_view.xml.
        // This assumes that 'activity' is the application's Activity.
        //---------------------------------------------------------------------
        aplLayout = activity.findViewById(R.id.apl); 
        //---------------------------------------------------------------------
        // Application needs to handle the correlation of window ids from the
        // visual characteristics configuration to the APLLayout instance.
        //---------------------------------------------------------------------
        HashMap aplLayouts = new HashMap<String, APLLayout>();
        aplLayouts.put(defaultWindowId, mAplLayout);
        //---------------------------------------------------------------------
        // Create APLPresenter to handle APL rendering.
        //---------------------------------------------------------------------
        mPresenter = new APLPresenter(aplLayouts, visualCharacteristics, defaultWindowId, this);
    }
```

The following list describes the parameters to `APLPresenter`:

* The first parameter is a map of the `APLLayout` objects. Each `APLLayout` is identified by a window ID, which specifies the window where the APL document is rendered. Typically, there is one `APLLayout` defined for the window where all the APL documents are rendered, but you can build skills that support rendering in multiple windows. 

* The second parameter is a JSON array`  pointing to the visual characteristics defined by the device. For more information about visual characteristics, see the Auto SDK APL module documentation and the [Smart Screen SDK documentation](https://github.com/alexa/alexa-smart-screen-sdk/blob/master/modules/GUI/config/SmartScreenSDKConfig.md#visual-characteristics-parameters). 
  
* The third parameter is the default window ID, specifying the window where APL documents are rendered if Alexa does not provide a window ID. 
  
* The last parameter is the object that implements the `IAPLEventSender` interface.

## Rendering an APL Document

To render an APL document, call the `onRenderDocument` API on the `APLPresenter`. The `APLHandler` can delegate the `APL` APIs to the `APLPresenter`, as shown in the following code:

```java
public class APLHandler extends APL implements IAPLEventSender {

    ...

    //---------------------------------------------------------------------
    // Override Auto SDK APL interfaces
    //---------------------------------------------------------------------
    @Override
    public void renderDocument(String payload, String token, String windowId) {
        mAplPresenter.onRenderDocument(payload, token, windowId); // APLRender implements these interfaces
    }

    @Override
    public void executeCommands(String payload, String token) {
        mPresenter.onExecuteCommands(payload, token);
    }

    ...
}
```

## Overriding Android View Host Options  

Rendering an APL document requires the APL Render module to set up an `APLOptions` object, which is  passed to the view host. The `APLOptions` object is configured with providers, callbacks, and listeners, as described in the following list: 

* Providers are objects implemented outside the view host.They provide objects used during the rendering process. For example, the data retriever provider downloads APL resources, such as layouts from content delivery networks (CDNs). The media player provider plays media, such as videos. 

* Callbacks are interfaces used by the view host to report events, such as:

    * user events (e.g., button clicks) 
    * document lifecycle events (e.g., completion of document rendering)  

* Listeners are interfaces for reporting the APL rendered document state or screen lock events. 

The APL Render module sets up all the providers, callbacks, and listeners. If the application needs to override any of them, it uses the `APLOptions.Builder` object.

### Using APLOptions.Builder

To override `APLOptions`, extend the `APLPresenter` object, as shown in the following code:

```java
class MyAPLPresenter extends APLPresenter {
    
    //--------------------------------------------------------------------- 
    // IAPLOptionsBuilderProvider override
    //--------------------------------------------------------------------- 
    @Override
    APLOptions.Builder getAPLOptionsBuilder() {
        APLOptions.Builder builder = super.getAPLOptionsBuilder();
        // Listen in on APL finish callback
        builder.onAplFinishCallback(() -> {
            // Do something here
            super.onAplFinish();
        });
        return builder;
    }
}
```
## APL Runtime Properties

The [`IAPLContentListener`](https://github.com/alexa/alexa-auto-sdk/blob/master/aacs/android/app-components/alexa-auto-apl-renderer/modules/apl-render/src/main/java/com/amazon/apl/android/render/interfaces/IAPLContentListener.java) exposes an interface to control some APL runtime properties that affect how APL documents are rendered. The `onAPLRuntimeProperties` API takes in a JSON string that contains one or more properties to update.

### Driving State

The `drivingState` property supports the values `moving` and `parked`. An APL experience may differ depending on the driving state in order to provide a safer driving experience.

### Theme

The `theme` property allows the APL experience to render in different color schemes. There are six supported values: light, light-gray1, light-gray2, dark, dark-black, dark-gray. The light themes can be during for day driving, while the dark themes can be used for night driving.
## APL Extensions 
### Backstack

This library supports the [Backstack](https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-ext-backstack.html) extension. The application must ensure that the `APLPresenter` is not destroyed and recreated when a new APL document with the same token id is received. Otherwise, the Backstack will be reinstantiated and the previous stack of documents will be lost.

### Local Information Data

This library contains a custom APL extension that is used by the `APLPresenter` to expose point of interest data to the application. This data can be used to drop corresponding pins on the head unit's integrated map. Two way communication is also provided so that the application or AP runtime can notify each other when a specific data point is active or selected.

There are two interfaces that the application can use to interact with Local information data: [ILocalInfoDataConsumer](https://github.com/alexa/alexa-auto-sdk/tree/4.0/aacs/android/app-components/alexa-auto-apl-renderer/modules/apl-render/src/main/java/com/amazon/apl/android/render/interfaces/ILocalInfoDataConsumer.java) and [ILocalInfoDataReporter](https://github.com/alexa/alexa-auto-sdk/tree/4.0/aacs/android/app-components/alexa-auto-apl-renderer/modules/apl-render/src/main/java/com/amazon/apl/android/render/interfaces/ILocalInfoDataReporter.java).

#### ILocalInfoDataConsumer

The `IPresenter` (`https://github.com/alexa/alexa-auto-sdk/blob/master/aacs/android/app-components/alexa-auto-apl-renderer/modules/apl-render/src/main/java/com/amazon/apl/android/render/interfaces/IPresenter.java`) interface exposes a method to set the data consumer, which must be set by the application:

```
    /**
     * Saves a reference to the local info data consumer.
     *
     * @param consumer The object that consume local info data events.
     */
    void setLocalInfoDataConsumer(ILocalInfoDataConsumer consumer);
```

The application will be notified through the consumer method `aplDataAvailable` with a JSON string object represention all the points of interest. The application will be notified when a specific data point is selected on the APL document using the consume method `aplDataItemSelectedById`. 

#### ILocalInfoDataReporter 

The `APLPresenter` implements the `ILocalInfoDataReporter` interface to allow the application to notify the APL runtime when a data point is selected outside of the APL runtime. To do this notification simply call `platformDataItemSelectedById` on the `APLPresenter` instance.

## Building the APL Render Library
**Note:** Before proceeding to build the APL Render library, download the Android APL resource from the developer portal according to instructions from your Solutions Architect or Partner Manager.

This library can be built using the included gradle wrapper as follows
```
./gradlew assembleRelease 
```