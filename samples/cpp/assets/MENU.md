# C++ Sample App Menu System

This document outlines the menu system that drives the C++ Sample App.

- [C++ Sample App Menu System](#c-sample-app-menu-system)
  - [Menu File<a id="menu-file"></a>](#menu-file)
  - [Menu Object<a id="menu-object"></a>](#menu-object)
  - [Menu Item Object<a id="menu-item-object"></a>](#menu-item-object)
  - [Menu Actions<a id="menu-actions"></a>](#menu-actions)
    - [AudioFile<a id="audiofile"></a>](#audiofile)
    - [GoBack<a id="goback"></a>](#goback)
    - [GoTo<a id="goto"></a>](#goto)
    - [Help<a id="help"></a>](#help)
    - [Login<a id="login"></a>](#login)
    - [Logout<a id="logout"></a>](#logout)
    - [Quit<a id="quit"></a>](#quit)
    - [Restart<a id="restart"></a>](#restart)
    - [Select<a id="select"></a>](#select)
    - [SetLocale<a id="setlocale"></a>](#setlocale)
    - [SetLoggerLevel<a id="setloggerlevel"></a>](#setloggerlevel)
    - [SetProperty<a id="setproperty"></a>](#setproperty)
    - [notify/*<a id="notify"></a>](#notify)
      - [Values](#values)
        - [onAudioManagerSpeaker](#onaudiomanagerspeaker)
        - [onConversationsReport](#onconversationsreport)
        - [onMessagingEndpointConnection](#onmessagingendpointconnection)
        - [onMessagingEndpointPermission](#onmessagingendpointpermission)
        - [onSendMessageStatus/onUpdateMessagesStatus](#onsendmessagestatusonupdatemessagesstatus)

## Menu File<a id="menu-file"></a>

A JSON format menu file describes the C++ Sample App interactive text based menu system. The menu file contains a single menu object (object format) or an array of menu objects (array format). The array format enables menus to be connected by id, and the object format enables a hierarchical menu structure.

Array format example:

```json
[
    {
        "id": "main",
        "item": [
            {
                "do": "GoTo",
                "key": "S",
                "name": "Settings",
                "value": "settings"
            }
        ],
        "name": "Main Menu"
    },
    {
        "id": "settings",
        "item": [ ... ],
        "name": "Settings Menu"
    }
]
```

Object format example:

```json
{
    "id": "main",
    "item": [
        {
            "do": "GoTo",
            "key": "S",
            "name": "Settings",
            "value": {
                "id": "settings",
                "item": [ ... ],
                "name": "Settings Menu"
            }
        }
    ],
    "name": "Main Menu"
}
```

>**Note:** `main` is a special menu identifier - the application loads and runs the `main` menu at runtime.

## Menu Object<a id="menu-object"></a>

| Property  | Type      | Condition |
| --------- | --------- | --------- |
| id        | `string`  | required  |
| index     | `integer` | optional  |
| item      | `array`   | required  |
| name      | `string`  | required  |
| text      | `object`  | optional  |

For example:

```json
{
    "id": "main",
    "item": [
        {
            "do": "Help",
            "key": "?",
            "name": "Print the current menu"
        },
        {
            "do": "Restart",
            "key": "R",
            "name": "Restart application"
        },
        {
            "do": "Quit",
            "key": "Q",
            "name": "Quit application"
        }
    ],
    "name": "Main Menu"
}
```

>**Note:** The global key options *escape* (`esc`), *help* (`?`), *quit* (`Q`), *stop* (`X`), and *talk* (`space`) are available on all menus.

**id \<string>**

A unique menu identifier. Special identifiers include `main`, `LOCALE`, and `LOCATION`.

**index \<integer>**

A variable for the [Select](#select) action. Set the zero-based index of the item to select, or set to `-1` for no selection. The application manages the index variable as the user makes selections. The variable is not saved when the application quits.

**item \<array>**

A collection of menu item objects.

**name \<string>**

The name of the menu to print.

**text \<object>**

A map of text items to print in the application. The main menu expects the following items:
- `banner` - welcome screen text, including copyright statement
- `keyTapError` - text to print when an invalid key is tapped
- `keyTapped` - text to print when a valid key is tapped

## Menu Item Object<a id="menu-item-object"></a>

| Property  | Type      | Condition |
| --------- | --------- | --------- |
| do        | `string`  | required  |
| key       | `string`  | required  |
| name      | `string`  | required  |
| note      | `string`  | optional  |
| test      | `string`  | optional  |
| value     | `any`     | optional  |

**do \<string>**

The [action](#menu-actions) to perform when the associated menu item key is tapped.

**key \<string>**

The key character associated with the menu item. Special key values include `esc` and `space`.

**name \<string>**

The name of the menu item to print.

**note \<string>**

Optional note to print in the application console after the menu item has been selected.

**test \<string>**

Optional test to determine if the menu item should be included or not.

**value \<any>**

The value of the menu item. The value type is determined by the associated action.

## Menu Actions<a id="menu-actions"></a>

### AudioFile<a id="audiofile"></a>

The **AudioFile** action sends the specified audio file to Alexa as if it were an utterance. The audio file path is relative to the menu file. To extend this menu, save your audio files under `assets/inputs/` and add menu items that point to those files.

>**Note:** The AudioFile menu appears only on platforms that do not provide built-in audio support (such as platforms that are under development). On platforms that provide built-in audio support, the AudioFile menu does not appear.

For example:

```json
{
    "do": "AudioFile",
    "key": "1",
    "name": "Alexa, tell me a joke",
    "value": "./inputs/alexa_tell_me_a_joke.wav"
}
```

>**Note:** Audio files must be 16KHz mono, 16-bit little-endian in PCM WAV format. Make sure that the audio starts with the `Alexa` wakeword.

### GoBack<a id="goback"></a>

The **GoBack** action pops the previous menu from the menu stack and prints it. Nothing happens if the stack is empty.

For example:

```json
{
    "do": "GoBack",
    "key": "esc",
    "name": "Go back"
}
```

### GoTo<a id="goto"></a>

The **GoTo** action pushes the current menu onto the menu stack, then prints the menu specified in the value. The value can be a menu object description or an identifier for a menu object described elsewhere.

For example:

```json
{
    "do": "GoTo",
    "key": "G",
    "name": "Geolocation",
    "value": {
        "id": "LOCATION",
        "index": 0,
        "item": [
            {
                "do": "Select",
                "key": "H",
                "name": "Use home location",
                "value": {
                    "latitude": 37.3790629,
                    "longitude": -122.116578
                }
            },
            {
                "do": "Select",
                "key": "W",
                "name": "Use work location",
                "value": {
                    "latitude": 37.40940725,
                    "longitude": -122.03644610148677
                }
            },
            {
                "do": "GoBack",
                "key": "esc",
                "name": "Go back"
            }
        ],
        "name": "Geolocation Menu"
    }
},
{
    "do": "GoTo",
    "key": "L",
    "name": "Locale",
    "value": {
        "id": "LOCALE",
        "item": [
            {
                // supported-locale-1
            },
            {
                // supported-locale-2
            },
            {
                // supported-locale-3
            },

            ...

            {
                "do": "GoBack",
                "key": "esc",
                "name": "Go back"
            }
        ],
        "name": "Locale Menu"
    }
}
```

>**Note:** `LOCALE` is a special menu identifier. `LOCATION` is used by the `LocationProviderHandler::getLocation()` function.

### Help<a id="help"></a>

The **Help** action prints the current menu.

For example:

```json
{
    "do": "Help",
    "key": "?",
    "name": "Print the current menu"
}
```

### Login<a id="login"></a>

***(Experimental)*** The **Login** action sets the user config file path and restarts the application (without confirmation). The application will stop and shut down, then reload and configure, register platform interfaces, and start the Engine. The user config file path is added to the configuration files.

For example:

```json
{
    "do": "Login",
    "key": "A",
    "name": "Login User A",
    "value": "./user-a.json"
},
{
    "do": "Login",
    "key": "B",
    "name": "Login User B",
    "value": "./user-b.json"
}
```

>**Note:** In this example, the main config file (config.json) contains device information and general settings, and the user config files (user-a.json and user-b.json) contain custom settings (e.g. database paths) for individual users A and B. The user config file paths are relative to the menu file.

### Logout<a id="logout"></a>

***(Experimental)*** The **Logout** action logs out the current user and restarts the application (with confirmation). The application will stop and shut down, then reload and configure, register platform interfaces, and start the Engine. The user config file path and authentication refresh token are both cleared.

For example:

```json
{
    "do": "Logout",
    "key": "O",
    "name": "Logout"
}
```

### Quit<a id="quit"></a>

The **Quit** action quits the application (without confirmation). For security reasons, authentication is not preserved.

For example:

```json
{
    "do": "Quit",
    "key": "Q",
    "name": "Quit application"
}
```

### Restart<a id="restart"></a>

The **Restart** action restarts the application (with confirmation). The application will stop and shut down, then reload and configure, register platform interfaces, and start the Engine. The user config file path and authentication refresh token are both preserved.

For example:

```json
{
    "do": "Restart",
    "key": "R",
    "name": "Restart application"
}
```

### Select<a id="select"></a>

The **Select** action sets the zero-based index of the selected item. The application platform interfaces use the value of the select item at runtime (see `LocationProviderHandler::getLocation()`). The variable is not saved when the application quits.

For example:

```json
{
    "do": "Select",
    "key": "H",
    "name": "Use home location",
    "value": {
        "latitude": 37.3790629,
        "longitude": -122.116578
    }
}
```

### SetLocale<a id="setlocale"></a>

The **SetLocale** action sets the [Alexa locale](https://developer.amazon.com/docs/alexa-voice-service/settings.html) setting used by the application.

For example:

```json
{
    "do": "SetLocale",
    "key": "1",
    "name": "en-US",
    "value": "en-US"
}
```

### SetLoggerLevel<a id="setloggerlevel"></a>

The **SetLoggerLevel** action sets the application logger level for filtering log output. This is separate from the build time options for setting the default logger level and sink.

For example:

```json
{
    "do": "SetLoggerLevel",
    "key": "V",
    "name": "VERBOSE",
    "value": "VERBOSE"
}
```

Supported logger levels:
- `VERBOSE`
- `INFO`
- `METRIC`
- `WARN`
- `ERROR`
- `CRITICAL`

>**Note:** Logger levels are not enabled by default. Use the menu or application `--level` option to specify a logger level.

### SetProperty<a id="setproperty"></a>

The **SetProperty** action sets a property value in the application Engine.

For example:

```json
{
    "do": "SetProperty",
    "key": "1",
    "name": "en-US",
    "value": "aace.alexa.setting.locale/en-US"
}
```

Supported properties:
- `aace.alexa.setting.locale`

### notify/*<a id="notify"></a>

The **notify/*** action exercises the application platform interfaces with event/value notifications.

>**Note:** Values within `[]` are optional.

| Event                                       | Value
| ------------------------------------------- | -------------------------------------------
| onStopActive                                | -
| **Alerts**                                  |
| onAlertsLocalStop                           | -
| onAlertsRemoveAllAlerts                     | -
| **AudioManager**                            |
| onAudioManagerSpeaker                       | `identity/name/value`
| **Communications**                          |
| onCommunicationAcceptCall                   | -
| onCommunicationStopCall                     | -
| onCommunicationShowDisplayInfo              | -
| onCommunicationShowState                    | -
| **Connectivity**                            |
| onConnectivityConnectivityStateChange       | `json`
| **DoNotDisturb**                            |
| onDoNotDisturbChanged                       | -
| **Logger**                                  |
| onLoggerLog                                 | `level/tag/message`
| **Messaging**                               |
| onConversationsReport                       | `path`
| onMessagingEndpointConnection               | `state`
| onMessagingEndpointPermission               | `permission/value`
| onSendMessageStatus                         | `[code/message]`
| onShowMessagingInfo                         | -
| onToggleAutomaticResponses                  | -
| onUpdateMessagesStatus                      | `[code/message]`
| **NetworkInfoProvider**                     |
| onNetworkInfoProviderNetworkStatusChanged   | `status[/wifiSignalStrength]`
| **PhoneCallController**                     |
| onPhoneCallControllerConnectionStateChanged | `state`
| onPhoneCallControllerCallStateChanged       | `state[/callId[/callerId]]`
| onPhoneCallControllerCallFailed             | `callId/code/message`
| onPhoneCallControllerCallerIdReceived       | `callId/callerId`
| onPhoneCallControllerSendDTMFSucceeded      | `callId`
| onPhoneCallControllerSendDTMFFailed         | `callId/code/message`
| onPhoneCallControllerShowPayload            | -
| **PlaybackController**                      |
| onPlaybackControllerButtonPressed           | `button`
| onPlaybackControllerTogglePressed           | `toggle/action`
| **SpeechRecognizer**                        |
| onSpeechRecognizerHoldToTalk                | -
| onSpeechRecognizerTapToTalk                 | -
| onSpeechRecognizerStartCapture              | `initiator`
| onSpeechRecognizerStopCapture               | -
| onSpeechRecognizerEnableWakewordDetection   | -
| onSpeechRecognizerDisableWakewordDetection  | -
| onSpeechRecognizerStartStreamingAudioFile   | [audio file path](#audiofile)
| onSpeechRecognizerStopStreamingAudioFile    | -
| **TextToSpeech**                            |
| onPrepareSpeech                             | `text/SSML`
| onGetCapabilities                           | -
| **AlexaClient**                             |
| onStopForegroundActivity                    | -

For example:

```json
{
    "do": "notify/onSpeechRecognizerHoldToTalk",
    "key": "B",
    "name": "Begin Capture (for Hold-to-Talk)",
    "note": "Tap ${KEYOPEN}S${KEYCLOSE} to stop capturing"
},
{
    "do": "notify/onSpeechRecognizerStopCapture",
    "key": "S",
    "name": "Stop Capture"
}
```
#### Values

##### onAudioManagerSpeaker

| Field               | Values
| ------------------- | ---------------------------------
| identity            | Speaker, Alerts
| name                | volume, mute
| value               | true, false, +/-*number*, number

##### onConversationsReport

| Field               | Values
| ------------------- | ---------------------------------
| path                | File path to file containing sample conversations

##### onMessagingEndpointConnection

| Field               | Values
| ------------------- | ---------------------------------
| state               | CONNECTED, DISCONNECTED (see `Messaging::ConnectionState`)

##### onMessagingEndpointPermission

| Field               | Values
| ------------------- | ---------------------------------
| permission          | SEND, READ
| value               | ON, OFF  (see `Messaging::PermissionState`)

##### onSendMessageStatus/onUpdateMessagesStatus

| Field               | Values
| ------------------- | ---------------------------------
| code                | GENERIC_FAILURE, NO_CONNECTIVITY, NO_PERMISSION (see `Messaging::ErrorCode`)
| message             | A string with the error code message


##### onPrepareSpeech

| Field               | Values
| ------------------- | ---------------------------------
| text/SSML           | A string/SSML used for speech synthesis