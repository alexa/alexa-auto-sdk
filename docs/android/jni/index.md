# Alternative JNI Integration

Your Android application should use the Alexa Auto Client Service (AACS) that provides an Auto SDK API that's better suited for an Android environment than the native core Auto SDK API. Additionally, AACS provides prebuilt feature integrations that are not available for you to use in your application if you do not use AACS. However, if you want to build an Auto SDK integration using the Message Broker API directly, Auto SDK provides JNI interfaces corresponding to the core native C++ API.

## JNI Class Reference

See the [JNI Class Reference](./classes/index.html) for detailed information about the JNI API including the `Engine`, `MessageBroker`, and configuration factory classes.