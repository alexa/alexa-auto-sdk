package com.amazon.maccandroid.model.state;

import android.support.v4.media.MediaMetadataCompat;

public class MediaAppMetaData {
    private final String mPlaybackSource;
    private final String mPlaybackSourceId;
    private final String mTrackName;
    private final String mTrackID;
    private final String mTrackNumber;
    private final String mArtist;
    private final String mAlbum;
    private final String mCoverUrls;
    private final String mMediaProvider;
    private final String mMediaType;
    private final long mDurationInMilliseconds;

    private static final String MEDIA_TYPE_TRACK = "TRACK";
    private static final String MEDIA_TYPE_AD = "AD";

    /**
     * Constructor for when we are not connected to the MediaApp
     */
    public MediaAppMetaData() {
        mPlaybackSource = null;
        mTrackName = null;
        mPlaybackSourceId = null;
        mTrackNumber = null;
        mTrackID = null;
        mArtist = null;
        mAlbum = null;
        mCoverUrls = null;
        mMediaProvider = null;
        mMediaType = null;
        mDurationInMilliseconds = 0;
    }

    public MediaAppMetaData(MediaMetadataCompat metadata) {
        mPlaybackSource = metadata.getString(MediaMetadataCompat.METADATA_KEY_DISPLAY_TITLE);
        mPlaybackSourceId = metadata.getString(MediaMetadataCompat.METADATA_KEY_DISPLAY_ICON_URI);
        mTrackName = metadata.getString(MediaMetadataCompat.METADATA_KEY_TITLE);
        mTrackID = metadata.getString(MediaMetadataCompat.METADATA_KEY_MEDIA_URI);
        mTrackNumber = Long.toString(metadata.getLong(MediaMetadataCompat.METADATA_KEY_TRACK_NUMBER));
        mArtist = metadata.getString(MediaMetadataCompat.METADATA_KEY_ARTIST);
        mAlbum = metadata.getString(MediaMetadataCompat.METADATA_KEY_ALBUM);
        mCoverUrls = metadata.getString(MediaMetadataCompat.METADATA_KEY_ALBUM_ART_URI);
        mMediaProvider = metadata.getString(MediaMetadataCompat.METADATA_KEY_AUTHOR);
        mMediaType = (metadata.getLong(MediaMetadataCompat.METADATA_KEY_ADVERTISEMENT) != 0L) ? MEDIA_TYPE_AD
                                                                                              : MEDIA_TYPE_TRACK;
        mDurationInMilliseconds = metadata.getLong(MediaMetadataCompat.METADATA_KEY_DURATION);
    }

    public String getPlaybackSource() {
        return mPlaybackSource;
    }

    public String getPlaybackSourceId() {
        return mPlaybackSourceId;
    }

    public String getTrackName() {
        return mTrackName;
    }

    public String getTrackId() {
        return mTrackID;
    }

    public String getTrackNumber() {
        return mTrackNumber;
    }

    public String getArtist() {
        return mArtist;
    }

    public String getAlbum() {
        return mAlbum;
    }

    public String getCoverUrls() {
        return mCoverUrls;
    }

    public String getMediaProvider() {
        return mMediaProvider;
    }

    public String getMediaType() {
        return mMediaType;
    }

    public long getDurationInMilliseconds() {
        return mDurationInMilliseconds;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }

        if (!MediaAppMetaData.class.isAssignableFrom(obj.getClass())) {
            return false;
        }

        final MediaAppMetaData otherMetaData = (MediaAppMetaData) obj;

        //        if (mPlaybackSource!= null || otherMetaData.mPlaybackSource != null) {
        //            if ((mPlaybackSource == null && otherMetaData.mPlaybackSource != null) ||
        //            (otherMetaData.mPlaybackSource == null && mPlaybackSource != null)) {
        //                return false;
        //            }
        //
        //            if (mPlaybackSource != null && otherMetaData.mPlaybackSource != null &&
        //            !mPlaybackSource.equals(otherMetaData.mPlaybackSource)) {
        //                return false;
        //            }
        //        }

        if (!checkObjectEquality(mPlaybackSource, otherMetaData.mPlaybackSource)) {
            return false;
        }

        if (!checkObjectEquality(mPlaybackSourceId, otherMetaData.mPlaybackSourceId)) {
            return false;
        }

        if (!checkObjectEquality(mTrackName, otherMetaData.mTrackName)) {
            return false;
        }

        if (!checkObjectEquality(mTrackNumber, otherMetaData.mTrackNumber)) {
            return false;
        }

        if (!checkObjectEquality(mArtist, otherMetaData.mArtist)) {
            return false;
        }

        if (!checkObjectEquality(mAlbum, otherMetaData.mAlbum)) {
            return false;
        }

        if (!checkObjectEquality(mCoverUrls, otherMetaData.mCoverUrls)) {
            return false;
        }

        if (!checkObjectEquality(mMediaProvider, otherMetaData.mMediaProvider)) {
            return false;
        }

        if (!checkObjectEquality(mMediaType, otherMetaData.mMediaType)) {
            return false;
        }

        if (!checkObjectEquality(mDurationInMilliseconds, otherMetaData.mDurationInMilliseconds)) {
            return false;
        }

        return true;
    }

    private boolean checkObjectEquality(Object one, Object two) {
        if (one != null || two != null) {
            if ((one == null && two != null) || (two == null && one != null)) {
                return false;
            }

            if (one != null && two != null && !one.equals(two)) {
                return false;
            }
        }
        return true;
    }
}
