package com.amazon.maccandroid;

import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.isNull;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.content.pm.Signature;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.media.session.MediaControllerCompat;
import android.support.v4.media.session.PlaybackStateCompat;

import com.amazon.maccandroid.model.APIConstants;
import com.amazon.maccandroid.model.Directive.PlayControlDirective;
import com.amazon.maccandroid.model.Directive.PlayDirective;
import com.amazon.maccandroid.model.errors.MediaAppPlayerError;
import com.amazon.maccandroid.model.players.AuthorizedPlayer;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.util.ArrayList;
import java.util.List;

@RunWith(RobolectricTestRunner.class)
@Config(shadows = {ShadowUri.class})
public class MediaAppsDirectivesHandlerTest {
    public static final String TEST_PLAYER_ID = "testPlayerId";
    public static final String TEST_TOKEN = "testToken";
    public static final long TEST_INDEX = 12;
    public static final long OFFSET = 10;
    public static final boolean PRELOAD = true;
    private static final String TEST_CLASS_NAME = "testPlayerId/Class";
    private static final String TEST_SPI_VERSION = "1.0";
    private static final String TEST_PLAYER_COOKIE = "test";
    private static final String TEST_NAVIGATION = "test";
    private MediaAppsDirectivesHandler mClassUnderTest;
    private Context mMockContext;
    private PackageManager mMockPackageManager;
    private PackageInfo mMockPackageInfo;
    private Bundle mMetaDataBundle;
    private ArrayList<ResolveInfo> mMockResolveInfo;
    private MediaControllerCompat mMediaControllerCompat;
    private MediaControllerCompat.TransportControls mMockTransportControls;
    private PlaybackStateCompat mMockPlayBackState;
    private MediaApp mMockMediaApp;

    @Before
    public void setUp() throws Exception {
        mMockContext = Mockito.mock(Context.class);
        mClassUnderTest = new MediaAppsDirectivesHandler(mMockContext);
        mMockPackageManager = Mockito.mock(PackageManager.class);
        mMockPlayBackState = Mockito.mock(PlaybackStateCompat.class);
        mMediaControllerCompat = Mockito.mock(MediaControllerCompat.class);
        mMockTransportControls = Mockito.mock(MediaControllerCompat.TransportControls.class);
        mMetaDataBundle = Mockito.mock(Bundle.class);

        Mockito.when(mMockContext.getPackageManager()).thenReturn(mMockPackageManager);
        Mockito.when(mMediaControllerCompat.getTransportControls()).thenReturn(mMockTransportControls);
        //        Mockito.when(mMediaControllerCompat.isSessionReady()).thenReturn(true);
        addTestMaccApp();

        Mockito.when(mMetaDataBundle.containsKey(Mockito.anyString())).thenReturn(true);
        ServiceInfo serviceInfo = new ServiceInfo();
        serviceInfo.packageName = TEST_PLAYER_ID;
        serviceInfo.name = TEST_CLASS_NAME;
        serviceInfo.metaData = mMetaDataBundle;
        serviceInfo.applicationInfo = new ApplicationInfo();
        ResolveInfo resolveInfo = new ResolveInfo();
        resolveInfo.serviceInfo = serviceInfo;
        resolveInfo.activityInfo = Mockito.mock(ActivityInfo.class);
        mMockResolveInfo = new ArrayList<>();
        mMockResolveInfo.add(resolveInfo);

        Mockito.when(mMediaControllerCompat.getPlaybackState()).thenReturn(mMockPlayBackState);
        Mockito.when(mMockContext.getResources()).thenReturn(Mockito.mock(Resources.class));
        Mockito.when(mMockPackageManager.queryBroadcastReceivers(Mockito.any(Intent.class), Mockito.anyInt()))
                .thenReturn(mMockResolveInfo);
    }

    private void addTestMaccApp() throws PackageManager.NameNotFoundException {
        mMockPackageInfo = new PackageInfo();
        Signature[] mockSignatures = new Signature[1];
        mockSignatures[0] = Mockito.mock(Signature.class);
        mMockPackageInfo.signatures = mockSignatures;
        Mockito.when(mMockPackageManager.getPackageInfo(Mockito.anyString(), Mockito.anyInt()))
                .thenReturn(mMockPackageInfo);
        mMockMediaApp =
                new MediaApp(mMockContext, TEST_PLAYER_ID, TEST_CLASS_NAME, TEST_SPI_VERSION, TEST_PLAYER_COOKIE) {
                    @Override
                    public MediaControllerCompat getMediaController() {
                        return mMediaControllerCompat;
                    }

                    @Override
                    void initValidationData(Context context) {}
                };
        MediaAppsRepository.getInstance().addDiscoveredMediaApp(mMockMediaApp);
        MediaAppsRepository.getInstance().addAuthorizedMediaApp(mMockMediaApp);
    }

    @After
    public void tearDown() throws Exception {}

    //    @Test
    //    public void handlePlayDirectiveWithPreloadTrue() {
    //        PlayDirective directive = new PlayDirective
    //                (TEST_PLAYER_ID, TEST_TOKEN, TEST_INDEX, OFFSET, true, TEST_NAVIGATION);
    //        mClassUnderTest.handleDirective(directive);
    //        Mockito.verify(mMockTransportControls).playFromUri(Mockito.any(Uri.class), isNull(Bundle.class));
    //    }

