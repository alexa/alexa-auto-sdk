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
