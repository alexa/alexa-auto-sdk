# Text To Speech Provider Module


<!-- omit in toc -->
## Table of Contents
- [Overview](#overview)
- [Understanding TTS Provider](#understanding-tts-provider)
- [Specifying the TTS Provider in APIs](#specifying-the-tts-provider-in-apis)
- [Using TTS Provider Module with Different Input Types](#using-tts-provider-module-with-different-input-types)
- [TTS Capability Returned](#tts-capability-returned)
- [SSML Examples](#ssml-examples)
- [Errors](#errors)

## Overview
The Text to Speech Provider module synthesizes Alexa speech on demand. Currently, the Auto SDK supports only one Text to Speech Provider, which uses Alexa's voice as the default voice for speech synthesis.

The TTS Provider module performs the following functions:

* Generating speech from a text or SSML document to provide the speech to the TTS module.
* Providing capabilities based on the properties of the TTS provider, such as available locales.
  
## Understanding TTS Provider
The TTS Provider module follows the existing AVS (Alexa Voice Service) protocol to carry out speech synthesis and requires connection to the Local Voice Control (LVC) service. If the device is disconnected from LVC, speech synthesis fails.

>**Note**: The module can synthesize speech only in the current locale as set by the application.

The TTS provider requires no configuration. 

## Specifying the TTS Provider in APIs

In all the text-to-speech platform interface APIs that involve the provider parameter, use the string, "text-to-speech-provider", to specify the TTS provider.

## Using TTS Provider Module with Different Input Types
How the TTS Provider module synthesizes speech depends on the input type and the `requestPayload` field in the options parameter of the `prepareSpeech()` API. The `requestPayload` structure is as follows:

~~~
{
	    "requestPayload" : {
	 		"voiceId" : {VOICE_IDENTIFIER},
	 		"locale" : {LOCALE_STRING}
	  	}
}
~~~

>**Note**: You do not need to specify the `requestPayload` in the options parameter because currently only the default voice (Alexa’s voice) is supported. Therefore, you can leave the options parameter unspecified or supply an empty string for it.

The following list explains how the TTS Provider module synthesizes speech depending on the input type and `requestPayload`:

* If input is text and `requestPayload` is empty, the text is synthesized to speech with Alexa's voice and the current locale.
* If input is text and `requestPayload` specifies the voice and locale, the text is synthesized to speech in the specified voice and locale. The locale must be the current locale. 
* If input is text and `requestPayload` contains one or more unsupported parameters, speech is not synthesized. The speech synthesis fails with the error `VOICE_NOT_SUPPORTED` or `LOCALE_NOT_SUPPORTED`, depending on the unsupported parameter.

* If input is SSML and contains all the supported tags and `requestPayload` specifies voice and locale, the SSML document is synthesized to speech in the specified voice and locale. The locale must be the current locale.
* If input is SSML and contains all the supported tags, `requestPayload` is empty, the SSML document is synthesized to speech with Alexa's voice and current locale.
* If input is SSML and contains all the supported tags, and `requestPayload` contains one or more unsupported parameters, speech is not synthesized. The speech synthesis fails with the error `VOICE_NOT_SUPPORTED` or `LOCALE_NOT_SUPPORTED`, depending on the unsupported parameter.
* If input is SSML and contains one or more unsupported tags, speech is synthesized but the unsupported tag is ignored. The text within the tag is synthesized normally.

## TTS Capability Returned
The capability returned by the `getCapabilities()` API to the platform has the following payload structure:

~~~
"text-to-speech-provider" : {

	"voices" : [	
		    {
                 "voiceId": "Alexa",
                 "locales": [CURRENT_LOCALE]
             }
	]
}
~~~

>**NOTE**: The locale returned is always the current locale because you can load only one locale model at a time, which is the current locale.

## SSML Examples
* `<speak> ETA <break time=\"3s\"/> three hours </speak>`
* `<speak> Turn right <emphasis level=\"strong\">in twenty feet</emphasis> </speak>`
* `<speak> Turn right on<lang xml:lang=\"fr-CA\">Moulin Rouge street.</lang> </speak>`
* `<speak> <p>Turn left in 500ft.</p> <p>Then turn right.</p> </speak>`
* `<speak>Turn left on, <phoneme alphabet=\"ipa\" ph=\"Bo.fort\">Beaufort</phoneme></speak>`
* `<speak> Turn right onto <phoneme alphabet='nt-sampa' ph='*\"stAk|t@n \"strit'>Stockton Street</phoneme> </speak>`
* `<speak> Your ETA is 5 minutes on <say-as interpret-as=\"date\" format=\"dmy\">12-10-2020</say-as>. </speak>`
* `<speak> Take a <prosody volume=\"-6dB\">U turn.</prosody> </speak>`
* `<speak> Take the next left onto <sub alias=\"John Doe\">JD</sub> street </speak>`

## Errors
The TTS provider defines its set of error strings or codes. The `prepareSpeechFailed()` API uses the `reason` parameter to send the error strings to the application.

The following list describes the error strings used by the Local TTS provider:

* `LOCALE_NOT_SUPPORTED` occurs in any of the following situations:
    * The language model for the current locale is missing. 
    * The locale specified in `requestPayload` is not the current locale.
    * The locale specified is invalid.

* `VOICE_NOT_SUPPORTED` occurs when the application specifies in `requestPayload` an invalid voice or is not Alexa.
  
* `INTERNAL_ERROR` is an internal error that signifies an error when the TTS or TTS Provider module processes a request.

* `PROVIDER_NOT_CONNECTED` occurs when the provider is not connected to LVC and a speech synthesis request is made.

>**NOTE**: If a speech synthesis request is made during an ongoing Alexa dialog, the speech is synthesized after the current Alexa dialog ends.