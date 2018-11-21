package com.amazon.maccandroid.model.Directive;

public class AdjustSeekDirective extends Directive {
    private final long mDelta;

    public AdjustSeekDirective(String playerId, long delta) {
        super(playerId);
        mDelta = delta;
    }

    public long getDelta() {
        return mDelta;
    }
}
