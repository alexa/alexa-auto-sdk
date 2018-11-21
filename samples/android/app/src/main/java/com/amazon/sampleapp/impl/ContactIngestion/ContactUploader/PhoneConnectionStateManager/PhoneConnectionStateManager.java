package com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager;

import com.amazon.aace.contactuploader.ContactUploader;

/**
 * The interface defining the actions which can be taken upon phone is connected or disconnected.
 */
public interface PhoneConnectionStateManager {

    /**
     * Handles states of Contact-Uploader upon connecting or disconnecting a phone.
     */
    void handleState();

    /**
     * The callback function from engine upon detecting a change in contact upload status.
     * @param contactUploadStatus the current status of contact upload,  {@link ContactUploader.ContactUploadStatus}
     * @param jsonReason the reason of failure
     */
    void onContactUploadStatusChanged(ContactUploader.ContactUploadStatus contactUploadStatus, String jsonReason);
}
