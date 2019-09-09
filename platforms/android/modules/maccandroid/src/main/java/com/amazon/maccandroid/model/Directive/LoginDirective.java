package com.amazon.maccandroid.model.Directive;

public class LoginDirective extends Directive {
    private final String mAccessToken;
    private final String mUserName;
    private final boolean mForce;
    private final long mRefreshInterval;

    public LoginDirective(String playerId, String accessToken, String userName, boolean force, long refreshInterval) {
        super(playerId);
        mAccessToken = accessToken;
        mUserName = userName;
        mForce = force;
        mRefreshInterval = refreshInterval;
    }

    public String getAccessToken() {
        return mAccessToken;
    }

    public String getUserName() {
        return mUserName;
    }

    public boolean isForce() {
        return mForce;
    }

    public long getRefreshInterval() {
        return mRefreshInterval;
    }
}
