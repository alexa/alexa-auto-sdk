package com.amazon.maccandroid.model.state;

import com.amazon.maccandroid.MediaApp;
import com.amazon.maccandroid.model.SupportedOperations;

public class ExternalMediaPlayerState {
    private final MediaAppPlaybackState mMediaAppPlaybackState;
    private final MediaAppSessionState mMediaAppSessionState;

    public ExternalMediaPlayerState(MediaApp app, SupportedOperations supportedOperations) {
        mMediaAppPlaybackState = new MediaAppPlaybackState(app, supportedOperations);
        mMediaAppSessionState = new MediaAppSessionState(app);
    }

    public MediaAppPlaybackState getMediaAppPlaybackState() {
        return mMediaAppPlaybackState;
    }

    public MediaAppSessionState getMediaAppSessionState() {
        return mMediaAppSessionState;
    }
}
