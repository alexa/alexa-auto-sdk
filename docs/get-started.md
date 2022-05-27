# Get Started with Auto SDK

This guide outlines the one-time steps required to get started building an Auto SDK integration for your vehicle.

## Register an AVS product

The Alexa features enabled by Auto SDK are built upon the [Alexa Voice Service (AVS)](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/get-started-with-alexa-voice-service.html). Prior to using Auto SDK, follow the [Register a Product with AVS](https://developer.amazon.com/en-US/docs/alexa/alexa-voice-service/register-a-product-with-avs.html) guide to set up an Amazon developer account, register a product for your vehicle, and create a security profile.
    
* For the **Please select your product type** option, select **Device with Alexa built-in**.
* For the **Product category** option, select **Automotive**.

In the product details page of your new AVS product, take note of the following fields for later use:

* The **Amazon ID** from the top of the page
* The **Client ID** from the **Other devices and platforms** tab

## Clone the Auto SDK source code

Auto SDK is open source. Clone the [`alexa-auto-sdk`](https://github.com/alexa/alexa-auto-sdk) project from Github on your development machine:
```
git clone https://github.com/alexa/alexa-auto-sdk.git
```
Auto SDK documentation refers to the root directory of the cloned project as `AUTO_SDK_HOME`.

## Install prerequisites

Prior to building Auto SDK, install the [build prerequisites](./native/building.md) outlined the Auto SDK build system documentation. 

> **Note:** If you are an Android developer, you will build the Alexa Auto Client Service (AACS) with Gradle rather than using the Auto SDK Builder Tool directly. However, you still need to install the build system prerequisites described in the build system documentation because the AACS Gradle build depends on the Auto SDK build system.

## Read the overview documentation

To get a high-level understanding of how to use Auto SDK, read the [concepts documentation](./explore/concepts/index.md). After understanding the API concepts, read the [feature overview documentation](./explore/features/index.md) to get a sense of which Auto SDK features you will use in your integration and to identify which Auto SDK extensions you want to download.

## (Optional) Download extensions

If you want to use any of the [optional features](./explore/features/index.md#extension-modules) provided by Auto SDK extensions, request your Amazon Solutions Architect (SA) or Partner Manager to grant Alexa developer console access to the extensions you need. Download the extension archives and follow the documentation in each extension archive to move the extension source code and dependencies into your Auto SDK source tree.

Some extensions, such as Local Voice Control (LVC), have additional resources to use, so ensure you thoroughly read the extension documentation for any more setup steps.

## Follow a developer guide

Use the developer guides for [Android](./android/index.md) or [Native C++](./native/index.md) to guide you through the next steps to develop your Auto SDK integration.