/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.amazon.sampleapp.impl.PlaybackController;

import android.app.Activity;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.ToggleButton;

import com.amazon.aace.alexa.PlaybackController;
import com.amazon.aace.audio.AudioOutput;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.Formatter;
import java.util.Locale;

public class PlaybackControllerHandler extends PlaybackController {
    private static final String sTag = PlaybackControllerHandler.class.getSimpleName();

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private final StringBuilder mStringBuilder;
    private final Formatter mFormatter;
    private ImageButton mControlPrev, mControlNext, mControlSkipForward, mControlSkipBackward;
    private ToggleButton mControlPlayPause, mShuffleToggle, mLoopToggle, mRepeatToggle, mThumbsUpToggle,
            mThumbsDownToggle;
    private TextView mProgressTime, mEndTime, mTitle, mArtist, mProvider;
    private ProgressBar mProgress;
    private String mCurrentProvider = "";
    private long mCurrentDuration = AudioOutput.TIME_UNKNOWN;

    public PlaybackControllerHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mStringBuilder = new StringBuilder();
        mFormatter = new Formatter(mStringBuilder, Locale.US);
        setupGUI();
    }

    public void previousButtonPressed() {
        mLogger.postVerbose(sTag, "Calling PREVIOUS buttonPressed");
        buttonPressed(PlaybackButton.PREVIOUS);
    }

    public void playButtonPressed() {
        mLogger.postVerbose(sTag, "Calling PLAY buttonPressed");
        buttonPressed(PlaybackButton.PLAY);
    }

    public void pauseButtonPressed() {
        mLogger.postVerbose(sTag, "Calling PAUSE buttonPressed");
        buttonPressed(PlaybackButton.PAUSE);
    }

    public void nextButtonPressed() {
        mLogger.postVerbose(sTag, "Calling NEXT buttonPressed");
        buttonPressed(PlaybackButton.NEXT);
    }

    public void skipForwardButtonPressed() {
        mLogger.postVerbose(sTag, "Calling SKIP_FORWARD buttonPressed");
        buttonPressed(PlaybackButton.SKIP_FORWARD);
    }

    public void skipBackwardButtonPressed() {
        mLogger.postVerbose(sTag, "Calling SKIP_BACKWARD buttonPressed");
        buttonPressed(PlaybackButton.SKIP_BACKWARD);
    }

    public void shuffleTogglePressed(boolean action) {
        mLogger.postVerbose(
                sTag, String.format("Calling SHUFFLE togglePressed %s", action ? "selected" : "deselected"));
        togglePressed(PlaybackToggle.SHUFFLE, action);
    }

    public void loopTogglePressed(boolean action) {
        mLogger.postVerbose(sTag, String.format("Calling LOOP togglePressed %s", action ? "selected" : "deselected"));
        togglePressed(PlaybackToggle.LOOP, action);
    }

    public void repeatTogglePressed(boolean action) {
        mLogger.postVerbose(sTag, String.format("Calling REPEAT togglePressed %s", action ? "selected" : "deselected"));
        togglePressed(PlaybackToggle.REPEAT, action);
    }

    public void thumbsUpTogglePressed(boolean action) {
        mLogger.postVerbose(
                sTag, String.format("Calling THUMBS_UP togglePressed %s", action ? "selected" : "deselected"));
        togglePressed(PlaybackToggle.THUMBS_UP, action);
    }

    public void thumbsDownTogglePressed(boolean action) {
        mLogger.postVerbose(
                sTag, String.format("Calling THUMBS_DOWN togglePressed %s", action ? "selected" : "deselected"));
        togglePressed(PlaybackToggle.THUMBS_DOWN, action);
    }

    //
    // GUI updates
    //

    private void setupGUI() {
        mControlPrev = mActivity.findViewById(R.id.prevControlButton);
        mControlPlayPause = mActivity.findViewById(R.id.playControlButton);
        mControlNext = mActivity.findViewById(R.id.nextControlButton);
        mControlSkipForward = mActivity.findViewById(R.id.skipForwardControlButton);
        mControlSkipBackward = mActivity.findViewById(R.id.skipBackwardControlButton);

        mShuffleToggle = mActivity.findViewById(R.id.shuffleToggle);
        mLoopToggle = mActivity.findViewById(R.id.loopToggle);
        mRepeatToggle = mActivity.findViewById(R.id.repeatToggle);
        mThumbsUpToggle = mActivity.findViewById(R.id.thumbsUpToggle);
        mThumbsDownToggle = mActivity.findViewById(R.id.thumbsDownToggle);

        mProgress = mActivity.findViewById(R.id.mediaProgressBar);
        mProgressTime = mActivity.findViewById(R.id.mediaProgressTime);
        mEndTime = mActivity.findViewById(R.id.mediaEndTime);
        mTitle = mActivity.findViewById(R.id.mediaTitle);
        mArtist = mActivity.findViewById(R.id.mediaArtist);
        mProvider = mActivity.findViewById(R.id.mediaProvider);

        // playback button listeners
        mControlPrev.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                previousButtonPressed();
                disableButtons();
                disableProgress();
            }
        });

        mControlPlayPause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mControlPlayPause.isChecked()) {
                    playButtonPressed();
                } else {
                    pauseButtonPressed();
                }

                disableButtons();
            }
        });

        mControlNext.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                nextButtonPressed();
                disableButtons();
                disableProgress();
            }
        });

        mControlSkipForward.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                skipForwardButtonPressed();

                // block UI input until state is synced by callback
                mControlSkipForward.setEnabled(false);
            }
        });

        mControlSkipBackward.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                skipBackwardButtonPressed();
                mControlSkipBackward.setEnabled(false);
            }
        });

        mShuffleToggle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Send UI toggle state
                shuffleTogglePressed(mShuffleToggle.isChecked());
                mShuffleToggle.setEnabled(false);
            }
        });

        mLoopToggle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                loopTogglePressed(mLoopToggle.isChecked());
                mLoopToggle.setEnabled(false);
            }
        });

        mRepeatToggle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                repeatTogglePressed(mRepeatToggle.isChecked());
                mRepeatToggle.setEnabled(false);
            }
        });

        mThumbsUpToggle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // ignore un-checking thumbs_up, cloud behavior does not support it
                if (mThumbsUpToggle.isChecked()) {
                    thumbsUpTogglePressed(true);
                    mThumbsUpToggle.setEnabled(false);
                } else
                    mThumbsUpToggle.setChecked(true);
            }
        });

        mThumbsDownToggle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // ignore un-checking thumbs_down, cloud behavior does not support it
                if (mThumbsDownToggle.isChecked()) {
                    thumbsDownTogglePressed(mThumbsDownToggle.isChecked());
                    mThumbsDownToggle.setEnabled(false);
                } else
                    mThumbsDownToggle.setChecked(true);
            }
        });

        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mControlPrev.setEnabled(false);
                mControlPlayPause.setChecked(false);
                mControlPlayPause.setEnabled(false);
                mControlNext.setEnabled(false);

                // MSP-specific controls from the RenderPlayerInfo template are hidden by default
                mControlSkipForward.setVisibility(View.GONE);
                mControlSkipBackward.setVisibility(View.GONE);

                mShuffleToggle.setVisibility(View.GONE);
                mLoopToggle.setVisibility(View.GONE);
                mRepeatToggle.setVisibility(View.GONE);
                mThumbsUpToggle.setVisibility(View.GONE);
                mThumbsDownToggle.setVisibility(View.GONE);
            }
        });

        resetProgress();
    }

    public void setPlayerInfo(final String title, final String artist, final String provider) {
        mCurrentProvider = provider;
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTitle.setText(title);
                mArtist.setText(artist);
                mProvider.setText(provider);
            }
        });
    }

    public String getProvider() {
        return mCurrentProvider;
    }

    public void start() {
        startWithButtonsEnabled(true);
    }

    public void startWithButtonsEnabled(boolean buttonsEnabled) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (buttonsEnabled) {
                    enableButtons();
                }
                enableProgress();
                mControlPlayPause.setChecked(true);
                mProgress.setMax(1000);
            }
        });
    }

    public void stop() {
        stopWithButtonsEnabled(true);
    }

    public void stopWithButtonsEnabled(boolean buttonsEnabled) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (buttonsEnabled) {
                    enableButtons();
                }
                enableProgress();
                mControlPlayPause.setChecked(false);
            }
        });
    }

    public void reset() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mControlPlayPause.setChecked(false);
                mControlPrev.setEnabled(false);
                mControlPlayPause.setEnabled(false);
                mControlNext.setEnabled(false);
                resetProgress();
                setPlayerInfo("", "", "");
            }
        });
    }

    // Updates Control Button's states
    public void updateControlButton(final String name, final boolean enabled) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (name) {
                    case "PREVIOUS":
                        mControlPrev.setEnabled(enabled);
                        break;
                    case "PLAY_PAUSE":
                        mControlPlayPause.setEnabled(enabled);
                        break;
                    case "NEXT":
                        mControlNext.setEnabled(enabled);
                        break;
                    case "SKIP_FORWARD":
                        mControlSkipForward.setVisibility(View.VISIBLE);
                        mControlSkipForward.setEnabled(enabled);
                        break;
                    case "SKIP_BACKWARD":
                        mControlSkipBackward.setVisibility(View.VISIBLE);
                        mControlSkipBackward.setEnabled(enabled);
                        break;
                }
            }
        });
    }

    // Updates Toggle's display states
    // NOTE: Disabled controls not hidden here for development visibility.
    public void updateControlToggle(final String name, final boolean enabled, final boolean selected) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (name) {
                    case "SHUFFLE":
                        mShuffleToggle.setVisibility(View.VISIBLE);
                        mShuffleToggle.setEnabled(enabled);
                        mShuffleToggle.setChecked(selected);
                        break;
                    case "LOOP":
                        mLoopToggle.setVisibility(View.VISIBLE);
                        mLoopToggle.setEnabled(enabled);
                        mLoopToggle.setChecked(selected);
                        break;
                    case "REPEAT":
                        mRepeatToggle.setVisibility(View.VISIBLE);
                        mRepeatToggle.setEnabled(enabled);
                        mRepeatToggle.setChecked(selected);
                        break;
                    case "THUMBS_UP":
                        mThumbsUpToggle.setVisibility(View.VISIBLE);
                        mThumbsUpToggle.setEnabled(enabled);
                        mThumbsUpToggle.setChecked(selected);
                        break;
                    case "THUMBS_DOWN":
                        mThumbsDownToggle.setVisibility(View.VISIBLE);
                        mThumbsDownToggle.setEnabled(enabled);
                        mThumbsDownToggle.setChecked(selected);
                        break;
                }
            }
        });
    }

    public void hidePlayerInfoControls() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mControlSkipForward.setVisibility(View.GONE);
                mControlSkipBackward.setVisibility(View.GONE);

                mShuffleToggle.setVisibility(View.GONE);
                mLoopToggle.setVisibility(View.GONE);
                mRepeatToggle.setVisibility(View.GONE);
                mThumbsUpToggle.setVisibility(View.GONE);
                mThumbsDownToggle.setVisibility(View.GONE);
            }
        });
    }

    public void hideAllPlayerControls() {
        hidePlayerInfoControls();
        disableButtons();
    }

    private void resetProgress() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mProgress.setProgress(0);
                mProgressTime.setText(stringForTime(AudioOutput.TIME_UNKNOWN));
                mEndTime.setText(stringForTime(AudioOutput.TIME_UNKNOWN));
            }
        });
    }

    public void setTime(long position, long duration) {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mCurrentDuration != duration) {
                    mEndTime.setText(stringForTime(duration));
                    mCurrentDuration = duration;
                }

                if (mProgress.isEnabled()) {
                    mProgress.setProgress((duration > 0) ? (int) (1000L * position / duration) : 0);
                    mProgressTime.setText(stringForTime(position));
                }
            }
        });
    }

    private String stringForTime(long timeMs) {
        if (timeMs == AudioOutput.TIME_UNKNOWN) {
            return "-:--";
        }

        int totalSeconds = (int) timeMs / 1000;
        int seconds = totalSeconds % 60;
        int minutes = (totalSeconds / 60) % 60;
        int hours = totalSeconds / 3600;

        mStringBuilder.setLength(0);
        if (hours > 0) {
            return mFormatter.format("%d:%02d:%02d", hours, minutes, seconds).toString();
        } else {
            return mFormatter.format("%02d:%02d", minutes, seconds).toString();
        }
    }

    private void enableButtons() {
        mControlPrev.setEnabled(true);
        mControlPlayPause.setEnabled(true);
        mControlNext.setEnabled(true);

        mControlSkipForward.setEnabled(true);
        mControlSkipBackward.setEnabled(true);

        mShuffleToggle.setEnabled(true);
        mLoopToggle.setEnabled(true);
        mRepeatToggle.setEnabled(true);
        mThumbsUpToggle.setEnabled(true);
        mThumbsDownToggle.setEnabled(true);
    }

    private void enableProgress() {
        mProgress.setEnabled(true);
    }

    private void disableButtons() {
        mControlPrev.setEnabled(false);
        mControlPlayPause.setEnabled(false);
        mControlNext.setEnabled(false);

        mControlSkipForward.setEnabled(false);
        mControlSkipBackward.setEnabled(false);

        mShuffleToggle.setEnabled(false);
        mLoopToggle.setEnabled(false);
        mRepeatToggle.setEnabled(false);
        mThumbsUpToggle.setEnabled(false);
        mThumbsDownToggle.setEnabled(false);
    }

    private void disableProgress() {
        mProgress.setEnabled(false);
    }
}
