/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.amazon.alexa.auto.apl.handler;

import android.content.Context;
import android.util.Log;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.apl.APLVisualController;
import com.amazon.alexa.auto.apl.Constants;
import com.amazon.alexa.auto.apps.common.util.FileUtil;
import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.render.APLPresenter;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IDismissible;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.lang.ref.WeakReference;
import java.util.HashMap;

/**
 * This class provides the functionality to handle AACS APL events.
 */
public class APLHandler implements APLVisualController, IAPLEventSender, IDismissible {
    private static final String TAG = APLHandler.class.getSimpleName();

    @NonNull
    private final WeakReference<Context> mContextWk;
    @NonNull
    private AACSMessageSender mAACSSender;

    private JSONArray mVisualConfig;
    private String mDefaultWindowId;

    private String mToken;
    private String mVersion;

    @VisibleForTesting
    APLPresenter mPresenter;

    public APLHandler(@NonNull WeakReference<Context> contextWk, @NonNull AACSMessageSender aacsSender) {
        mContextWk = contextWk;
        mAACSSender = aacsSender;

        Log.d(TAG, "Initializing APL presenter.");
        APLPresenter.initialize(contextWk.get());
        FileUtil.readAACSConfigurationAsync(contextWk.get())
                .subscribe(configs -> initializeAPLPresenter(contextWk.get(), configs));
    }

    @Override
    public void renderDocument(String jsonPayload, String token, String windowId) {
        try {
            // Extract document and data
            JSONObject payload = new JSONObject(jsonPayload);
            Log.i(TAG, "APL render document: " + payload.toString(4));
            JSONObject document = payload.getJSONObject(Constants.DOCUMENT);

            mToken = token;
            mVersion = document.getString(Constants.VERSION);
        } catch (JSONException e) {
            Log.e(TAG, e.getMessage());
        }

        Log.i(TAG, "APL render document version: " + mVersion + " token: " + mToken + " windowId: " + windowId);

        mPresenter.onRenderDocument(jsonPayload, token, windowId);
    }

    @Override
    public void clearDocument(String token) {
        Log.i(TAG, "clearDocument and visual context");
        mPresenter.onClearDocument(token);
    }

    @Override
    public void executeCommands(String payload, String token) {
        Log.v(TAG, "executeCommands: token: " + token);
        mToken = token;

        mPresenter.onExecuteCommands(payload, token);
    }

    @Override
    public void handleAPLRuntimeProperties(String aplRuntimeProperties) {
        Log.v(TAG, "handleAPLRuntimeProperties: aplRuntimeProperties: " + aplRuntimeProperties);
        mPresenter.onAPLRuntimeProperties(aplRuntimeProperties);
    }

    @Override
    public void handleAPLDataSourceUpdate(String dataType, String payload, String token) {
        Log.v(TAG, "handleAPLDataSourceUpdate");
        mPresenter.onDataSourceUpdate(dataType, payload, token);
    }

    @Override
    public void interruptCommandSequence(String token) {
        Log.v(TAG, "interruptCommandSequence: token: " + token);
        mPresenter.onInterruptCommandSequence(token);
    }

