# ExternalMediaAdapter Interface

The External Media Player (EMP) Adapter allows you to declare and use external media application sources in your application. In order to interface with the EMP Adapter, you must use one of the following:

* A media connection client to interface the EMP Adapter to the external app. AACS provides an app component called the Media App Command and Control (MACC) client that provides most of the deep-linking integration specified below. The AACS Sample App supports an example using the Media App Command and Control (MACC) client to play the Android Spotify app. 
* An embedded media app. For information about external embedded media app solutions, contact your SA or Partner Manager.

>**Note:** If the media app service requires additional customer experience details, incorporate the requirement in your implementation. For example, if the provider requires your application to show the provider's logo in a particular way, modify the implementation to meet the requirement. 

When advised by your SA or Partner Manager, configure the External Media Player Adapter to the device's capabilities. See `aace::alexa::config::AlexaConfiguration::createExternalMediaPlayerConfig` for details on configuring the supported agent, or provide the equivalent JSON values in a configuration file.

```
{
    "aace.alexa": {
        "externalMediaPlayer": {
            "agent": "<agent>"
     
  }
}
```

You must register and implement each ExternalMediaAdapter (along with its associated external client or library). After the engine establishes a connection to the Alexa service, you can run discovery to validate each external media application. You can report discovered external media players by calling `reportDiscoveredPlayers()` at any point during runtime. When the Alexa service recognizes the player, you will get a call to the `authorize()` method including the player's authorization status. Both the `reportDiscoveredPlayers()` method and the `authorize()` method can contain one or more players in their JSON payloads. Validating the application enables Alexa to exercise playback control over the registered source type. 

The `login()` and `logout()` methods inform AVS of login state changes, if applicable. If your application has the ability to handle cloud-based login and logout, you should also call the `loginComplete()` and `logoutComplete()` methods where appropriate. 

When the user makes an Alexa voice request (for example, "Play Spotify"), the `play()` method is invoked. This method contains various parameters, including the player id of the player to which the playback information should be routed. 

Whether through voice or GUI event, the `playControl()` method is called with the relevant `PlayControlType`. Similar to `play()` the control should be routed to the appropriate player.

The `PlayControlType` is determined by player's `supportedOperations`, which are specified by your implementation in the return value of `getState()`. 

The `ExternalMediaAdapter` interface provides methods `playerEvent()` and `playerError()` for your implementation to report events regarding the state of the playback session managed by your external player. Even though your player manages its own playback, including reacting to on-device transport control button presses from the user and reacting appropriately to other non-Alexa audio events on the system, the `playerEvent()` and `playerError()` calls provide important information to the Engine:
    
1. The Engine may use calls to these methods to synchronize the state of your player’s playback session with Alexa.

2. The Engine may react to these calls according to the event name specified to update its internal view of your player’s state. Particular event names indicate if the player is focused on the system (meaning it has an active playback session) or if it is un-focused (meaning it is not in use and is brought into use only by further on-device interaction by the user or a user voice request to Alexa). The Engine uses this information to sync its internal focus management.

The tables below describe each supported event name and what it means to the Engine. Usage of these events depends on the particular type of player controlled by the `ExternalMediaAdapter` instance, so contact your Solutions Architect (SA) or Partner Manager for guidance regarding supported embedded and external app solutions.

| playerEvent() event name | Description |
|:--|:--|
| "PlaybackSessionStarted" | A new playback session has started, either from a GUI interaction or as a result of a user voice request to Alexa. The Engine considers the player active and in focus (although it may or may not yet be playing). |
| "PlaybackStarted" | During an active session, the player has started to play or resumed from a paused state. The Engine considers the player active and in focus. |
| "TrackChanged" | During an active session, one track has ended and another has started. The Engine uses this primarily for state reporting.  |
| "PlaybackNext" | During an active session, the player skipped from one track to the next track, either as a result of a GUI interaction or a user voice request to Alexa. The Engine uses this primarily for state reporting. |
| "PlaybackPrevious" | During an active session, the player skipped from one track to the previous track, either as a result of a GUI interaction or a user voice request to Alexa. The Engine uses this primarily for state reporting. |
| "PlayModeChanged" | During an active session, some user setting for the track or playback session changed, such as the favorite setting or the shuffle mode. The Engine uses this primarily for state reporting.|
| "PlaybackStopped" | During an active session, the player has paused or stopped, either as a result of a GUI interaction or a user voice request to Alexa. The Engine considers the player active and in focus, just not currently playing. User voice requests to resume still control the player. |
| "PlaybackSessionEnded" | An active playback session has ended. The player should no longer be playing or playable until a new session is started by GUI interaction or user voice request to Alexa. The Engine considers the player inactive and no longer in focus. |

