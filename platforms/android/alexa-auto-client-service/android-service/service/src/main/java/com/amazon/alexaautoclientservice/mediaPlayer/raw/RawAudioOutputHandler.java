package com.amazon.alexaautoclientservice.mediaPlayer.raw;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aace.aasb.AASBStream;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants.AudioOutput.MutedState;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.MediaConstants;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexaautoclientservice.mediaPlayer.AACSMediaPlayer;
import com.amazon.alexaautoclientservice.mediaPlayer.AudioFocusAttributes;
import com.amazon.alexaautoclientservice.mediaPlayer.EventReceiver;
import com.amazon.alexaautoclientservice.util.MediaPlayerUtil;

import org.json.JSONObject;

import java.util.concurrent.atomic.AtomicLong;

public class RawAudioOutputHandler implements AACSMediaPlayer, AudioManager.OnAudioFocusChangeListener {
    private static final String TAG = AACSConstants.AACS + "-" + RawAudioOutputHandler.class.getSimpleName();

    private static int WRITE_BUFFER_SIZE = 640;

    private final String mChannel;
    private final EventReceiver mEventReceiver;
    private AudioTrack mAudioTrack;
    private Thread mAudioPlaybackThread;
    private int mSampleRateInHzz;
    private int mAudioFormat;
    private int mStreamType;
    private AASBStream mMediaStream;
    private String mCurrentToken = "";
    private AtomicLong mBytesWritten = new AtomicLong(0);
    private boolean mStopped = false;
    private float mVolume = 0.5f;
    private String mMutedState = MutedState.UNMUTED;

    private final AudioManager mAudioManager;
    private final AudioFocusRequest mRawAudioFocusRequest;

    public RawAudioOutputHandler(@NonNull Context context, @NonNull String channel, @NonNull String type,
            @NonNull EventReceiver eventReceiver) {
        mChannel = channel;
        mEventReceiver = eventReceiver;
        mSampleRateInHzz = 16000;
        mAudioFormat = AudioFormat.CHANNEL_OUT_MONO;
        mStreamType = AudioManager.STREAM_VOICE_CALL;

        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        AudioFocusAttributes audioFocusAttributes = new AudioFocusAttributes(type);
        AudioAttributes audioAttributes = new AudioAttributes.Builder()
                                                  .setUsage(audioFocusAttributes.mUsage)
                                                  .setContentType(audioFocusAttributes.mContentType)
                                                  .build();

        mRawAudioFocusRequest = new AudioFocusRequest.Builder(audioFocusAttributes.mFocusGain)
                                        .setAudioAttributes(audioAttributes)
                                        .setOnAudioFocusChangeListener(this)
                                        .setWillPauseWhenDucked(audioFocusAttributes.mWillPauseWhenDucked)
                                        .setAcceptsDelayedFocusGain(audioFocusAttributes.mAcceptsDelayedFocusGain)
                                        .build();

        initializePlayer();
    }

    private void initializePlayer() {
        int audioBufferSize =
                AudioTrack.getMinBufferSize(mSampleRateInHzz, mAudioFormat, AudioFormat.ENCODING_PCM_16BIT);

        mAudioTrack = new AudioTrack(new AudioAttributes.Builder().setLegacyStreamType(mStreamType).build(),
                new AudioFormat.Builder()
                        .setChannelMask(mAudioFormat)
                        .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                        .setSampleRate(mSampleRateInHzz)
                        .build(),
                audioBufferSize, AudioTrack.MODE_STREAM, AudioManager.AUDIO_SESSION_ID_GENERATE);

        if (mAudioTrack.getState() == AudioTrack.STATE_UNINITIALIZED) {
            throw new RuntimeException("Failed to create AudioTrack");
        }
    }

    private void resetPlayer() {
        if (mAudioTrack != null) {
            mAudioTrack.release();
            mAudioTrack.flush();
            mBytesWritten.set(0);
        }
        initializePlayer();
    }

    public boolean isPlaying() {
        return mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING;
    }

    @Override
    public void prepare(AASBStream mediaStream, boolean repeating, String token) {
        Log.v(TAG, String.format("(%s) Handling prepare() given AASBStream.", mChannel));
        mCurrentToken = token;
        mMediaStream = mediaStream;
        resetPlayer();
    }

    @Override
    public void prepare(String url, boolean repeating, String token) {
        Log.e(TAG, String.format("(%s) Playing with URL not supported.", mChannel));
    }

    @Override
    public void play() {
        Log.v(TAG, String.format("(%s) Handling play()", mChannel));
        int result = mAudioManager.requestAudioFocus(mRawAudioFocusRequest);
        if (result == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
            mAudioTrack.play();
            mStopped = false;
            mAudioPlaybackThread = new Thread(new AudioSampleReadWriteRunnable());
            mAudioPlaybackThread.start();
        } else {
            Log.w(TAG, "AudioFocus request not granted. Audio not playing.");
            String message = "Audio focus request failed.";
            MediaPlayerUtil.sendMediaErrorMessage(mEventReceiver, mCurrentToken,
                    MediaConstants.MediaError.MEDIA_ERROR_INTERNAL_DEVICE_ERROR, message, mChannel);
        }
    }

