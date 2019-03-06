/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.Communication;

import com.amazon.aace.alexa.MediaPlayer;
import com.amazon.aace.alexa.Speaker;
import com.amazon.aace.communication.AlexaComms;
import com.amazon.sampleapp.impl.Common.AudioInputManager;

import java.util.HashSet;
import java.util.Set;

public class AlexaCommsHandler extends AlexaComms implements AudioInputManager.AudioInputConsumer {
    /**
     * Observer for receiving call Alexa comms state change notifications
     */
    public interface AlexaCommsObserver {
        public void onStateChanged(final AlexaCommsState newState);
    }

    private AlexaCommsState mCommsState = new AlexaCommsState();
    private AudioInputManager mAudioInputManager;
    private Set<AlexaCommsObserver> mCommsObservers;

    public AlexaCommsHandler(AudioInputManager audioInputManager,
                             MediaPlayer ringtoneMediaPlayer,
                             Speaker ringtoneSspeaker,
                             MediaPlayer callAudioMediaPlayer,
                             Speaker callAudioSpeaker) {
        super(ringtoneMediaPlayer, ringtoneSspeaker, callAudioMediaPlayer, callAudioSpeaker);
        mAudioInputManager = audioInputManager;
        mCommsObservers = new HashSet<AlexaCommsObserver>();
    }

    public void addObserver(AlexaCommsObserver observer) {
        mCommsObservers.add(observer);
    }

    public void removeObserver(AlexaCommsObserver observer) {
        mCommsObservers.remove(observer);
    }

    @Override
    protected void callStateChanged(CallState state) {
        mCommsState.m_currentCallState = state;
        if (mCommsState.getCurrentCallState() == CallState.CALL_CONNECTED) {
            mAudioInputManager.startAudioInput(this);
        } else {
            mAudioInputManager.stopAudioInput(this);
        }
        notifyCurrentStateToObservers();
    }

    @Override
    public String getAudioInputConsumerName() {
        return "AlexaComms";
    }

    @Override
    public void onAudioInputAvailable(byte[] buffer, int size) {
        writeMicrophoneAudioData(buffer, size);
    }

    private void notifyCurrentStateToObservers() {
        for (AlexaCommsObserver observer: mCommsObservers) {
            observer.onStateChanged(mCommsState);
        }
    }
}
