package com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager.ContactIngestionAsyncTasks;

import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.ContactUploaderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

/**
 * The asynchronous task to abort uploading of contacts from the connected phone.
 */
public class CancelUploadContactsTask implements Runnable {

    private final String sTag = CancelUploadContactsTask.class.getSimpleName();

    final LoggerHandler mLogger;
    final ContactUploaderHandler mContactUploaderHandler;

    public CancelUploadContactsTask(final LoggerHandler logger,
                                    final ContactUploaderHandler contactUploaderHandler) {
        mLogger = logger;
        mContactUploaderHandler = contactUploaderHandler;
    }

    @Override
    public void run() {
        mLogger.postInfo(sTag, "Calling engine to cancel contact upload.");
            final boolean cancelUpload = mContactUploaderHandler.addContactsCancel();
            if (cancelUpload) {
                mLogger.postInfo(sTag, "Started cancelling contact upload.");
            } else {
                mLogger.postError(sTag, "Failed to cancel contact upload.");
            }
    }
}
