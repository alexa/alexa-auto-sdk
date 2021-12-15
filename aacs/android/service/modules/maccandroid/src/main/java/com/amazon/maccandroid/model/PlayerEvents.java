package com.amazon.maccandroid.model;

public enum PlayerEvents {
    PlaybackSessionStarted("PlaybackSessionStarted"),
    PlaybackSessionEnded("PlaybackSessionEnded"),
    PlaybackStarted("PlaybackStarted"),
    PlaybackStopped("PlaybackStopped"),
    PlaybackPrevious("PlaybackPrevious"),
    PlaybackNext("PlaybackNext"),
    TrackChanged("TrackChanged"),
    PlayModeChanged("PlayModeChanged"),
    ;

    private final String mName;

    PlayerEvents(String name) {
        mName = name;
    }

    public String getName() {
        return mName;
    }
}
