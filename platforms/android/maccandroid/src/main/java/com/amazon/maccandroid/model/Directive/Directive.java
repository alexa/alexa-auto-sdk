package com.amazon.maccandroid.model.Directive;

public abstract class Directive {
    private final String mPlayerId;

    protected Directive(String playerId) {
        mPlayerId = playerId;
    }

    public String getPlayerId() {
        return mPlayerId;
    }
}
