package com.amazon.sampleapp.impl.ContactIngestion;

/**
 * The Enum describing the actions of Contact-Uploader.
 */
public enum ContactUploadAction {

    UPLOAD("UPLOAD"),

    CANCEL("CANCEL"),

    REMOVE("REMOVE");

    ContactUploadAction(final String contactUploadActionName) {
        this.mContactUploadActionName = contactUploadActionName;
    }

    private final String mContactUploadActionName;

    @Override
    public String toString() {
        return mContactUploadActionName;
    }
}
