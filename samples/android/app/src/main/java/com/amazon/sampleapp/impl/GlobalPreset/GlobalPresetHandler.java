package com.amazon.sampleapp.impl.GlobalPreset;

import android.content.Context;

import com.amazon.aace.alexa.GlobalPreset;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class GlobalPresetHandler extends com.amazon.aace.alexa.GlobalPreset {
    private static final String TAG = GlobalPresetHandler.class.getSimpleName();

    private final LoggerHandler mLogger;

    public GlobalPresetHandler(Context context, LoggerHandler logger) {
        mLogger = logger;
    }

    @Override
    public void setGlobalPreset(int preset) {
        mLogger.postInfo(TAG, "global preset to: " + preset);
    }
}
