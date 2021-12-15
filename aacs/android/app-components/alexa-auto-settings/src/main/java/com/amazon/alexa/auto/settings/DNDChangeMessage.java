package com.amazon.alexa.auto.settings;

/**
 * Holds do not disturb change attributes
 */
public class DNDChangeMessage {

    private boolean checked;

    public DNDChangeMessage(boolean checked) {
        this.checked = checked;
    }

    public boolean isChecked() {
        return checked;
    }
}
