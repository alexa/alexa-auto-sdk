package com.amazon.alexa.auto.media.session;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.support.v4.media.MediaMetadataCompat;
import android.support.v4.media.RatingCompat;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.TemplateRuntimeConstants;
import com.amazon.alexa.auto.aacs.common.ImageSource;
import com.amazon.alexa.auto.aacs.common.PlaybackControl;
import com.amazon.alexa.auto.aacs.common.RenderPlayerContent;
import com.amazon.alexa.auto.aacs.common.RenderPlayerInfo;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.media.Constants;
import com.amazon.alexa.auto.media.R;
import com.bumptech.glide.RequestBuilder;
import com.bumptech.glide.RequestManager;
import com.bumptech.glide.load.DataSource;
import com.bumptech.glide.load.engine.DiskCacheStrategy;
import com.bumptech.glide.load.engine.GlideException;
import com.bumptech.glide.request.FutureTarget;
import com.bumptech.glide.request.RequestListener;
import com.bumptech.glide.request.RequestOptions;
import com.bumptech.glide.request.target.Target;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Objects;
import java.util.Optional;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.core.Single;
import io.reactivex.rxjava3.disposables.Disposable;

/**
 * Provides the media metadata for media session.
 */
public class MediaMetadataProvider {
    private static final String TAG = MediaMetadataProvider.class.getSimpleName();

    public static final String CUSTOM_MUSIC_SOURCE_PROVIDER_IMG_URI = "MUSIC_SOURCE_PROVIDER_LOGO_URI";

    public static final String MEDIA_PROVIDER_AMAZON_MUSIC = "Amazon Music";
    public static final String MEDIA_PROVIDER_APPLE_MUSIC = "Apple Music";
    public static final String MEDIA_PROVIDER_AUDIBLE = "Audible";
    public static final String MEDIA_PROVIDER_TUNE_IN = "TuneIn";
    public static final String MEDIA_PROVIDER_PANDORA = "Pandora";
    public static final String MEDIA_PROVIDER_KINDLE = "Kindle Books";
    public static final String MEDIA_PROVIDER_I_HEART_RADIO = "iHeartRadio";

    @NonNull
    private final RequestManager mGlideRequestManager;
    @NonNull
    private final WeakReference<Context> mContextWk;
    @NonNull
    private final Handler mMainHandler;
    @Nullable
    private MediaMetadataCompat mPlayerMetadata;

    public MediaMetadataProvider(
            @NonNull RequestManager glideRequestManager, @NonNull WeakReference<Context> contextWk) {
        mGlideRequestManager = glideRequestManager;
        mContextWk = contextWk;
        mMainHandler = new Handler(Looper.getMainLooper());
    }

    /**
     * Computes new metadata for Media Session based on given @c RenderPlayerInfo.
     *
     * @param renderPlayerInfo Template runtime instructions for computing metadata.
     * @return Observable to signal one or maximum two metadata updates. After each
     * update querying this object will also yield the same metadata that was observed.
     * Only one update is sent if there is no media art to download. Two updates are
     * sent when there is a album art. First update is without album art and the second
     * one with album art.
     */
    public Observable<Optional<MediaMetadataCompat>> updateMetadata(@Nullable RenderPlayerInfo renderPlayerInfo) {
        if (renderPlayerInfo == null) {
            this.mPlayerMetadata = null;
            return Observable.just(Optional.empty());
        }

        return observeMediaMetadataUpdates(renderPlayerInfo)
                // Before subscribe callback is executed, we will set the current
                // metadata to the one we just calculated so that fresh metadata
                // can be queried by subscriber from subscribe callback.
                .doOnNext(metadataCompat -> mPlayerMetadata = metadataCompat)
                .map(Optional::of);
    }

