# Core Module

## Overview

The `Core` module is the heart of the Alexa Auto SDK. The `Core` module provides the following elements that are the foundation for all Auto SDK features:
 - **Defining core API for your application to access the features of Auto SDK—** `Core` defines the `Engine` and `MessageBroker` components. Alongside the Alexa Auto Services Bridge (AASB) messages defined by each Auto SDK module, these components comprise the core API for your application to access the features of Auto SDK. To learn about the API, see [Auto SDK Core API Overview](https://alexa.github.io/alexa-auto-sdk/docs/explore/concepts/core-api-overview/).

- **Providing an infrastructure to other modules—** `Core` provides the base infrastructure of the Engine, which each Auto SDK module extends to add module-specific features. 

- **Providing core services to other modules—** `Core` defines the common Engine services and corresponding AASB message interfaces for logging, audio I/O, authorization, device settings, network info, and more. Each module uses these Engine services to provide its own module-specific features.

## Configure the Core module

The `Core` module defines required and optional configuration objects that you include in the Engine configuration for your application. You can define the configuration objects in a file or construct them programmatically with the relevant configuration factory functions.

### (Required) Vehicle info configuration

Your application must provide the `aace.vehicle` configuration specified below. Amazon uses the vehicle configuration properties for analytics and metrics.

```
"aace.vehicle": {
    "deviceInfo": {
        "manufacturer": "${DEVICE_MANUFACTURER}",
        "model": "${DEVICE_MODEL}",
        "platform": "${DEVICE_PLATFORM}",
        "osVersion": "${DEVICE_OS_VERSION}",
        "hardwareArch": "${DEVICE_HARDWARE_ARCH}",
        "serialNumber":  "${DEVICE_SERIAL_NUMBER}"
    },
    "appInfo": {
        "softwareVersion": "${APP_VERSION}"
    },
    "vehicleInfo": {
        "make": "${VEHICLE_MAKE}",
        "model": "${VEHICLE_MODEL}",
        "year": "${VEHICLE_YEAR}",
        "trim": "${VEHICLE_TRIM}",
        "microphoneType": "${VEHICLE_MICROPHONE}",
        "operatingCountry": "${VEHICLE_COUNTRY}",
        "vehicleIdentifier": "${VEHICLE_IDENTIFIER}",
        "engineType": "${VEHICLE_ENGINE_TYPE}",
        "rseEmbeddedFireTvs": "${RSE_EMBEDDED_FIRE_TV_COUNT}"
    }
}
```
The following table describes the properties in the configuration:

| Property | Type | Required | Description | Example |
| ---------| ---- | -------- | ----------- | ------- |
| deviceInfo.<br>manufacturer | String | Yes | The manufacturer of the head unit hardware | "Alpine", "Pioneer" |
| deviceInfo.<br>model | String | Yes | The model name of the head unit hardware | "Coral" |
| deviceInfo.<br>platform | String | Yes | The head unit software platform or operating system name | "Android", "Ubuntu" |
| deviceInfo.<br>osVersion | String | Yes | The version of the head unit operating system | "12", "18.04.6 LTS" |
| deviceInfo.<br>hardwareArch | String | Yes | The hardware architecture of the head unit or CPU+instruction set | "arm64-v8a", "x86_64", "armv7hf", "armv8" |
| deviceInfo.<br>serialNumber | String | Yes | The serial number of the head unit | "A01BCDEFGH2I" |
| appInfo.<br>softwareVersion | String | Yes | The version of the Auto SDK client application | "1.0.1" |
| vehicleInfo.<br>make | String | Yes | The make of the vehicle | "BMW", "Ford" |
| vehicleInfo.<br>model | String | Yes | The model of the vehicle | "MDX", "Q5", "Accord" |
| vehicleInfo.<br>year | String | Yes | The model year of the vehicle. The value must be an integer in the range 1900-2100 expressed as a string | "2023" |
| vehicleInfo.<br>trim | String | No | The trim package of the vehicle, identifying the vehicle's level of equipment or special features | "Limited", "Base package", "Type S" |
| vehicleInfo.<br>microphoneType | String | No |  The type and arrangement of microphone used in the vehicle | "7 mic array centrally mounted" |
| vehicleInfo.<br>operatingCountry | String | Yes | The current (or intended, if current is not available) operating country for the vehicle. The value must be an ISO 3166 Alpha-2 country code | "US", "MX", "JP" |
| vehicleInfo.<br>vehicleIdentifier | String | Yes | The automaker's identifier for the vehicle. The value should not be the vehicle identification number (VIN).| "12345ABCDE" |
| vehicleInfo.<br>engineType | String | No | The type of engine in the vehicle. <br><br>**Accepted values:**<ul><li>`"GAS"`</li><li>`"ELECTRIC"`</li><li>`"HYBRID"`</li></ul> | "ELECTRIC" |
| vehicleInfo.<br>rseEmbeddedFireTvs | String | No | The number of Rear Seat Entertainment embedded Fire TVs in the vehicle, expressed as a string | "2" |

<details markdown="1">
<summary>Click to expand or collapse details— Generate the configuration programmatically with the C++ factory functions</summary>

Auto SDK provides the [`VehicleConfiguration`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1vehicle_1_1config_1_1_vehicle_configuration.html) factory functions to generate the configuration programmatically. 


```c++
#include <AACE/Vehicle/VehicleConfiguration.h>

std::vector<aace::vehicle::config::VehicleConfiguration::VehicleInfoProperty> vehicleProperties = {
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::MAKE, "SampleMake"},
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::MODEL, "SampleModel" },
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::YEAR, "2020" },
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::TRIM, "Sport" },
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::OPERATING_COUNTRY, "US" },
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::MICROPHONE_TYPE, "7 mic array, centrally mounted" },
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::VEHICLE_IDENTIFIER, "1234abcd" },
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::ENGINE_TYPE, "GAS" },
    { aace::vehicle::config::VehicleConfiguration::VehicleInfoPropertyType::RSE_EMBEDDED_FIRE_TVS, "2" }
};

auto vehicleConfig = aace::vehicle::config::VehicleConfiguration::createVehicleInfoConfig(vehicleProperties);

std::vector<aace::vehicle::config::VehicleConfiguration::DeviceInfoProperty> deviceProperties = {
    { aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType::MANUFACTURER, "SampleManufacturer"},
    { aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType::MODEL, "SampleModel" },
    { aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType::SERIAL_NUMBER, "A01BCDEFGH2I" },
    { aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType::PLATFORM, "Android" },
    { aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType::OS_VERSION, "12" },
    { aace::vehicle::config::VehicleConfiguration::DeviceInfoPropertyType::HARDWARE_ARCH, "arm64-v8a" }
};

auto deviceConfig = aace::vehicle::config::VehicleConfiguration::createDeviceInfoConfig(deviceProperties);

std::vector<aace::vehicle::config::VehicleConfiguration::AppInfoProperty> appProperties = {
    { aace::vehicle::config::VehicleConfiguration::AppInfoPropertyType::SOFTWARE_VERSION, "1.0.1"}
};

auto appConfig = aace::vehicle::config::VehicleConfiguration::createAppInfoConfig(appProperties);

engine->configure(
    {
        // ...other config objects...,
        vehicleConfig,
        deviceConfig,
        appConfig
    }
);
```

</details>

### (Required) Storage configuration

Your application must provide the `aace.storage` configuration specified below. The Engine uses the configured path to create a database to persist data across device reboots.

```
{
    "aace.storage": {
        "localStoragePath": {{STRING}},
        "storageType": "sqlite"
    }
}
```

The following table describes the properties in the configuration:

| Property         | Type   | Required | Description                                                                              | Example                         |
| ---------------- | ------ | -------- | ---------------------------------------------------------------------------------------- | ------------------------------- |
| localStoragePath | String | Yes      | The absolute path where the Engine will create the local storage database, including the database name | "/opt/AAC/data/aace-storage.db" |
| storageType      | String | Yes      | The type of storage to use                                                               | "sqlite"                        |

>**Note:** This database is not the only one used by the Engine. For example, components in the `Alexa` module have similar configuration to store feature-specific data. See [Configure the Alexa module](https://alexa.github.io/alexa-auto-sdk/docs/explore/features/alexa#configure-the-alexa-module) for details.

<details markdown="1">
<summary>Click to expand or collapse details— Generate the configuration programmatically with the C++ factory function</summary>

Auto SDK provides the [`StorageConfiguration::createLocalStorageConfig()`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1storage_1_1config_1_1_storage_configuration.html#a1619cb1c7be8fcb913454e16ceb49a1d) factory function to generate the configuration programmatically. 


```c++
#include <AACE/Storage/StorageConfiguration.h>

auto storageConfig = aace::storage::config::StorageConfiguration::createLocalStorageConfig("/opt/AAC/data/storage.db");

engine->configure(
    {
        // ...other config objects...,
        storageConfig
    }
);
```

</details>

### (Required) cURL configuration

The Auto SDK uses cURL for network connections. Your application can provide Engine configuration to specify the cURL configuration:

```
{
    "aace.alexa": {
        "avsDeviceSDK": {
            "libcurlUtils" {
                "CURLOPT_CAPATH": {{STRING}},
                "CURLOPT_INTERFACE": {{STRING}},
                "CURLOPT_PROXY": {{STRING}}

            }
        }
    }
}
```

The following table describes the properties in the configuration:

| Property          | Type   | Required | Description                                                                                    | Example                 |
| ----------------- | ------ | -------- | ---------------------------------------------------------------------------------------------- | ----------------------- |
| CURLOPT_CAPATH    | String | Yes      | The path to the directory containing the CA certificates                                       | "/opt/AAC/certs"        |
| CURLOPT_INTERFACE | String | Yes      | The outgoing network interface. Can be a network interface name, an IP address, or a host name | "wlan0"                 |
| CURLOPT_PROXY     | String | No       | The address of the HTTP proxy                                                                  | "http://127.0.0.1:8888" |

> **Note:** If the HTTP proxy requires credentials in HTTP headers to authenticate a user agent, you can specify the headers at runtime with the [`PropertyManager`](./PropertyManager.md) interface by using the `aace.network.httpProxyHeaders` property name. You can also change the network interface at runtime with the `aace.network.networkInterface` property name.

<details markdown="1">
<summary>Click to expand or collapse details— Generate the configuration programmatically with the C++ factory function</summary>

Auto SDK provides the [`AlexaConfiguration::createCurlConfig()`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1alexa_1_1config_1_1_alexa_configuration.html#a221fd921e3b5f297a99520a6e09c283d) factory function to generate the configuration programmatically. 


```c++
#include <AACE/Alexa/AlexaConfiguration.h>

auto curlConfig = aace::alexa::config::AlexaConfiguration::createCurlConfig("/opt/AAC/etc/certs");

engine->configure(
    {
        // ...other config objects...,
        curlConfig
    }
);
```

</details>

### (Required) Metrics configuration

Your application must provide the `aace.metrics` configuration specified below. The Engine uses the configured values for uploading metrics to Amazon metric services.

```
{
    "aace.metrics": {
        "metricStoragePath": ${METRIC_STORAGE_PATH,
        "metricDeviceIdTag": "${METRIC_TAG}"
    }
}
```

The following table describes the properties in the configuration:

| Property         | Type   | Required | Description                                                                              | Example                         |
| ---------------- | ------ | -------- | ---------------------------------------------------------------------------------------- | ------------------------------- |
| metricStoragePath | String | Yes      | An absolute path to a directory where metrics may be stored prior to upload. The directory must exist and should not be used for any other purpose. | "/opt/AAC/data/metrics" |
| metricDeviceIdTag      | String | Yes      | A tag that Auto SDK Engine will use in combination with DSN to generate a unique anonymous device identifier. Neither Alexa nor Auto SDK will store this tag and hence cannot reverse the hash to identify a single DSN from an individual metric. The metricDeviceIdTag may be any nonempty alphanumeric string that does not change across device reboots, factory resets, app data reset, or software updates. The recommended value is a 32 character string that is not the DSN or VIN. The value may be unique to an individual vehicle, provided it is stable, but it is not required to be unique. | "yXGO5U1ylqauXa5LwSx2ppQPFTQbFtu4" |

<details markdown="1">
<summary>Click to expand or collapse details— Generate the configuration programmatically with the C++ factory functions</summary>

Auto SDK provides the [`MetricsConfiguration`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1metrics_1_1config_1_1_metrics_configuration.html) factory functions to generate the configuration programmatically. 


```c++
#include <AACE/Metrics/MetricsConfiguration.h>

auto metricTag = aace::metrics::config::MetricsConfiguration::createMetricsTagConfig("yXGO5U1ylqauXa5LwSx2ppQPFTQbFtu4");

auto metricPath = aace::metrics::config::MetricsConfiguration::createMetricsStorageConfig("/opt/AAC/data/metrics");

engine->configure(
    {
        // ...other config objects...,
        metricTag,
        metricPath
    }
);
```

</details>

### (Optional) Logger configuration

By default, the Engine writes Auto SDK logs to the following places:

* The console, for C++ native applications
* Logcat, for Android applications

You can configure the Engine to save logs to a file with the `aace.logger` configuration:

```
{
  "aace.logger": {
    "sinks": [
        {
            "id": {{STRING}},
            "type": "aace.logger.sink.file",
            "config": {
                "path": {{STRING}},
                "prefix": {{STRING}},
                "maxSize": {{INTEGER}},
                "maxFiles": {{INTEGER}},
                "append": {{BOOLEAN}}
            },
            "rules": [
                {
                    "level": {{STRING}}
                }
            ]
        }
    ]
}
```

The following table describes the properties in the configuration:

| Property                                         | Type             | Required | Description                                                                                                                                                                                                  | Example                 |
| ------------------------------------------------ | ---------------- | -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------- |
| aace.logger.<br>sinks[i].<br>id                  | String           | Yes      | A unique identifier for the log sink.                                                                                                                                                                        | "debug-logs"            |
| aace.logger.<br>sinks[i].<br>type                | String           | Yes      | The type of the log sink. Use "aace.logger.sink.file" to write logs to a file.                                                                                                                               | "aace.logger.sink.file" |
| aace.logger.<br>sinks[i].<br>config.<br>path     | String           | Yes      | An absolute path to a directory where the Engine creates the log file.                                                                                                                                                      | "/opt/AAC/data"         |
| aace.logger.<br>sinks[i].<br>config.<br>prefix   | String           | Yes      | The prefix for the log file.                                                                                                                                                                                 | "auto-sdk-logs"              |
| aace.logger.<br>sinks[i].<br>config.<br>maxSize  | Integer          | Yes      | The maximum size of the log file in bytes.                                                                                                                                                                   | 5242880                 |
| aace.logger.<br>sinks[i].<br>config.<br>maxFiles | Integer          | Yes      | The maximum number of log files.                                                                                                                                                                            | 5                       |
| aace.logger.<br>sinks[i].<br>config.<br>append   | Boolean          | Yes      | Whether the Engine should overwrite log files.<br>Use true to append logs to the existing file. Use false to overwrite the log files.                                                                                                                          | false                   |
| aace.logger.<br>sinks[i].<br>rules[j].<br>level  | Enum string | Yes      | The log level filter the Engine uses when writing logs to the sink. <br><br>**Accepted values:**<ul><li>`"VERBOSE"`</li><li>`"INFO"`</li><li>`"WARN"`</li><li>`"ERROR"`</li><li>`"CRITICAL"`</li><li>`"METRIC"`</li></ul> | "VERBOSE"               |

<details markdown="1">
<summary>Click to expand or collapse details— Generate the configuration programmatically with the C++ factory function</summary>

Auto SDK provides the [`LoggerConfiguration::createFileSinkConfig()`](https://alexa.github.io/alexa-auto-sdk/docs/native/api/classes/classaace_1_1logger_1_1config_1_1_logger_configuration.html#ab58d322f01f350cf87c89c41e8257037) factory function to generate the configuration programmatically. 

```c++
#include "AACE/Logger/Logger.h"
#include "AACE/Logger/LoggerConfiguration.h"


auto fileSinkConfig = aace::logger::config::LoggerConfiguration::createFileSinkConfig(
    "debug-logs",
    aace::logger::LoggerEngineInterface::Level::VERBOSE,
    "opt/AAC/data",
    "auto-sdk-logs",
    5242880,
    5,
    false);

engine->configure(
    {
        // ...other config objects...,
        fileSinkConfig
    }
);

```

</details>

### (Optional) AASB and MessageBroker configuration

#### Configure enabled interfaces

When you use a module, the Engine services of that module enable every interface the module defines. This means that for every interface in a module you use, if your application does not subscribe to the AASB messages of the interface, the Engine performs default handling (typically no-op) for the messages you do not handle.

To disable this setting, provide the following `aace.messageBroker` configuration object in your Engine configuration:

```
{
    "aace.messageBroker": {
        "autoEnableInterfaces": false
    }
}
```

You can also configure the enablement on a per-interface basis. If you don't want the Engine to provide a default handler for a particular interface, you can disable the interface using the following configuration that specifies the name of the interface and the Engine service that owns it:

```
{
    "aasb.<engine_service_ID>": {
        "<interface_name>": {
            "enabled": {{BOOLEAN}}
        }
    }
}
```

For example, the following configuration disables the `TemplateRuntime` interface from the `Alexa` module's `alexa` Engine service and the `LocationProvider` interface from `Core` module's `location` Engine service: 

```
{
    "aasb.alexa": {
        "TemplateRuntime": {
            "enabled": false
        }
    },
   "aasb.location": {
      "LocationProvider": {
         "enabled": false
      }
   }
}
```

#### Configure the synchronous message timeout

All the messages published by the Engine through the Message Broker are asynchronous; however, certain messages require your application to respond with a special synchronous-style `Reply` message. Your application must publish the reply quickly because the Engine blocks its execution thread while waiting for the response, and the Message Broker cannot dispatch more messages while waiting. The following messages are examples of the [`LocationProvider.GetLocation`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/LocationProvider/index.html#getlocation) message and its reply message:

`LocationProvider.GetLocation` message:
```
{
  "header": {
    "id": "23b578ed-6dc3-460a-998e-1647ba6cde42",
    "messageType": "Publish",
    "version": "4.0",
    "messageDescription": {
        "topic": "LocationProvider",
        "action": "GetLocation"
    }
  }
}
```
`LocationProvider.GetLocation` reply message:
```
{
  "header": {
    "id": "4c4d13b6-6a8d-445b-931a-a3feb0878311",
    "messageType": "Reply",
    "version": "4.0",
    "messageDescription": {
      "topic": "LocationProvider",
      "action": "GetLocation",
      "replyToId": "23b578ed-6dc3-460a-998e-1647ba6cde42"
    }
  },
  "payload": {
    "location": {
      "latitude": 37.410,
      "longitude": -122.025
    }
  }
}
```
The AASB message documentation for each interface specifies whether the interface requires any `Reply` messages. However, not every `Reply` message is synchronous-style.

To avoid waiting indefinitely for "synchronous" replies, the Engine uses a timeout when waiting for these reply messages. If your application does not publish a reply before the timeout elapses, the Message Broker proceeds to avoid waiting, but the relevant Engine operation won't execute properly. The default timeout value for reply messages is 500 milliseconds. In a busy system, the default timeout might not be long enough. You can configure this value by adding the optional field `defaultMessageTimeout` to the `aace.messageBroker` JSON object in your Engine configuration. The following example configuration sets the reply timeout to 1000 ms:
```
{
    "aace.messageBroker": {
        "defaultMessageTimeout": 1000
    }
}
```
> **Important!** Since increasing the timeout increases the Engine's message processing time, use this configuration carefully. Consult with your Amazon Solutions Architect (SA) as needed.

## Use the Core module interfaces

The following list describes the AASB message interfaces provided by the `Core` module:

### (Required) Provide access tokens with Authorization

The `Authorization` interface specifies messages for your application to initiate device authorization, terminate device authorization, or provide authorization data, such as Alexa access tokens, to the Engine.

**[>> Authorization interface](./Authorization.md)**

### (Required) Provide audio channels with AudioInput and AudioOutput

The core audio Engine service provides a mechanism for Engine components of any module to open audio input and output channels in your application. Each component that requests an audio channel specifies its audio channel type so your application can provide different microphone and media player implementations for each channel.

**[>> AudioInput interface](./AudioInput.md)**

**[>> AudioOutput interface](./AudioOutput.md)**

### (Required) Manage runtime properties with PropertyManager

Different Auto SDK modules define properties based on their supported features. For example, the `Alexa` module requires a locale setting to notify Alexa which language to use when interacting with the user. The `Core` module provides a mechanism for Engine services to register properties they manage and listen to changes in properties managed by other modules. The `PropertyManager` interface specifies messages for your application and the Engine to query and update these properties.

**[>> PropertyManager interface](./PropertyManager.md)**

### (Optional) Report location with LocationProvider

For an accurate and personalized user experience, the Engine uses the vehicle's location from `LocationProvider`.

**[>> LocationProvider interface](./LocationProvider.md)**

### (Optional) Report network status changes with NetworkInfoProvider

To adapt the Engine behavior dynamically based on the state of the head unit's network connection, provide network connection reporting through `NetworkInfoProvider`.

**[>> NetworkInfoProvider interface](./NetworkInfoProvider.md)**

### (Optional) Report data usage with DeviceUsage

Report metrics about the head unit's data usage with `DeviceUsage`.

**[>> DeviceUsage interface](./DeviceUsage.md)**

### (Optional) Manage multiple voice assistants dialog requests and status

The Arbitrator interface manages the voice assistant agents for both Alexa and 3rd parties. You can control how arbitration is handled for multiple voice assistant requests based on the rules you configure at the system level or at agent registration.

**[>> Arbitrator interface](./Arbitrator.md)**

### (Optional) Provide 3rd Party Wakeword Support

The Wakeword interface manages the voice assistant agents for both Alexa and 3rd party assistants. Using the Wakeword module, OEM partners can simultaneously support multiple voice assistant wake words like Siri on the same in-vehicle infotainment system. 

**[>> Wakeword interface](./Wakeword.md)**



