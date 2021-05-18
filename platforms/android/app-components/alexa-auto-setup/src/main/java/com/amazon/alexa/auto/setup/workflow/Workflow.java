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