    private void processActivityEvent(String source, String event) {
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(Constants.SOURCE)
                                     .value(source)
                                     .key(Constants.EVENT)
                                     .value(event)
                                     .endObject()
                                     .toString();

            mAACSSender.sendMessage(Topic.APL, Action.APL.PROCESS_ACTIVITY_EVENT, payload);
        } catch (Exception exception) {
            Log.e(TAG, "Failed to process activity event. Error: " + exception);
        }
    }

    private void renderDocumentResult(String token, boolean result, String error) {
        sendResult(Action.APL.RENDER_DOCUMENT_RESULT, token, result, error);
    }

    private void executeCommandsResult(String token, boolean result, String error) {
        sendResult(Action.APL.EXECUTE_COMMANDS_RESULT, token, result, error);
    }

    private void sendResult(String resultAction, String token, boolean result, String error) {
        try {
            String payload = new JSONStringer()
                                     .object()
                                     .key(Constants.TOKEN)
                                     .value(token)
                                     .key(Constants.RESULT)
                                     .value(result)
                                     .key(Constants.ERROR)
                                     .value(error == null ? "" : error)
                                     .endObject()
                                     .toString();

            mAACSSender.sendMessage(Topic.APL, resultAction, payload);
        } catch (Exception exception) {
            Log.e(TAG, "Failed to send result for " + resultAction + " Error: " + exception);
        }
    }

    private void sendUserEvent(String payload) {
        try {
            String msgPayload =
                    new JSONStringer().object().key(Constants.PAYLOAD).value(payload).endObject().toString();

            mAACSSender.sendMessage(Topic.APL, Action.APL.SEND_USER_EVENT, msgPayload);
        } catch (Exception exception) {
            Log.e(TAG, "Failed to send user event. Error: " + exception);
        }
    }

    // IAPLEventSender implementation.

    @Override
    public void sendWindowState(String state) {
        Log.v(TAG, "sendWindowState: " + state);
        try {
            String msgPayload = new JSONStringer().object().key(Constants.STATE).value(state).endObject().toString();

            mAACSSender.sendMessage(Topic.APL, Action.APL.SEND_DEVICE_WINDOW_STATE, msgPayload);
        } catch (Exception exception) {
            Log.e(TAG, "Failed to send device window state. Error: " + exception);
        }
    }

    @Override
    public void sendClearCard() {
        Log.v(TAG, "sendClearCard");
        mAACSSender.sendMessage(Topic.APL, Action.APL.CLEAR_CARD, null);
    }

    @Override
    public void sendClearAllExecuteCommands() {
        Log.v(TAG, "sendClearAllExecuteCommands");
        mAACSSender.sendMessage(Topic.APL, Action.APL.CLEAR_ALL_EXECUTE_COMMANDS, null);
    }

    @Override
    public void sendUserEventRequest(String payload) {
        Log.v(TAG, "sendUserEventRequest: payload: " + payload);
        sendUserEvent(payload);
    }

    @Override
    public void sendDataSourceFetchEventRequest(String type, String payload) {
        Log.v(TAG, "sendDataSourceFetchEventRequest: type: " + type + " payload: " + payload);
        try {
            String msgPayload = new JSONStringer()
                                        .object()
                                        .key(Constants.TYPE)
                                        .value(type)
                                        .key(Constants.PAYLOAD)
                                        .value(payload)
                                        .endObject()
                                        .toString();

            mAACSSender.sendMessage(Topic.APL, Action.APL.SEND_DATA_SOURCE_FETCH_REQUEST_EVENT, msgPayload);
        } catch (Exception exception) {
            Log.e(TAG, "Failed to send data source fetch request event. Error: " + exception);
        }
    }

    @Override
    public void sendRuntimeErrorEventRequest(String payload) {
        Log.v(TAG, "sendRuntimeErrorEvent: payload: " + payload);
        try {
            String msgPayload =
                    new JSONStringer().object().key(Constants.PAYLOAD).value(payload).endObject().toString();

            mAACSSender.sendMessage(Topic.APL, Action.APL.SEND_RUNTIME_ERROR_EVENT, msgPayload);
        } catch (Exception exception) {
            Log.e(TAG, "Failed to send runtime error event. Error: " + exception);
        }
    }

    @Override
    public void sendRenderDocumentResult(String token, boolean result, String error) {
        Log.v(TAG, "sendRenderDocumentResult: token: " + token + " boolean: " + result + " error: " + error);
        renderDocumentResult(token, result, error);
    }

    @Override
    public void sendExecuteCommandsResult(String token, boolean result, String error) {
        Log.v(TAG, "sendExecuteCommandsResult: token: " + token + " result: " + result + " error: " + error);
        executeCommandsResult(token, result, error);
    }

    @Override
    public void sendActivityEventRequest(String token, IAPLEventSender.ActivityEvent event) {
        Log.v(TAG, "sendActivityEventRequest: token: " + token + " event: " + event);
        processActivityEvent(token, translateActivityEvent(event));
    }

    @Override
    public void sendContext(String state) {
        Log.v(TAG, "sendContext: " + state);
        try {
            String msgPayload = new JSONStringer().object().key(Constants.STATE).value(state).endObject().toString();

            mAACSSender.sendMessage(Topic.APL, Action.APL.SEND_DOCUMENT_STATE, msgPayload);
        } catch (Exception exception) {
            Log.e(TAG, "Failed to send user event. Error: " + exception);
        }
    }

    /**
     * Cancel execution of APL commands. Should be called
     * on a barge in.
     */
    @Override
    public void cancelExecution() {
        if (mPresenter != null) {
            mPresenter.cancelExecution();
        }
    }

    private String translateActivityEvent(IAPLEventSender.ActivityEvent event) {
        switch (event) {
            case ACTIVATED:
                return Constants.APL_EVENT_STATE_ACTIVATED;
            case DEACTIVATED:
                return Constants.APL_EVENT_STATE_DEACTIVATED;
            case ONE_TIME:
                return Constants.APL_EVENT_STATE_ONE_TIME;
            case INTERRUPT:
                return Constants.APL_EVENT_STATE_INTERRUPT;
            case UNKNOWN:
                return Constants.APL_EVENT_STATE_UNKNOWN;
        }

        return Constants.APL_EVENT_STATE_UNKNOWN;
    }

    @Override
    public void onDismiss() {
        Log.d(TAG, "onDismiss");
        clearDocument(mToken);
    }

    @Override
    public void initializeAPLPresenter(Context context, String configs) {
        try {
            JSONObject config = new JSONObject(configs);
            mVisualConfig =
                    config.getJSONObject("aacs.alexa").getJSONObject("gui").getJSONArray("visualCharacteristics");

            if (mVisualConfig.length() > 0) {
                for (int i = 0; i < mVisualConfig.length(); i++) {
                    JSONObject currentElement = mVisualConfig.getJSONObject(i);
                    if ("Alexa.Display.Window".equals(currentElement.getString("interface"))) {
                        JSONArray templates = currentElement.getJSONObject("configurations").getJSONArray("templates");
                        JSONObject template = templates.getJSONObject(0);
                        mDefaultWindowId = template.getString("id");
                    }
                }
            }
        } catch (JSONException e) {
            Log.w(TAG, "Failed to parse APL visual characteristics" + e);
        }

        mPresenter = new APLPresenter(mVisualConfig, mDefaultWindowId, this);

        mPresenter.setDismissibleCallback(this);
    }

    @Override
    public void setAPLLayout(View view) {
        Log.d(TAG, "Setting APL layout");
        HashMap aplLayouts = new HashMap<String, APLLayout>();
        aplLayouts.put(mDefaultWindowId, (APLLayout) view);

        mPresenter.setAPLLayout(aplLayouts);
    }
}
