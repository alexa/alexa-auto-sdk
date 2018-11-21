package com.amazon.sampleapp.impl.ContactIngestion.ContactPojos;

/**
 * An Enum describing the source type of a contact.
 */
public enum ContactSourceType {

    PHONENUMBER("PHONENUMBER");

    ContactSourceType(final String sourceType) {
        this.sourceType = sourceType;
    }

    private String sourceType;

    @Override
    public String toString() {
        return sourceType.toLowerCase();
    }
}
