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
package com.amazon.alexa.auto.apis.media;

import android.content.Intent;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.alexa.auto.aacs.common.RenderPlayerInfo;

import java.util.Optional;

import io.reactivex.rxjava3.core.Observable;

/**
 * Cache (in memory) the most recent AudioPlayer info for the lifetime of AACS to facilitate re-populating a media
 * session the right metadata if the session is destroyed and re-created within a single AACS runtime cycle.
 *
 * Note that it is intentionally not implemented to cache the data across reboots because Auto SDK does not yet support
 * initializing AudioPlayer and TemplateRuntime capability agents with data from the last cycle. Restoring the data
 * into the MediaSession without doing the same to the capability agents would produce unexpected behavior.
 */
public interface PlayerInfoCache {
    /**
     * Update the current RenderPlayerInfo.
     * @param playerInfo The current RenderPlayerInfo. Empty Optional to reset the player info.
     **/
    void setPlayerInfo(@NonNull Optional<RenderPlayerInfo> playerInfo);

    /**
     * Get the current RenderPlayerInfo.
     * @return An Optional<RenderPlayerInfo>. Empty optional when there is no active player metadata.
     **/
    Optional<RenderPlayerInfo> getPlayerInfo();

    /**
     * Observe the current RenderPlayerInfo.
     * @return An Observable of the RenderPlayerInfo. The Optional<RenderPlayerInfo> will be empty when there is no
     * active player metadata.
     **/
    Observable<Optional<RenderPlayerInfo>> observePlayerInfo();

    /**
     * Update the info for the current media item.
     * @param url The current media info. Null to reset.
     **/
    void setMediaSourceInfo(Optional<MediaSourceInfo> info);

    /**
     * Get the current media source info.
     * @return An Optional<MediaSourceInfo>. Empty optional when there is no active media item.
     **/
    Optional<MediaSourceInfo> getMediaSourceInfo();

    /**
     * Observe the current media source info.
     * @return An Observable of the media source info. Optional empty when there is no active media item.
     **/
    Observable<Optional<MediaSourceInfo>> observeMediaSourceInfo();

    /**
     * Set the current position of the media buffered in the player.
     * @param position The current playback position.
     **/
    void setPlaybackPosition(long position);

    /**
     * Get the last known position of the media buffered in the player.
     * @return The last known playback position.
     **/
    long getPlaybackPosition();

    /**
     * Clear all data from the cache.
     **/
    void clearAllData();
}
