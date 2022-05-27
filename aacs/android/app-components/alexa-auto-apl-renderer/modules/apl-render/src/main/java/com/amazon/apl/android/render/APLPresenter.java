/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render;

import android.content.Context;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

import androidx.annotation.NonNull;
import androidx.core.util.Preconditions;

import com.amazon.apl.android.APLController;
import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.APLOptions;
import com.amazon.apl.android.Action;
import com.amazon.apl.android.Content;
import com.amazon.apl.android.IAPLViewPresenter;
import com.amazon.apl.android.RootConfig;
import com.amazon.apl.android.RootContext;
import com.amazon.apl.android.RuntimeConfig;
import com.amazon.apl.android.configuration.ConfigurationChange;
import com.amazon.apl.android.dependencies.IDataSourceErrorCallback;
import com.amazon.apl.android.dependencies.IDataSourceFetchCallback;
import com.amazon.apl.android.dependencies.ISendEventCallback;
import com.amazon.apl.android.dependencies.IVisualContextListener;
import com.amazon.apl.android.render.content.APLHttpContentRetriever;
import com.amazon.apl.android.render.dagger.component.ActivityComponent;
import com.amazon.apl.android.render.dagger.component.ApplicationComponent;
import com.amazon.apl.android.render.dagger.component.DaggerActivityComponent;
import com.amazon.apl.android.render.dagger.module.ActivityModule;
import com.amazon.apl.android.render.extension.ExtensionManager;
import com.amazon.apl.android.render.extension.back.BackExtension;
import com.amazon.apl.android.render.extension.back.BackStack;
import com.amazon.apl.android.render.extension.back.BackStackDocument;
import com.amazon.apl.android.render.extension.localinfo.LocalInfoExtension;
import com.amazon.apl.android.render.font.AutoEmbeddedFontResolver;
import com.amazon.apl.android.render.interfaces.IAPLContentListener;
import com.amazon.apl.android.render.interfaces.IAPLEventSender;
import com.amazon.apl.android.render.interfaces.IAPLTokenProvider;
import com.amazon.apl.android.render.interfaces.IDismissible;
import com.amazon.apl.android.render.interfaces.ILocalInfoDataConsumer;
import com.amazon.apl.android.render.interfaces.ILocalInfoDataReporter;
import com.amazon.apl.android.render.interfaces.IPresenter;
import com.amazon.apl.android.render.payload.ExecuteCommandPayload;
import com.amazon.apl.android.render.payload.RenderDocumentPayload;
import com.amazon.apl.android.render.payload.RenderedDocumentStatePayload;
import com.amazon.apl.android.render.payload.UserEventPayload;
import com.amazon.apl.android.render.utils.RenderDocumentUtils;
import com.amazon.apl.android.scaling.Scaling;
import com.amazon.apl.enums.ViewportMode;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Collections;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * This class facilitates the integration of Alexa Presentation Language (APL).
 * The application is required to initialize the APL runtime by passing the Android
 * context to @see initialize(context) prior to the inflation of @see APLLayout components.
 * The visual configuration is used to provide window state context.
 */
