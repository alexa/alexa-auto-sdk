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

import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * Workflow contains a list of workflow steps. It provides methods to set/get workflow or workflow step
 * based on the workflow event.
 */
public class Workflow {
    private static final String TAG = Workflow.class.getSimpleName();

    private final List<WorkflowStep> mWorkflowSteps;

    public Workflow() {
        mWorkflowSteps = new ArrayList<>();
    }

    public void addWorkflowStep(String event, String type, String resource) {
        mWorkflowSteps.add(new WorkflowStep(event, type, resource));
    }

    public List<WorkflowStep> getWorkflowSteps() {
        return mWorkflowSteps;
    }

    public WorkflowStep getWorkflowStep(String event) {
        for (WorkflowStep step : mWorkflowSteps) {
            if (step.getEvent().equals(event)) {
                return step;
            }
        }

        Log.e(TAG, "Failed to get the workflow step with event");
        return null;
    }
}
