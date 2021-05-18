package com.amazon.alexa.auto.setup.workflow;

/**
 * Alexa setup workflow step.
 */
public class WorkflowStep {
    private final String mEvent;
    private final String mType;
    private final String mResource;

    /**
     * Constructor.
     * @param event Setup workflow event.
     * @param type  Setup workflow type, each workflow step could be a view or an action.
     * @param resource Resource to execute workflow step.
     */
    public WorkflowStep(String event, String type, String resource) {
        mEvent = event;
        mType = type;
        mResource = resource;
    }

    public String getEvent() {
        return mEvent;
    }

    public String getType() {
        return mType;
    }

    public String getResource() {
        return mResource;
    }
}
