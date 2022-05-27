# Understand Auto SDK Modules

## Overview

Auto SDK organizes its features into `modules`. A single module groups the logically related Auto SDK components that are required to enable a particular feature area. For example, the `Navigation` module contains the SDK components that enable your application to build a deep integration with a navigation provider, whereas the `Address Book` module contains the SDK components that enable your application to upload a user's contacts to Alexa.

A typical Auto SDK module includes the following types of components:

* **Platform abstraction components** such as AASB message interfaces and Engine configuration specifications. These components define the module-specific API that your application interacts with.

* **Engine implementation components** such as Engine infrastructure and implementations that correspond to platform APIs in the module. The module's Engine components augment the Engine with the feature set that the module provides.

* **Dependencies** such as external libraries (e.g., AVS Device SDK) that the module uses to enable its feature set. Only the build system and Engine implementation components of the module directly use these dependencies, so they are abstracted from your application.

* **Build system tools** such as scripts, Conan recipes, dependency management tools, and more, that support building the Auto SDK code and dependencies that belong to the particular module.

The module organization of Auto SDK enables you to easily use only the features you want and leave out the ones you don't. "Using a module" typically means including the module in the Auto SDK build, linking the generated module library in your application, configuring the Engine with any configuration the module specifies, and implementing the logic to integrate with the AASB messages defined by the module. On Android, the [Auto SDK Android API](./android-api-overview) further simplifies this setup and integration.

The `Core` module is the only module that your application is required to use for a bare minimum Auto SDK integration. `Core` is required because it defines Auto SDK infrastructure and AASB message interfaces that all other modules depend on, and it provides the `Engine` and `MessageBroker` classes that are the primary surface API to the native integration layer. To add Alexa to your application, the `Alexa` module is required because it adds the support for Auto SDK to communicate with Alexa; however, you are not required to integrate every AASB message interface in the `Alexa` module platform layer. See [Auto SDK Features](../features/index.md) for detailed descriptions and integration guides for `Core`, `Alexa`, and all other Auto SDK modules available to your application.

## Extensions

Some Auto SDK modules are not available on Github with the rest of Auto SDK. With help from your Amazon Solutions Architect (SA) or Partner Manager, you can access these modules from each respective "Auto SDK extension" on the Alexa developer console. For each extension you download, use the version that corresponds to the Auto SDK version that you use. When you upgrade Auto SDK versions, ensure you download and use the corresponding version of any extensions as well.

An extension includes one or more modules and the documentation for each module's API and setup instructions. In general, the contents of a particular extension look, build, and function as any other standard Auto SDK modules; only the delivery mechanism differs.