package com.amazon.alexa.auto.voiceinteraction.service;

import android.os.Bundle;
import android.service.voice.VoiceInteractionSession;
import android.service.voice.VoiceInteractionSessionService;

/**
 * Active Alexa Auto voice interaction session, initiated by Alexa Auto Interaction Service.
 */
public class AutoVoiceInteractionSessionService extends VoiceInteractionSessionService {
    @Override
    public VoiceInteractionSession onNewSession(Bundle args) {
        return new AutoVoiceInteractionSession(this);
    }
}
