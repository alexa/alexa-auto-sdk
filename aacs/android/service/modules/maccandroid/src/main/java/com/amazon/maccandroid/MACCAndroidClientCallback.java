package com.amazon.maccandroid;

import com.amazon.maccandroid.model.PlayerEvents;
import com.amazon.maccandroid.model.players.DiscoveredPlayer;

import java.util.List;
import java.util.Set;
import java.util.UUID;

public interface MACCAndroidClientCallback {
    /**
     * Returns a list of MACC compatible players discovered by MACC client
     * @param discoveredPlayers
     */
    public void onPlayerDiscovered(List<DiscoveredPlayer> discoveredPlayers);

    /**
     * Reports error that happened in the MACC Client
     * @param errorName
     * @param errorCode
     * @param fatal
     * @param playerId
     * @param playbackSessionId
     */
    public void onError(String errorName, int errorCode, boolean fatal, String playerId, UUID playbackSessionId);

    /**
     * Reports a player event which happened to the media apps handled by the MACC client
     * @param playerId
     * @param playerEvents
     * @param skillToken
     * @param playbackSessionId
     */
    public void onPlayerEvent(
            String playerId, Set<PlayerEvents> playerEvents, String skillToken, UUID playbackSessionId);

    /**
     *
     * @param localPlayerId local player id that is requesting a token
     */
    public void requestTokenForPlayerId(String localPlayerId);

    /**
     * A local player id has been removed. e.g. app has been uninstalled or no longer becomes
     * MACC compliant
     * @param localPlayerId
     */
    public void onRemovedPlayer(String localPlayerId);
}
