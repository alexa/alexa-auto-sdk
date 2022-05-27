package com.amazon.alexaautoclientservice.instrumentedtest;

import static com.amazon.aacsconstants.Action.LAUNCH_SERVICE;

import static org.junit.Assert.assertTrue;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelFileDescriptor;
import android.service.notification.StatusBarNotification;
import android.util.Log;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.rule.ServiceTestRule;

import com.amazon.aace.core.MessageStream;
import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSReceiver;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexaautoclientservice.AlexaAutoClientService;
import com.amazon.alexaautoclientservice.R;
import com.amazon.alexaautoclientservice.util.AASBUtil;
import com.amazon.alexaautoclientservice.util.FileUtil;

import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.io.IOException;
import java.lang.annotation.Target;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

@RunWith(AndroidJUnit4.class)
public class TestAlexaAutoClientService {
    Context mContext;
    Context mTestContext;
    AACSSender mAACSSender;
    AACSReceiver mAACSReceiver;
    boolean mServiceStarted;
    TargetComponent mTarget;

    @Before
    public void setup() {
        mContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        mTestContext = InstrumentationRegistry.getInstrumentation().getContext();
        mServiceStarted = false;
        mTarget = TargetComponent.withComponent(
                new ComponentName(AACSConstants.getAACSPackageName(new WeakReference<>(mContext)),
                        AlexaAutoClientService.class.getCanonicalName()),
                TargetComponent.Type.SERVICE);
        initIPC();
    }

    @Test
    public void testStartService() throws TimeoutException {
        Intent startServiceIntent = new Intent(mContext, AlexaAutoClientService.class);
        startServiceIntent.putExtra(AACSConstants.NEW_CONFIG, true);
        mTestContext.startForegroundService(startServiceIntent);

        // Test that sending message does not cause any errors
        String cblStartMessage =
                AASBUtil.constructAASBMessage("", Topic.AUTHORIZATION, Action.Authorization.START_AUTHORIZATION, "");
        mAACSSender.sendAASBMessageEmbedded(
                cblStartMessage, Action.Authorization.START_AUTHORIZATION, Topic.AUTHORIZATION, mTarget, mTestContext);
    }

    @Test
    public void testStartAACSWithConfiguration() throws Exception {
        Intent startServiceIntent = new Intent(mContext, AlexaAutoClientService.class);
        startServiceIntent.setAction(LAUNCH_SERVICE);
        startServiceIntent.putExtra(AACSConstants.NEW_CONFIG, true);
        mTestContext.startForegroundService(startServiceIntent);

        sendConfig(mTestContext, TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE);

        // it takes a few seconds for AACS to receive and process the configuration intent.
        TimeUnit.SECONDS.sleep(6);
        Intent stopServiceIntent = new Intent(mContext, AlexaAutoClientService.class);
        mTestContext.stopService(stopServiceIntent);

        // it takes a few seconds for AACS to clean up the resources and write the cached
        // config to the shared preference after getting the stop intent.
        TimeUnit.SECONDS.sleep(3);
        assertTrue(FileUtil.isConfigurationSaved(mContext));
    }

    private void initIPC() {
        mAACSSender = new AACSSender();
        AACSReceiver.MessageReceivedCallback messageReceivedCallback = new AACSReceiver.MessageReceivedCallback() {
            @Override
            public void onMessageReceived(String s) {
                try {
                    JSONObject msg = new JSONObject(s);
                    String topic = msg.getJSONObject(AASBConstants.HEADER)
                                           .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                           .getString(AASBConstants.TOPIC);
                    String action = msg.getJSONObject(AASBConstants.HEADER)
                                            .getJSONObject(AASBConstants.MESSAGE_DESCRIPTION)
                                            .getString(AASBConstants.ACTION);

                    if (topic.equals(Topic.AASB) && action.equals(Action.AASB.START_SERVICE)) {
                        mServiceStarted = true;
                    }
                } catch (JSONException e) {
                }
            }
        };

        AACSReceiver.FetchStreamCallback fetchCallback = new AACSReceiver.FetchStreamCallback() {
            @Override
            public void onStreamRequested(String streamId, ParcelFileDescriptor writePipe) {}

            @Override
            public void onStreamFetchCancelled(String streamId) {}
        };

        AACSReceiver.StreamPushedFromSenderCallback streamPushedCallback =
                new AACSReceiver.StreamPushedFromSenderCallback() {
                    @Override
                    public void onStreamPushedFromSenderCallback(String streamId, ParcelFileDescriptor readPipe) {}
                };
        AACSReceiver.Builder builder = new AACSReceiver.Builder();
        mAACSReceiver = builder.withAASBCallback(messageReceivedCallback)
                                .withFetchCallback(fetchCallback)
                                .withPushCallback(streamPushedCallback)
                                .build();
    }

    private void sendConfig(Context context, String pathInAssets) throws Exception {
        JSONObject configJson = TestUtil.readConfig(context.getAssets().open(pathInAssets));
        TestUtil.addDeviceInfoToConfig(configJson, TestUtil.CLIENT_ID, TestUtil.PRODUCT_ID, TestUtil.DSN);
        JSONObject configMessage = TestUtil.constructOEMConfigMessage(
                new String[] {}, new String[] {TestUtil.escapeJsonString(configJson)});
        mAACSSender.sendConfigMessageEmbedded(configMessage.toString(), mTarget, context);
    }
}
