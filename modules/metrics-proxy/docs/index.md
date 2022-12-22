# Metrics Proxy

## Overview

The Alexa Auto SDK allows to integrate Alexa and OEM custom assistants into the IVI. The different components of the SDK emit metrics that can be uploaded to the cloud using a dedicated extension.

Metrics proxy module is responsible to separate Alexa vs custom assistant voice metrics when necessary.

## Configuring the Metrics Proxy Module

The `Metrics Proxy` module only requires a specific configuration when using the Alexa Custom Assistant. Please refer to the custom assistant extension documentation for details.

When the client software is responsible for Alexa voice assistant only, the `Metrics Proxy` module does not require any configuration. All metrics are sent automatically, without any filtering, to the extension responsible for the upload.
