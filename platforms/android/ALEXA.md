## Alexa API

### Overview

The AAC Alexa API provides interfaces for standard AVS features. For an Android project you should consider creating a class per interface you wish to handle. 


### Handling Alexa State Changes

The AAC SDK manages internal state information for Alexa and provides an interface for developers to handle Alexa state changes in their platform. To implement a custom handler for Alexa state changes, the `AlexaClient` class should be extended:

```
public class AlexaClientHandler extends AlexaClient
{
	@Override
	public dialogStateChanged( DialogState state ) {
        // handle dialog state change!
    }
    @Override
    public authStateChanged( AuthState state, AuthError error ) {
        // handle auth state change!
    }
    @Override
    public connectionStatusChanged( ConnectionStatus status, ConnectionChangedReason reason ) {
        // handle connection status change!
    }
}
...
```	

### Handling Authorization

It is the respoonsibility of the platform implementation to provide an authorization method for establishing a connection to AVS. The AAC SDK provides an interface to handle authorization state changes and storing context.

To implement the handler for authorization, the `AuthProvider` class should be extended:

```
public class AuthProviderHandler extends AuthProvider 
{
	@Override
	public String getAuthToken() 
	{
    	//return the currently obtained auth refresh token
	...
	@Override
	public AuthState getAuthState() {
		//return the current auth state
	}
	...
		//notify the engine that a valid refresh token has been obtained
		authStateChange( AuthProvider.AuthState.REFRESHED, AuthProvider.AuthError.NO_ERROR );
	...
```

### Handling Speech Input

It is the responsibility of the platform implementation to supply audio data to the Engine so that Alexa can process voice input. Since the Engine does not know how audio is managed on a specific platform, the specific audio capture implementation is up to the platform developer. The default platform implementation provides methods for writing raw PCM data to the Engine's audio input processor, which includes handling wakeword recognition and end-of-speech detection.

To implement a custom handler for speech input, the `SpeechRecognizer` class should be extended:

```
public class SpeechRecognizerHandler extends SpeechRecognizer
{    
    @Override
    public boolean startAudioInput() {
        // implement start recording 
    }
    @Override
    public boolean stopAudioInput() {
        // implement stop recording 
    }
    ...
    	// call the write audio method while reading audio input data 
    	write( (byte[]) data, (long) size )
    ...
...
```

The initiation of a speech recognition event must be handled by the platform. The simplest way to invoke the speech recognizer is via its `holdToTalk()` and `tapToTalk()` methods.

For wakeword implementations, the following methods should be extended in the handler class:

```
...
    @Override
    public void endOfSpeechDetected() {
        // implement speech end detected after wakeword detected, and play correct audio cue
    }
    @Override
    public boolean wakewordDetected( String wakeWord ) {
        // implement wakeword detected, and play correct audio cue
    }
...
```

The interface also includes the methods `enableWakewordDetection()`, `disableWakewordDetection()`, and `isWakewordDetectionEnabled()`, for wakeword support.

### Handling Speech Output

The speech synthesizer is responsible for handling Alexa's speech. The `SpeechSynthesizer` class extends the abstract `AudioChannel` class. This is to separate each audio-capable class on the platform while allowing the engine to control their interactive behaviors. The platform implementation provides a reference to an instance of a *media player* and a *speaker*. (The term "speaker" is used here in the sense of "output device") [**Read more about handling media and volume here**](#handling-media-and-volume).

 To implement a custom handler for speech output the `SpeechSynthesizer` class should be extended:

```
public class SpeechSynthesizerHandler extends SpeechSynthesizer
{
	// passing in AndroidMediaPlayer 
	public SpeechSynthesizerHandler( Context context, AndroidMediaPlayer mediaPlayer ) {
		this( context, mediaPlayer, mediaPlayer.getSpeaker() );
    }
  ...
```  

### Handling Audio Player Output

When an audio media stream is received from Alexa it is the responsibility of the platform implementation to play the stream in a platform-specific media player and to notify the Engine when the audio starts and stops playing.

