package com.amazon.sampleapp.impl.ContactIngestion;

/**
 * The Enum describing the input type of the Contacts.
 */
public enum ContactInputSourceType {

    FILE("FILE"),

    PHONE_CONTACT("PHONE_CONTACT");

    ContactInputSourceType(final String inputSourceType) {
        this.mInputSourceName = inputSourceType;
    }

    private String mInputSourceName;

    @Override
    public String toString() {
        return mInputSourceName;
    }
}

