package com.amazon.aacsconstants;

import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Constants required for PlaybackController.
 */
public class PlaybackConstants {
    /**
     * Playback button command types.
     */
    public static class PlaybackButton {
        public static final String PLAY = "PLAY";
        public static final String PAUSE = "PAUSE";
        public static final String NEXT = "NEXT";
        public static final String PREVIOUS = "PREVIOUS";
        public static final String SKIP_FORWARD = "SKIP_FORWARD";
        public static final String SKIP_BACKWARD = "SKIP_BACKWARD";
    }

    /**
     * Toggle button types.
     */
    public static class ToggleButton {
        public static final String SHUFFLE = "SHUFFLE";
        public static final String LOOP = "LOOP";
        public static final String REPEAT = "REPEAT";
        public static final String THUMBS_UP = "THUMBS_UP";
        public static final String THUMBS_DOWN = "THUMBS_DOWN";
    }

    public static final String BUTTON = "button";
    public static final String TOGGLE = "toggle";
    public static final String ACTION = "action";
}
