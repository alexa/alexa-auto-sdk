package com.amazon.alexa.auto.media.aacs.handlers;

import android.net.Uri;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.MediaConstants;
import com.amazon.aacsconstants.PlaybackConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.aacs.common.PlaybackControlMessages;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.MusicStreamAttributeUpdater;
import com.amazon.alexa.auto.media.player.MediaPlayerAudioFocusController;
import com.amazon.alexa.auto.media.player.MediaPlayerExo;

import org.json.JSONObject;
import org.json.JSONStringer;

/**
 * Handler for AudioPlayer commands coming from AACS.
 */
public class AudioPlayerHandler implements AutoCloseable {
    private final static String TAG = AudioPlayerHandler.class.getSimpleName();
    private final static String AUDIO_PLAYER_CHANNEL = "AudioPlayer";

    // Dependencies.
    @NonNull
    private final AACSMessageSender mAACSSender;
    @NonNull
    private final MediaPlayerExo mTargetMediaPlayer;
    @NonNull
    private final MediaPlayerAudioFocusController mAudioFocusController;
    @NonNull
    private final PlaybackControlMessages mPlaybackControlMessages;
    @NonNull
    private final MusicStreamAttributeUpdater mMusicStreamAttributeUpdater;

    // Internal Dependencies.
    @NonNull
    private final AudioFocusPlaybackController mAudioFocusPlaybackController;

    // State.
    private String currentSourceToken;

    /**
     * Constructs the AACS AudioPlayer commands handler.
     *
     * @param sender To send messages back to AACS.
     * @param targetMediaPlayer Target media player.
     * @param audioFocusController Manages audio focus for playback.
     * @param playbackControlMessages For sending playback control messages to AACS.
     * @param musicStreamAttributeUpdater Helper to change volume for music stream.
     */
    public AudioPlayerHandler(@NonNull AACSMessageSender sender, @NonNull MediaPlayerExo targetMediaPlayer,
            @NonNull MediaPlayerAudioFocusController audioFocusController,
            @NonNull PlaybackControlMessages playbackControlMessages,
            @NonNull MusicStreamAttributeUpdater musicStreamAttributeUpdater) {
        mAACSSender = sender;
        mTargetMediaPlayer = targetMediaPlayer;
        mAudioFocusController = audioFocusController;
        mPlaybackControlMessages = playbackControlMessages;
        mMusicStreamAttributeUpdater = musicStreamAttributeUpdater;

        mAudioFocusPlaybackController = new AudioFocusPlaybackController();
        mAudioFocusController.setPlaybackController(mAudioFocusPlaybackController);
    }

    @Override
    public void close() throws Exception {
        mAudioFocusController.close();
    }

    /**
     * Handle commands coming from AACS.
     *
     * @param message AACS Message.
     */
    public void handleAACSCommand(@NonNull AACSMessage message) {
        switch (message.action) {
            case Action.AudioOutput.PREPARE:
                handlePrepare(message.messageId, message.payload);
                break;
            case Action.AudioOutput.PAUSE:
                pauseAndDisableAutoPlay();
                break;
            case Action.AudioOutput.STOP:
                handleStop();
                break;
            case Action.AudioOutput.PLAY:
            case Action.AudioOutput.RESUME:
                mAudioFocusController.startPlaybackAfterAcquiringFocus();
                break;
            case Action.AudioOutput.GET_POSITION:
                handleGetPosition(message.messageId, message.payload);
                break;
            case Action.AudioOutput.GET_DURATION:
                handleGetDuration(message.messageId, message.payload);
                break;
            case Action.AudioOutput.SET_POSITION:
                handleSetPosition(message.messageId, message.payload);
                break;
            case Action.AudioOutput.VOLUME_CHANGED:
                handleChangeVolume(message.messageId, message.payload);
                break;
            case Action.AudioOutput.MUTED_STATE_CHANGED:
                handleSetMuteState(message.messageId, message.payload);
                break;
            default:
                throw new RuntimeException(String.format("Unrecognized audio manager command %s", message.action));
        }
    }

