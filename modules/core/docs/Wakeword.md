# Wakeword Module

## Overview

The `Wakeword` module has interfaces to manage 3rd Party agent’s wakeword detection. This module manages 3rd party agent's wakeword detection via amazonlite supported wakeword engine in auto sdk" to indicate that it only manages the 3rd party agent's detection through its own Wakeword extension and not external. 


The [WakewordManager interface](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/WakewordManager) provides messages to

* Enable or Disable 3rd Party wakeword
* Notify the application when their wakeword is detected

## Configuring the Wakeword Module

The following changes are added in config.json for the Wakeword module to supply the list of supported 3rd Party wakewords. This configuration is manadatory to specify when supporting 3rd Party wakewords. If this is not provided as a part of the config when using a Amazonlite WWE that supports 3rd Party Wakewords any 3rd Party wakeword detections can result in undesired behavior

    "aace.wakewordManager": {
        "WakewordManager": {
            "3PWakewords": ["SIRI"]
        }
    }


