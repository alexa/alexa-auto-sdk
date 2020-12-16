package com.amazon.alexa.auto.lwa;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.IPCConstants;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.auth.CBLAuthState;
import com.amazon.alexa.auto.apis.auth.CBLAuthWorkflowData;
import com.amazon.alexa.auto.apis.auth.CodePair;

import org.greenrobot.eventbus.EventBus;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.Optional;
import java.util.UUID;

public class CBLReceiver extends BroadcastReceiver {
    private static final String TAG = CBLReceiver.class.getSimpleName();
    private static final String REFRESH_TOKEN_KEY = "com.amazon.alexa.lwa.key";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            switch (message.action) {
                case Action.CBL.CBL_STATE_CHANGED:
                    handleCBLStateChanged(message);
                    break;
                case Action.CBL.SET_REFRESH_TOKEN:
                    handleSetRefreshToken(context, message);
                    break;
                case Action.CBL.GET_REFRESH_TOKEN:
                    handleGetRefreshToken(context, message);
                    break;
                case Action.CBL.CLEAR_REFRESH_TOKEN:
                    handleClearRefreshToken(context, message);
                    break;
            }
        });
    }

    /**
     * Handles AASB CBL message with CBLStateChanged action.
     * Generates QR code if code/url is provided.
     *
     */
    private void handleCBLStateChanged(AACSMessage message) {
        String state = null;
        String url = "";
        String code = "";

        try {
            JSONObject obj = new JSONObject(message.payload);

            if (obj.has("state") && obj.has("url") && obj.has("code")) {
                url = obj.getString("url");
                code = obj.getString("code");
                state = obj.getString("state");

                if (code.isEmpty() || url.isEmpty()) {
                    Log.w(TAG, "CBL code or URL empty.");
                }
            } else {
                Log.w(TAG, "CBLStateChanged JSON is missing data. json:" + message.payload);
            }

        } catch (JSONException e) {
            Log.e(TAG, "CBLStateChanged JSON cannot be parsed.");
        }

        switch (state) {
            case "CODE_PAIR_RECEIVED":
                Log.d(TAG, "CODE_PAIR_RECEIVED");
                EventBus.getDefault().post(
                        new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_CodePair_Received, new CodePair(url, code)));
                break;
            case "STARTING":
                Log.d(TAG, "CBLStateChanged: Starting");
                EventBus.getDefault().post(new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Started, null));
                break;
            case "STOPPING":
                Log.d(TAG, "CBLStateChanged: Stopping");
                EventBus.getDefault().post(new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Not_Started, null));
                break;
            default:
                break;
        }
    }

    private void handleSetRefreshToken(Context context, AACSMessage message) {
        Log.d(TAG, "handleSetRefreshToken " + message.payload);
        String refreshToken = "";
        try {
            JSONObject obj = new JSONObject(message.payload);

            if (obj.has("refreshToken")) {
                refreshToken = obj.getString("refreshToken");
                if (refreshToken.isEmpty()) {
                    Log.w(TAG, "refreshToken is empty.");
                }
            } else {
                Log.w(TAG, "SetRefreshToken JSON is missing data. json:" + message.payload);
            }

        } catch (JSONException e) {
            Log.e(TAG, "SetRefreshToken JSON cannot be parsed.");
        }

        TokenStore.saveRefreshToken(context, refreshToken);

        EventBus.getDefault().post(new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Finished, null));
    }

    private void handleGetRefreshToken(Context context, AACSMessage message) {
        Log.d(TAG, "handleGetRefreshToken");
        Optional<String> refreshToken = TokenStore.getRefreshToken(context);
        JSONObject payloadJson;
        try {
            payloadJson = new JSONObject();
            payloadJson.put("refreshToken", refreshToken.orElse(""));
        } catch (Exception e) {
            Log.e(TAG, e.toString());
            return;
        }

        new AACSMessageSender(new WeakReference<>(context), new AACSSender())
                .sendReplyMessage(message.messageId, Topic.CBL, Action.CBL.GET_REFRESH_TOKEN, payloadJson.toString());
    }

    private void handleClearRefreshToken(Context context, AACSMessage message) {
        Log.d(TAG, "handleClearRefreshToken " + message.payload);
        TokenStore.resetRefreshToken(context);
    }

    public static void sendMessage(Context context, String topic, String action, String message) {
        Intent intent = new Intent();
        intent.setComponent(new ComponentName(
                "com.amazon.alexaautoclientservice", "com.amazon.alexaautoclientservice.AlexaAutoClientService"));
        intent.setAction(action);
        intent.addCategory(topic);

        Bundle bundle = new Bundle();
        bundle.putString("type", IPCConstants.AacsIpcMessageType.EMBEDDED.getTypeAsString());
        bundle.putString("message", message);

        intent.putExtra("payload", bundle);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }
}
