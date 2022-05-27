# Alexa Auto Setup
The following list describes the contents of this package:

* This package provides the UI for the setup workflow which includes login and subsequent setup steps, as well as interruption popups (listening for network status and driving state changes). 
* The UI is provided as Android ``Fragment``s  along with ``View Model``s which can be used independently for building a different flavor of the UI.
* At present, this package provides UI/ViewModel for CBL (Code Based Linking) and Preview mode authentication.

Note: For the app to work it must either be configured as a system app or app permissions need to be manually enabled by navigating to `Settings > Apps & Notifications > Show All Apps > com.amazon.alexaautoclientservice > Permissions > Enable Microphone`

## Integration
### Gradle
Include the project with gradle.

### Dependencies
Please refer to alexa-auto-apis doc to find details on how to fetch/publish dependencies.

* ``CBLLoginViewModel`` needs access to following implementations (interfaces for them are defined in ``alexa-auto-apis`` package):

  * ``AuthController``: This interface provides business logic for new authentication workflow. The interface must be made available from ``AlexaAppRootComponent``.
  * ``LoginUIEventListener [Optional]``: This interface allows UI to let the observer know when login is finished. This event can be used by observer to progress the app UI to logged-in state. The interface should be made available through ``AlexaAppLoggedOutScopedComponent``, that should be made available through ``AlexaAppRootComponent#getScopedComponents()``
* With the current implementation of the setup flow, there is a need for the developer to implement a ``CarUxRestrictionsController`` and listen for intents from ``com.amazon.alexa.auto.uxrestrictions.drivingStateChanged`` in order to be notified of driving state changes. This works out of the box if the developer is also using the default ``alexa-auto-ux-restrictions`` module, however if the developer wants to roll out their own solution for listening to driving state changes, they will need to override the aforementioned interface and intent. For more details, please refer to the [``alexa-auto-ux-restrictions`` module README](../alexa-auto-ux-restrictions/README.md), and take a look at the ``CarUxRestrictionsModule`` and ``DefaultCarUxRestrictionsController`` reference implementations.

## Known Gaps
The setup flow in this app does satisfy CX requirements. Refer to HMI Guidelines - Setup section for guidance or work with your Amazon partner manager.

### Missing Steps
* Enable Push-to-talk permission