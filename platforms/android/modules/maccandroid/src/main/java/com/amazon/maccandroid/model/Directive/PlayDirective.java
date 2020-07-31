package com.amazon.maccandroid.model.Directive;

public class PlayDirective extends Directive {
    private final String mToken;
    private final long mIndex;
    private final long mOffest;
    private final boolean mPreload;
    private final String mNavigation;

    public PlayDirective(String playerId, String token, long index, long offset, boolean preload, String navigation) {
        super(playerId);
        mToken = token;
        mIndex = index;
        mOffest = offset;
        mPreload = preload;
        mNavigation = navigation;
    }

    @Override
    public String toString() {
        return "PlayDirective | mtoken: " + mToken + " | mIndex: " + mIndex + " | mOffset: " + mOffest
                + " | mPreload: " + mPreload + " | mNavigation: " + mNavigation;
    }

    public String getToken() {
        return mToken;
    }

    public long getIndex() {
        return mIndex;
    }

    public long getOffest() {
        return mOffest;
    }

    public boolean isPreload() {
        return mPreload;
    }

    public String getNavigation() {
        return mNavigation;
    }
}