public class APLPresenter implements IPresenter, ISendEventCallback, IDataSourceFetchCallback, IDataSourceErrorCallback,
                                     IVisualContextListener, IAPLTokenProvider, ILocalInfoDataReporter {
    private static final String TAG = APLPresenter.class.getSimpleName();

    private IAPLEventSender mAplEventSender;
    private Map<String, APLLayout> mAplLayouts;
    private Action mAction;
    private APLController mAplController;
    private String mToken;
    private APLOptions mAplOptions;
    private ActivityComponent mActivityComponent;
    private ApplicationComponent mApplicationComponent;
    private RenderDocumentPayload mRenderDocumentPayload;
    private RenderedDocumentStatePayload mLatestRenderedDocumentState;
    private String mDefaultWindowId;
    private JSONArray mVisualCharacteristics;
    private Hashtable<String, String> mRuntimeProperties;
    private ExtensionManager mExtensionManager;
    private LocalInfoExtension mLocalInfoExtension;
    private BackExtension mBackHandler;
    private BackStack mBackStack;
    private ILocalInfoDataConsumer mLocalInfoDataConsumer;
    private IDismissible mDismissibleCallback;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private static Context mContext;
    private RootConfig mRootConfig;

    /**
     * Initialize the APL runtime. Must be done during Activity onCreate or
     * prior to inflation of APLLayout view.
     *
     * @param context
     */
    public static void initialize(Context context) {
        mContext = context;
        APLController.initializeAPL(
                context, RuntimeConfig.builder().fontResolver(new AutoEmbeddedFontResolver(context)).build());
    }

    /**
     * @deprecated
     *
     * Constructs the APLPresenter object.
     *
     * @param aplLayouts A map of windows ids to the corresponding APL layouts.
     * @param visualCharacteristics The array of defined visual characteristics for the device.
     * @param defaultWindowId The Id of the window where APL will be rendered if one is not specified in the render
     *         document request.
     * @param aplEventSender The object used to send events back to the cloud.
     */
    public APLPresenter(@NonNull final Map<String, APLLayout> aplLayouts, @NonNull JSONArray visualCharacteristics,
            @NonNull String defaultWindowId, @NonNull final IAPLEventSender aplEventSender)
            throws IllegalStateException {
        // Initialize must be called prior to instantiation of this class.
        if (mContext == null) {
            throw new IllegalStateException("Context is null. Make sure initialize() is called with non null context.");
        }

        Preconditions.checkNotNull(aplLayouts);
        Preconditions.checkNotNull(visualCharacteristics);
        Preconditions.checkNotNull(defaultWindowId);
        Preconditions.checkState(!defaultWindowId.isEmpty());
        Preconditions.checkNotNull(aplEventSender);

        mAplLayouts = aplLayouts;
        mVisualCharacteristics = visualCharacteristics;
        mAplEventSender = aplEventSender;
        mDefaultWindowId = defaultWindowId;
        APLSingleton.getInstance().init(mContext, mAplEventSender, this);
        mApplicationComponent = APLSingleton.getInstance().getApplicationComponent();
        mActivityComponent = initActivityComponent(mContext);
        mBackStack = new BackStack();

        // Default property values
        mRuntimeProperties = new Hashtable<>();
        mRuntimeProperties.put("drivingState", "moving");
        mRuntimeProperties.put("theme", "dark");
        mRuntimeProperties.put("video", "disabled");

        // Initial window state
        sendDeviceWindowState();

        // Listen for touch events
        setupTouchListener();
    }

    /**
     * Constructs the APLPresenter object.
     *
     * @param visualCharacteristics The array of defined visual characteristics for the device.
     * @param defaultWindowId The Id of the window where APL will be rendered if one is not specified in the render
     *         document request.
     * @param aplEventSender The object used to send events back to the cloud.
     */
    public APLPresenter(@NonNull JSONArray visualCharacteristics, @NonNull String defaultWindowId,
            @NonNull final IAPLEventSender aplEventSender) throws IllegalStateException {
        // Initialize must be called prior to instantiation of this class.
        if (mContext == null) {
            throw new IllegalStateException("Context is null. Make sure initialize() is called with non null context.");
        }

        Preconditions.checkNotNull(visualCharacteristics);
        Preconditions.checkNotNull(defaultWindowId);
        Preconditions.checkState(!defaultWindowId.isEmpty());
        Preconditions.checkNotNull(aplEventSender);

        mVisualCharacteristics = visualCharacteristics;
        mAplEventSender = aplEventSender;
        mDefaultWindowId = defaultWindowId;
        APLSingleton.getInstance().init(mContext, mAplEventSender, this);
        mApplicationComponent = APLSingleton.getInstance().getApplicationComponent();
        mActivityComponent = initActivityComponent(mContext);
        mBackStack = new BackStack();

        // Default property values
        mRuntimeProperties = new Hashtable<>();
        mRuntimeProperties.put("drivingState", "moving");
        mRuntimeProperties.put("theme", "dark");
        mRuntimeProperties.put("video", "disabled");

        // Initial window state
        sendDeviceWindowState();
    }

    public void setAPLLayout(@NonNull final Map<String, APLLayout> aplLayouts) {
        Preconditions.checkNotNull(aplLayouts);
        mAplLayouts = aplLayouts;

        setupTouchListener();
    }

    private synchronized void updateRuntimeProperties() {
        Log.v(TAG, "updateRuntimeProperties" + mRuntimeProperties.toString());
        if (mAplController != null) {
            APLLayout aplLayout = getAplLayout();
            if (aplLayout != null) {
                aplLayout.post(() -> {
                    Map<String, String> autoEnvironmentValues = new HashMap<>();
                    autoEnvironmentValues.put("drivingState", mRuntimeProperties.get("drivingState"));
                    ConfigurationChange configurationChange =
                            aplLayout.createConfigurationChange()
                                    .theme(mRuntimeProperties.get("theme"))
                                    .disallowVideo(mRuntimeProperties.get("video").equalsIgnoreCase("disabled"))
                                    .environmentValue("automobile", autoEnvironmentValues)
                                    .build();
                    try {
                        aplLayout.handleConfigurationChange(configurationChange);
                        Log.v(TAG, "onConfigChange succeeded");
                    } catch (Exception e) {
                        Log.e(TAG, "Document cannot be rendered in this configuration", e);
                    }
                });
            }
        }
    }

    //-------------------------------------------------------------------------
    // IAPLContentListener
    //-------------------------------------------------------------------------

    /**
     * Application will notify of APL content through this listener. Content includes
     * render document payload, execute commands, data source updates, etc.
     *
     * @return IAPLContentListener The object that listens for APL content.
     */
    public IAPLContentListener getAPLContentListener() {
        return this;
    }

    /**
     * Handle updates from the vehicle that affect how the APL document
     * should be rendered.
     *
     * @param properties JSON string containing one or more properties.
     */
    @Override
    public void onAPLRuntimeProperties(String properties) {
        try {
            JSONObject propertiesObject = new JSONObject(properties);
            if (propertiesObject.has("drivingState")) {
                mRuntimeProperties.put("drivingState", propertiesObject.getString("drivingState").toLowerCase());
            }

            if (propertiesObject.has("theme")) {
                mRuntimeProperties.put("theme", propertiesObject.getString("theme").toLowerCase());
            }

            if (propertiesObject.has("video")) {
                mRuntimeProperties.put("video", propertiesObject.getString("video").toLowerCase());
            }

            updateRuntimeProperties();
        } catch (JSONException e) {
            Log.e(TAG, "propertiesJSONError");
        }
    }

    @Override
    public void onRenderDocument(String jsonPayload, String token, String windowId) {
        try {
            mToken = token;
            // Extract document and data
            mRenderDocumentPayload = RenderDocumentUtils.convertToRenderDocument(jsonPayload);
            mRootConfig = mActivityComponent.getRootConfig();

            Log.i(TAG, "APL render document token: " + mToken + " windowId: " + mRenderDocumentPayload.getWindowId());

            // Set up content retriever and render callback
            APLHttpContentRetriever contentRetriever =
                    new APLHttpContentRetriever(mApplicationComponent.getOkHttpClientWrapper(),
                            mApplicationComponent.getNetworkExecutor(), mRenderDocumentPayload);
            contentRetriever.addCompleteCallback(content -> doRender(content));

            // Inflate the document
            try {
                Content.create(mRenderDocumentPayload.getDocument(), contentRetriever);
            } catch (Content.ContentException exception) {
                Log.e(TAG, exception.getMessage());
            }
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
            renderResponse(token, false, e.getMessage());
        }
    }

    @Override
    public void onClearDocument(String token) {
        Log.i(TAG, "clearDocument and visual context " + token + " mToken: " + mToken);
        getAplLayout().post(() -> {
            // Clean up current view
            destroyAplView();
            // Clear back stack since skill session is done
            mBackStack.clear();
            // Notify that clear document is done
            mExecutor.submit(() -> {
                Log.i(TAG, "Clearing card: token: " + token);
                mLatestRenderedDocumentState = null;
                mRenderDocumentPayload = null;
                mToken = null;
                sendDeviceWindowState();
                mAplEventSender.sendClearCard();
            });
        });
    }

    @Override
    public void onExecuteCommands(String payload, String token) {
        mToken = token;
        getAplLayout().post(() -> {
            try {
                // Log payload and token
                Log.v(TAG, "onExecuteCommands: payload: " + payload);
                ExecuteCommandPayload commandPayload = ExecuteCommandPayload.convertToExecuteCommand(payload);
                String commandsString = commandPayload.getCommands();
                if (!commandsString.isEmpty()) {
                    mAplController.executeCommands(commandsString, action -> {
                        if (action != null) {
                            action.then(() -> {
                                Log.i(TAG,
                                        "onExecuteCommands: result: true command: token:" + token
                                                + " command: " + commandsString);
                                mAplEventSender.sendExecuteCommandsResult(token, true, "");
                                // Inactive after command completion
                                mAplEventSender.sendActivityEventRequest(
                                        token, IAPLEventSender.ActivityEvent.DEACTIVATED);
                            });
                            action.addTerminateCallback(() -> {
                                Log.e(TAG, "onExecuteCommands: result: false token: " + token + " command terminated");
                                mAplEventSender.sendExecuteCommandsResult(token, false, "Missing commands");
                                // Inactive after command termination
                                mAplEventSender.sendActivityEventRequest(
                                        token, IAPLEventSender.ActivityEvent.DEACTIVATED);
                            });
                            // Active while commands run
                            mAplEventSender.sendActivityEventRequest(token, IAPLEventSender.ActivityEvent.ACTIVATED);
                        }
                    });
                } else {
                    mExecutor.submit(() -> {
                        Log.e(TAG, "onExecuteCommands: result: false token: " + token);
                        mAplEventSender.sendExecuteCommandsResult(token, false, "Missing commands");
                    });
                }
            } catch (Exception e) {
                Log.e(TAG, "onExecuteCommands: token: result: false token: " + token + " error: " + e.getMessage());
                mExecutor.submit(() -> { mAplEventSender.sendExecuteCommandsResult(token, false, e.getMessage()); });
            }
        });
    }

    @Override
    public void onDataSourceUpdate(String sourceType, String payload, String token) {
        Log.i(TAG, "dataSourceUpdate called: " + sourceType + " payload: " + payload + " token: " + token);
        // Update data in APL Controller
        if (!mAplController.updateDataSource(sourceType, payload)) {
            Log.e(TAG, "handleDataSourceFetchRequest: updateDataSource was unsuccessful");
        }
    }

    @Override
    public void onInterruptCommandSequence(String token) {
        getAplLayout().post(() -> {
            try {
                Log.i(TAG, "Interrupting command sequence");
                mAplController.cancelExecution();
            } catch (Exception e) {
                Log.e(TAG, "Interrupting command sequence did not succeed: " + e.getMessage());
            }
        });
    }

    //-------------------------------------------------------------------------
    // IAPLOptionsBuilderProvider
    //-------------------------------------------------------------------------

    @NonNull
    @Override
    public APLOptions.Builder getAPLOptionsBuilder() {
        return mActivityComponent.getAPLOptionsBuilder()
                .sendEventCallback(this)
                .dataSourceFetchCallback(this)
                .dataSourceErrorCallback(this)
                .onAplFinishCallback(this)
                .extensionEventCallback(mExtensionManager)
                .visualContextListener(this);
    }

    //-------------------------------------------------------------------------
    // IAPLEventSender
    //-------------------------------------------------------------------------

    @Override
    public IAPLEventSender getAplEventSender() {
        return mAplEventSender;
    }

    //-------------------------------------------------------------------------
    // IAPLTokenProvider
    //-------------------------------------------------------------------------

    @Override
    public String getToken() {
        return mToken;
    }

    //-------------------------------------------------------------------------
    // IVisualContextListener - Android Viewhost
    //-------------------------------------------------------------------------

    @Override
    public void onVisualContextUpdate(JSONObject visualContext) {
        Log.v(TAG, "Visual context update: " + visualContext);

        if (mRenderDocumentPayload != null) {
            JSONArray compsVisibleOnScreen = new JSONArray();
            compsVisibleOnScreen.put(visualContext);
            RenderedDocumentStatePayload payload =
                    RenderedDocumentStatePayload.builder()
                            .presentationToken(mRenderDocumentPayload.getPresentationToken())
                            .presentationSession(mRenderDocumentPayload.getPresentationSession())
                            .versionName(BuildConfig.VERSION_NAME)
                            .componentsVisibleOnScreenArray(compsVisibleOnScreen)
                            .build();
            mLatestRenderedDocumentState = payload;
            mAplEventSender.sendContext(mLatestRenderedDocumentState.toString());
        }
    }

    //-------------------------------------------------------------------------
    // IPresenter
    //-------------------------------------------------------------------------

    @Override
    public void onTouchEvent(MotionEvent event) {
        Log.v(TAG, "onTouchEvent: " + event);
        String token = getToken();
        if (token != null) {
            mAplEventSender.sendActivityEventRequest(token, IAPLEventSender.ActivityEvent.ONE_TIME);
        }
    }

    @Override
    public boolean onKeyEvent(KeyEvent event) {
        Log.v(TAG, "onKeyEvent: " + event);
        String token = getToken();
        if (token != null) {
            mAplEventSender.sendActivityEventRequest(token, IAPLEventSender.ActivityEvent.ONE_TIME);
        }
        return false;
    }

    @Override
    public APLLayout getAplLayout() {
        String windowId = mRenderDocumentPayload != null && !mRenderDocumentPayload.getWindowId().isEmpty()
                ? mRenderDocumentPayload.getWindowId()
                : mDefaultWindowId;
        return mAplLayouts.get(windowId);
    }

    @Override
    public void setLocalInfoDataConsumer(ILocalInfoDataConsumer consumer) {
        mLocalInfoDataConsumer = consumer;
    }

    @Override
    public void setDismissibleCallback(IDismissible dismissibleCallback) {
        mDismissibleCallback = dismissibleCallback;
    }

    @Override
    public void cancelExecution() {
        if (mAplController != null) {
            Log.i(TAG, "cancelExecution");
            mAplController.cancelExecution();
        }
    }

    /**
     * The following overrides are for methods defined by interfaces in the
     * Android viewhost.
     */

    //-------------------------------------------------------------------------
    // IOnAplFinishCallback - Android Viewhost
    //-------------------------------------------------------------------------

    @Override
    public void onAplFinish() {
        Log.v(TAG, "APL Rendering finished");
        if (mDismissibleCallback != null) {
            mDismissibleCallback.onDismiss();
        }
    }

    //-------------------------------------------------------------------------
    // ISendEventCallback - Android Viewhost
    //-------------------------------------------------------------------------

    @Override
    public void onSendEvent(Object[] args, Map<String, Object> components, Map<String, Object> sources) {
        try {
            JSONArray jsonArgs = new JSONArray((args));
            JSONObject jsonComponents = new JSONObject(components);
            JSONObject jsonSources = new JSONObject(sources);

            String userEvent = new UserEventPayload(getToken(), jsonArgs, jsonComponents, jsonSources).toString();
            Log.v(TAG, "UserEvent: " + userEvent);
            mAplEventSender.sendUserEventRequest(userEvent);
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }

    //-------------------------------------------------------------------------
    // IDataSourceErrorCallback - Android Viewhost
    //-------------------------------------------------------------------------

    @Override
    public void onDataSourceError(Object errors) {
        JSONObject payload = new JSONObject();
        try {
            final JSONArray errorsJson = new JSONArray(errors);
            payload.put("errors", errorsJson);
            payload.put("presentationToken", getToken());

            Log.v(TAG, "onDataSourceError: " + payload.toString());
            // Notify that an error occurred
            mAplEventSender.sendRuntimeErrorEventRequest(payload.toString());
        } catch (JSONException e) {
            Log.e(TAG, "Exception occurred while preparing RuntimeError Event payload", e);
        }
    }

    //-------------------------------------------------------------------------
    // IDataSourceFetchCallback - Android Viewhost
    //-------------------------------------------------------------------------

    @Override
    public void onDataSourceFetchRequest(String dataSourceType, Map<String, Object> eventPayload) {
        if (dataSourceType == null) {
            Log.e(TAG, "onDataSourceFetchRequest: dataSource type is null");
            return;
        }

        if (eventPayload == null) {
            Log.e(TAG, "onDataSourceFetchRequest: eventPayload type is null");
            return;
        }

        // Construct data source fetch request payload
        JSONObject payload;
        payload = new JSONObject(eventPayload);
        try {
            payload.put("presentationToken", getToken());
            Log.i(TAG, "onDataSourceFetchRequest: " + dataSourceType + " payload: " + payload.toString());
            // Notify that event should be sent
            mAplEventSender.sendDataSourceFetchEventRequest(dataSourceType, payload.toString());

        } catch (JSONException e) {
            Log.e(TAG, "onDataSourceFetchRequest: unable to add presentation token", e);
        }
    }

    //-------------------------------------------------------------------------
    // IDocumentLifecycleListener - Android Viewhost
    //-------------------------------------------------------------------------

    @Override
    public void onDocumentRender(@NonNull RootContext rootContext) {
        Log.v(TAG, "onDocumentRender: ");
        renderResponse(getToken(), true, "");
    }

    @Override
    public void onDocumentFinish() {
        Log.v(TAG, "onDocumentFinish: ");
    }

    //-------------------------------------------------------------------------
    // ILocalInfoDataReporter
    //-------------------------------------------------------------------------

    @Override
    public void platformDataItemSelectedById(String poiId) {
        if (mLocalInfoExtension != null) {
            Log.v(TAG, "poiSelected " + poiId);
            mLocalInfoExtension.poiSelected(mAplController, poiId);
            mAplEventSender.sendActivityEventRequest(mToken, IAPLEventSender.ActivityEvent.ONE_TIME);
        }
    }

    //-------------------------------------------------------------------------
    // Helper private methods
    //-------------------------------------------------------------------------

    /**
     * Returns the APL options that will be used in rendering document.
     *
     * @return APLOptions The options used for rendering.
     */
    private synchronized APLOptions getAPLOptions() {
        if (mAplOptions == null) {
            mAplOptions = getAPLOptionsBuilder().build();
        }
        return mAplOptions;
    }

    /**
     * Initialize the activity component object that provides APL options and configuration.
     *
     * @param context
     * @return ActivityComponent The activity component object.
     */
    private ActivityComponent initActivityComponent(Context context) {
        return DaggerActivityComponent.builder()
                .activityModule(new ActivityModule(context))
                .applicationComponent(mApplicationComponent)
                .build();
    }

    /**
     * Listen for touch events on the APL layout so that
     * we can report activity events to prevent document from
     * timing out when there is user interaction.
     */
    private void setupTouchListener() {
        for (APLLayout aplLayout : mAplLayouts.values()) {
            aplLayout.setOnTouchListener((view, event) -> {
                if (event.getAction() == MotionEvent.ACTION_UP) {
                    onTouchEvent(event);
                    aplLayout.performClick();
                }

                // Let the parent consume the event
                return false;
            });
        }
    }

    /**
     * Render the document.
     *
     * @param content the inflated contents
     */
    private void doRender(Content content) {
        // Finish the old document
        destroyAplView();
        applyScaling();

        mExtensionManager = new ExtensionManager(mRootConfig);
        Set<String> extensionRequests = content.getExtensionRequests();

        mBackHandler = new BackExtension(mBackStack, this::goBack, this);
        mBackHandler.setResponsibleForBackButton(true);
        mExtensionManager.addBuiltInExtension(mBackHandler);

        Map<String, Object> extensionSettings = content.getExtensionSettings(mBackHandler.getUri());
        if (extensionSettings != null) {
            Object backstackSettings = extensionSettings.get(BackExtension.SETTINGS_PROPERTY_BACKSTACK_ID);
            if (backstackSettings != null) {
                mBackHandler.setDocumentId(backstackSettings.toString());
            }
        }

        if (extensionRequests.contains(LocalInfoExtension.URI)) {
            mLocalInfoExtension = new LocalInfoExtension(mRootConfig);
            mLocalInfoExtension.setDataConsumer(mLocalInfoDataConsumer);
            mExtensionManager.addBuiltInExtension(mLocalInfoExtension);
        }

        // Register extension and render when done
        mExtensionManager.registerRequestedExtensions(extensionRequests, () -> performLayout(content));
    }

    private void performLayout(Content content) {
        // Make sure mAplLayout is inflated and initialized
        APLLayout aplLayout = getAplLayout();
        APLOptions options = getAPLOptions();

        try {
            aplLayout.post(() -> {
                IAPLViewPresenter presenter = aplLayout.getPresenter();
                aplLayout.getPresenter().addDocumentLifecycleListener(this);
                try {
                    mAplController = APLController.renderDocument(content, options, mRootConfig, presenter);
                    sendDeviceWindowState();
                    updateRuntimeProperties();
                } catch (Exception e) {
                    Log.e(TAG, "Cannot render ", e);
                    renderResponse(getToken(), false, e.getMessage());
                }
            });

        } catch (Exception e) {
            Log.e(TAG, "Render failed", e);
        }
    }

    private void renderResponse(String token, boolean result, String message) {
        mExecutor.submit(() -> {
            Log.i(TAG, "Render document result: " + result + " token: " + token + " message: " + message);
            mAplEventSender.sendRenderDocumentResult(token, result, message);
        });
    }

    /**
     * Clean up rendering session.
     */
    private synchronized void destroyAplView() {
        if (mAplController == null) {
            return;
        } else {
            if (mBackHandler != null) {
                BackStackDocument document =
                        new BackStackDocument(mBackHandler.getDocumentId(), mAplController.getDocumentState());
                mBackHandler.addDocument(document);
            }
        }

        mLocalInfoExtension = null;
        mExtensionManager = null;
        mAplController.finishDocument();
        mAplController = null;
        mAction = null;
    }

    /**
     * Set the scaling based on the supported view ports.
     */
    private void applyScaling() {
        List<ViewportMode> fallbackModes = Collections.singletonList(ViewportMode.kViewportModeAuto);
        List<Scaling.ViewportSpecification> viewportSpecifications = mRenderDocumentPayload.getViewportSpecifications();
        Log.v(TAG, "viewportSpecification: " + viewportSpecifications.toString());
        Scaling scaling = viewportSpecifications.isEmpty() ? new Scaling()
                                                           : new Scaling(10.0, viewportSpecifications, fallbackModes);

        // Apply the scaling
        getAplLayout().setScaling(scaling);
    }

    /**
     * Builds the initial device window state that will be reported after
     * platform interface registration.
     * {
     *                     "defaultWindowId": "string",
     *                     "instances" : [
     *                         {
     *                           "id": "string",
     *                           "templateId": "string",
     *                           "token" : "",
     *                           "configuration": {
     *                              "interactionMode": "string",
     *                              "sizeConfigurationId": "string"
     *                           }
     *                         }
     *                   ]
     * }
     *
     */
    private void sendDeviceWindowState() {
        JSONObject deviceWindowState = new JSONObject();
        JSONArray windowInstances = new JSONArray();
        String windowId = mRenderDocumentPayload != null && !mRenderDocumentPayload.getWindowId().isEmpty()
                ? mRenderDocumentPayload.getWindowId()
                : mDefaultWindowId;
        String token = getToken();

        try {
            if (mVisualCharacteristics.length() > 0) {
                for (int i = 0; i < mVisualCharacteristics.length(); i++) {
                    JSONObject currentElement = mVisualCharacteristics.getJSONObject(i);
                    if (currentElement.getString("interface").equals("Alexa.Display.Window")) {
                        JSONArray templates = currentElement.getJSONObject("configurations").getJSONArray("templates");
                        for (int j = 0; j < templates.length(); j++) {
                            JSONObject template = templates.getJSONObject(j);
                            JSONObject windowInstance = new JSONObject();
                            JSONObject windowConfiguration = new JSONObject();
                            JSONObject configuration = template.getJSONObject("configuration");
                            String templateId = template.getString("id");

                            windowInstance.put("id", templateId);
                            windowInstance.put("templateId", templateId);
                            windowInstance.put("token", "");

                            // Token must on the rendering window
                            if (windowId.equals(templateId)) {
                                windowInstance.put("token", token);
                            }

                            windowConfiguration.put(
                                    "interactionMode", configuration.getJSONArray("interactionModes").getString(0));
                            windowConfiguration.put("sizeConfigurationId",
                                    configuration.getJSONArray("sizes").getJSONObject(0).getString("id"));
                            windowInstance.put("configuration", windowConfiguration);

                            windowInstances.put(windowInstance);
                        }
                    }
                }

                deviceWindowState.put("defaultWindowId", mDefaultWindowId);
                deviceWindowState.put("instances", windowInstances);

                Log.v(TAG, "deviceWindowState: " + deviceWindowState.toString());
                mAplEventSender.sendWindowState(deviceWindowState.toString());
            }
        } catch (JSONException e) {
            Log.e(TAG, "Unable to build window state", e);
        }
    }

    //-------------------------------------------------------------------------
    // BackStack Support
    //-------------------------------------------------------------------------

    public void goBack(@NonNull BackStackDocument backStackDocument) {
        getAplLayout().post(() -> {
            mAplController.finishDocument();
            backStackDocument.getDocumentState().setOptions(getAPLOptions());
            try {
                mAplController = APLController.restoreDocument(
                        backStackDocument.getDocumentState(), getAplLayout().getPresenter());
            } catch (APLController.APLException e) {
                Log.e(TAG, "Document failed to restore.");
            }
        });
    }
}
