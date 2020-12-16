# Alexa Auto API Package

This Alexa Auto API Package provides:
* Types that are used across multiple Java packages. A Java package is a collection of related types, which is created to avoid type name collisions. 
* Interfaces that allow packages to communicate with each other by using standard Java, as long as the consumer and provider of the interface meet these requirements:
  * They are in the same Android Package (APK).
  * They are loaded and used in the same process.

## Component Registry (Service Locator)
To enable a package to locate the implementation of an API, the Alexa Auto API Package defines the component registry interfaces and the mechanism to obtain the component registry (also called the service locator).  
  
### Consuming Implementations from Other Packages

The following list explains the component registry interfaces:
* `AlexaAppRootComponent` is a component registry interface with an application scope. It provides interfaces that are in scope for the lifetime of the app. This interface provides access to `AlexaAppScopedComponents`, among other interfaces. 
* `AlexaAppScopedComponents` provides interfaces that are available for a limited scope. For example, when an app is in logged-off state,  `AlexaAppLoggedOutScopedComponent` can be queried by using `AlexaAppScopedComponents`.

Any library or application class can obtain `AlexaAppRootComponent` as long as it has the Android context. The following code example illustrates how an app obtains `AlexaAppRootComponent`:

```
class MyActivity extends AppCompatActivity {

    public void onStart() {
        AlexaApp app = AlexaApp.from(this);
        AlexaAppRootComponent componentRegistry = app.getRootComponent();
        componentRegistry.getXYZ().doSomethingUseful();
    }
}
```

### Publishing Implementations for Other Packages
How a package publishes the implementation of an API for another package to use depends on the scope, as explained in the following list:

* App lifecycle implementation: If an object's lifecycle is bound to the lifecycle of an app, then the main Alexa app APK creates an instance of the object and makes it available through the implementation of `AlexaAppRootComponent`.
* Limited scoped implementations: A package can publish scoped components into the component registry to be discovered by other packages. To publish a scoped component, the package can obtain `AlexaAppScopedComponentsActivator` from `AlexaAppRootComponent`.
