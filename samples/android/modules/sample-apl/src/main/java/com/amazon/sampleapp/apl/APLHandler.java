/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
import android.view.View;
import android.support.v4.view.ViewPager;

import com.amazon.aace.apl.APL;
import com.amazon.aace.audio.AudioOutputProvider;
import com.amazon.apl.android.APLController;
import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.APLOptions;
import com.amazon.apl.android.APLOptionsBuilder;
import com.amazon.apl.android.Content;
import com.amazon.apl.android.APLController;
import com.amazon.apl.android.Scaling;
import com.amazon.apl.android.dependencies.IOpenUrlCallback;
import com.amazon.apl.android.dependencies.impl.OpenUrlCallback;
import com.amazon.apl.android.dependencies.ITtsPlayer;
import com.amazon.apl.android.dependencies.TtsSourceProvider;
import com.amazon.apl.android.dependencies.impl.TtsPlayer;
import com.amazon.apl.android.providers.impl.HttpRetrieverProvider;;
import com.amazon.apl.android.providers.IDataRetriever;
import com.amazon.apl.android.providers.ITtsPlayerProvider;
import com.amazon.apl.enums.ViewportMode;
import com.amazon.sampleapp.apl.R;
import com.amazon.sampleapp.apl.content.AssetContentCallback;
import com.amazon.sampleapp.core.SampleAppContext;

import java.net.MalformedURLException;
import java.net.URL;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Vector;

public class APLHandler extends APL {

    private static final String sTag = "APLHandler";

    private final Activity mActivity;
    private APLLayout mAplLayout;
    private APLController mAplController;
    private AudioOutputProvider mAudioOutputProvider;
    private String mToken;
    private String mNewToken;
    private JSONObject mVisualContext;
    private String mVersion;
    private JSONArray mSupportedViewPorts;
    private ViewPager mViewPager;
    private APLTtsPlayerProvider mTtsPlayerProvider;

    public APLHandler(SampleAppContext sampleAppContext) {
        mActivity = sampleAppContext.getActivity();
        mAudioOutputProvider = sampleAppContext.getAudioOutputProvider();
        mViewPager = sampleAppContext.getViewPager();

        // Initialize APL

        mAplLayout = mActivity.findViewById(R.id.apl);
        mTtsPlayerProvider = new APLTtsPlayerProvider(mAudioOutputProvider);
    }

    @Override
    public String getVisualContext() {
        try {
            JSONObject context = new JSONObject()
                    .put("token", mToken)
                    .put("presentationToken", mToken)
                    .put("version", mVersion)
                    .put("componentsVisibleOnScreen", new JSONArray().put(mVisualContext));

            Log.i(sTag, "Visual context requested: " + context.toString());
            return context.toString();
        } catch (JSONException e) {
            Log.e(sTag, "Visual context requested failed: " + e.getMessage());
            return "";
        }
    }

