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

package com.amazon.sampleapp.impl.EqualizerController;

import android.app.Activity;
import android.media.audiofx.Equalizer;
import android.support.annotation.NonNull;
import android.view.View;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazon.aace.alexa.EqualizerController;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * An equalizer controller that receives controls from the Alexa Auto SDK Engine to set equalizer
 * band decibel levels. Provides UI to display the settings and mock locally-initiated changes and
 * notify the Engine.
 */
public class EqualizerControllerHandler extends EqualizerController {
    /// String to identify log entries originating from this file
    private static final String TAG = "EqualizerControllerHandler";
    /// A Logger instance
    private final LoggerHandler mLogger;
    /// A reference to the containing Activity
    private final Activity mActivity;
    /// The minimum level in dB for an equalizer band
    private final int mMinLevel = EqualizerConfiguration.getMinBandLevel();
    /// The maximum level in dB for an equalizer band
    private final int mMaxLevel = EqualizerConfiguration.getMaxBandLevel();
    /// A set of the supported equalizer bands
    private final Set<EqualizerBand> mSupportedBands;
    /// The current state of band level settings
    private Map<EqualizerBand, Integer> mBandSettings;
    /// A map of supported EqualizerBands to corresponding UI controllers
    private Map<EqualizerBand, SeekBar> mBandLevelSeekBars;
    /// The Executor for task handling
    private ExecutorService mExecutor;
    /// EqualizerEventListener
    private EqualizerProvider mEventListener;

    public EqualizerControllerHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mExecutor = Executors.newSingleThreadExecutor();

        // note: Current device settings may be loaded on initialization before the
        // EqualizerController platform interface is registered. Using defaults here for simplicity
        mBandSettings = EQUtils.convertBandLevels(EqualizerConfiguration.getDefaultBandLevels());

        EqualizerBand[] bands = EqualizerConfiguration.getSupportedBands();
        Collections.addAll(mSupportedBands = new HashSet<>(bands.length), bands);

