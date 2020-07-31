package com.amazon.maccandroid.model;

public final class APIConstants {
    public static class ExtrasKeys {
        public static final String SPI_VERSION_KEY = "com.amazon.alexa.externalmediaplayer.spiVersion";
        public static final Double MIN_SUPPORTED_SPI_VERSION = 1.0;
        public static final String PLAYER_COOKIE_KEY = "com.amazon.alexa.externalmediaplayer.playerCookie";
    }

    public static class Events {
        public static final String SESSION_EVENT_REQUEST_TOKEN = "com.amazon.alexa.externalmediaplayer.REQUEST_TOKEN";
    }

    public static class Timeouts {
        public static final long ALEXA_RESPONSE_TIMEOUT = 5000;
        public static final long MEDIA_SESSION_CONNECTION_TIMEOUT = 10000;
    }

    public static class Actions {
        public static final String CONNECT_ACTION = "com.amazon.alexa.externalmediaplayer.CONNECT";
    }

    public static class Permissions {
        public static final String EMP_CONNECT_PERMISSION = "com.amazon.alexa.externalmediaplayer.permission.CONNECT";
    }

    public static class Directives {
        public static class PlayControl {
            public static final String PLAY_CONTROL_PLAY = "RESUME";
            public static final String PLAY_CONTROL_STOP = "STOP";
            public static final String PLAY_CONTROL_PAUSE = "PAUSE";
            public static final String PLAY_CONTROL_STARTOVER = "START_OVER";
            public static final String PLAY_CONTROL_PREVIOUS = "PREVIOUS";
            public static final String PLAY_CONTROL_NEXT = "NEXT";
            public static final String PLAY_CONTROL_REWIND = "REWIND";
            public static final String PLAY_CONTROL_FASTFORWARD = "FAST_FORWARD";
            public static final String PLAY_CONTROL_ENABLE_REPEAT_ONE = "ENABLE_REPEAT_ONE";
            public static final String PLAY_CONTROL_ENABLE_REPEAT = "ENABLE_REPEAT";
            public static final String PLAY_CONTROL_DISABLE_REPEAT = "DISABLE_REPEAT";
            public static final String PLAY_CONTROL_ENABLE_SHUFFLE = "ENABLE_SHUFFLE";
            public static final String PLAY_CONTROL_DISABLE_SHUFFLE = "DISABLE_SHUFFLE";
            public static final String PLAY_CONTROL_FAVORITE = "FAVORITE";
            public static final String PLAY_CONTROL_UNFAVORITE = "UNFAVORITE";
            public static final String SEEK_CONTROL_SET_SEEK_POSITION = "SET_SEEK_POSITION";
        }

        public static class ExternalMediaPlayerDirectives {
            public static final String EXTERNAL_MEDIA_PLAYER_PLAY = "ExternalMediaPlayer.Play";
            public static final String EXTERNAL_MEDIA_PLAYER_LOGIN = "ExternalMediaPlayer.Login";
            public static final String EXTERNAL_MEDIA_PLAYER_LOGOUT = "ExternalMediaPlayer.Logout";

            public static final String ALEXA_PLAYBACK_CONTROLLER_PLAY = "Alexa.PlaybackController.Play";
            public static final String ALEXA_PLAYBACK_CONTROLLER_STOP = "Alexa.PlaybackController.Stop";
            public static final String ALEXA_PLAYBACK_CONTROLLER_PAUSE = "Alexa.PlaybackController.Pause";
            public static final String ALEXA_PLAYBACK_CONTROLLER_STARTOVER = "Alexa.PlaybackController.StartOver";
            public static final String ALEXA_PLAYBACK_CONTROLLER_PREVIOUS = "Alexa.PlaybackController.Previous";
            public static final String ALEXA_PLAYBACK_CONTROLLER_NEXT = "Alexa.PlaybackController.Next";
            public static final String ALEXA_PLAYBACK_CONTROLLER_REWIND = "Alexa.PlaybackController.Rewind";
            public static final String ALEXA_PLAYBACK_CONTROLLER_FASTFORWARD = "Alexa.PlaybackController.FastForward";

            public static final String ALEXA_SEEK_CONTROLLER_ADJUST_SEEK = "Alexa.SeekController.AdjustSeekPosition";
            public static final String ALEXA_SEEK_CONTROLLER_SET_SEEK = "Alexa.SeekController.SetSeekPosition";

            public static final String ALEXA_PLAYLIST_CONTROLLER_ENABLE_SHUFFLE =
                    "Alexa.PlaylistController.EnableShuffle";
            public static final String ALEXA_PLAYLIST_CONTROLLER_DISABLE_SHUFFLE =
                    "Alexa.PlaylistController.DisableShuffle";
            public static final String ALEXA_PLAYLIST_CONTROLLER_ENABLE_REPEAT =
                    "Alexa.PlaylistController.EnableRepeat";
            public static final String ALEXA_PLAYLIST_CONTROLLER_ENABLE_REPEAT_ONE =
                    "Albuilexa.PlaylistController.EnableRepeatOne";
            public static final String ALEXA_PLAYLIST_CONTROLLER_DISABLE_REPEAT =
                    "Alexa.PlaylistController.DisableRepeat";

            public static final String ALEXA_FAVORITES_CONTROLLER_FAVORITE = "Alexa.FavoritesController.Favorite";
            public static final String ALEXA_FAVORITES_CONTROLLER_UNFAVORITE = "Alexa.FavoritesController.UnFavorite";
            public static final String CUSTOM_ACTION_LOGIN = "com.amazon.alexa.externalmediaplayer.ACCOUNT_LINKING";
        }
    }
}
