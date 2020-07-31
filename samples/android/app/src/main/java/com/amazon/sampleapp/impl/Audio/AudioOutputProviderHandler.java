package com.amazon.sampleapp.impl.Audio;

import android.app.Activity;
import android.media.AudioManager;

import com.amazon.aace.audio.AudioOutput;
import com.amazon.aace.audio.AudioOutputProvider;
import com.amazon.sampleapp.impl.AlexaClient.AlexaClientHandler;
import com.amazon.sampleapp.impl.AlexaClient.AuthStateObserver;
import com.amazon.sampleapp.impl.EqualizerController.EqualizerControllerHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.HashMap;

public class AudioOutputProviderHandler extends AudioOutputProvider {
    private static final String sTag = AudioOutputProviderHandler.class.getSimpleName();

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final AlexaClientHandler mAlexaClientHandler;
    private EqualizerControllerHandler mEqualizerControllerHandler;

    private HashMap<String, AudioOutput> mAudioOutputMap;

    public AudioOutputProviderHandler(Activity activity, LoggerHandler logger, AlexaClientHandler alexaClientHandler,
            EqualizerControllerHandler equalizerControllerHandler) {
        mActivity = activity;
        mLogger = logger;
        mAlexaClientHandler = alexaClientHandler;
        mAudioOutputMap = new HashMap<>();
        mEqualizerControllerHandler = equalizerControllerHandler;
    }

    public AudioOutput getOutputChannel(String name) {
        return mAudioOutputMap.containsKey(name) ? mAudioOutputMap.get(name) : null;
    }

    @Override
    public AudioOutput openChannel(String name, AudioOutputType type) {
        mLogger.postInfo(sTag, String.format("openChannel[name=%s,type=%s]", name, type.toString()));

        UnifiedAudioOutput audioOutput = new UnifiedAudioOutput(
                mActivity.getApplicationContext(), mLogger, name, type, mEqualizerControllerHandler);
        mAlexaClientHandler.registerAuthStateObserver(audioOutput);

        mAudioOutputMap.put(name, audioOutput);
        return audioOutput;
    }
}
