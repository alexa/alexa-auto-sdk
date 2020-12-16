/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
import android.support.v4.view.ViewPager;
import android.util.Log;

import com.amazon.aace.apl.APL;
import com.amazon.apl.android.APLController;
import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.APLOptions;
import com.amazon.apl.android.APLOptionsBuilder;
import com.amazon.apl.android.Content;
import com.amazon.apl.android.Scaling;
import com.amazon.apl.android.dependencies.IOpenUrlCallback;
import com.amazon.apl.android.dependencies.ITtsPlayer;
import com.amazon.apl.android.dependencies.TtsSourceProvider;
import com.amazon.apl.android.dependencies.impl.OpenUrlCallback;
import com.amazon.apl.android.dependencies.impl.TtsPlayer;
import com.amazon.apl.android.providers.IDataRetriever;
import com.amazon.apl.android.providers.ITtsPlayerProvider;
import com.amazon.apl.android.providers.impl.HttpRetrieverProvider;
import com.amazon.apl.enums.ViewportMode;
import com.amazon.sampleapp.apl.R;
import com.amazon.sampleapp.apl.content.AssetContentCallback;
import com.amazon.sampleapp.core.SampleAppContext;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.Vector;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * This class implements the Auto SDK APL platform interface. This handles coordinates taking
 * APL document data from the AUto SDK and passed it to the APL rendering engine.
 */
public class APLHandler extends APL {
    private static final String sTag = "APLHandler";

    private final Activity mActivity;
    private APLLayout mAplLayout;
    private APLController mAplController;
    private String mToken;
    private String mNewToken;
    private JSONObject mVisualContext;
    private String mVersion;
    private JSONArray mSupportedViewPorts;
    private ViewPager mViewPager;
    private APLOptions mOptions;
    private ITtsPlayerProvider mPlayer;
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    public APLHandler(SampleAppContext sampleAppContext) {
        mActivity = sampleAppContext.getActivity();
        mViewPager = sampleAppContext.getViewPager();

        // Initialize APL layout and TTS player
        mAplLayout = mActivity.findViewById(R.id.apl);
        createTTSPlayer();
    }