        mBandLevelSeekBars = new HashMap<>(bands.length);
        setupUI();
    }

    @Override
    public void setBandLevels(final EqualizerBandLevel[] bandLevels) {
        mExecutor.submit(new Runnable() {
            public void run() {
                mLogger.postInfo(TAG, String.format("setBandLevels: %s", EQUtils.bandLevelsToString(bandLevels)));

                Map<EqualizerBand, Integer> settings = EQUtils.convertBandLevels(bandLevels);
                updateBandLevelsUI(settings);

                // Notify equalizer listener
                if (mEventListener != null) {
                    Equalizer equalizer = mEventListener.getEqualizer();
                    updateEqualizerLevels(equalizer, settings);
                }
            }
        });
    }

    @Override
    public EqualizerBandLevel[] getBandLevels() {
        // note: Don't do any heavy work here that may block Engine initialization
        mLogger.postVerbose(TAG, "getBandLevels");
        return EQUtils.convertBandLevels(mBandSettings);
    }

    /// Handle a locally-initiated absolute band level change
    private void onLocalBandLevelChange(final EqualizerBand band, final int level) {
        mExecutor.submit(new Runnable() {
            public void run() {
                mLogger.postInfo(TAG, String.format("Local set band: %s=%s", band, level));

                // Tell the Engine...
                EqualizerBandLevel bandLevel = new EqualizerBandLevel(band, level);
                localSetBandLevels(new EqualizerBandLevel[] {bandLevel});
                // Alternatively, we could use localSetBandLevels with the complete current settings
            }
        });
    }

    /// Handle a locally-initiated relative band level change
    private void onLocalBandLevelAdjustment(final EqualizerBand band, final int adjustment) {
        mExecutor.submit(new Runnable() {
            public void run() {
                mLogger.postInfo(TAG, String.format("Local adjust band: %s=%s", band, adjustment));

                // Tell the Engine...
                EqualizerBandLevel bandLevel = new EqualizerBandLevel(band, adjustment);
                localAdjustBandLevels(new EqualizerBandLevel[] {bandLevel});
            }
        });
    }

    /// Handle a locally-initiated all band reset
    private void onLocalResetBands() {
        mExecutor.submit(new Runnable() {
            public void run() {
                mLogger.postInfo(TAG, "Local reset all bands");

                // Tell the Engine...
                localResetBands();
                // Alternatively, we could explicitly specify all the bands we wish to reset...
                // localResetBands( mSupportedBands.toArray( new EqualizerBand[0] ) );
            }
        });
    }

    /// Handle a locally-initiated one band reset
    private void onLocalResetBand(final EqualizerBand band) {
        mExecutor.submit(new Runnable() {
            public void run() {
                mLogger.postInfo(TAG, String.format("Local reset band: %s", band));

                // Tell the Engine...
                localResetBands(new EqualizerBand[] {band});
            }
        });
    }

    /// Initialize UI components
    private void setupUI() {
        // min dB level label
        TextView minLevelText = mActivity.findViewById(R.id.eq_min_level);
        String minText = "" + mMinLevel;
        minLevelText.setText(minText);

        // max dB level label
        TextView maxLevelText = mActivity.findViewById(R.id.eq_max_level);
        String maxText = "" + mMaxLevel;
        maxLevelText.setText(maxText);

        // set controls for each EqualizerBand
        for (final EqualizerBand band : EqualizerBand.values()) {
            View bandControls = null;
            switch (band) {
                case BASS:
                    bandControls = mActivity.findViewById(R.id.eq_bass_control);
                    break;
                case MIDRANGE:
                    bandControls = mActivity.findViewById(R.id.eq_midrange_control);
                    break;
                case TREBLE:
                    bandControls = mActivity.findViewById(R.id.eq_treble_control);
                    break;
            }
            if (bandControls != null) {
                // set text label for controls
                ((TextView) bandControls.findViewById(R.id.band_text)).setText(band.toString());

                // set SeekBar range to the min/max range from config
                SeekBar bandSeekBar = bandControls.findViewById(R.id.band_seekbar);
                bandSeekBar.setMax(Math.abs(mMinLevel) + Math.abs(mMaxLevel));

                // enable the SeekBar if the corresponding EqualizerBand is supported
                if (mSupportedBands.contains(band)) {
                    bandSeekBar.setEnabled(true);
                    // store a mapping of the EqualizerBand to the corresponding SeekBar
                    mBandLevelSeekBars.put(band, bandSeekBar);
                    bandSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {}
                        public void onStartTrackingTouch(SeekBar seekBar) {}
                        public void onStopTrackingTouch(SeekBar seekBar) {
                            onLocalBandLevelChange(band, EQUtils.progressToEqSetting(seekBar.getProgress(), mMinLevel));
                        }
                    });

                    // set SeekBar progress to initial dB setting
                    int level = mBandSettings.containsKey(band) ? mBandSettings.get(band) : 0;
                    bandSeekBar.setProgress(EQUtils.eqSettingToProgress(level, mMinLevel));

                    // set decrement band level button OnClickListener
                    ImageButton adjustDown = bandControls.findViewById(R.id.band_down);
                    adjustDown.setOnClickListener(new View.OnClickListener() {
                        public void onClick(View v) {
                            onLocalBandLevelAdjustment(band, -1);
                        }
                    });

                    // set increment band level button OnClickListener
                    ImageButton adjustUp = bandControls.findViewById(R.id.band_up);
                    adjustUp.setOnClickListener(new View.OnClickListener() {
                        public void onClick(View v) {
                            onLocalBandLevelAdjustment(band, 1);
                        }
                    });

                    // set reset band OnClickListener
                    ImageButton reset = bandControls.findViewById(R.id.reset_band);
                    reset.setOnClickListener(new View.OnClickListener() {
                        public void onClick(View v) {
                            onLocalResetBand(band);
                        }
                    });

                } else {
                    bandSeekBar.setEnabled(false);
                }
            }
        }

        // reset bands button
        View resetBandsButton = mActivity.findViewById(R.id.eq_resetBands);
        resetBandsButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                onLocalResetBands();
            }
        });
    }

    /// Update band level setting SeekBar UI to the values provided in @a bandLevels
    private void updateBandLevelsUI(@NonNull final Map<EqualizerBand, Integer> bandLevels) {
        for (final Map.Entry<EqualizerBand, Integer> bandLevel : bandLevels.entrySet()) {
            final EqualizerBand band = bandLevel.getKey();
            final SeekBar bandSeekBar = mBandLevelSeekBars.get(band);
            if (bandSeekBar != null) {
                mActivity.runOnUiThread(new Runnable() {
                    public void run() {
                        bandSeekBar.setProgress(EQUtils.eqSettingToProgress(bandLevel.getValue(), mMinLevel));
                    }
                });
            }
        }
    }

    /**
     * Interface to send equalizer data.
     */
    public interface EqualizerProvider {
        /**
         * Retrieve equalizer for the current audio session.
         */
        Equalizer getEqualizer();
    }

    /**
     * Object that should be notified of equalizer events.
     * @param eventListener
     */
    public void setEqualizerEventListener(EqualizerProvider eventListener) {
        mEventListener = eventListener;
    }

    /**
     *  Apply equalizer changes
     * @param equalizer The equalizer where changes should be performed
     * @param settings The new values to set
     */
    private void updateEqualizerLevels(Equalizer equalizer, Map<EqualizerBand, Integer> settings) {
        if (equalizer != null) {
            // Get total equalization bands
            int bands = equalizer.getNumberOfBands();
            // Values to band levels can be set to
            short[] levelRange = equalizer.getBandLevelRange();
            // Calculate a mapping between sample app values -8 to 8 to levelRange[0] to levelRange[1]
            short rangeSize = levelRange.length > 1 ? (short) (levelRange[1] - levelRange[0]) : (short) 1;
            // Sample app uses -8db to 8db for a total of 16db range
            short convertedRangeStep = (short) (rangeSize / 16);

            // Equalizer must support more than 1 band. Typically 5, but could be more
            // depending on device
            if (bands > 0) {
                for (final Map.Entry<EqualizerController.EqualizerBand, Integer> bandLevel : settings.entrySet()) {
                    // Use the following indices (bass: 0, mids: 1 to (bands-2), treble: bands-1)
                    final EqualizerController.EqualizerBand band = bandLevel.getKey();
                    if (band.equals(EqualizerController.EqualizerBand.BASS)) {
                        equalizer.setBandLevel((short) 0, (short) (convertedRangeStep * bandLevel.getValue()));
                    } else if (band.equals(EqualizerController.EqualizerBand.MIDRANGE)) {
                        for (short mid = 1; mid < bands - 1; mid++) {
                            equalizer.setBandLevel(mid, (short) (convertedRangeStep * bandLevel.getValue()));
                        }
                    } else if (band.equals(EqualizerController.EqualizerBand.TREBLE)) {
                        equalizer.setBandLevel(
                                (short) (bands - 1), (short) (convertedRangeStep * bandLevel.getValue()));
                    }
                }
            }
        }
    }
}
