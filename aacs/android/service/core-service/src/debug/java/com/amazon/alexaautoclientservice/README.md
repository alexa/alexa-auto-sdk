# AACS Instrumentation
AACS instrumentation enables you to better understand the interactions between your application and AACS. Through instrumentation, you log Alexa Auto Service Bridge (AASB) messages to a file, which you can review for debugging purposes. You can start or stop instrumentation at any time and customize the log file location. 

To use AACS instrumentation, you must use the debug option when building the Alexa Auto SDK with AACS.

<!-- omit in toc -->
## Table of Contents
- [Advantages of Using Instrumentation](#advantages-of-using-instrumentation)
- [Information Captured by Instrumentation](#information-captured-by-instrumentation)
- [Using Instrumentation](#using-instrumentation)
- [Sample Log File](#sample-log-file)

## Advantages of Using Instrumentation

Without instrumentation, you can use the Android `Log()` method with the `android.util.Log` class to monitor the interactions between your application and AACS. To view the resultant logs, use the Android `Logcat` utility. The information captured in this way does not include the details of each AASB message. To view the message details, use instrumentation. For example, you can view the message payload in the instrumentation log, which `Logcat` does not display to prevent privacy violations.

Instrumentation produces logs that are more relevant for debugging your application, as the logs provided by `android.util.Log` often include Android system logs. Also, instrumentation captures all the messages exchanged among the AACS core service, AASB Message Broker interface, and your application. If you use the `Log()` method, you would need to define the type of message to be logged, the source of the message, and so on.

Using a text editor, you can view instrumentation log entries. If you want to view them in a tabular format, use a web tool, such as [json2table](http://json2table.com/).

## Information Captured by Instrumentation
The following list describes the information in each instrumentation log entry:

* Time stamp for message arrival at AACS or message dispatch from AACS
* Message direction (e.g., "FROM_ENGINE" or "TO_ENGINE")
* Topic for the message (e.g., Navigation, CarControl)
* Action (e.g., `EndOfSpeechDetected`)
* Message ID
* Whether the message expects a reply
* Payload (e.g., `{"wakeword":"ALEXA"}`)
* If the message is a reply, whether the reply is sent within the timeout period
* If the message is a reply, the ID of the message that requests the reply

## Using Instrumentation
To manage instrumentation, use the Android activity manager through the Android Debug Bridge. Enter the `adb shell am` command as described in the following list:

* To start instrumentation, enter the following command:
  
    `adb shell am broadcast -a com.amazon.aacs.startinstrumentation -n com.amazon.alexaautoclientservice/.receiver.InstrumentationReceiver`

* To stop instrumentation, enter the following command:
  
    `adb shell am broadcast -a com.amazon.aacs.stopinstrumentation -n com.amazon.alexaautoclientservice/.receiver.InstrumentationReceiver`

* To specify the log file location, enter the following command. In this example, the log file location is `/sdcard/Log/aacs.log`.
  
    `adb shell am broadcast -a com.amazon.aacs.startinstrumentation -n com.amazon.alexaautoclientservice/.receiver.InstrumentationReceiver  --es fileLocation "/sdcard/Log/aacs.log"`

    By default, the location is `/sdcard`. 

## Sample Log File
The following is an instrumentation log file example:

~~~
{"timeStamp":"2021-03-04 14:30:06.989","MessageDirection":"FROM_ENGINE","topic":"SpeechRecognizer","action":"WakewordDetected","messageId":"4f1bcd5d-098a-43dc-9447-340d69c75f2f","replyExpected":false,"payload":"{\"wakeword\":\"ALEXA\"}"}
{"timeStamp":"2021-03-04 14:30:12.523","MessageDirection":"FROM_ENGINE","topic":"AudioOutput","action":"GetPosition","messageId":"0475aa45-050b-43a5-b943-d89254cc0261","replyExpected":true,"payload":"{\"channel\":\"SpeechSynthesizer\",\"token\":\"d3565d16-e85a-4b4d-b18e-bcbcee2374d3\"}"}
{"timeStamp":"2021-03-04 14:30:13.517","MessageDirection":"TO_ENGINE","topic":"AudioOutput","action":"GetPosition","replyReceivedTimeout":false,"replyToId":"e283cecc-7a9b-42d4-b164-ef0ee63a4f91","payload":"{\"position\":489}"}
~~~