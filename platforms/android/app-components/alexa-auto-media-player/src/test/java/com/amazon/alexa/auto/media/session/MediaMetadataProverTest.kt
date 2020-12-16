package com.amazon.alexa.auto.media.session

import android.app.Application
import android.net.Uri
import android.os.Build
import android.support.v4.media.MediaMetadataCompat
import android.support.v4.media.MediaMetadataCompat.METADATA_KEY_ALBUM
import android.support.v4.media.MediaMetadataCompat.METADATA_KEY_ALBUM_ART_URI
import android.support.v4.media.MediaMetadataCompat.METADATA_KEY_ARTIST
import android.support.v4.media.MediaMetadataCompat.METADATA_KEY_DISPLAY_TITLE
import android.support.v4.media.MediaMetadataCompat.METADATA_KEY_TITLE
import android.support.v4.media.MediaMetadataCompat.METADATA_KEY_USER_RATING
import androidx.test.core.app.ApplicationProvider
import com.amazon.aacsconstants.TemplateRuntimeConstants
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_THUMBS_DOWN
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_NAME_THUMBS_UP
import com.amazon.aacsconstants.TemplateRuntimeConstants.CONTROL_TYPE_TOGGLE
import com.amazon.alexa.auto.aacs.common.ImageSource
import com.amazon.alexa.auto.aacs.common.MediaSourceProvider
import com.amazon.alexa.auto.aacs.common.MediaSourceProviderLogo
import com.amazon.alexa.auto.aacs.common.PlaybackControl
import com.amazon.alexa.auto.aacs.common.RenderPlayerArt
import com.amazon.alexa.auto.media.Constants.CONTENT_PROVIDER_DRAWABLE_PATH_PREFIX
import com.amazon.alexa.auto.media.R
import com.amazon.alexa.auto.media.session.MediaMetadataProvider.MEDIA_PROVIDER_AMAZON_MUSIC
import com.amazon.alexa.auto.media.session.MediaMetadataProvider.MEDIA_PROVIDER_APPLE_MUSIC
import com.amazon.alexa.auto.media.session.MediaMetadataProvider.MEDIA_PROVIDER_AUDIBLE
import com.amazon.alexa.auto.media.session.MediaMetadataProvider.MEDIA_PROVIDER_I_HEART_RADIO
import com.amazon.alexa.auto.media.session.MediaMetadataProvider.MEDIA_PROVIDER_TUNE_IN
import com.bumptech.glide.RequestBuilder
import com.bumptech.glide.RequestManager
import com.bumptech.glide.load.engine.GlideException
import com.bumptech.glide.request.FutureTarget
import com.bumptech.glide.request.RequestListener
import com.nhaarman.mockitokotlin2.doReturn
import com.nhaarman.mockitokotlin2.firstValue
import com.nhaarman.mockitokotlin2.mock
import com.nhaarman.mockitokotlin2.thirdValue
import com.nhaarman.mockitokotlin2.times
import com.nhaarman.mockitokotlin2.verify
import io.reactivex.rxjava3.observers.TestObserver
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.ArgumentCaptor
import org.mockito.ArgumentMatchers.any
import org.mockito.ArgumentMatchers.anyString
import org.mockito.Captor
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.MockitoAnnotations
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config
import java.io.File
import java.lang.ref.WeakReference
import java.util.*

@RunWith(RobolectricTestRunner::class)
@Config(sdk = intArrayOf(Build.VERSION_CODES.O))
class MediaMetadataProverTest {
    companion object Constants {
        const val TITLE = "test-title"
        const val ARTIST = "test-artist"
        const val ALBUM = "test-album"
        const val MEDIA_LENGTH : Long = 42

        val mspLogoImage = ImageSource(null, "logo-url")
        val smallSizeImage = ImageSource(TemplateRuntimeConstants.IMAGE_SIZE_SMALL, "small-url")
    }

    lateinit var mClassUnderTest: MediaMetadataProvider

    @Mock lateinit var mMockGlideRequestManager: RequestManager

    @Captor private lateinit var mAlbumArtRequestListener : ArgumentCaptor<RequestListener<File>>

    private lateinit var mApplication: Application

    @Before
    fun setup() {
        MockitoAnnotations.openMocks(this)
        mApplication = ApplicationProvider.getApplicationContext<Application>()
        mClassUnderTest = MediaMetadataProvider(mMockGlideRequestManager, WeakReference(mApplication))
    }

    @Test
    fun testOnNullRenderInfoNoMetadataIsPublished() {
        val metaDataObserver = mClassUnderTest.updateMetadata(null).test()
        metaDataObserver.awaitCount(1)
        metaDataObserver.assertValue { !it.isPresent }
    }

