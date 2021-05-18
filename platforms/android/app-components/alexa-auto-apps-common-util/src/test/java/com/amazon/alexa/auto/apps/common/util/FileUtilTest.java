package com.amazon.alexa.auto.apps.common.util;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Handler;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.mockito.junit.MockitoJUnitRunner;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;

import io.reactivex.rxjava3.observers.TestObserver;

@RunWith(MockitoJUnitRunner.class)
public class FileUtilTest {
    private static final String CONFIG_DIR = "config";
    private static final String DEVICE_CONFIG_FILE = "aacs_config.json";
    private static final String TEST_AACS_CONFIG_FILE_PATH = "src/test/resources";

    @Mock
    private Context mMockContext;
    @Mock
    private AssetManager mMockAssetManager;
    @Mock
    private Handler mMockSameThreadHandler;

    @Before
    public void setup() throws IOException {
        MockitoAnnotations.openMocks(this);

        Mockito.when(mMockSameThreadHandler.post(ArgumentMatchers.any(Runnable.class))).thenAnswer(invocation -> {
            ((Runnable) invocation.getArgument(0)).run();
            return null;
        });
        Mockito.when(mMockContext.getAssets()).thenReturn(mMockAssetManager);
        InputStream stream = new ByteArrayInputStream("this is a sample stream".getBytes());
        Mockito.when(mMockAssetManager.open(CONFIG_DIR + "/" + DEVICE_CONFIG_FILE)).thenReturn(stream);
    }

    @Test
    public void test_use_config_from_external_path() throws IOException {
        TestObserver<String> readObserver =
                FileUtil.readAACSConfigurationAsync(mMockContext, mMockSameThreadHandler, TEST_AACS_CONFIG_FILE_PATH)
                        .test();
        readObserver.awaitCount(1);
        readObserver.assertValue(config -> { return config != null && !config.isEmpty(); });

        Mockito.verify(mMockAssetManager, Mockito.times(0)).open(CONFIG_DIR + "/" + DEVICE_CONFIG_FILE);
    }

    @Test
    public void test_use_default_config_in_asset_if_external_path_is_invalid() throws IOException {
        TestObserver<String> readObserver =
                FileUtil.readAACSConfigurationAsync(mMockContext, mMockSameThreadHandler, "invalidPath").test();
        readObserver.awaitCount(1);
        readObserver.assertValue(config -> config != null && !config.isEmpty());

        Mockito.verify(mMockAssetManager, Mockito.times(1)).open(CONFIG_DIR + "/" + DEVICE_CONFIG_FILE);
    }
}
