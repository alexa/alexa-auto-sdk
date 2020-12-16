/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.amazon.aace.textToSpeech;

import com.amazon.aace.audio.AudioStream;
import com.amazon.aace.core.PlatformInterface;

/**
 * The @c TextToSpeech platform interface should be extended to allow the platform to synthesize
 * Alexa and non-Alexa speech using a text or a SSML document.
 *
 * The platform implementation is responsible for making requests for speech synthesis
 * and requesting capabilities of any TextToSpeech provider. The platform should also handle
 * callbacks for each @c prepareSpeech() and @c getCapabilities() requests.
 *
 * @note The TextToSpeech platform is not responsible for the playback of the synthesized speech.
 */

abstract public class TextToSpeech extends PlatformInterface {
    public TextToSpeech() {}

    /**
     * Notifies the platform implementation that the @c prepareSpeech() operation of
     * speech asset with @c speechId was successful.
     *
     * @param speechId The unique identifier of the speech asset.
     * @param preparedAudio The audio asset.
     * @param metadata The metadata of the speech asset.
     *
     * Refer to the Text to Speech provider module documentation for complete details of the
     * metadata schema.
     *
     */
    public void prepareSpeechCompleted(String speechId, AudioStream preparedAudio, String metadata) {}

    /**
     * Notifies the platform implementation that the @c prepareSpeech() operation for
     * the speech asset with @c speechId failed.
     *
     * @param speechId The unique identifier of the speech asset.
     * @param reason The reason for the failure.
     *
     * Refer to the Text to Speech provider module documentation for complete details of the
     * error responses.
     *
     */
    public void prepareSpeechFailed(String speechId, String reason) {}

    /**
     * Notifies the platform implementation of a Text to Speech provider's capabilities.
     * The @c requestId corresponds to the ID from the original @c getCapabilities() request.
     *
     * @param capabilities The capabilities corresponding to a Text to Speech provider.
     * @param requestId  The unique identifier for the original @c getCapabilities() request.
     *
     * Refer to the Text to Speech provider module documentation for complete details of the
     * capabilities payload.
     *
     */
    public void capabilitiesReceived(String requestId, String capabilities) {}

    /**
     * Notifies the Engine to prepare a speech asset with @c speechId and @c text.
     * This is an asynchronous call. The platform is notified about the result of
     * this operation through the @c prepareSpeechCompleted() or @c prepareSpeechFailed()
     * callback.
     *
     * @param speechId The unique identifier of the speech asset.
     * @param text The text in plain or SSML format.
     * @param provider The Text To Speech provider to be used to generate the speech asset.
     * This parameter should be "text-to-speech-provider" since there is only one provider in
     * Auto SDK.
     * @param options Additional options for the speech synthesis request.
     * Default value is empty string which generates speech in Alexa's voice.
     * This parameter need not be specified if the speech synthesis is expected to be
     * in Alexa's voice
     *
     * Sample options payload :
     * @code{.json})
     * {
     *      "requestPayload" : {
     *          // Payload defined by the Text To Speech provider
     *      }
     * }
     * @endcode
     *
     * Refer to the Text to Speech provider module documentation for complete details of the
     * requestPayload schema.
     * @return true if the request was successful, else false if an error occurred.
     *
     */
    protected final boolean prepareSpeech(String speechId, String text, String provider, String options) {
        return prepareSpeech(getNativeRef(), speechId, text, provider, options);
    }

    protected final boolean prepareSpeech(String speechId, String text, String provider) {
        return prepareSpeech(getNativeRef(), speechId, text, provider, "");
    }

    /**
     * Retrieves the capabilities of the @c provider.
     *
     * This is an asynchronous call. The result of the operation is provided by
     * the @c capabilitiesReceived() callback.
     * @param provider The unique identifier of the provider.
     * This parameter should be "text-to-speech-provider" since there is only one provider in
     * Auto SDK.
     * @param requestId  The unique identifier for this request.
     * @return true if the request was successful, else false if an error occurred.
     *
     */
    protected final boolean getCapabilities(String requestId, String provider) {
        return getCapabilities(getNativeRef(), requestId, provider);
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native boolean prepareSpeech(long nativeRef, String speechId, String text, String provider, String options);
    private native boolean getCapabilities(long nativeRef, String requestId, String provider);
}