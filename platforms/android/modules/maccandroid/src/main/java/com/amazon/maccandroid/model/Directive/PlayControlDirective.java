package com.amazon.maccandroid.model.Directive;

public class PlayControlDirective extends Directive {
    private final String mType;

    public PlayControlDirective(String playerId, String type) {
        super(playerId);
        mType = type;
    }

    public String getType() {
        return mType;
    }

    @Override
    public String toString() {
        return "PlayControlDirective: "
                + "| mType: " + mType;
    }
}
