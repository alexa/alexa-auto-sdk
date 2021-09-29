package com.amazon.alexa.auto.setup.workflow.model;

public enum LocationConsent {
    ENABLED("ENABLED"),
    DISABLED("DISABLED");

    String value;

    LocationConsent(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }
}
