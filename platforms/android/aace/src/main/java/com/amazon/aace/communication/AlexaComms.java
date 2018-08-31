/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

// aace/communication/AlexaComms.java
// This is an automatically generated file.

package com.amazon.aace.communication;

import com.amazon.aace.alexa.AudioChannel;
import com.amazon.aace.alexa.MediaPlayer;
import com.amazon.aace.alexa.Speaker;


/**
 * The @c AlexaComms class should be extended by the platform implementation to participate in Communications via Alexa.
 */
abstract public class AlexaComms extends AudioChannel
{
    /// An enumeration representing the state of a call.
    public enum CallState {
        /// The call is connecting.
        CONNECTING,
        /// An incoming call is causing a ringtone to be played.
        INBOUND_RINGING,
        /// The call has successfully connected.
        CALL_CONNECTED,
        /// The call has ended.
        CALL_DISCONNECTED,
        /// No current call state to be relayed to the user.
        NONE
    };
    
    public AlexaComms( MediaPlayer mediaPlayer, Speaker speaker ) {
        super( mediaPlayer, speaker, Speaker.Type.AVS_SPEAKER );
    }

    /// @name aace::communication::AlexaComms Functions
    /// @{
    /**
     * Allows the observer to react to a change in call state.
     *
     * @param state The new CallState.
     */
    protected abstract void callStateChanged(CallState state);
    /// @}

    /**
     * Accepts an incoming call.
     */
    public void acceptCall() {
        acceptCall( getNativeObject() );
    }

    /**
     * Stops the call.
     */
    public void stopCall() {
        stopCall( getNativeObject() );
    }

    private native void acceptCall( long nativeObject );
    private native void stopCall( long nativeObject );
}

// END OF FILE
