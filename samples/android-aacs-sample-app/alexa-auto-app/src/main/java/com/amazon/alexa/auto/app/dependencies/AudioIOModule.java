package com.amazon.alexa.auto.app.dependencies;

import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.app.audio.AudioInputHandler;
import com.amazon.alexa.auto.app.audio.AudioInputReader;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects for Audio I/O.
 */
@Module
public class AudioIOModule {
    /**
     * Provides an instance of {@link AudioInputHandler}.
     *
     * @return an instance of {@link AudioInputHandler}.
     */
    @Provides
    @ServiceScope
    public AudioInputHandler provideAudioInputHandler() {
        return new AudioInputHandler(new AudioInputReader());
    }
}