| playerError() event name | Description |
|:--|:--|
| "INTERNAL\_ERROR" | Any fatal player error has occurred
| "UNKNOWN\_ERROR" | An unknown error occurred
| "UNPLAYABLE\_BY\_AUTHORIZATION" | The media couldn't be played due to an unauthorized account
| "UNPLAYABLE\_BY\_STREAM\_CONCURRENCY" | The media couldn't be played due to the number of accounts currently streaming
| "UNPLAYABLE\_BY\_ACCOUNT" | The media couldn't be played due to the account type
| "UNPLAYABLE\_BY\_REGION" | The media couldn't be played due to the current region
| "UNPLAYABLE\_BY\_PARENTAL\_CONTROL" | The media couldn't be played due to parental settings
| "UNPLAYABLE\_BY\_SUBSCRIPTION" | The media couldn't be played due to the subscription type 
| "OPERATION\_REJECTED\_UNINTERRUPTIBLE" | The operation could not be performed due to non interruptible media
| "OPERATION\_REJECTED\_END\_OF\_QUEUE" | The operation could not be performed due to the end of media being reached
| "OPERATION\_UNSUPPORTED" | The operation was not supported
| "OPERATION\_REJECTED\_SKIP\_LIMIT" | The operation failed because a skip limit was reached 
| "PLAYER\_UNKNOWN" | An unknown player was detected
| "PLAYER\_NOT\_FOUND" | The player was not discovered
| "PLAYER\_CONNECTION\_REJECTED" | The connection to the player failed
| "PLAYER\_CONNECTION\_TIMEOUT" | The connection to the player timed out

The `seek()` and `adjustSeek()` methods are invokable via Alexa if the currently in-focus external player supports them. `seek()` specifies an absolute offset, whereas `adjustSeek()` specifies a relative offset. 

The `volumeChanged()` and `mutedStateChanged()` methods are invoked to change the volume and mute state of the currently-focused external player. `volumeChanged()` specifies the new volume. `mutedStateChanged()` specifies the new `MutedState`.

The `getState()` method is called to synchronize the external player's state with the cloud. This method is used to maintain correct state during startup, and after every Alexa request. 

You construct the `ExternalMediaAdapterState` object using the data taken from the media app connection client or embedded player app (associated via `localPlayerId`) and return the state information.

The following table describes the fields comprising a `ExternalMediaAdapterState`, which includes two sub-components: `PlaybackState`, and `SessionState`.

| State        | Type           | Required | Notes  |
| :------------- |:-------------| :-----| :-----|
| **PlaybackState**      |
| state      | String        | Yes | "IDLE"/"STOPPED"/"PLAYING" |
| supportedOperations | SupportedPlaybackOperation[] | Yes | see SupportedOperation |
| trackOffset      | long  | No |  optional |
| shuffleEnabled      | boolean       |  Yes | report shuffle status |
| repeatEnabled      | boolean       |  Yes | report repeat status |
| favorites      | Favorites  | No | see Favorites |
| type      | String  |  Yes | must be set as "ExternalMediaPlayerMusicItem" |
| playbackSource      | String       | No |   If available else use local player name |
| playbackSourceId      | String  | No |  empty |
| trackName      | String   | No |  If available else use local player name |
| trackId      | String    |  No | empty |
| trackNumber      | String   | No | optional |
| artistName      | String    | No | optional |
| artistId      | String   |  No | empty |
| albumName      | String |  No | optional |
| albumId      | String | No |  empty |
| tinyURL      | String | No |  optional |
| smallURL      | String | No |  optional |
| mediumURL      | String | No |  optional |
| largeURL      | String | No |  optional |
| coverId      | String  |  No | empty |
| mediaProvider      | String  | No |  optional |
| mediaType      | MediaType | Yes |  see MediaType |
| duration      | long  | No |  optional |
| **SessionsState** |
| endpointId      | String  | No |  empty |
| loggedIn      | boolean  | No |  empty |
| userName      | String  |  No | empty |
| isGuest      | boolean  | No |  empty |
| launched      | boolean  | Yes |  true if the source is enabled, false otherwise |
| active      | boolean  | Yes |  true if the application is in an active state  |
| accessToken      | String  |  No | empty |
| tokenRefreshInterval      | long  | No |   empty |
| playerCookie      | String  | No |  A player may declare arbitrary information for itself |
| spiVersion      | String  |  Yes | must be set as "1.0" |


`supportedOperations` should be a list of the operations that the external media adapter supports. Below is a list of all possible `supportedOperations`.

```
SupportedPlaybackOperation.PLAY,
SupportedPlaybackOperation.PAUSE,
SupportedPlaybackOperation.STOP,
SupportedPlaybackOperation.PREVIOUS,
SupportedPlaybackOperation.NEXT,
SupportedPlaybackOperation.ENABLE_SHUFFLE,
SupportedPlaybackOperation.DISABLE_SHUFFLE,
SupportedPlaybackOperation.ENABLE_REPEAT_ONE,
SupportedPlaybackOperation.ENABLE_REPEAT,
SupportedPlaybackOperation.DISABLE_REPEAT,
SupportedPlaybackOperation.SEEK,
SupportedPlaybackOperation.ADJUST_SEEK,
SupportedPlaybackOperation.FAVORITE,
SupportedPlaybackOperation.UNFAVORITE,
SupportedPlaybackOperation.FAST_FORWARD,
SupportedPlaybackOperation.REWIND,
SupportedPlaybackOperation.START_OVER
```
>**Note:** Currently PLAY/PAUSE/STOP will always be supported for a source. Passing null will allow ALL supported operations for the source. 
