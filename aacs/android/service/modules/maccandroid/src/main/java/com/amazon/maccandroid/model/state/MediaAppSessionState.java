package com.amazon.maccandroid.model.state;

import com.amazon.maccandroid.MediaApp;

public class MediaAppSessionState {
    private final String mPlayerid;
    private final boolean mLaunched;
    private final boolean mActive;
    private final String mSpiVersion;
    private final String mPlayerCookie;

    public MediaAppSessionState(MediaApp app) {
        mPlayerid = app.getLocalPlayerId();
        mLaunched = app.getMediaController() != null;
        mActive = app.isSessionReady();
        mSpiVersion = app.getSpiVersion();
        mPlayerCookie = app.getPlayerCookie();
    }

    public String getPlayerid() {
        return mPlayerid;
    }

    public boolean isLaunched() {
        return mLaunched;
    }

    public boolean isActive() {
        return mActive;
    }

    public String getSpiVersion() {
        return mSpiVersion;
    }

    public String getPlayerCookie() {
        return mPlayerCookie;
    }
}
