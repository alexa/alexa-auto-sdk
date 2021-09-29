/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexaautoclientservice.modules.mediaManager;

import android.content.ComponentName;
import android.media.MediaMetadata;

/**
 * This is a POJO which holds the configuration of every media source
 */
public class MediaSource {
    private String sourceType;
    private String packageName;
    private String className;
    private boolean playByChannel;
    private boolean playByFrequency;
    private boolean playByPreset;
    private ComponentName componentName;

    private String metadataTitleKey = MediaMetadata.METADATA_KEY_TITLE;
    private String metadataTrackIdKey = MediaMetadata.METADATA_KEY_MEDIA_ID;
    private String metadataTrackNumberKey = MediaMetadata.METADATA_KEY_TRACK_NUMBER;
    private String metadataArtistKey = MediaMetadata.METADATA_KEY_ARTIST;
    private String metadataAlbumKey = MediaMetadata.METADATA_KEY_ALBUM;
    private String metadataDurationKey = MediaMetadata.METADATA_KEY_DURATION;

    public static final String DEFAULT = "DEFAULT";

    /**
     *
     * @param sourceType one of following values BLUETOOTH, USB, FM_RADIO, AM_RADIO, SATELLITE_RADIO, LINE_IN,
     *         COMPACT_DISC, SIRIUS_XM, DAB and DEFAULT
     * @param packageName Package name of the media application
     * @param className Canonical class name of the MediaBrowserService of the local media application
     * @param playByChannel Does this source accept play by channel command?
     * @param playByFrequency Does this source accept play by frequency command?
     * @param playByPreset Does this source accept play by preset command?
     */
    public MediaSource(String sourceType, String packageName, String className, boolean playByChannel,
            boolean playByFrequency, boolean playByPreset) {
        this.sourceType = sourceType;
        this.packageName = packageName;
        this.className = className;
        this.playByChannel = playByChannel;
        this.playByFrequency = playByFrequency;
        this.playByPreset = playByPreset;
    }

    public String getSourceType() {
        return sourceType;
    }

    public String getPackageName() {
        return packageName;
    }

    public void putPackageName(String packageName) {
        this.packageName = packageName;
    }

    public void putClassName(String className) {
        this.className = className;
    }

    public boolean isPlayByChannel() {
        return playByChannel;
    }

    public boolean isPlayByFrequency() {
        return playByFrequency;
    }

    public boolean isPlayByPreset() {
        return playByPreset;
    }

    public ComponentName getComponentName() {
        if (componentName == null) {
            componentName = new ComponentName(packageName, className);
        }
        return componentName;
    }

    public void putMetadataTitleKey(String titleKey) {
        metadataTitleKey = titleKey;
    }

    public void putMetadataTrackIdKey(String trackIdKey) {
        metadataTrackIdKey = trackIdKey;
    }

    public void putMetadataTrackNumberKey(String trackNumberKey) {
        metadataTrackNumberKey = trackNumberKey;
    }

    public void putMetadataArtistKey(String artistKey) {
        metadataArtistKey = artistKey;
    }

    public void putMetadataAlbumKey(String albumKey) {
        metadataAlbumKey = albumKey;
    }

    public void putMetadataDurationKey(String durationKey) {
        metadataDurationKey = durationKey;
    }

    public String getMetadataTitleKey() {
        return metadataTitleKey;
    }

    public String getMetadataTrackIdKey() {
        return metadataTrackIdKey;
    }

    public String getMetadataTrackNumberKey() {
        return metadataTrackNumberKey;
    }

    public String getMetadataArtistKey() {
        return metadataArtistKey;
    }

    public String getMetadataAlbumKey() {
        return metadataAlbumKey;
    }

    public String getMetadataDurationKey() {
        return metadataDurationKey;
    }
}
