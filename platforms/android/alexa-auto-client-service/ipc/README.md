# Using the IPC Library with AACS

## Overview
The IPC (inter-process communication) library implements the IPC protocol required for communications between an HMI (human-machine interface) application
and AACS (Alexa Auto Client Service).  The library enables AACS to send and receive arbitrary types of payloads of
arbitrary sizes.  The library is implemented using standard Android constructs. You can use it as-is or
as a reference when implementing IPC for your app.

## Getting Started with the IPC Library
**NOTE**: This section assumes that you have completed the steps for building and installing AACS.

To build the IPC library, follow these steps:
1) Enter the following commands:
   ~~~
   cd $AACS_HOME/ipc
   gradle assembleDebug
   ~~~
   The `aacsipc-debug.aar` file is built.

1) Copy `aacsipc-debug.aar` to the `libs` folder of your app.

## API Usage Guide

The IPC library consists of two main classes for apps to interface with: [AACSSender](*AACSSender) 
and [AACSReceiver](*AACSReceiver).  For an additional in-code example that illustrates how an app uses the IPC library, see an AACS Android-service project.

### AACSSender <a id ="AACSSender"></a>

`AACSSender` enables an application to send data to another application that 
uses [AACSReceiver](*AACSReceiver).

**Initialization** - When `AACSSender` initializes, it  instantiates an `AACSSender` object as follows:

```java
    // AACSSender posts callbacks to the looper specified in the constructor
    // argument.  If none is provided, it uses the mainlooper.
    AACSSender mAACSSender = new AACSSender(Looper.getMainLooper());
```

**Sending a message** - `AACSSender` sends non-streaming data, such as an AASB (Alexa Auto Service Bridge) JSON message, to AACS as follows:

```java    
    // replace with the entire aasb json message
    String aasbMessage = "...";

    // replace with topic of the aasb message
    String aasbTopic = "...";

    // replace with action of the aasb message
    String aasbAction = "...";

    // can also be a list of multiple targets List<TargetComponent>
    TargetComponent target = TargetComponent.withComponent(new ComponentName(
        "com.amazon.alexaautoclientservice", "com.amazon.alexaautoclientservice.AlexaAutoClientService"), 
        TargetComponent.Type.SERVICE);

    Context context = getApplicationContext();

    mAACSSender.sendAASBMessageAnySize(aasbMessage,  aasbAction, aasbTopic, target(s), context);
```

**Fetching data from AACS** - To stream data to the application, such as audio data,
AACS first sends the application a `streamId` (for example, the AASB message `AudioOutput/Prepare`).  The application
then requests the stream associated with that `streamId` from AACS through a `fetch` function as follows:

```java
    // streamId previously extracted from the aasb message AudioOutput/Prepare>
    String streamId = "...";

    Context context = getApplicationContext();

    TargetComponent target = TargetComponent.withComponent(new ComponentName(
        "com.amazon.alexaautoclientservice", "com.amazon.alexaautoclientservice.AlexaAutoClientService"), 
        TargetComponent.Type.SERVICE);

    AACSSender.StreamFetchedFromReceiverCallback fetchCallback  = (readPipe) -> {
        // readPipe is a ParcelFileDescriptor
        // inputStream is a ParcelFileDescriptor.AutoCloseInputStream that can be created from the given ParcelFileDescriptor
        ParcelFileDescriptor.AutoCloseInputStream inputStream = new ParcelFileDescriptor.AutoCloseInputStream(readPipe);

        // this callback gets triggered on the looper provided during construction.
        // If this callback is triggered on the mainlooper, it is advised
        // to delegate the work to a different thread to avoid
        // triggering an ANR (application not responding) error.   
                
        // perform application logic as necessary, reading the stream
        // (you could read the stream here, or queue it onto a different thread, etc)
        
        // once finished with the stream, you must close it.  
        // (if you queue it onto a different thread, close it there)
        inputStream.close();
};

mAACSSender.fetch(streamId, fetchCallback, target, context);
```


**AACS fetching data from OEM application** - If the default audio input platform handler is enabled and an external audio stream source is used, when AACS receives a `StartAudioInput` or `StopAudioInput` message, it sends an IPC fetch request or an IPC cancel fetch request. This request fetches or cancels fetching the audio input stream from the application. The application must implement `AACSReceiver.FetchStreamCallback`, provide the write pipe associated with the audio stream in `onStreamRequested(String streamId, ParcelFileDescriptor writePipe)` method, and stop providing the stream in `onStreamFetchCancelled(String streamId)` method, as shown in the following example:

