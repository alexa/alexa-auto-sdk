package com.amazon.maccandroid;

import static org.junit.Assert.assertEquals;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.content.pm.Signature;
import android.os.Bundle;
import android.os.Looper;
import android.test.AndroidTestCase;
import android.test.mock.MockContext;

import com.amazon.maccandroid.model.PackageMetadata;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.ArrayList;
import java.util.List;

public class DiscoverAndReportMediaAppsHandlerTest {
    private static final String JAVA_VERSION_PROPERTY = "java.version";
    public static final String COM_AMAZON_TEST_MACC_APP = "com.amazon.testMaccApp";
    public static final String COM_AMAZON_TEST_MACC_APP_TEST_SERVICE_TEST_MEDIA_BROWSER_SERVICE =
            "com.amazon.testMaccApp.testService.TestMediaBrowserService";

    private DiscoverAndReportMediaAppsHandler mClassToTest;
    private Context mMockContext;
    private PackageManager mMockPackageManager;
    private List<ResolveInfo> mMockResolveInfo;
    private Bundle mMetaDataBundle;
    private PackageInfo mMockPackageInfo;

    @Before
    public void setUp() throws Exception {
        System.setProperty(JAVA_VERSION_PROPERTY, "1.7.0_79");
        mMockContext = Mockito.mock(Context.class);
        mMockPackageManager = Mockito.mock(PackageManager.class);
        Mockito.when(mMockContext.getPackageManager()).thenReturn(mMockPackageManager);
        mClassToTest = new DiscoverAndReportMediaAppsHandler(Looper.getMainLooper(), mMockContext) {
            @Override
            PackageMetadata getPlayerMetaData(ResolveInfo resolveInfo) {
                return new PackageMetadata("testSpiVersion", null);
            }
        };

        mMetaDataBundle = Mockito.mock(Bundle.class);
        Mockito.when(mMetaDataBundle.containsKey(Mockito.anyString())).thenReturn(true);
        ServiceInfo serviceInfo = new ServiceInfo();
        serviceInfo.packageName = COM_AMAZON_TEST_MACC_APP;
        serviceInfo.name = COM_AMAZON_TEST_MACC_APP_TEST_SERVICE_TEST_MEDIA_BROWSER_SERVICE;
        serviceInfo.metaData = mMetaDataBundle;
        serviceInfo.applicationInfo = new ApplicationInfo();
        ResolveInfo resolveInfo = new ResolveInfo();
        resolveInfo.serviceInfo = serviceInfo;
        mMockResolveInfo = new ArrayList<>();
        mMockResolveInfo.add(resolveInfo);

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
    public void handleMessage() {}

    @Test
    public void discoverMediaApps() {
        Mockito.when(mMockPackageManager.queryIntentServices(Mockito.any(Intent.class), Mockito.anyInt()))
                .thenReturn(mMockResolveInfo);
        mClassToTest.discoverMediaApps();
        assertEquals(COM_AMAZON_TEST_MACC_APP,
                MediaAppsRepository.getInstance().getDiscoveredMediaApp(COM_AMAZON_TEST_MACC_APP).getLocalPlayerId());
    }
}