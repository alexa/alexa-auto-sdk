package com.amazon.alexa.auto.setup.workflow;

/**
 * Alexa setup workflow message.
 */
public class WorkflowMessage {
    private String mEvent;

    public WorkflowMessage(String event) {
        this.mEvent = event;
    }

    public String getWorkflowEvent() {
        return this.mEvent;
    }
}
