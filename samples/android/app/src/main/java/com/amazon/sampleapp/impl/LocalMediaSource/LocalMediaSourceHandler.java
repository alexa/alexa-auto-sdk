package com.amazon.sampleapp.impl.LocalMediaSource;

import com.amazon.aace.alexa.LocalMediaSource;
import com.amazon.aace.audio.AudioOutput;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.UUID;

public abstract class LocalMediaSourceHandler extends LocalMediaSource {
    private static final String sTag = LocalMediaSourceHandler.class.getSimpleName();

    private final LoggerHandler mLogger;

    private float mVolume = 0.5f;
    private AudioOutput.MutedState mMutedState = AudioOutput.MutedState.UNMUTED;
    private String mSessionId;

    protected LocalMediaSourceHandler(LoggerHandler logger, Source type) {
        super(type);
        mLogger = logger;
        mSessionId = new String();
    }

    @Override
    public boolean play(ContentSelector selector, String payload, String sessionId) {
        mLogger.postInfo(sTag,
                String.format("play [source=%s,selector=%s,payload=%s,sessionId=%s]", getSource(), selector.toString(),
                        payload, sessionId));
        // Call playerEvent("PlaybackSessionStarted","<sessionId>") to inform Alexa the media is switched.
        // This call gets the focus for the media source. This is very crucial step.
        // Report PlaybackSessionStarted only when the media source becomes the active
        // Call playerEvent("PlaybackStarted", "<sessionId>") to inform Alexa the media is playing.
        mSessionId = sessionId;
        setPlaybackState("PLAYING");
        playerEvent("PlaybackSessionStarted", mSessionId);
        playerEvent("PlaybackStarted", mSessionId);
        return true;
    }

    @Override
    public boolean playControl(PlayControlType controlType) {
        mLogger.postInfo(
                sTag, String.format("playControl [source=%s,controlType=%s]", getSource(), controlType.toString()));
        switch (controlType) {
            case STOP:
                // Call playerEvent("PlaybackStopped", "<sessionId>") to inform Alexa the media stopped playing
                setPlaybackState("STOPPED");
                playerEvent("PlaybackStopped", mSessionId);
                playerEvent("PlaybackSessionEnded", mSessionId);
                mSessionId = ""; // Reset the session Id
                break;
            case PAUSE:
                // Call playerEvent("PlaybackStopped", "<sessionId>") to inform Alexa the media stopped playing
                setPlaybackState("PAUSED");
                playerEvent("PlaybackStopped", mSessionId);
                break;
            case RESUME:
                // If Resuming a non active media, consider calling playerEvent("PlaybackSessionStarted","<sessionId>")
                // to inform Alexa the media is switched. Call playerEvent("PlaybackStarted", "<sessionId>") to inform
                // Alexa the media is playing
                if (mSessionId.isEmpty()) {
                    mSessionId = UUID.randomUUID().toString();
                    playerEvent("PlaybackSessionStarted", mSessionId);
                }
                setPlaybackState("PLAYING");
                playerEvent("PlaybackStarted", mSessionId);
                break;
            default:
                break;
        }
        return true;
    }

    @Override
    public boolean seek(long offset) {
        mLogger.postInfo(sTag, String.format("seek [source=%s,offset=%d]", getSource(), offset));
        return true;
    }

    @Override
    public boolean adjustSeek(long deltaOffset) {
        mLogger.postInfo(sTag, String.format("adjustSeek [source=%s,deltaOffset=%d]", getSource(), deltaOffset));
        return true;
    }

    @Override
    public LocalMediaSourceState getState() {
        LocalMediaSourceState stateToReturn = new LocalMediaSourceState();
        stateToReturn.playbackState = new PlaybackState();
        stateToReturn.playbackState.state = getSourcePlaybackState();
        stateToReturn.playbackState.supportedOperations = getSupportedPlaybackOperations();
        stateToReturn.sessionState = new SessionState();
        stateToReturn.sessionState.supportedContentSelectors = getSupportedContentSelectors();
        return stateToReturn;
    }

    @Override
    public boolean volumeChanged(float volume) {
        mLogger.postInfo(sTag,
                String.format("volumeChanged [source=%s,oldVolume=%2f,newVolume=%2f]", getSource(), mVolume, volume));

        if (mVolume != volume) {
            mVolume = volume;
        }

        return true;
    }

    @Override
    public boolean mutedStateChanged(AudioOutput.MutedState state) {
        mLogger.postInfo(sTag,
                String.format("mutedStateChanged [source=%s,oldState=%s,newState=%s]", getSource(),
                        mMutedState.toString(), state.toString()));

        if (state != mMutedState) {
            mMutedState = state;
        }

        return true;
    }

    protected SupportedPlaybackOperation[] getSupportedPlaybackOperations() {
        return new SupportedPlaybackOperation[0];
    }

    protected ContentSelector[] getSupportedContentSelectors() {
        return new ContentSelector[0];
    }

    protected String getSourcePlaybackState() {
        return "IDLE";
    }

    protected void setPlaybackState(String state) {}
}