    /**
     * Observe media metadata updates.
     *
     * @param playerInfo Information to build media metadata.
     * @return Observable to signal metadata updates.
     */
    private Observable<MediaMetadataCompat> observeMediaMetadataUpdates(@NonNull RenderPlayerInfo playerInfo) {
        Optional<String> albumArtUrl = getAlbumArtUrl(playerInfo);

        if (!albumArtUrl.isPresent()) {
            return Observable.just(toMediaMetadataCompat(playerInfo).build());
        }

        Optional<RequestBuilder<File>> albumArtRequest = albumArtUrl.map(this::prepareImageDownload);

        return Observable.create(emitter -> {
            // Send initial update without album art. Update only if new renderInfo
            // will yield different metadata. Else do not update lest we want to see
            // a flash on UI (yield no album art, then yield same album art).
            if (!isEqivalentToExistingMetadata(playerInfo)) {
                emitter.onNext(toMediaMetadataCompat(playerInfo).build());
            }

            Disposable metadataSubscription = downloadImage(albumArtRequest.orElse(null)).subscribe(albumArtFile -> {
                if (albumArtFile.isPresent()) {
                    MediaMetadataCompat.Builder metadataBuilder = toMediaMetadataCompat(playerInfo);
                    updateAlbumArt(metadataBuilder, albumArtFile.get());
                    emitter.onNext(metadataBuilder.build());
                }
                emitter.onComplete();
            });

            emitter.setCancellable(metadataSubscription::dispose);
        });
    }

    /**
     * Download the image file and return the {@link Single} to observe the
     * status of the download and to cancel download if results are no longer
     * required.
     *
     * @param imageDownloadRequest Image download request.
     * @return {@link Single} to observe download.
     */
    private Single<Optional<File>> downloadImage(@Nullable RequestBuilder<File> imageDownloadRequest) {
        if (imageDownloadRequest == null) {
            return Single.just(Optional.empty());
        }

        return Single.create(emitter -> {
            FutureTarget<File> downloadFuture =
                    imageDownloadRequest
                            .addListener(new RequestListener<File>() {
                                @Override
                                public boolean onLoadFailed(@Nullable GlideException exception, Object model,
                                        Target<File> target, boolean isFirstResource) {
                                    Log.w(TAG, "Failed to download the image. Error: " + exception);
                                    mMainHandler.post(() -> emitter.onSuccess(Optional.empty()));
                                    return true;
                                }

                                @Override
                                public boolean onResourceReady(File resource, Object model, Target<File> target,
                                        DataSource dataSource, boolean isFirstResource) {
                                    mMainHandler.post(() -> emitter.onSuccess(Optional.of(resource)));
                                    return true;
                                }
                            })
                            .submit(); // Start download now.

            emitter.setCancellable(() -> {
                if (!downloadFuture.isDone()) {
                    downloadFuture.cancel(true);
                }
            });
        });
    }

    /**
     * Convert the media metadata to Compat metadata builder. The builder would yield
     * metadata understandable by Android Media Session.
     *
     * @param playerInfo Media Metadata
     * @return Media Metadata Compat Builder.
     */
    @NonNull
    private MediaMetadataCompat.Builder toMediaMetadataCompat(@NonNull RenderPlayerInfo playerInfo) {
        String album = null;
        String artist_and_msp = null;
        String title = null;

        RenderPlayerContent content = playerInfo.getPayload().getContent();
        switch (content.getProvider().getName()) {
            case MEDIA_PROVIDER_TUNE_IN:
                title = content.getTitleSubtext1();
                artist_and_msp = content.getTitleSubtext2();
                break;
            case MEDIA_PROVIDER_AUDIBLE:
            case MEDIA_PROVIDER_KINDLE:
                title = content.getTitle();
                artist_and_msp = content.getHeader();
                album = content.getTitleSubtext1();
                break;
            case MEDIA_PROVIDER_I_HEART_RADIO:
            case MEDIA_PROVIDER_APPLE_MUSIC:
            case MEDIA_PROVIDER_AMAZON_MUSIC:
            case MEDIA_PROVIDER_PANDORA:
            default:
                title = content.getTitle();
                artist_and_msp = content.getTitleSubtext1();
                album = content.getTitleSubtext2();
                if (title == null) {
                    title = content.getHeader();
                } else if (title.equals(album)) {
                    album = content.getHeader();
                }
                break;
        }

        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);
        String providerName = playerInfo.getPayload().getContent().getProvider().getName();
        artist_and_msp = (artist_and_msp == null || artist_and_msp.isEmpty())
                ? providerName
                : (artist_and_msp + context.getResources().getString(R.string.artist_msp_seperator) + providerName);