    @Test
    fun testMetadataCanBeConstructedFromMinimalRenderInfo() {
        val playerInfo = RenderPlayerInfoBuilder().build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)
    }

    @Test
    fun testTitleAlbumArtistWithMusicProvider() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withMediaSourceProvider(
                MediaSourceProvider(
                    MediaSourceProviderLogo(listOf()),
                    MEDIA_PROVIDER_I_HEART_RADIO
                )
            )
            .withTitle(TITLE)
            .withSubText1(ARTIST)
            .withSubText2(ALBUM)
            .build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)

        val artist_and_msp = ARTIST + mApplication.resources.getString(R.string.artist_msp_seperator) + MEDIA_PROVIDER_I_HEART_RADIO

        assertEquals(TITLE, metadata.getString(METADATA_KEY_DISPLAY_TITLE))
        assertEquals(TITLE, metadata.getString(METADATA_KEY_TITLE))
        assertEquals(artist_and_msp, metadata.getString(METADATA_KEY_ARTIST))
        assertEquals(ALBUM, metadata.getString(METADATA_KEY_ALBUM))
    }

    @Test
    fun testTitleAlbumArtistWithMusicProviderAndTitleInHeader() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withMediaSourceProvider(
                MediaSourceProvider(
                    MediaSourceProviderLogo(listOf()),
                    MEDIA_PROVIDER_AMAZON_MUSIC
                )
            )
            .withHeader(TITLE)
            .withSubText1(ARTIST)
            .withSubText2(ALBUM)
            .build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)

        val artist_and_msp = ARTIST + mApplication.resources.getString(R.string.artist_msp_seperator) + MEDIA_PROVIDER_AMAZON_MUSIC

        assertEquals(TITLE, metadata.getString(METADATA_KEY_DISPLAY_TITLE))
        assertEquals(TITLE, metadata.getString(METADATA_KEY_TITLE))
        assertEquals(artist_and_msp, metadata.getString(METADATA_KEY_ARTIST))
        assertEquals(ALBUM, metadata.getString(METADATA_KEY_ALBUM))
    }

    @Test
    fun testTitleAlbumArtistWithMusicProviderAndSameTitleAndAlbum() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withMediaSourceProvider(
                MediaSourceProvider(
                    MediaSourceProviderLogo(listOf()),
                    MEDIA_PROVIDER_APPLE_MUSIC
                )
            )
            .withHeader(ALBUM)
            .withTitle(TITLE)
            .withSubText1(ARTIST)
            .withSubText2(TITLE)
            .build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)

        val artist_and_msp = ARTIST + mApplication.resources.getString(R.string.artist_msp_seperator) + MEDIA_PROVIDER_APPLE_MUSIC

        assertEquals(TITLE, metadata.getString(METADATA_KEY_DISPLAY_TITLE))
        assertEquals(TITLE, metadata.getString(METADATA_KEY_TITLE))
        assertEquals(artist_and_msp, metadata.getString(METADATA_KEY_ARTIST))
        assertEquals(ALBUM, metadata.getString(METADATA_KEY_ALBUM))
    }

    @Test
    fun testTitleAlbumArtistWithBookProvider() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withMediaSourceProvider(
                MediaSourceProvider(
                    MediaSourceProviderLogo(listOf()),
                    MEDIA_PROVIDER_AUDIBLE
                )
            )
            .withHeader(ARTIST)
            .withTitle(TITLE)
            .withSubText1(ALBUM)
            .build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)

        val artist_and_msp = ARTIST + mApplication.resources.getString(R.string.artist_msp_seperator) + MEDIA_PROVIDER_AUDIBLE

        assertEquals(TITLE, metadata.getString(METADATA_KEY_DISPLAY_TITLE))
        assertEquals(TITLE, metadata.getString(METADATA_KEY_TITLE))
        assertEquals(artist_and_msp, metadata.getString(METADATA_KEY_ARTIST))
        assertEquals(ALBUM, metadata.getString(METADATA_KEY_ALBUM))
    }

    @Test
    fun testTitleAlbumArtistWithRadioProvider() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withMediaSourceProvider(
                MediaSourceProvider(
                    MediaSourceProviderLogo(listOf()),
                    MEDIA_PROVIDER_TUNE_IN
                )
            )
            .withSubText1(TITLE)
            .withSubText2(ARTIST)
            .build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)

        val artist_and_msp = ARTIST + mApplication.resources.getString(R.string.artist_msp_seperator) + MEDIA_PROVIDER_TUNE_IN
        assertEquals(TITLE, metadata.getString(METADATA_KEY_DISPLAY_TITLE))
        assertEquals(TITLE, metadata.getString(METADATA_KEY_TITLE))
        assertEquals(artist_and_msp, metadata.getString(METADATA_KEY_ARTIST))
    }

    @Test
    fun testMetadataHostsUnRatedRatingObject() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_THUMBS_UP, false, CONTROL_TYPE_TOGGLE))
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_THUMBS_DOWN, false, CONTROL_TYPE_TOGGLE))
            .build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)

        val ratingCompat = metadata.getRating(METADATA_KEY_USER_RATING)
        assertNotNull(ratingCompat)
        assertFalse(ratingCompat.isRated)
    }

    @Test
    fun testMetadataHostsThumbsUpRatingObject() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_THUMBS_UP, true, CONTROL_TYPE_TOGGLE))
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_THUMBS_DOWN, false, CONTROL_TYPE_TOGGLE))
            .build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)

        val ratingCompat = metadata.getRating(METADATA_KEY_USER_RATING)
        assertNotNull(ratingCompat)
        assertTrue(ratingCompat.isRated)
        assertTrue(ratingCompat.isThumbUp)
    }

    @Test
    fun testMetadataHostsThumbsDownRatingObject() {
        val playerInfo = RenderPlayerInfoBuilder()
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_THUMBS_UP, false, CONTROL_TYPE_TOGGLE))
            .addPlaybackControl(PlaybackControl(
                true, CONTROL_NAME_THUMBS_DOWN, true, CONTROL_TYPE_TOGGLE))
            .build()

        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)

        val ratingCompat = metadata.getRating(METADATA_KEY_USER_RATING)
        assertNotNull(ratingCompat)
        assertTrue(ratingCompat.isRated)
        assertFalse(ratingCompat.isThumbUp)
    }

    @Test
    fun testAlbumArt() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withArt(RenderPlayerArt(listOf(smallSizeImage)))
            .build()

        // Set up mockito so that mock request builder is created when download
        // is started by the classUnderTest.
        val mockDownload = makeMockDownload()

        // Start update operation.
        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        assertNotNull(testAndFetchUpdate(metaDataObserver, 0))

        Mockito.verify(mockDownload.first, Mockito.times(1)).addListener(
            mAlbumArtRequestListener.capture())

        val downloadPath = "a/b/downloaded.png"
        val downloadedFile = makeMockDownloadedFile(downloadPath)

        // Signal File Download
        Mockito.`when`(mockDownload.second.isDone).thenReturn(true)
        mAlbumArtRequestListener.value.onResourceReady(downloadedFile, null, null, null, true)

        // Wait for 2nd update (index 1), it should have media art
        val metadataWithArt = testAndFetchUpdate(metaDataObserver, /* 2nd update */ 1)
        metaDataObserver.assertComplete()

        val artFileUri = metadataWithArt.getString(METADATA_KEY_ALBUM_ART_URI)

        assertNotNull(artFileUri)
        assertEquals("/" + downloadPath, Uri.parse(artFileUri).path)

        // Verify that download future is not cancelled (because task finished).
        verify(mockDownload.second, times(0)).cancel(true)
    }

    @Test
    fun `test albumart download is cancelled on finishing before download completion`() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withArt(RenderPlayerArt(listOf(smallSizeImage)))
            .build()

        // Set up mockito so that mock request builder is created when download
        // is started by the classUnderTest.
        val mockDownload = makeMockDownload()

        // Start update operation.
        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo)
        Mockito.`when`(mockDownload.second.isDone).thenReturn(false)

        // Start download.
        val metadataSubscription = metaDataObserver.subscribe {  }

        // Exit before download completion.
        metadataSubscription.dispose()

        // Verify that download future is cancelled.
        verify(mockDownload.second, times(1)).cancel(true)
    }

    @Test
    fun testUpdatingSameMetadataDoesNotYieldWithoutAlbumArt() {
        // This test helps us verify that if we update with same render player info
        // as the one we updated with last time, then we would not yield metadata
        // that doesn't carry album art. Typically we emit metadata without album
        // art while we download album art, and then emit with album art.
        // However if the media is same that would cause flashes on UI and we want
        // to avoid such flashes by not yielding metadata without album art for
        // same update.

        val playerInfo = RenderPlayerInfoBuilder()
            .withTitle(TITLE) // Title is a key for recognizing metadata equivalence.
            .withArt(RenderPlayerArt(listOf(smallSizeImage)))
            .build()

        // Set up mockito so that mock request builder is created when download
        // is started by the classUnderTest.
        val mockDownload = makeMockDownload()

        // Start update operation.
        mClassUnderTest.updateMetadata(playerInfo).test()

        Mockito.verify(mockDownload.first, Mockito.times(1)).addListener(
            mAlbumArtRequestListener.capture())

        val downloadPath = "/a/b/downloaded.png"
        val downloadedFile = makeMockDownloadedFile(downloadPath)

        // Signal File Download
        mAlbumArtRequestListener.firstValue.onResourceReady(downloadedFile, null, null, null, true)

        // Start another update with same render info.
        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()

        // And signal another file download
        Mockito.verify(mockDownload.first, Mockito.times(2)).addListener(
            mAlbumArtRequestListener.capture())

        // Argument captor is cumulative. It captured one listener first time around, and 2
        // listeners second time around (out of which 1st one is the one it had already
        // captured before)
        mAlbumArtRequestListener.thirdValue.onResourceReady(downloadedFile, null, null, null, true)

        // Verify that first update we receive carries album art.
        val metadataWithArt = testAndFetchUpdate(metaDataObserver, /* 1st update */ 0)
        metaDataObserver.assertComplete() // No more metadata update is expected.

        val artFileUri = metadataWithArt.getString(METADATA_KEY_ALBUM_ART_URI)

        assertNotNull(artFileUri)
        assertEquals(downloadPath, Uri.parse(downloadPath).path)
    }

    @Test
    fun testAlbumArtDownloadFailure() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withArt(RenderPlayerArt(listOf(smallSizeImage)))
            .build()

        // Set up mockito so that mock request builder is created when download
        // is started by the classUnderTest.
        val mockDownload = makeMockDownload()

        // Start update operation.
        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        assertNotNull(testAndFetchUpdate(metaDataObserver, 0))

        Mockito.verify(mockDownload.first, Mockito.times(1)).addListener(
            mAlbumArtRequestListener.capture())

        // Signal File Download failed.
        mAlbumArtRequestListener.value.onLoadFailed(GlideException("error"), null, null, true)

        // Verify that stream is completed with 1 value that is emitted prior to
        // download.
        metaDataObserver.assertComplete()
        metaDataObserver.assertValueCount(1)
    }

    @Test
    fun testMSPIsIncludedInMetadata() {
        val playerInfo = RenderPlayerInfoBuilder()
            .withMediaSourceProvider(MediaSourceProvider(
                MediaSourceProviderLogo(
                    listOf(mspLogoImage)
                ),
                MEDIA_PROVIDER_AMAZON_MUSIC
            )).build()

        val mspLogoDownload = makeMockDownload()

        // Start update operation.
        val metaDataObserver = mClassUnderTest.updateMetadata(playerInfo).test()
        val metadata = testAndFetchUpdate(metaDataObserver, 0)
        assertNotNull(metadata)

        val mspResourceUri = metadata.getString(MediaMetadataProvider.CUSTOM_MUSIC_SOURCE_PROVIDER_IMG_URI)

        assertNotNull(mspResourceUri)
        assertEquals(mspLogoImage.url, Uri.parse(mspResourceUri).path)
    }

    /**
     * Wait for nth metadata update, and test that it is present.
     *
     * @param metadataObserver Test observer for metadata.
     * @param nthIndex Index for Nth update to wait for (Index starts from 0).
     * @return Nth metadata update.
     */
    private fun testAndFetchUpdate(
        metadataObserver: TestObserver<Optional<MediaMetadataCompat>>,
        nthIndex: Int) : MediaMetadataCompat {

        metadataObserver.awaitCount(nthIndex + 1)
        metadataObserver.assertValueAt(nthIndex) { it.isPresent }

        val metadata = metadataObserver.values()[nthIndex]
        assertTrue(metadata.isPresent)

        return metadata.get()
    }

    /**
     * Makes an mock instance of Download Request Builder.
     */
    private fun makeMockDownload() : Pair<RequestBuilder<File>, FutureTarget<File>> {
        val mockRequestBuilder = mock<RequestBuilder<File>> ()
        val mockDownloadFuture = mock<FutureTarget<File>> ()

        Mockito.`when`(mockRequestBuilder.load(anyString())).thenReturn(mockRequestBuilder)
        Mockito.`when`(mockRequestBuilder.addListener(any())).thenReturn(mockRequestBuilder)
        Mockito.`when`(mMockGlideRequestManager.applyDefaultRequestOptions(Mockito.any()))
            .thenReturn(mMockGlideRequestManager)
        Mockito.`when`(mMockGlideRequestManager.downloadOnly()).thenReturn(mockRequestBuilder)
        Mockito.`when`(mockRequestBuilder.submit()).thenReturn(mockDownloadFuture)

        return Pair(mockRequestBuilder, mockDownloadFuture)
    }

    /**
     * Make a mocked downloaded file.
     */
    private fun makeMockDownloadedFile(downloadPath: String) =
        mock<File> {
            on { path } doReturn downloadPath
        }
}