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

import java.util.HashMap;
import java.util.Map;

/**
 * Various utility functions supporting the {@link EqualizerControllerHandler} implementation
 */
public class EQUtils {
    /// Convert EqualizerBandLevel[] to Map<EqualizerBand,Integer>
    public static Map<EqualizerBand, Integer> convertBandLevels(EqualizerBandLevel[] bandLevels) {
        int count = bandLevels.length;
        HashMap<EqualizerBand, Integer> bandsMap = new HashMap<>(count);
        for (int i = 0; i < count; i++) {
            EqualizerBandLevel bandLevel = bandLevels[i];
            bandsMap.put(bandLevel.getBand(), bandLevel.getLevel());
        }
        return bandsMap;
    }

    /// Convert Map<EqualizerBand,Integer> to EqualizerBandLevel[]
    public static EqualizerBandLevel[] convertBandLevels(Map<EqualizerBand, Integer> bandLevels) {
        EqualizerBandLevel[] bandsArr = new EqualizerBandLevel[bandLevels.size()];
        int i = 0;
        for (Map.Entry<EqualizerBand, Integer> entry : bandLevels.entrySet()) {
            EqualizerBandLevel level = new EqualizerBandLevel(entry.getKey(), entry.getValue());
            bandsArr[i] = level;
            i++;
        }
        return bandsArr;
    }

    /// Create a String representation of an EqualizerBandLevel[] for logging
    public static String bandLevelsToString(final EqualizerBandLevel[] bandLevels) {
        StringBuilder sb = new StringBuilder();
        for (EqualizerBandLevel bandLevel : bandLevels) {
            sb.append(bandLevel.getBand()).append("=").append(bandLevel.getLevel()).append(" ");
        }
        return sb.toString();
    }

    /// Convert a SeekBar progress value to EQ setting range
    public static int progressToEqSetting(int progress, int min) {
        return progress + min;
    }

    /// Convert an EQ setting value to SeekBar progress range
    public static int eqSettingToProgress(int setting, int min) {
        return setting - min;
    }
}