    //    @Test
    //    public void handlePlayDirectiveWithPreloadFalse() {
    //        PlayDirective directive = new PlayDirective
    //                (TEST_PLAYER_ID, TEST_TOKEN, TEST_INDEX, OFFSET, false, TEST_NAVIGATION);
    //        mClassUnderTest.handleDirective(directive);
    //        Mockito.verify(mMockTransportControls).prepareFromUri(Mockito.any(Uri.class), isNull(Bundle.class));
    //    }

    @Test
    public void handlePlayControlPlayDirective() {
        PlayControlDirective directive =
                new PlayControlDirective(TEST_PLAYER_ID, APIConstants.Directives.PlayControl.PLAY_CONTROL_PLAY);
        Mockito.when(mMockPlayBackState.getActions()).thenReturn(PlaybackStateCompat.ACTION_PLAY);
        mClassUnderTest.handleDirective(directive);
        Mockito.verify(mMockTransportControls).play();
    }

    @Test
    public void handlePlayControlStopDirective() {
        PlayControlDirective directive =
                new PlayControlDirective(TEST_PLAYER_ID, APIConstants.Directives.PlayControl.PLAY_CONTROL_STOP);
        Mockito.when(mMockPlayBackState.getActions()).thenReturn(PlaybackStateCompat.ACTION_STOP);
        mClassUnderTest.handleDirective(directive);
        Mockito.verify(mMockTransportControls).stop();
    }

    @Test
    public void handlePlayControlPauseDirective() {
        PlayControlDirective directive =
                new PlayControlDirective(TEST_PLAYER_ID, APIConstants.Directives.PlayControl.PLAY_CONTROL_PAUSE);
        Mockito.when(mMockPlayBackState.getActions()).thenReturn(PlaybackStateCompat.ACTION_PAUSE);
        mClassUnderTest.handleDirective(directive);
        Mockito.verify(mMockTransportControls).pause();
    }

    @Test
    public void handlePlayControlStartOverDirective() {
        PlayControlDirective directive =
                new PlayControlDirective(TEST_PLAYER_ID, APIConstants.Directives.PlayControl.PLAY_CONTROL_STARTOVER);
        Mockito.when(mMockPlayBackState.getActions()).thenReturn(PlaybackStateCompat.ACTION_SEEK_TO);
        mClassUnderTest.handleDirective(directive);
        long val = 0;
        Mockito.verify(mMockTransportControls).seekTo(Mockito.eq(val));
    }

    @Test
    public void handlePlayControlPreviousDirective() {
        PlayControlDirective directive =
                new PlayControlDirective(TEST_PLAYER_ID, APIConstants.Directives.PlayControl.PLAY_CONTROL_PREVIOUS);
        Mockito.when(mMockPlayBackState.getActions()).thenReturn(PlaybackStateCompat.ACTION_SKIP_TO_PREVIOUS);
        mClassUnderTest.handleDirective(directive);
        Mockito.verify(mMockTransportControls).skipToPrevious();
    }

    @Test
    public void handlePlayControlNextDirective() {
        PlayControlDirective directive =
                new PlayControlDirective(TEST_PLAYER_ID, APIConstants.Directives.PlayControl.PLAY_CONTROL_NEXT);
        Mockito.when(mMockPlayBackState.getActions()).thenReturn(PlaybackStateCompat.ACTION_SKIP_TO_NEXT);
        mClassUnderTest.handleDirective(directive);
        Mockito.verify(mMockTransportControls).skipToNext();
    }

    @Test
    public void handlePlayControlRewindDirective() {
        PlayControlDirective directive =
                new PlayControlDirective(TEST_PLAYER_ID, APIConstants.Directives.PlayControl.PLAY_CONTROL_REWIND);
        Mockito.when(mMockPlayBackState.getActions()).thenReturn(PlaybackStateCompat.ACTION_REWIND);
        mClassUnderTest.handleDirective(directive);
        Mockito.verify(mMockTransportControls).rewind();
    }

    @Test
    public void handlePlayControlFastForwardDirective() {
        PlayControlDirective directive =
                new PlayControlDirective(TEST_PLAYER_ID, APIConstants.Directives.PlayControl.PLAY_CONTROL_FASTFORWARD);
        Mockito.when(mMockPlayBackState.getActions()).thenReturn(PlaybackStateCompat.ACTION_FAST_FORWARD);
        mClassUnderTest.handleDirective(directive);
        Mockito.verify(mMockTransportControls).fastForward();
    }

    @Test
    public void testAuthorizedPlayers() {
        List<AuthorizedPlayer> authorizedPlayerList = new ArrayList<>();
        authorizedPlayerList.add(new AuthorizedPlayer(TEST_PLAYER_ID, true));
        mClassUnderTest.handleAuthorizedPlayers(authorizedPlayerList);
        assertTrue(MediaAppsRepository.getInstance().isAuthorizedApp(TEST_PLAYER_ID));
    }

    @Test
    public void testNotAuthorizedPlayers() {
        List<AuthorizedPlayer> authorizedPlayerList = new ArrayList<>();
        authorizedPlayerList.add(new AuthorizedPlayer(TEST_PLAYER_ID, false));
        mClassUnderTest.handleAuthorizedPlayers(authorizedPlayerList);
        assertNull(MediaAppsRepository.getInstance().getAuthorizedMediaApp(TEST_PLAYER_ID));
    }
}