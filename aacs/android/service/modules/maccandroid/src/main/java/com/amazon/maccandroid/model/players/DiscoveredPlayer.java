/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.maccandroid.model.players;

import com.amazon.maccandroid.MediaApp;

import java.util.ArrayList;
import java.util.List;

public class DiscoveredPlayer {
    public static List<DiscoveredPlayer> convertMediaApps(List<MediaApp> mediaApps) {
        List<DiscoveredPlayer> discoveredPlayers = new ArrayList<>();
        for (MediaApp mediaApp : mediaApps) {
            discoveredPlayers.add(new DiscoveredPlayer(mediaApp.getLocalPlayerId(), mediaApp.getSpiVersion(),
                    mediaApp.getValidationData(), mediaApp.getValidationMethod()));
        }
        return discoveredPlayers;
    }

    private final String mLocalPlayerId;

    private final String mSpiVersion;
    private final List<String> mValidationData;
    private final String mValidationMethod;

    public DiscoveredPlayer(
            String localPlayerId, String spiVersion, List<String> validationData, String validationMethod) {
        mLocalPlayerId = localPlayerId;
        mSpiVersion = spiVersion;
        mValidationData = validationData;
        mValidationMethod = validationMethod;
    }

    public String getLocalPlayerId() {
        return mLocalPlayerId;
    }

    public List<String> getValidationData() {
        return mValidationData;
    }

    public String getValidationMethod() {
        return mValidationMethod;
    }

    public String getSpiVersion() {
        return mSpiVersion;
    }
}
