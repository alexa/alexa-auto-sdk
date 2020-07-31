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

package com.amazon.aace.alexa;

import com.amazon.aace.core.PlatformInterface;

/**
 * EqualizerController should be extended to use Alexa to control device equalizer settings. It
 * supports adjusting levels of amplitude gain in decibels (dB) for a 3-band equalizer with a 1dB
 * level of precision.
 *
 * In addition to performing the audio equalization, the platform implementation is also
 * responsible for the following:
 * @li Mapping the provided equalizer bands to frequency ranges.
 * @li Scaling values as absolute levels of amplitude gain in integer dB to the platform's internal
 *  range.
 * @li Mapping the provided bands to a different number of platform-specific target bands, if
 *  necessary.
 * @li Applying equalization to only selected portions of the audio output. Alexa speech, alarms,
 *  etc. may not be affected.
 * @li Persisting level settings across restart/power cycle.
 *
 * Device equalizer settings are configurable. See
 * @c com.amazon.aace.alexa.config.AlexaConfiguration.createEqualizerControllerConfig for details
 * on configuring supported bands, default state, and supported decibel ranges.
 */
abstract public class EqualizerController extends PlatformInterface {
    public EqualizerController() {}

    /**
     * Describes the equalizer bands supported by Alexa. The platform implementation may support a
     * subset of these.
     */
    public enum EqualizerBand {
        /**
         * Bass equalizer band
         * @hideinitializer
         */
        BASS("BASS"),

        /**
         * Midrange equalizer band
         * @hideinitializer
         */
        MIDRANGE("MIDRANGE"),

        /**
         * Treble equalizer band
         * @hideinitializer
         */
        TREBLE("TREBLE");

        /**
         * @internal
         * The string representation of the EqualizerBand
         */
        private String mName;

        /**
         * @internal
         */
        private EqualizerBand(String name) {
            mName = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return mName;
        }
    }

    /**
     * Describes the level of gain of a particular equalizer band as an integer dB value.
     */
    public static class EqualizerBandLevel {
        /// The equalizer band
        private EqualizerBand mBand;
        /// The gain level setting of the band in integer dB
        private int mLevel;

        /**
         * EqualizerBandLevel constructor
         *
         * @param  band The equalizer band
         * @param  level The gain level setting of the band in integer dB
         */
        public EqualizerBandLevel(EqualizerBand band, int level) {
            mBand = band;
            mLevel = level;
        }

        /**
         * Get the equalizer band
         *
         * @return The equalizer band
         */
        public EqualizerBand getBand() {
            return mBand;
        }

        /**
         * Get the gain level setting of the equalizer band
         *
         * @return The level setting in dB
         */
        public int getLevel() {
            return mLevel;
        }
    }

    /**
     * Notifies the platform implementation to apply the provided gain settings to the
     * corresponding equalizer bands.
     *
     * @note Calling @c EqualizerController.localSetBandLevels,
     * @c EqualizerController.localAdjustBandLevels, or @c EqualizerController.localResetBands from
     * inside this function will cause deadlock.
     *
     * @param  bandLevels The equalizer bands and their gain settings to apply as integer dB values.
     */
    public void setBandLevels(EqualizerBandLevel[] bandLevels) {}

    /**
     * Retrieves the current equalizer gain settings on the device for each supported band. If
     * unsupported band levels are provided, the Engine will truncate levels to the configured
     * range.
     *
     * @return An array of the supported equalizer bands and their current gain settings as integer
     *         dB values.
     */
    public EqualizerBandLevel[] getBandLevels() {
        return new EqualizerBandLevel[0];
    }

    /**
     * Notifies the Engine that gain levels for one or more equalizer bands are being set directly
     * on the device. If unsupported levels are provided, the Engine will truncate the settings to
     * the configured range.
     *
     * @param  bandLevels The equalizer bands to change and their gain settings as integer dB
     *         values.
     */
    public final void localSetBandLevels(EqualizerBandLevel[] bandLevels) {
        if (bandLevels != null) {
            localSetBandLevels(getNativeRef(), bandLevels);
        }
    }

    /**
     * Notifies the Engine that relative adjustments to equalizer band gain levels are being made
     * directly on the device. If adjustments put the band level settings beyond the configured dB
     * range, the Engine will truncate the settings to the configured range.
     *
     * @param  bandAdjustments The equalizer bands to adjust and their relative gain adjustments
     *         as integer dB values.
     */
    public final void localAdjustBandLevels(EqualizerBandLevel[] bandAdjustments) {
        if (bandAdjustments != null) {
            localAdjustBandLevels(getNativeRef(), bandAdjustments);
        }
    }

    /**
     * Notifies the Engine that the gain levels for the specified equalizer bands are being reset
     * to their defaults.
     *
     * @param  bands The equalizer bands to reset.
     */
    public final void localResetBands(EqualizerBand[] bands) {
        localResetBands(getNativeRef(), bands);
    }

    /**
     * Notifies the Engine that the gain levels for all equalizer bands are being reset to their
     * defaults.
     */
    public final void localResetBands() {
        localResetBands(getNativeRef(), null);
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native void localSetBandLevels(long nativeObject, EqualizerBandLevel[] bandLevels);
    private native void localAdjustBandLevels(long nativeObject, EqualizerBandLevel[] bandAdjustments);
    private native void localResetBands(long nativeObject, EqualizerBand[] bands);
}
