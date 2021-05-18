/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.apl;

import android.app.Activity;
import android.util.Log;

import androidx.viewpager.widget.ViewPager;

import com.amazon.aace.apl.APL;
import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.render.APLPresenter;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.sampleapp.core.SampleAppContext;

import org.json.JSONArray;

import java.util.HashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * This class implements the Auto SDK APL platform interface. It uses the
 * APLPresenter to render APL.
 *
 * @see com.amazon.apl.android.render.APLPresenter
 */
public class APLHandler extends APL implements IAPLEventSender {
    private static final String sTag = "APLHandler";

    private APLLayout mAplLayout;
    private ViewPager mViewPager;
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private APLPresenter mPresenter;

    public APLHandler(SampleAppContext sampleAppContext) {
        Activity activity = sampleAppContext.getActivity();
        mViewPager = sampleAppContext.getViewPager();
        mAplLayout = activity.findViewById(R.id.apl);
    }

    public void buildAPLPresenter(JSONArray visualCharacteristics, String defaultWindowId) {
        //---------------------------------------------------------------------
        // Application needs to handle the correlation of window ids from the
        // visual characteristics configuration to the APLLayout instance.
        //---------------------------------------------------------------------
        HashMap aplLayouts = new HashMap<String, APLLayout>();
        aplLayouts.put(defaultWindowId, mAplLayout);
        //---------------------------------------------------------------------
        // Create APLPresenter to handle APL rendering
        //---------------------------------------------------------------------
        mPresenter = new APLPresenter(aplLayouts, visualCharacteristics, defaultWindowId, this);
    }

    @Override
    public void renderDocument(String payload, String token, String windowId) {
        Log.v(sTag, "renderDocument: token" + token + " payload: " + payload);
        mPresenter.onRenderDocument(payload, token, windowId);

        // Display APLLayout view
        mViewPager.post(() -> { mViewPager.setCurrentItem(1, true); });
    }

    @Override
    public void clearDocument(String token) {
        Log.v(sTag, "clearDocument: token: " + token);
        mPresenter.onClearDocument(token);
    }

    @Override
    public void executeCommands(String payload, String token) {
        Log.v(sTag, "executeCommands: token: " + token + " payload: " + payload);
        mPresenter.onExecuteCommands(payload, token);
    }

    @Override
    public void dataSourceUpdate(String sourceType, String payload, String token) {
        Log.v(sTag, "dataSourceUpdate: token: " + token + " type: " + sourceType + " payload: " + payload);
        mPresenter.onDataSourceUpdate(sourceType, payload, token);
    }

    @Override
    public void interruptCommandSequence(String token) {
        Log.v(sTag, "interruptCommandSequence: token: " + token);
        mPresenter.onInterruptCommandSequence(token);
    }

    // Send initial device window state
    @Override
    public void sendWindowState(String state) {
        Log.v(sTag, "sendWindowState: " + state);
        sendDeviceWindowState(state);
    }

    // IAPLEventSender

    @Override
    public void sendClearCard() {
        Log.v(sTag, "sendClearCard");
        mViewPager.setCurrentItem(0, true);
        clearCard();
    }

    @Override
    public void sendClearAllExecuteCommands() {
        Log.v(sTag, "sendClearAllExecuteCommands");
        clearAllExecuteCommands();
    }

    @Override
    public void sendUserEventRequest(String payload) {
        Log.v(sTag, "sendUserEventRequest: payload: " + payload);
        sendUserEvent(payload);
    }

    @Override
    public void sendDataSourceFetchEventRequest(String type, String payload) {
        Log.v(sTag, "sendDataSourceFetchEventRequest: type: " + type + " payload: " + payload);
        sendDataSourceFetchRequestEvent(type, payload);
    }

    @Override
    public void sendRuntimeErrorEventRequest(String payload) {
        Log.v(sTag, "sendRuntimeErrorEvent: payload: " + payload);
        sendRuntimeErrorEvent(payload);
    }

    @Override
    public void sendRenderDocumentResult(String token, boolean result, String error) {
        Log.v(sTag, "sendRenderDocumentResult: token: " + token + " boolean: " + result + " error: " + error);
        renderDocumentResult(token, result, error);
    }

    @Override
    public void sendExecuteCommandsResult(String token, boolean result, String error) {
        Log.v(sTag, "sendExecuteCommandsResult: token: " + token + " result: " + result + " error: " + error);
        executeCommandsResult(token, result, error);
    }

    @Override
    public void sendActivityEventRequest(String token, IAPLEventSender.ActivityEvent event) {
        Log.v(sTag, "sendActivityEventRequest: token: " + token + " event: " + event);
        processActivityEvent(token, translateActivityEvent(event));
    }

    @Override
    public void sendContext(String state) {
        Log.v(sTag, "sendContext: " + state);
        sendDocumentState(state);
    }

    private APL.ActivityEvent translateActivityEvent(IAPLEventSender.ActivityEvent event) {
        switch (event) {
            case ACTIVATED:
                return APL.ActivityEvent.ACTIVATED;
            case DEACTIVATED:
                return APL.ActivityEvent.DEACTIVATED;
            case ONE_TIME:
                return APL.ActivityEvent.ONE_TIME;
            case INTERRUPT:
                return APL.ActivityEvent.INTERRUPT;
            case UNKNOWN:
                return APL.ActivityEvent.UNKNOWN;
        }

        return APL.ActivityEvent.UNKNOWN;
    }
}