~~~java
        AACSReceiver.FetchStreamCallback aasbFetchCallback = new AACSReceiver.FetchStreamCallback() {
            @Override
            public void onStreamRequested(String streamId, ParcelFileDescriptor writePipe) {
                // Save the streamId and create a stream using the writePipe
                ParcelFileDescriptor.AutoCloseOutputStream stream = new ParcelFileDescriptor.AutoCloseOutputStream(writePipe);

                // Start writing your streamed data to the ParcelFileDescriptor.AutoCloseOutputStream

                // Close stream and pipe as needed when streaming is done
            }

            @Override
            public void onStreamFetchCancelled(String streamId) {
                // Stop writing to the ParcelFileDescriptor.AutoCloseOutputStream 
                // associated with the given streamId
            }
        }
~~~

**Pushing data to AACS** - To receive streamed data from the application, such as 
microphone data, AACS first sends the application a `streamId` (for example, the AASB message `AudioInput/StartAudioInput`).
The application then sends a stream associated with AACS together with `streamId` to indicate which request 
it's fulfilling through the `push` function as shown in the following code:

```java
    // streamId previously extracted from the aasb message AudioInput/StartAudioInput>
    String streamId = "...";

    Context context = getApplicationContext();

    TargetComponent target = TargetComponent.withComponent(new ComponentName(
        "com.amazon.alexaautoclientservice", "com.amazon.alexaautoclientservice.AlexaAutoClientService"), 
        TargetComponent.Type.SERVICE);

    AACSSender.PushToStreamIdCallback pushCallback = (streamId, writePipe) -> {
        // writePipe is a ParcelFileDescriptor
        // outputStream is a ParcelFileDescriptor.AutoCloseOutputStream that can be created from the given ParcelFileDescriptor
        ParcelFileDescriptor.AutoCloseOutputStream outputStream = new ParcelFileDescriptor.AutoCloseOutputStream(writePipe);

        // this callback gets triggered on the looper provided during construction.
        // If this callback is triggered on the mainlooper, it is advised
        // to delegate the work to a different thread to avoid
        // triggering an ANR error.   

        // copy microphone data to outputStream

        //once finished with the stream, you must close it.
        // (if you queue it onto a different thread, close it there)        
        outputStream.close();
    };

    mAACSSender.push(streamId, pushCallback, target, context);
```
**Sending configuration data to AACS** - The application can send configuration data to AACS as follows:

```java
    String configMessage = "{\n" +
                "  \"configFilepaths\" : [\"" + path + "\"]," +
                "  \"configStrings\" : []" +
                "}";

    TargetComponent target = TargetComponent.withComponent(new ComponentName(
        "com.amazon.alexaautoclientservice", "com.amazon.alexaautoclientservice.AlexaAutoClientService"), 
        TargetComponent.Type.SERVICE);

    mAACSSender.sendConfigMessageAnySize(configMessage, target, getApplicationContext());
```

`configMessage` must include the paths to the configuration file or the config strings.


### AACSReceiver <a id ="AACSReceiver"></a>

`AACSReceiver` enables an application to receive data from another application that 
uses [AACSSender](*AACSSender).

**Initialization** - Most of the logic for `AACSReceiver` is specified in callbacks.  `AACSReceiver` is initialized  
by a builder.  Callbacks must be provided to the builder before the builder creates `AACSReceiver`. The following code shows how a builder creates `AACSReceiver`:

```java
    AACSReceiver.Builder builder = new AACSReceiver.Builder();
    AACSReceiver receiver = builder

                // this step is optional.  If no looper is provided,
                // the main looper is used by default.
                // .receive() and .shutdown() must be called from the indicated 
                // looper.  All callbacks that AACSReceiver calls or invokes are also posted
                // to that looper.
                .withLooper(Looper.Looper.getMainLooper)
                
                // this callback is invoked when the receiver gets a non-streamed message,
                // such as an AASB message.
                .withAASBCallback((message) -> {
                    // Perform application logic based 
                    // on the AASB message in messageString.
                }).build();
```


**Receiving from the sender** - `AACSReceiver` doesn't create any direct intent receiver, which can be a service, activity, or broadcast receiver. The `AACSReceiver` relies
on the service, activity, or broadcast receiver in the app to forward intents to it.  The following code is an example illustrating how an activity overrides `onNewIntent` and sends an intent to be received by `AACSReceiver`:
    
```java    
    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);

        setIntent(intent);

        // Intents that are sent by AACS should be received here.
        mAACSReceiver.receiveMessage(intent);
    }
```