    @Override
    public void renderDocument(String jsonPayload, String token, String windowId) {
        try {
            // Extract document and data
            JSONObject payload = new JSONObject(jsonPayload);
            JSONObject document = payload.getJSONObject("document");
            JSONObject dataSources = payload.getJSONObject("datasources");

            // Save version
            mVersion = document.getString("version");

            Log.i(sTag, "APL render document version: " + mVersion + " token: " + token + " windowId: " + windowId);

            if (payload.has("supportedViewports")) {
                mSupportedViewPorts = payload.getJSONArray("supportedViewports");
            } else {
                mSupportedViewPorts = null;
            }

            String doc = document.toString();
            String data = dataSources.toString();

            // Fetch assets and render
            AssetContentCallback assetContentCallback = new AssetContentCallback(mActivity, data);
            assetContentCallback.addCompleteCallback(content -> doRender(doc, content, token));

            // Inflate the document
            try {
                Content.create(doc, assetContentCallback);
            } catch (Content.ContentException exception) {
                Log.e(sTag, exception.getMessage());
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }
    }

    @Override
    public void clearDocument() {
        Log.v(sTag, "clearDocument and visual context");
        mVisualContext = null;
        clearCard();
    }

    @Override
    public void executeCommands(String payload, String token) {
        try {
            // Log payload and token
            Log.i(sTag, "executeCommands called: " + payload);
            JSONObject commands = new JSONObject(payload);
            if (commands.has("commands")) {
                mAplController.executeCommands(commands.getJSONArray("commands").toString());
                executeCommandsResult(mToken, true, "");
            } else {
                Log.e(sTag, "executeCommands did have commands object");
                executeCommandsResult(mToken, false, "Missing commands");
            }
        } catch (Exception e) {
            Log.e(sTag, e.getMessage());
            executeCommandsResult(mToken, false, e.getMessage());
        }
    }

    @Override
    public void interruptCommandSequence() {
        Log.i(sTag, "interruptCommandSequence");
    }

    /**
     * Render the document.
     *
     * @param name    Document name, for logging
     * @param doc     raw document for test command extraction
     * @param content the inflated contents
     */
    private void doRender(String doc, Content content, String token) {
        APLOptions options = createAPLOptions(doc);

        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    // IF token is different, then we are dealing with a different skill
                    if (mToken != null && !mToken.equals(token)) {
                        Log.i(sTag, "Finishing previous document token: " + mToken + " newToken: " + token);
                        if (mAplController != null) {
//                            mAplController.finishDocument();
                            mAplController = null;
                        }
                    }

                    mToken = token;
                    mAplController = APLController.renderDocument(content, options, mAplLayout.getPresenter());
                    mViewPager.setCurrentItem(1, true);
                    renderDocumentResult(mToken, true, "");
                } catch (Exception e) {
                    Log.e(sTag, e.getMessage());
                    renderDocumentResult(mToken, false, e.getMessage());
                }
            }

        });
    }

    /**
     * Creates an APLOptions object, extracting any scaling details from a document/
     *
     * @param doc possibly containing scaling details.
     * @return an APLOptions object.
     */
    private APLOptions createAPLOptions(String doc) {
        return APLOptionsBuilder
                .create("APLSampleApp configuration", "1.0")
                .allowOpenUrl(true)
                //.mediaPlayerProvider(mediaPlayerProvider)
                .openUrlCallback(createOpenURLCallback())
                .dataRetrieverProvider(() -> new IDataRetriever() {
                    private HttpRetrieverProvider mProvider = new HttpRetrieverProvider();

                    @Override
                    public void fetch(String source, Callback callback) {
                        mProvider.get().fetch(source, callback);
                    }

                    @Override
                    public void cancelAll() {
                    }
                })
                .ttsPlayerProvider(mTtsPlayerProvider)
                .sendEventCallback((args, components, sources) -> {
                    try {
                        String userEvent = new UserEventPayload(mToken, args, components, sources).toString();
                        Log.v(sTag, "UserEvent: " + userEvent);
                        // Call platform interface to send user event
                        sendUserEvent(userEvent);
                    } catch (Exception e) {
                        Log.e(sTag, e.getMessage());
                    }
                })
                .openUrlCallback(new OpenUrlCallback(mActivity))
                .visualContextListener(visualContext -> {
                    Log.v(sTag, "Visual context update: " + visualContext);
                    mVisualContext = visualContext;
                })
                .addScaling(createScaling())
                .build(mActivity);
    }

    private IOpenUrlCallback createOpenURLCallback() {
        return new OpenUrlCallback(mActivity.getApplicationContext());
    }

    private Scaling createScaling() {
        Scaling scaling = null;

        try {
            if (mSupportedViewPorts != null) {
                Log.v(sTag, "Creating scaling");
                Vector<Scaling.ViewportSpecification> specifications = new Vector<>();
                for (int i = 0; i < mSupportedViewPorts.length(); i++) {
                    JSONObject spec = mSupportedViewPorts.getJSONObject(i);
                    String mode = spec.getString("mode");

                    ViewportMode vpMode = ViewportMode.kViewportModeAuto;
                    switch (mode) {
                        case "TV":
                            vpMode = ViewportMode.kViewportModeTV;
                            break;
                        case "HUB":
                            vpMode = ViewportMode.kViewportModeHub;
                            break;
                        case "PC":
                            vpMode = ViewportMode.kViewportModeTV;
                            break;
                        case "MOBILE":
                            vpMode = ViewportMode.kViewportModeMobile;
                            break;
                        default:
                            vpMode = ViewportMode.kViewportModeAuto;

                    }

                    specifications.add(new Scaling.ViewportSpecification(
                            spec.getInt("minWidth"),
                            spec.getInt("maxWidth"),
                            spec.getInt("minHeight"),
                            spec.getInt("maxHeight"),
                            spec.getString("shape").equals("ROUND"),
                            vpMode
                    ));
                }
                scaling = new Scaling(10, specifications);
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }

        return scaling;
    }

}