    /**
     * Process media state change and also publish the new media state to AACS.
     *
     * @param state New state to publish.
     */
    public void processMediaStateChange(String state) {
        try {
            if (MediaConstants.MediaState.STOPPED.equals(state)) {
                Log.d(TAG, "Media state is stopped. Disabling auto playback");
                pauseAndDisableAutoPlay();
            }

            String payload = new JSONStringer()
                                     .object()
                                     .key(MediaConstants.CHANNEL)
                                     .value(AUDIO_PLAYER_CHANNEL)
                                     .key(MediaConstants.TOKEN)
                                     .value(this.currentSourceToken)
                                     .key(MediaConstants.STATE)
                                     .value(state)
                                     .endObject()
                                     .toString();

            Log.d(TAG, "publishing media state to AACS. state:" + state);
            mAACSSender.sendMessage(Topic.AUDIO_OUTPUT, Action.AudioOutput.MEDIA_STATE_CHANGED, payload);
        } catch (Exception e) {
            Log.e(TAG, "failed to create mediaStateChanged JSON payload.");
        }
    }

    /**
     * Process the media error and publish it to AACS.
     *
     * @param error Error to process.
     */
    public void processMediaError(String error) {
        try {
            Log.d(TAG, "Media errored. Disabling auto playback. Error: " + error);
            pauseAndDisableAutoPlay();

            String payload = new JSONStringer()
                                     .object()
                                     .key(MediaConstants.CHANNEL)
                                     .value(AUDIO_PLAYER_CHANNEL)
                                     .key(MediaConstants.TOKEN)
                                     .value(this.currentSourceToken)
                                     .key(MediaConstants.MEDIA_ERROR)
                                     .value(MediaConstants.MediaError.MEDIA_ERROR_UNKNOWN)
                                     .key(MediaConstants.ERROR_DESCRIPTION)
                                     .value(error)
                                     .endObject()
                                     .toString();

            Log.d(TAG, "publishing media error to AACS. error:" + error);
            mAACSSender.sendMessage(Topic.AUDIO_OUTPUT, Action.AudioOutput.MEDIA_ERROR, payload);
        } catch (Exception e) {
            Log.e(TAG, "failed to create mediaError JSON payload.");
        }
    }

    private void handlePrepare(@NonNull String messageId, @Nullable String payload) {
        try {
            Preconditions.checkNotNull(payload);

            JSONObject payloadJson = new JSONObject(payload);
            boolean repeating = payloadJson.getBoolean(AASBConstants.AudioOutput.REPEATING);
            String token = payloadJson.getString(AASBConstants.AudioOutput.TOKEN);
            if (payloadJson.has(AASBConstants.AudioOutput.URL)) {
                String url = payloadJson.getString(AASBConstants.AudioOutput.URL);
                Log.d(TAG, String.format("Preparing media player with %s", url));
                mTargetMediaPlayer.prepare(Uri.parse(url));
                this.currentSourceToken = token;
            } else {
                Log.e(TAG, "Prepare json didn't have url:" + payload);
            }
        } catch (Exception e) {
            Log.e(TAG, "failed to prepare, " + e.toString());
        }
    }

    /**
     * Disable auto playback such that next prepare request doesn't auto start
     * the playback until distinct play signal is received from AACS.
     */
    private void pauseAndDisableAutoPlay() {
        mTargetMediaPlayer.requestPause();
        mAudioFocusController.relinquishAudioFocusIfCurrentlyAcquired();
    }

    private void handleStop() {
        MediaPlayerExo targetMediaPlayer = mTargetMediaPlayer;
        if (targetMediaPlayer.isStopped()) {
            processMediaStateChange(MediaConstants.MediaState.STOPPED);
        } else {
            targetMediaPlayer.requestStop();
        }
        mAudioFocusController.relinquishAudioFocusIfCurrentlyAcquired();
    }