The class is similar to the `SpeechSynthesizer` class. However, you may want to handle your media player or speaker differently. [**Read more about handling media and volume here**](#handling-media-and-volume).

To implement a custom handler for audio player output the `AudioPlayer` class should be extended:

```
public class AudioPlayerHandler extends AudioPlayer
{
	public AudioPlayerHandler( Context context, AndroidMediaPlayer mediaPlayer ) {
    this( context, mediaPlayer, mediaPlayer.getSpeaker() );
}
```

### Handling Notifications

It is the responsibility of the platform implementation to provide a visual and audio indication to the user when notifications (package shipment notifications, notifications from skills, etc.) are available from Alexa. [**Read more about Notifications here**](https://alexa.design/AVSDevNotifications).

To implement a custom handler for Notifications the `Notifications` class should be extended:

```
public class NotificationsHandler extends Notifications
{
	@Override
	public void setIndicator( IndicatorState state )
	{ 
		// set your notifications indicator ON/OFF
	...
```

### Handling Alerts

When an alert is received from Alexa it is the responsibility of the platform implementation to play the alert sounds in a platform-specific media player and notify the Engine when the audio starts and stops playing. The state of the alert is also made available for the platform to react to. The class is similar to the `SpeechSynthesizer` class. However, you may want to handle your media player or speaker differently. [**Read more about handling media and volume here**](#handling-media-and-volume).

To implement a custom handler for alerts the `Alerts` class should be extended:

```
public class AlertsHandler extends Alerts
{
	// pass in a media player
	public AlertsHandler( Context context, LoggerHandler logger, AndroidMediaPlayer mediaPlayer ) {
        this( context, logger, mediaPlayer, mediaPlayer.getSpeaker() );
    }
	...
	@Override
	public void alertStateChanged( String alertToken, AlertState state, String reason ) {
		//handle alert state change
	}
```

For local Alerts control, the methods `localStop`(stop current playing alert), and `removeAllAlerts`(remove all locally stored alerts) should be used.

### Handling Media and Volume

When audio data is received from Alexa it is the responsibility of the platform implementation to read the data from the Engine and play it using a platform-specific media player and speaker. It is also the responsibility of the platform implementation to notify the Engine when the speech audio actually begins playing in the media player and when it is finished. Alexa will notify the Engine that the platform should play/stop/pause/resume playing the speech or other audio in which case the platform implementation must play/stop/pause/resume the media player playback. The platform should extend both `MediaPlayer` and `Speaker` and implement the platform-specific functionality. These classes will then be instantiated by other platform interfaces (i.e. `Alerts`, `SpeechSynthesizer`, `AudioPlayer`) as separate audio channels.

To implement a custom handler for MediaPlayer the `MediaPlayer` class should be extended:

```
//extending the AACE Media Player using the Android MediaPlayer
public class AndroidMediaPlayer extends com.amazon.aace.alexa.MediaPlayer implements android.media.MediaPlayer.OnCompletionListener, android.media.MediaPlayer.OnErrorListener
{
	...
	@Override
    public boolean prepare()
    {
    	//handle preparing for audio
	...
	@Override
    public boolean prepare( String url )
    {
    	//handle preparing to play a url
	...
	@Override
    public boolean play()
    {
    	//handle playing audio
    	...
    	//inform engine playback started
    	playbackStarted();
	...
	@Override
    public boolean pause()
    {
    	//handle pausing the audio
    	...
    	//inform engine playback paused
    	playbackPaused();
	...
	@Override
    public boolean resume()
    {
    	//handle resuming the audio
    	...
    	//inform engine playback resumed
    	playbackResumed();
	...
	@Override
    public boolean stop()
    {
    	//handle stopping audio
    	...
    	//inform engine playback stopped
    	playbackStopped();
	...
	@Override
    public long getPosition()
    {
    	//handle getting the current position of the audio
	...
	@Override
    public long setPosition()
    {
    	//handle setting the current position of the audio
	...
	@Override
    public void onCompletion( android.media.MediaPlayer mediaPlayer )
    {
    	//handle media completing	
    	if( isRepeating() )
		{
			//handle repeating audio condition
			...
		} else {
			//inform engine playback finished
			playbackFinished();
	...
	@Override
    public boolean onError( android.media.MediaPlayer mediaPlayer, int what, int extra ) 
    {
       	...
       	//inform engine of playback error
       	playbackError(...);
    }
	

``` 

The platform implementation's Speaker class will likely need a reference to the media player class. For an Android implementation, the Speaker can be subclassed in the MediaPlayer for example.

To implement a custom handler for Speaker, the `Speaker` class should be extended:

```
...
	private class SpeakerHandler extends Speaker
	{
		...
		@Override
		public boolean setVolume( byte volume )
		{
			//handle setting the volume
		...
		@Override
		public boolean adjustVolume( byte value ) {
			//handle ajdusting the volume by the value
		...
		@Override
		public byte getVolume() {
			//return the volume 
		...
		@Override
		public boolean setMute( boolean mute )
		{
			//handle set mute
		...
		@Override
		public boolean isMuted() {
			//return mute state
		...
```

### Handling GUI Templates

When template info is received from Alexa, it is the responsibility of the platform implementation to handle the rendering of any UI with the info that is received from Alexa. There are two template flavours: [Templates](https://alexa.design/DevDocRenderTemplate) and [PlayerInfo](https://amzn.to/DevDocTemplatePlayerInfo). You can view sample JSON payloads from these AVS documentation links as well.

In case of lists, it is the responsibility of the platform to handle pagination. Alexa sends down the entire list as a JSON response and starts reading out the first five elements of the list. At the end of reading the first five elements it will prompt the user to see if they want to read the remaining elements from the list. If the user chooses to proceed with the remaining elements, Alexa sends down the entire list as a JSON response but starts reading from the sixth element onwards.

To implement a custom handler for GUI templates, the `TemplateRuntime` class should be extended:

```
public class TemplateRuntimeHandler extends TemplateRuntime
{
	...
	@Override
	public void renderTemplate( String payload )
	{
		//handle JSON string payload
	...
	@Override
	public void renderPlayerInfo( String payload )
	{
		//handle JSON string payload
	...
	@Override
    public void clearTemplate() {
        // Handle dismissing display card here
    ...
    @Override
    public void clearPlayerInfo() {
        // Handle clearing player info here
```



### Handling Playback Controller Events

The Engine provides methods for notifying it of playback controller events. If the platform implements playback control features, it must inform the Engine.

To implement a custom handler for the playback controller, the `PlaybackController` class should be extended:

```
public class PlaybackControllerHandler extends PlaybackController
{
	@Override
    public void previousButtonPressed()
    {
		//notify the engine the previous button was pressed
		super.previousButtonPressed();
	...
		//notify the engine the play button was pressed
		playButtonPressed();
	...
		//notify the engine the pause button was pressed
		pauseButtonPressed();
	...
		//notify the engine the next button was pressed
		nextButtonPressed();   
```

