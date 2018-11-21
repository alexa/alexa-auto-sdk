package com.amazon.maccandroid.model.Directive;

public class SeekDirective extends Directive {
    private final long mOffset;

    public SeekDirective(String playerId, long offset) {
        super(playerId);
        mOffset = offset;
    }

    public long getOffset() {
        return mOffset;
    }
}
