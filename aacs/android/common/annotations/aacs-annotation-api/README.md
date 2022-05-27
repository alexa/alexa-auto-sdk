# AACS Annotation Documentation

## aacs-annotation-api

This library contains all the annotation classes and must be included in the Gradle file to access the annotations in the code.

### **ContextBroadcastReceiver**

AACS has introduced this annotation to optimize many foreground services used in the code. Some functionalities like the navigation provider will need to register broadcast receivers for receiving third-party proprietary framework intents, android intents, or third-party Auto SDK intents to be notified of the system change. 

Due to android imposed [limitations](https://developer.android.com/guide/components/broadcasts#manifest-declared-receivers), the implementation may not declare those intent filters in the manifest file. Creating a foreground service to register the broadcast receiver and running it forever is expensive. Unfortunately, it may be unavoidable in some cases e.g. receiving navigation state from navigation application. The `@ContextBroadcastReceiver` annotation is introduced to help solve this problem. Declaring a class with this annotation registers the class object and intent filter in the `AlexaAutoClientService` ,which is the backbone service of the app, throughout the lifecycle of the service. Individual app components do not need to create a separate foreground service for this purpose.

**Note:** All the AASB-related intent filters must follow its documentation. They either need to be Services and Manifest declared Broadcast Receivers or Activities. This annotation does not change the behavior. But this annotation should be used whenever you want to implement the context registered Broadcast Receivers.

This annotation can be used in the class as shown in the following example.
<pre><code>@ContextBroadcastReceiver(categories = {NaviProviderConstants.CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION},
        actions = {NaviProviderConstants.ACTION_RESPONSE_SHOWED_ALTERNATE_ROUTES,
                NaviProviderConstants.ACTION_RESPONSE_NAVIGATION_STATE,
                NaviProviderConstants.ACTION_RESPONSE_NAVIGATION_FAVORITES,
                NaviProviderConstants.ACTION_SIGNAL_NAVIGATION_ERROR,
                NaviProviderConstants.ACTION_SIGNAL_NAVIGATION_EVENT})
public class NaviObserver extends BroadcastReceiver {

    private static NaviObserver INSTANCE;

    private NaviObserver() {}

    public static NaviObserver getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new NaviObserver();
        }
        return INSTANCE;
    }
    
}
</code></pre>

### **Optional `categories`:** 

Array of the name of the category to match, such as `NaviProviderConstants.CATEGORY_ALEXA_AUTO_CLIENT_NAVIGATION` is required in the intent filter.

### **Mandatory `actions` :** 
Array of the action to match, such as `NaviProviderConstants.ACTION_RESPONSE_NAVIGATION_STATE` is required in the intent filter.

### **Optional `permission`:** 
String naming permissions that a broadcaster must hold to send an Intent to you. Don’t provide it if no permission is required.

### Inherit from `BroadcastReceiver`:
The annotated class must inherit class `android.content.BroadcastReceiver` to ensure that the receiver class is qualified for receiving the intents.

### **Singleton Class:**
AACS implementation expects a singleton implementation of the `BroadcastReceiver` class and calls the `getInstance` method. This method ensures that only one object is active and multiple intents are not received unnecessarily. Client implementation and registered receiver always refer to the same object.

### **Metadata in the manifest file:**
Declare the following metadata in the AndroidManifest.xml file. `AlexaAutoClientService` scans all the metadata declarations from the APK, refers to annotation auto-generated reference classes in the given packages, and register and unregister all the broadcast receivers declared with annotation `@ContextBroadcastReceive` in `onCreate` and `onDestroy` service methods respectively.

**For example:** `AlexaAutoClientService` gets package `com.amazon.alexa.auto.navigation.providers.external` from the metadata, refers the auto generated class `com.amazon.alexa.auto.navigation.providers.external.ContextBroadcastReceivers` and register BroadcastReceiver `com.amazon.alexa.auto.navigation.providers.external.NaviObserver` for the declared actions, categories and permissions.

```
<meta-data android:name="com.amazon.alexa.auto.navigation.providers.external" android:value="package"/>
```

**Note:** `android:name` is **key**, and `android:value` is **value**. Value `package` is mandatory. Please do not duplicate the key because it can replace the earlier metadata. android:name must provide the package name of the Broadcast Receiver class.

**`@NaviProviderModule`**

This annotation is required to declare your class as a navigation provider. If you are a leading map provider and want to create a prebuilt AAR compatible with AACS and your proprietary navigation application, you can implement the `NavigationProviderModule`.

#### Example:
<pre><code>
@NaviProviderModule(enabled = true)
public class GoogleMapsNaviProvider implements NaviProvider {

    private static GoogleMapsNaviProvider INSTANCE;

    private GoogleMapsNaviProvider() {
        // For singleton
    }

    public static GoogleMapsNaviProvider getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new GoogleMapsNaviProvider();
        }
        return INSTANCE;
    }

}
</code></pre>
### **enabled = true**

During build time, you can disable navigation provider by setting `enabled = false` or eliminating the `enabled` variable. This implementation is expecting only one navigation provider enabled. If no provider is included or no provider is enabled, by default `DefaultNaviProvider` is used as a active navigation provider.

### **Inherit `NaviProvider` interface**

Navigation provider must inherit the `com.amazon.alexa.auto.navigation.providers.NaviProvider` interface.  Navigation app component provides weak reference of the context object through `public void initialize(WeakReference<Context> weakContext, NaviResponseReporter reporter)` method. This Context can be used for broadcasting the intents. Refer to the ContextBroadcastReceiver section for receiving the intents. Provide all the asynchronous responses to the navigation app components using `NaviResponseReporter` object. Follow all the payload documentation to ensure that correct data is provided as quickly as possible. 

### **Singleton Implementation**
Navigation app component implementation expects a singleton implementation of the NaviProvider class and calls `getInstance` method. This method ensures that only one object is active, and the same is used by client implementation and the AACS navigation app component.