    private void handleGetDuration(@NonNull String messageId, @Nullable String payload) {
        try {
            long duration = mTargetMediaPlayer.getDuration();
            String replyPayload =
                    new JSONStringer().object().key(MediaConstants.DURATION).value(duration).endObject().toString();

            mAACSSender.sendReplyMessage(messageId, Topic.AUDIO_OUTPUT, Action.AudioOutput.GET_DURATION, replyPayload);
        } catch (Exception e) {
            Log.e(TAG, "failed to create getDurationReply JSON payload. Error: " + e);
        }
    }

    private void handleGetPosition(@NonNull String messageId, @Nullable String payload) {
        try {
            long positionMs = mTargetMediaPlayer.getPosition();
            String replyPayload =
                    new JSONStringer().object().key(MediaConstants.POSITION).value(positionMs).endObject().toString();

            Log.d(TAG, "Dispatching current media position to AACS. Position:" + positionMs);
            mAACSSender.sendReplyMessage(messageId, Topic.AUDIO_OUTPUT, Action.AudioOutput.GET_POSITION, replyPayload);
        } catch (Exception e) {
            Log.e(TAG, "failed to create getPositionReply JSON payload. Error: " + e);
        }
    }

    private void handleSetPosition(@NonNull String messageId, @Nullable String payload) {
        Preconditions.checkArgument(payload != null && !payload.isEmpty());

        try {
            JSONObject jsonPayload = new JSONObject(payload);

            long position = jsonPayload.getLong(AASBConstants.AudioOutput.POSITION);
            Log.d(TAG, "Setting position:" + position);
            if (position > 0) {
                mTargetMediaPlayer.seekToPosition(position);
            }
        } catch (Exception e) {
            Log.e(TAG, "failed to set position");
        }
    }

    private void handleChangeVolume(@NonNull String messageId, @Nullable String payload) {
        try {
            Preconditions.checkArgument(payload != null && !payload.isEmpty());

            JSONObject jsonObject = new JSONObject(payload);
            double volume = jsonObject.getDouble(AASBConstants.AudioOutput.VOLUME);
            mMusicStreamAttributeUpdater.changeVolumeForMusicStream(volume);
        } catch (Exception exception) {
            Log.w(TAG, String.format("Failed to change volume %s", exception.toString()));
        }
    }

    private void handleSetMuteState(@NonNull String messageId, @Nullable String payload) {
        try {
            Preconditions.checkArgument(payload != null && !payload.isEmpty());

            JSONObject jsonObject = new JSONObject(payload);
            String state = jsonObject.getString(AASBConstants.AudioOutput.STATE);
            mMusicStreamAttributeUpdater.setMuteForMusicStream("MUTED".equals(state));
        } catch (Exception exception) {
            Log.w(TAG, String.format("Failed to change volume %s", exception.toString()));
        }
    }

    /**
     * Playback controller for Audio Focus.
     */
    private class AudioFocusPlaybackController implements MediaPlayerAudioFocusController.PlaybackController {
        @Override
        public void startPlaybackNow() {
            mTargetMediaPlayer.requestPlay();
        }

        @Override
        public void requestResumingPlayback() {
            mPlaybackControlMessages.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.PLAY);
        }

        @Override
        public void requestPausePlayback() {
            mPlaybackControlMessages.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.PAUSE);
        }

        @Override
        public void requestStopPlayback() {
            mPlaybackControlMessages.sendButtonCommandToAACS(PlaybackConstants.PlaybackButton.PAUSE);
        }

        @Override
        public void adjustPlaybackVolume(float multiplier) {
            mTargetMediaPlayer.setVolume(multiplier);
        }

        @Override
        public void failedToAcquireFocus() {
            String errorMsg = "Failed to acquire audio focus";
            Log.w(TAG, errorMsg);
            processMediaError(errorMsg);
        }
    }
}
