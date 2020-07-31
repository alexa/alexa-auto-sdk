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

import com.amazon.aace.alexa.EqualizerController.EqualizerBand;
import com.amazon.aace.alexa.EqualizerController.EqualizerBandLevel;

import java.util.Arrays;

/**
 * A sample device equalizer configuration. Values refer to elements provided in
 * {@link com.amazon.aace.alexa.config.AlexaConfiguration#createEqualizerControllerConfig }
 */
public class EqualizerConfiguration {
    /**
     * The absolute gain in integer dB that represents the minimum level for an equalizer band
     */
    private static final int MIN_BAND_LEVEL = -8;

    /**
     * The absolute gain in integer dB that represents the maximum level for an equalizer band
     */
    private static final int MAX_BAND_LEVEL = 8;

    /**
     * The supported equalizer bands
     *
     * note: It is not required to support all 3 bands. Un-configured defaults to all 3 bands
     * supported.
     */
    private static final EqualizerBand[] SUPPORTED_BANDS = {
            EqualizerBand.BASS, EqualizerBand.MIDRANGE, EqualizerBand.TREBLE};

    /**
     * The default or "reset" gain in integer dB of each supported equalizer band
     *
     * note: The default configuration settings must obey the provided min/max range, and each
     * supported band must have a default setting if the EQ default setting config is provided. The
     * settings here are arbitrary and for example. Un-configured defaults to 0dB for each band.
     */
    private static final EqualizerBandLevel[] DEFAULT_BAND_LEVELS = {new EqualizerBandLevel(EqualizerBand.BASS, 0),
            new EqualizerBandLevel(EqualizerBand.MIDRANGE, 0), new EqualizerBandLevel(EqualizerBand.TREBLE, 0)};

    /**
     * Get the absolute gain representing the minimum level for an equalizer band
     *
     * @return The minimum level in integer dB
     */
    public static int getMinBandLevel() {
        return MIN_BAND_LEVEL;
    }

    /**
     * Get the absolute gain representing the maximum level for an equalizer band
     *
     * @return The minimum level in integer dB
     */
    public static int getMaxBandLevel() {
        return MAX_BAND_LEVEL;
    }

    /**
     * Get the supported equalizer bands
     *
     * @return An array of supported equalizer bands
     */
    public static EqualizerBand[] getSupportedBands() {
        return Arrays.copyOf(SUPPORTED_BANDS, SUPPORTED_BANDS.length);
    }

    /**
     * Get the default or "reset" gain settings of the supported equalizer bands
     *
     * @return An array of default equalizer band level settings, with settings expressed in
     *         integer dB
     */
    public static EqualizerBandLevel[] getDefaultBandLevels() {
        return Arrays.copyOf(DEFAULT_BAND_LEVELS, DEFAULT_BAND_LEVELS.length);
    }
}