        MediaMetadataCompat.Builder metadataCompataBuilder =
                new MediaMetadataCompat.Builder()
                        .putLong(MediaMetadataCompat.METADATA_KEY_DURATION,
                                playerInfo.getPayload().getContent().getMediaLengthInMilliseconds())
                        .putString(MediaMetadataCompat.METADATA_KEY_DISPLAY_TITLE, title)
                        .putString(MediaMetadataCompat.METADATA_KEY_TITLE, title)
                        .putString(MediaMetadataCompat.METADATA_KEY_ALBUM, album)
                        .putString(MediaMetadataCompat.METADATA_KEY_ARTIST, artist_and_msp);

        getRatingObject(playerInfo)
                .ifPresent(rating
                        -> metadataCompataBuilder.putRating(MediaMetadataCompat.METADATA_KEY_USER_RATING, rating));

        getMediaSourceProviderImageUrl(playerInfo)
                .ifPresent(mspUrl -> metadataCompataBuilder.putString(CUSTOM_MUSIC_SOURCE_PROVIDER_IMG_URI, mspUrl));

        return metadataCompataBuilder;
    }

    /**
     * Update the album art in the supplied builder.
     *
     * @param builder Builder that is updated with album art.
     * @param albumArtFile Album art file.
     */
    private void updateAlbumArt(@NonNull MediaMetadataCompat.Builder builder, @NonNull File albumArtFile) {
        String albumArtContentUri = toContentProviderUrl(albumArtFile.getPath());
        builder.putString(MediaMetadataCompat.METADATA_KEY_ALBUM_ART_URI, albumArtContentUri)
                .putString(MediaMetadataCompat.METADATA_KEY_DISPLAY_ICON_URI, albumArtContentUri);
    }

    /**
     * Prepare download request for given image url.
     *
     * @param imageUrl Image url.
     * @return Download request which when submitted would yield local file
     * where content is downloaded.
     */
    private RequestBuilder<File> prepareImageDownload(@NonNull String imageUrl) {
        return mGlideRequestManager.applyDefaultRequestOptions(getGlideOptionForAlbumArt())
                .downloadOnly()
                .load(imageUrl);
    }

    /**
     * Converts the file path to Content Provider Url over which the file
     * content can be accessed over @c AlbumArtContentProvider.
     *
     * @param path Path that is to be converted to Content Provider url.
     * @return Content Provider url.
     */
    private static String toContentProviderUrl(@NonNull String path) {
        return new Uri.Builder()
                .scheme(ContentResolver.SCHEME_CONTENT)
                .authority(Constants.CONTENT_PROVIDER_AUTHORITY)
                .appendPath(path)
                .build()
                .toString();
    }

    /**
     * Compare the metadata that was previously computed  with new metadata
     * computation request and determine if they are for same playback.
     * NOTE: This is an approximation test and we cannot say for sure that
     * metadata are equal unless we have some sort of Id in metadata.
     *
     * @param playerInfo Render Player Info for computing new metadata.
     * @return true if the existing metadata is same as the one being computed
     * with new Render Player Info.
     */
    private boolean isEqivalentToExistingMetadata(@NonNull RenderPlayerInfo playerInfo) {
        if (mPlayerMetadata == null) {
            // No metadata existed, so the new non null playerinfo will yield non null
            // metadata.
            return false;
        }

        // We will use title as key to recognize equivalence of metadata.
        // Ideally metadata should have had some sort of Id to help identify it.
        String title = null;
        if (playerInfo.getPayload().getContent().getTitle() == null) {
            title = playerInfo.getPayload().getContent().getTitleSubtext1();
        } else {
            title = playerInfo.getPayload().getContent().getTitle();
        }

        String metadataTitle = null;
        if (mPlayerMetadata.containsKey(MediaMetadataCompat.METADATA_KEY_DISPLAY_TITLE)) {
            metadataTitle = mPlayerMetadata.getString(MediaMetadataCompat.METADATA_KEY_DISPLAY_TITLE);
        }

        return Objects.equals(title, metadataTitle);
    }

    /**
     * Select the most appropriate size for Album Art and return the URL
     * for that.
     *
     * @param metadata Player metadata.
     * @return Album Art Url, if found.
     */
    private static Optional<String> getAlbumArtUrl(@NonNull RenderPlayerInfo metadata) {
        HashMap<String, String> sourceBySize = new HashMap<>();
        for (ImageSource source : metadata.getPayload().getContent().getArt().getSources()) {
            sourceBySize.put(source.getSize(), source.getUrl());
        }

        String[] sizePriority = new String[] {// First priority is to find medium image, and so on.
                TemplateRuntimeConstants.IMAGE_SIZE_MEDIUM, TemplateRuntimeConstants.IMAGE_SIZE_LARGE,
                TemplateRuntimeConstants.IMAGE_SIZE_XLARGE, TemplateRuntimeConstants.IMAGE_SIZE_SMALL,
                TemplateRuntimeConstants.IMAGE_SIZE_XSMALL};

        for (String size : sizePriority) {
            String url = sourceBySize.get(size);
            if (url != null) {
                return Optional.of(url);
            }
        }

        // Did we miss any size that wasn't in our priority list
        if (sourceBySize.size() > 0) {
            // We did miss. lets blindly use the first available url.
            return Optional.of(sourceBySize.entrySet().iterator().next().getValue());
        }

        return Optional.empty();
    }

    /**
     * Obtain the Content Provider url for media source provider logo image.
     *
     * @param playerInfo Player Info.
     * @return Media source provider logo url if available.
     */
    private Optional<String> getMediaSourceProviderImageUrl(@NonNull RenderPlayerInfo playerInfo) {
        if (playerInfo.getPayload().getContent().getProvider().getLogo().getSources().size() > 0) {
            String url = playerInfo.getPayload().getContent().getProvider().getLogo().getSources().get(0).getUrl();
            return Optional.of(url);
        }

        return Optional.empty();
    }

    /**
     * Build rating object if player metadata has rating instructions.
     *
     * @param metadata Player metadata.
     * @return Optional rating object.
     */
    private static Optional<RatingCompat> getRatingObject(@NonNull RenderPlayerInfo metadata) {
        boolean thumbsUp = false, thumbsDown = false;
        boolean thumbsUpSelected = false, thumbsDownSelected = false;
        for (PlaybackControl control : metadata.getPayload().getControls()) {
            switch (control.getName()) {
                case TemplateRuntimeConstants.CONTROL_NAME_THUMBS_UP:
                    if (control.getEnabled()) {
                        thumbsUp = true;
                        thumbsUpSelected = control.getSelected();
                    }
                    break;
                case TemplateRuntimeConstants.CONTROL_NAME_THUMBS_DOWN:
                    if (control.getEnabled()) {
                        thumbsDown = true;
                        thumbsDownSelected = control.getSelected();
                    }
                    break;
            }
        }

        if (thumbsUp || thumbsDown) {
            if (thumbsUpSelected) {
                return Optional.of(RatingCompat.newThumbRating(true));
            } else if (thumbsDownSelected) {
                return Optional.of(RatingCompat.newThumbRating(false));
            } else {
                return Optional.of(RatingCompat.newUnratedRating(RatingCompat.RATING_THUMB_UP_DOWN));
            }
        }

        return Optional.empty();
    }

    /**
     * Provides the glide option that maintains album art cache on File System.
     *
     * @return Glide option to fetch album art.
     */
    private static RequestOptions getGlideOptionForAlbumArt() {
        return new RequestOptions()
                .fallback(R.drawable.default_album_image)
                .diskCacheStrategy(DiskCacheStrategy.RESOURCE);
    }
}
