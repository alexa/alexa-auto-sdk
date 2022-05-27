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
package com.amazon.alexa.auto.setup.workflow;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.res.Resources;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.amazon.alexa.auto.apps.common.util.ModuleProvider;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.nio.charset.StandardCharsets;
import java.util.Optional;
import java.util.concurrent.ExecutorService;

import io.reactivex.rxjava3.core.Single;

/**
 * Alexa setup workflow provider helps parse step workflow from workflow specification file based on the
 * supported extra modules, it also provides the workflow step based on the event.
 */
public class WorkflowProvider {
    private static final String TAG = WorkflowProvider.class.getSimpleName();

    private static final String SPECIFICATION_DIR = "workflowSpecification";
    private static final String CBL_LOGIN_WORKFLOW_SPECIFICATION_FILE = "CBLLoginWorkflowSpecification.json";
    private static final String PREVIEW_MODE_LOGIN_WORKFLOW_SPECIFICATION_FILE =
            "PreviewModeLoginWorkflowSpecification.json";
    private static final String VOICE_ASSISTANCE_LOGIN_WORKFLOW_SPECIFICATION_FILE =
            "VoiceAssistanceWorkflowSpecification.json";

    private final WeakReference<Context> mContext;

    @NonNull
    private final ExecutorService mExecutorService;
    @NonNull
    private final Handler mMainThreadHandler;

    @VisibleForTesting
    Workflow mWorkflowSpecification;
    private String mWorkflowStartStep = "";

    /**
     * Constructor.
     * @param context Android Context.
     * @param executorService Executor Service.
     */
    public WorkflowProvider(WeakReference<Context> context, @NonNull ExecutorService executorService) {
        mContext = context;
        mExecutorService = executorService;
        mMainThreadHandler = new Handler(Looper.getMainLooper());
    }

    /**
     * Get first workflow's starting point.
     * @return Setup workflow's starting point.
     */
    public Optional<WorkflowStep> getWorkflowStartingStep() {
        return Optional.of(mWorkflowSpecification.getWorkflowStep(mWorkflowStartStep));
    }

    /**
     * Navigate to next workflow step with workflow event.
     * @param event Setup workflow event.
     * @return Next setup workflow step.
     */
    public Optional<WorkflowStep> nextWorkflowStep(String event) {
        for (WorkflowStep step : mWorkflowSpecification.getWorkflowSteps()) {
            if (step.getEvent().equals(event)) {
                return Optional.of(step);
            }
        }

        Log.d(TAG, "failed to find the workflow step with event " + event);
        return Optional.empty();
    }

    /**
     * Fetch Alexa setup workflow specification from file.
     * @param workflow Alexa setup workflow.
     */
    @VisibleForTesting
    Optional<Workflow> fetchWorkflowSpecificationFromFile(String workflow) {
        if (!workflow.equals("")) {
            Workflow workflowSpecification = new Workflow();

            try {
                JSONObject jsonObject = new JSONObject(workflow);
                mWorkflowStartStep = jsonObject.getString("startStep");

                JSONArray array = jsonObject.getJSONArray("workflow");
                for (int i = 0; i < array.length(); i++) {
                    JSONObject objectInArray = array.getJSONObject(i);
                    String event = objectInArray.getString("event");
                    JSONObject next = objectInArray.getJSONObject("next");
                    String type = next.getString("type");
                    String resource = next.getString("resource");
                    workflowSpecification.addWorkflowStep(event, type, resource);
                }
            } catch (JSONException e) {
                Log.e(TAG, "Failed to parse workflow items from workflow specification json file");
                return Optional.empty();
            }
            return Optional.of(workflowSpecification);
        }
        return Optional.empty();
    }

    /**
     * Read setup workflow specification file asynchronously.
     * @param extraModules Extra modules supported by Auto SDK.
     * @return future for Alexa setup workflow.
     */
    public Single<Optional<Workflow>> readWorkflowSpecificationAsync(String extraModules) {
        return Single.create(emitter -> {
            mExecutorService.submit(() -> {
                String workflowFile = readWorkflowSpecificationSync(mContext.get(), extraModules);
                Optional<Workflow> workflowOptional = fetchWorkflowSpecificationFromFile(workflowFile);
                workflowOptional.ifPresent(workflow -> mWorkflowSpecification = workflow);
                mMainThreadHandler.post(() -> emitter.onSuccess(workflowOptional));
            });
        });
    }

    /**
     * Reads the content of setup workflow specification file.
     *
     * @param context Android Context.
     * @param extraModules Extra modules supported by Auto SDK.
     * @return File contents.
     */
    @NonNull
    public String readWorkflowSpecificationSync(@NonNull Context context, String extraModules) {
        AssetManager assetManager = context.getAssets();
        try {
            return readStream(
                    assetManager.open(SPECIFICATION_DIR + "/" + getWorkflowSpecificationWithModules(extraModules)));
        } catch (IOException e) {
            Log.e(TAG, "Failed to fetch workflow contents from setup workflow specification file.");
        }
        return "";
    }

    /**
     * Get setup workflow step's resource ID.
     * @param workflowStep Setup workflow step.
     * @return Resource ID.
     */
    public int getResourceIdFromWorkflowStep(WorkflowStep workflowStep) {
        Resources res = mContext.get().getResources();
        return res.getIdentifier(workflowStep.getResource(), "id", mContext.get().getPackageName());
    }

    /**
     * Read the content of stream as text string.
     *
     * @param inputStream Input stream.
     * @return Content of input stream as text string.
     */
    private String readStream(@NonNull InputStream inputStream) throws IOException {
        byte[] buffer = new byte[inputStream.available()];
        inputStream.read(buffer);
        return new String(buffer, StandardCharsets.UTF_8);
    }

    /**
     * Get setup workflow specification file based on the supported modules.
     * @param extraModules Extra modules supported by Auto SDK.
     * @return File name.
     */
    private String getWorkflowSpecificationWithModules(String extraModules) {
        if (extraModules.contains(ModuleProvider.ModuleName.ALEXA_CUSTOM_ASSISTANT.name())) {
            Log.d(TAG, "Loading workflow specification for Alexa Custom Assistant.");
            return VOICE_ASSISTANCE_LOGIN_WORKFLOW_SPECIFICATION_FILE;
        } else if (extraModules.contains(ModuleProvider.ModuleName.PREVIEW_MODE.name())) {
            Log.d(TAG, "Loading workflow specification for preview mode.");
            return PREVIEW_MODE_LOGIN_WORKFLOW_SPECIFICATION_FILE;
        } else {
            Log.d(TAG, "Loading workflow specification for CBL login by default.");
            return CBL_LOGIN_WORKFLOW_SPECIFICATION_FILE;
        }
    }
}
