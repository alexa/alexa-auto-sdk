# Alexa Auto UX Restrictions

The following list describes the purposes of this library:

* It provides `CarUxRestrictionsModule` and controls `CarUxRestrictionsController` life cycle.
  * `CarUxRestrictionsModule` is responsible for broadcasting driving state changes as intents to other app components. Current usecases for this module include the `APL` module and the `setup` module.
* It provides default implementation for `CarUxRestrictionsController`, which initializes Android `CarUxRestrictionsManager`, registers and unregisters `OnUxRestrictionsChangedListener`. To use the default implemetation, it requires [Android car package](https://developer.android.com/reference/android/car/package-summary) exists on the device. OEM can also provide their own implementation for the car UX restrictions updates.

This app component requires Android API 29. To build this component with AACS sample app, add the `-PenabledUXRestrictions` to your build command:
```
./gradlew assembleLocalRelease -PenabledUXRestrictions
```