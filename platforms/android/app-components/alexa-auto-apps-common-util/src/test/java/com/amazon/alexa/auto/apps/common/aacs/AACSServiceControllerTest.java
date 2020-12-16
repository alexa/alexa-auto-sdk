package com.amazon.alexa.auto.apps.common.aacs;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.os.Build;

import androidx.test.core.app.ApplicationProvider;

import com.amazon.aacsconstants.AACSConstants;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.Shadows;
import org.robolectric.annotation.Config;

@RunWith(RobolectricTestRunner.class)
@Config(sdk = Build.VERSION_CODES.P)
public class AACSServiceControllerTest {
    @Test
    public void testAACSServiceIsStartedWithWaitForConfig() {
        testStartAACSService(true);
    }

    @Test
    public void testAACSServiceIsStartedWithoutWaitForConfig() {
        testStartAACSService(false);
    }

    @Test
    public void testAACSServiceIsStopped() {
        Application application = ApplicationProvider.<Application>getApplicationContext();
        AACSServiceController.stopAACS(application);

        Intent stopSvcIntent = Shadows.shadowOf(application).getNextStoppedService();
        assertEquals(AACSConstants.AACS_CLASS_NAME, stopSvcIntent.getComponent().getClassName());
        assertEquals(AACSConstants.AACS_PACKAGE_NAME, stopSvcIntent.getComponent().getPackageName());
    }

    private void testStartAACSService(boolean waitForConfig) {
        Application application = ApplicationProvider.<Application>getApplicationContext();
        AACSServiceController.startAACS(application, waitForConfig);

        Intent startSvcIntent = Shadows.shadowOf(application).getNextStartedService();
        assertNotNull(startSvcIntent);
        assertEquals(AACSConstants.AACS_CLASS_NAME, startSvcIntent.getComponent().getClassName());
        assertEquals(AACSConstants.AACS_PACKAGE_NAME, startSvcIntent.getComponent().getPackageName());
        assertEquals(waitForConfig, startSvcIntent.getBooleanExtra(AACSConstants.NEW_CONFIG, !waitForConfig));
    }
}
