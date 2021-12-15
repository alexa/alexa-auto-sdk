package com.amazon.maccandroid;

import static org.junit.Assert.*;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.os.Looper;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

public class MediaAppsRepositoryTest {
    public static final String TEST_PACKAGE_NAME = "testPackageName";
    public static final String TEST_CLASS_NAME = "testClassName";
    public static final String TEST_SPI_VERSION = "testSpiVersion";
    public static final String TEST_PLAYER_COOKIE = "testPlayerCookie";
    MediaAppsRepository mClassUnderTest;
    Context mMockContext;
    PackageManager mMockPackageManager;
    PackageInfo mMockPackageInfo;

    private boolean mPrepared = false;

    @Before
    public void setUp() throws Exception {
        mClassUnderTest = MediaAppsRepository.getInstance();
        mMockContext = Mockito.mock(Context.class);
        mMockPackageManager = Mockito.mock(PackageManager.class);
        mMockPackageInfo = new PackageInfo();
        Signature[] mockSignatures = new Signature[1];
        mockSignatures[0] = Mockito.mock(Signature.class);
        mMockPackageInfo.signatures = mockSignatures;
        Mockito.when(mMockContext.getPackageManager()).thenReturn(mMockPackageManager);
        Mockito.when(mMockPackageManager.getPackageInfo(Mockito.anyString(), Mockito.anyInt()))
                .thenReturn(mMockPackageInfo);
    }

    @After
    public void tearDown() throws Exception {}

    @Test
    public void addDiscoveredApp() {
        MediaApp discoveredApp =
                MediaApp.create(mMockContext, TEST_PACKAGE_NAME, TEST_CLASS_NAME, TEST_SPI_VERSION, TEST_PLAYER_COOKIE);
        mClassUnderTest.addDiscoveredMediaApp(discoveredApp);
        assertEquals(discoveredApp, mClassUnderTest.getDiscoveredMediaApp(TEST_PACKAGE_NAME));
    }

    @Test
    public void addAuthorizedApp() {
        MediaApp authorizedApp =
                MediaApp.create(mMockContext, TEST_PACKAGE_NAME, TEST_CLASS_NAME, TEST_SPI_VERSION, TEST_PLAYER_COOKIE);
        mClassUnderTest.addAuthorizedMediaApp(authorizedApp);
        assertEquals(authorizedApp, mClassUnderTest.getAuthorizedMediaApp(TEST_PACKAGE_NAME));
    }

    @Test
    public void removeApp() {
        MediaApp app =
                MediaApp.create(mMockContext, TEST_PACKAGE_NAME, TEST_CLASS_NAME, TEST_SPI_VERSION, TEST_PLAYER_COOKIE);
        mClassUnderTest.addDiscoveredMediaApp(app);
        mClassUnderTest.addAuthorizedMediaApp(app);

        mClassUnderTest.removeMediaApp(TEST_PACKAGE_NAME);

        assertNull(mClassUnderTest.getAuthorizedMediaApp(TEST_PACKAGE_NAME));
        assertNull(mClassUnderTest.getDiscoveredMediaApp(TEST_PACKAGE_NAME));
    }
}