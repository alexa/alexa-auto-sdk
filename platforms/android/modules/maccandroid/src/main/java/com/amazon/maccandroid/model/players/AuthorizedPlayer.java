package com.amazon.maccandroid.model.players;

public class AuthorizedPlayer {
    private final String mLocalPlayerId;
    private final boolean mAuthorized;

    public AuthorizedPlayer(String localPlayerId, boolean authorized) {
        mLocalPlayerId = localPlayerId;
        mAuthorized = authorized;
    }

    public String getLocalPlayerId() {
        return mLocalPlayerId;
    }

    public boolean isAuthorized() {
        return mAuthorized;
    }

    @Override
    public String toString() {
        return "mLocalPlayerId: " + mLocalPlayerId + " | mAuthorized: " + mAuthorized;
    }
}
