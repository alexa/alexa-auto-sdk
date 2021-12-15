package com.amazon.alexa.auto.voiceinteraction.service;

import android.app.assist.AssistContent;
import android.app.assist.AssistStructure;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.service.voice.VoiceInteractionSession;
import android.util.Log;

import com.amazon.alexa.auto.voice.ui.VoiceActivity;

import androidx.annotation.Nullable;

import org.jetbrains.annotations.NotNull;

/**
 * Active Alexa Auto voice interaction session, providing a facility for starting a new assistant activity
 * displayed on top of other activities in the system.
 */
public class AutoVoiceInteractionSession extends VoiceInteractionSession {
    private static final String TAG = AutoVoiceInteractionSession.class.getCanonicalName();

    public AutoVoiceInteractionSession(@NotNull Context context) {
        super(context);
    }

    @Override
    public void onShow(Bundle args, int showFlags) {
        Log.d(TAG, "onShow");
        super.onShow(args, showFlags);

        Intent intent = new Intent(getContext(), VoiceActivity.class);
        intent.putExtras(args);
        startAssistantActivity(intent);
    }

    @Override
    public void onPrepareShow(Bundle args, int showFlags) {
        Log.d(TAG, "onPrepareShow");
        super.onPrepareShow(args, showFlags);
        setUiEnabled(false);
    }

    @Override
    public void onHandleAssist(
            @Nullable Bundle data, @Nullable AssistStructure structure, @Nullable AssistContent content) {
        Log.d(TAG, "onHandleAssist");
        super.onHandleAssist(data, structure, content);
    }
}