    @Override
    public boolean pause() {
        Log.v(TAG, String.format("(%s) Handling pause()", mChannel));
        mAudioTrack.pause();
        return true;
    }

    @Override
    public boolean resume() {
        Log.v(TAG, String.format("(%s) Handling resume()", mChannel));
        if (!mStopped)
            mAudioTrack.play();
        mAudioPlaybackThread = new Thread(new AudioSampleReadWriteRunnable());
        mAudioPlaybackThread.start();
        return true;
    }

    @Override
    public boolean stop() {
        Log.v(TAG, String.format("(%s) Handling stop()", mChannel));
        mAudioManager.abandonAudioFocusRequest(mRawAudioFocusRequest);
        mAudioTrack.stop();
        mBytesWritten.set(0);
        mStopped = true;
        return true;
    }

    @Override
    public long getDuration(String replyToId) {
        Log.w(TAG, String.format("(%s) Duration is not supported for Raw Audio", mChannel));
        return 0;
    }

    @Override
    public long getPosition(String replyToId) {
        Log.v(TAG, String.format("(%s) Handling getPosition()", mChannel));
        long position = Math.abs(mAudioTrack.getPlaybackHeadPosition());
        JSONObject payload = new JSONObject();
        try {
            payload.put("position", position);
        } catch (Exception e) {
            Log.e(TAG, "Failed to create getPositionReply JSON payload.");
        }
        MediaPlayerUtil.sendEvent(mEventReceiver, replyToId, Topic.AUDIO_OUTPUT, Action.AudioOutput.GET_POSITION,
                payload.toString(), mChannel);
        return position;
    }

    @Override
    public boolean setPosition(long position) {
        Log.w(TAG, String.format("(%s) Seek is not supported for Raw Audio", mChannel));
        return true;
    }

    @Override
    public void cleanUp() {
        if (mAudioTrack != null) {
            mAudioTrack.release();
        }
        mAudioTrack = null;
    }

    @Override
    public void volumeChanged(float volume) {
        Log.v(TAG, String.format("(%s) Volume changed to " + volume, mChannel));
        mVolume = volume;
        mAudioTrack.setVolume(volume);
    }

    @Override
    public void mutedStateChanged(String state) {
        Log.v(TAG, String.format("(%s) Muted state changed to " + state, mChannel));
        mAudioTrack.setVolume(state.equals(MutedState.MUTED) ? 0 : mVolume);
        mMutedState = state;
    }

    @Override
    public long getNumBytesBuffered(String replyToId) {
        long bufferedBytes = 0;
        if (mAudioTrack != null) {
            long playbackHead = mAudioTrack.getPlaybackHeadPosition() & 0x00000000ffffffffL;
            int sampleSize = 2;
            int channelCount = 2;
            long writtenFrames = mBytesWritten.get() / channelCount / sampleSize;
            if (writtenFrames >= playbackHead) {
                bufferedBytes = (writtenFrames - playbackHead) * sampleSize * channelCount;
            }
        } else {
            Log.w(TAG, "mAudioTrack is null");
        }
        JSONObject payload = new JSONObject();
        try {
            payload.put("bufferedBytes", bufferedBytes);
        } catch (Exception e) {
            Log.e(TAG, "Failed to create getPositionReply JSON payload.");
        }
        MediaPlayerUtil.sendEvent(mEventReceiver, replyToId, Topic.AUDIO_OUTPUT,
                Action.AudioOutput.GET_NUM_BYTES_BUFFERED, payload.toString(), mChannel);
        return bufferedBytes;
    }

    @Override
    public void onAudioFocusChange(int focusChange) {
        Log.d(TAG, String.format("onAudioFocusChange for RawAudioOutput, new focus=%s", focusChange));
    }

    private class AudioSampleReadWriteRunnable implements Runnable {
        @Override
        public void run() {
            MediaPlayerUtil.sendMediaStateChangedMessage(
                    mEventReceiver, mChannel, mCurrentToken, MediaConstants.MediaState.PLAYING);

            try {
                Log.d(TAG, String.format("(%s) Audio Playback loop started", mChannel));
                byte[] audioBuffer = new byte[WRITE_BUFFER_SIZE];
                while (isPlaying() && !mMediaStream.isClosed()) {
                    int dataRead = mMediaStream.read(audioBuffer);
                    if (dataRead > 0) {
                        int byteWritten = mAudioTrack.write(audioBuffer, 0, dataRead);
                        if (byteWritten > 0) {
                            mBytesWritten.getAndAdd(dataRead);
                        }
                    }
                }
            } catch (Exception exp) {
                Log.e(TAG, exp.getMessage());
                String message = exp.getMessage() != null ? exp.getMessage() : "";
                MediaPlayerUtil.sendMediaErrorMessage(mEventReceiver, mCurrentToken,
                        MediaConstants.MediaError.MEDIA_ERROR_UNKNOWN, message, mChannel);
            } finally {
                MediaPlayerUtil.sendMediaStateChangedMessage(
                        mEventReceiver, mChannel, mCurrentToken, MediaConstants.MediaState.STOPPED);
            }

            Log.d(TAG, String.format("(%s) Audio Playback loop exited", mChannel));
        }
    }
}
