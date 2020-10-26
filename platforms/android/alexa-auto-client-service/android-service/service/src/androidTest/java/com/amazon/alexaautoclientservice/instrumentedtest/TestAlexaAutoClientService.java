package com.amazon.alexaautoclientservice.instrumentedtest;

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

import com.amazon.aace.aasb.AASBStream;
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
        mTarget = TargetComponent.withComponent(new ComponentName("com.amazon.alexaautoclientservice",
                                                        "com.amazon.alexaautoclientservice.AlexaAutoClientService"),
                TargetComponent.Type.SERVICE);

        initIPC();
    }

    @Test
    public void testStartService() throws TimeoutException {
        Intent startServiceIntent = new Intent(mContext, AlexaAutoClientService.class);
        startServiceIntent.putExtra(AACSConstants.NEW_CONFIG, true);
        mTestContext.startForegroundService(startServiceIntent);

        // Test that sending message does not cause any errors
        String cblStartMessage = AASBUtil.constructAASBMessage("", Topic.CBL, Action.CBL_START, "");
        TargetComponent target =
                TargetComponent.withComponent(new ComponentName("com.amazon.alexaautoclientservice",
                                                      "com.amazon.alexaautoclientservice.AlexaAutoClientService"),
                        TargetComponent.Type.SERVICE);
        mAACSSender.sendAASBMessageEmbedded(cblStartMessage, Action.CBL_START, Topic.CBL, target, mContext);
    }

    @Test
    public void testStartAACSWithConfiguration() throws Exception {
        Intent startServiceIntent = new Intent(mContext, AlexaAutoClientService.class);
        startServiceIntent.putExtra(AACSConstants.NEW_CONFIG, true);
        mTestContext.startForegroundService(startServiceIntent);

        sendConfig(mTestContext, TestUtil.ASSETS_PATH + TestUtil.AACS_CONFIG_FILE);

        SharedPreferences sp = mContext.getSharedPreferences(mContext.getPackageName(), Context.MODE_PRIVATE);
        assertTrue(sp.contains(FileUtil.CONFIG_KEY));
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

                    if (topic.equals(Topic.AASB) && action.equals(Action.START_SERVICE)) {
                        mServiceStarted = true;
                    }
                } catch (JSONException e) {
                }
            }
        };

        AACSReceiver.FetchStreamCallback fetchCallback = new AACSReceiver.FetchStreamCallback() {
            @Override
            public void onStreamRequested(String streamId, ParcelFileDescriptor.AutoCloseOutputStream stream) {}
            @Override
            public void onStreamFetchCancelled() {}
        };

        AACSReceiver.StreamPushedFromSenderCallback streamPushedCallback =
                new AACSReceiver.StreamPushedFromSenderCallback() {
                    @Override
                    public void onStreamPushedFromSenderCallback(
                            String streamId, ParcelFileDescriptor.AutoCloseInputStream stream) {}
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
        TargetComponent target =
                TargetComponent.withComponent(new ComponentName("com.amazon.alexaautoclientservice",
                                                      "com.amazon.alexaautoclientservice.AlexaAutoClientService"),
                        TargetComponent.Type.SERVICE);
        mAACSSender.sendConfigMessageEmbedded(configMessage.toString(), target, mContext);
    }
}
