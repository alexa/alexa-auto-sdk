package com.amazon.maccandroid.model.state;

import com.amazon.maccandroid.MediaApp;

public class ExternalMediaPlayerState {
    private final MediaAppPlaybackState mMediaAppPlaybackState;
    private final MediaAppSessionState mMediaAppSessionState;

    public ExternalMediaPlayerState(MediaApp app) {
        mMediaAppPlaybackState = new MediaAppPlaybackState(app);
        mMediaAppSessionState = new MediaAppSessionState(app);
    }

    public MediaAppPlaybackState getMediaAppPlaybackState() {
        return mMediaAppPlaybackState;
    }

    public MediaAppSessionState getMediaAppSessionState() {
        return mMediaAppSessionState;
    }
}
