# Alexa Auto Login
The following list describes the contents of this package:
* `LoginDialogFragment` is the fragment for displaying the login screen where the user logs in with the code-based linking (CBL) method.

* `QRCodeGenerator` generates a QR code bitmap that accompanies the CBL code and Amazon URL. 

The login package provides the UI for login workflow. The UI is provided as an Android ``Fragment`` called ``CBLLoginFragment``. The package also provides the View Model called ``CBLLoginViewModel`` which can be used independently for building a different flavor of Login UI.

At present this package only provides UI/ViewModel for CBL (Code Based Linking) authentication.

## Integration
### Gradle
Include the project with gradle.

### Dependencies
Please refer to alexa-auto-apis doc to find details on how to fetch/publish dependencies.

``CBLLoginViewModel`` needs access to following implementations (interfaces for them are defined in ``alexa-auto-apis`` package):
* ``AuthController``: This interface provides business logic for new authentication workflow. The interface must be made available from ``AlexaAppRootComponent``.
* ``LoginUIEventListener [Optional]``: This interface allows UI to let the observer know when login is finished. This event can be used by observer to progress the app UI to logged-in state. The interface should be made available through ``AlexaAppLoggedOutScopedComponent``, that should be made available through ``AlexaAppRootComponent#getScopedComponents()``