    @Override
    public String getVisualContext() {
        try {
            JSONObject context = new JSONObject().put("token", mToken).put("version", mVersion);
            JSONArray components = new JSONArray();
            if (mVisualContext != null && !mVisualContext.equals("")) {
                components.put(mVisualContext);
            }
            context.put("componentsVisibleOnScreen", components);
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
            Log.i(sTag, "APL render document: " + payload.toString(4));
            JSONObject document = payload.getJSONObject("document");
            JSONObject dataSources = payload.getJSONObject("datasources");

            mToken = token;
            mVersion = document.getString("version");

            Log.i(sTag, "APL render document version: " + mVersion + " token: " + mToken + " windowId: " + windowId);

            if (payload.has("supportedViewports")) {
                mSupportedViewPorts = payload.getJSONArray("supportedViewports");
            } else {
                mSupportedViewPorts = null;
            }

            String doc = document.toString();
            String data = dataSources.toString();

            // Fetch assets and render
            AssetContentCallback assetContentCallback = new AssetContentCallback(mActivity, data);
            assetContentCallback.addCompleteCallback(content -> doRender(doc, content));

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
        Log.i(sTag, "clearDocument and visual context");
        mVisualContext = null;

        mActivity.runOnUiThread(() -> {
            if (mAplController != null) {
                Log.i(sTag, "Finishing previous document token: " + mToken);
                mAplController.finishDocument();
                mAplController = null;
            } else {
                Log.w(sTag, "APL controller will not be shutdown: " + mToken);
            }
            // Show main fragment
            mViewPager.setCurrentItem(0, true);
            mExecutor.submit(() -> {
                Log.i(sTag, "Clearing card: token: " + mToken);
                clearCard();
            });
        });
    }

    @Override
    public void executeCommands(String payload, String token) {
        mToken = token;
        mActivity.runOnUiThread(() -> {
            try {
                // Log payload and token
                Log.i(sTag, "executeCommands called: " + payload);
                JSONObject commands = new JSONObject(payload);
                if (commands.has("commands")) {
                    if (commands.getJSONArray("commands").getJSONObject(0).getString("type").equals("Finish")) {
                        executeCommandsResult(mToken, true, "");
                        processActivityEvent(mToken, APL.ActivityEvent.DEACTIVATED);
                    } else {
                        mAplController.executeCommands(commands.getJSONArray("commands").toString()).then(() -> {
                            Log.i(sTag, "executeCommands result succeeded. token: " + mToken);
                            executeCommandsResult(mToken, true, "");
                            processActivityEvent(mToken, APL.ActivityEvent.ACTIVATED);
                        });
                    }
                } else {
                    mExecutor.submit(() -> {
                        Log.e(sTag, "executeCommands failed. token: " + mToken);
                        executeCommandsResult(mToken, false, "Missing commands");
                    });
                }
            } catch (Exception e) {
                Log.e(sTag, "executeCommands failed. token: " + mToken + " error: " + e.getMessage());
                mExecutor.submit(() -> { executeCommandsResult(mToken, false, e.getMessage()); });
            }
        });
    }

    @Override
    public void interruptCommandSequence() {
        mActivity.runOnUiThread(() -> {
            try {
                Log.i(sTag, "Interrupting command sequence");
                mAplController.cancelExecution();
            } catch (Exception e) {
                Log.e(sTag, "Interrupting command sequence did not succeed: " + e.getMessage());
            }
        });
    }

    /**
     * Render the document.
     *
     * @param name    Document name, for logging
     * @param doc     raw document for test command extraction
     * @param content the inflated contents
     */
    private void doRender(String doc, Content content) {
        mActivity.runOnUiThread(() -> {
            try {
                Log.i(sTag, "Render sending activity event ACTIVATED. token: " + mToken);
                processActivityEvent(mToken, APL.ActivityEvent.ACTIVATED);
                mOptions = createAPLOptions(doc);
                mAplController = APLController.renderDocument(content, mOptions, mAplLayout.getPresenter());
                mViewPager.setCurrentItem(1, true);
                mExecutor.submit(() -> {
                    Log.i(sTag, "Render result succeeded. token: " + mToken);
                    renderDocumentResult(mToken, true, "");
                });
            } catch (Exception e) {
                mExecutor.submit(() -> {
                    Log.e(sTag, "Render result failed: token: " + mToken + " error: " + e.getMessage());
                    renderDocumentResult(mToken, false, e.getMessage());
                });
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
        return APLOptionsBuilder.create("APLSampleApp configuration", "1.0")
                .allowOpenUrl(true)
                .openUrlCallback(createOpenURLCallback())
                .dataRetrieverProvider(() -> new IDataRetriever() {
                    private HttpRetrieverProvider mProvider = new HttpRetrieverProvider();

                    @Override
                    public void fetch(String source, Callback callback) {
                        mProvider.get().fetch(source, callback);
                    }

                    @Override
                    public void cancelAll() {}
                })
                .ttsPlayerProvider(mPlayer)
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

                    if (spec.has("minWidth") && spec.has("maxWidth") && spec.has("minHeight") && spec.has("maxHeight")
                            && spec.has("shape")) {
                        specifications.add(new Scaling.ViewportSpecification(spec.getInt("minWidth"),
                                spec.getInt("maxWidth"), spec.getInt("minHeight"), spec.getInt("maxHeight"),
                                spec.getString("shape").equals("ROUND"), vpMode));
                    }
                }
                scaling = new Scaling(10, specifications);
            }
        } catch (JSONException e) {
            Log.e(sTag, e.getMessage());
        }

        return scaling;
    }

    /**
     * Create the TTS player that will handle text to speech transformers and speech marks.
     */
    void createTTSPlayer() {
        mPlayer = new ITtsPlayerProvider() {
            private TtsPlayer ttsPlayer;

            @Override
            public ITtsPlayer getPlayer() {
                if (ttsPlayer == null) {
                    ttsPlayer = new TtsPlayer(mActivity);
                    Log.i(sTag, "Created TTS player");
                }
                return ttsPlayer;
            }

            @Override
            public void prepare(String source, TtsSourceProvider ttsSourceProvider) {
                try {
                    Log.i(sTag, "prepare: " + source);
                    ttsSourceProvider.onSource(new URL(source));
                } catch (MalformedURLException e) {
                    Log.e(sTag, "Malformed TTS Url " + source);
                }
            }

            @Override
            public void onDocumentFinish() {
                if (ttsPlayer != null) {
                    Log.i(sTag, "Releasing TTS player");
                    ttsPlayer.release();
                    ttsPlayer = null;
                } else {
                    Log.w(sTag, "TTS player is null");
                }
            }
        };
    }
}
