package com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager.ContactIngestionAsyncTasks;

import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.ContactUploaderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

/**
 * The asynchronous task to remove all of the uploaded address-books from a connected phone.
 */
public class RemoveUploadedContactsTask implements Runnable {

    private static final String sTag = RemoveUploadedContactsTask.class.getSimpleName();

    final LoggerHandler mLogger;
    final ContactUploaderHandler mContactUploaderHandler;

    public RemoveUploadedContactsTask(final LoggerHandler logger,
                                      final ContactUploaderHandler contactUploaderHandler) {
        mLogger = logger;
        mContactUploaderHandler = contactUploaderHandler;
    }

    @Override
    public void run() {
        mContactUploaderHandler.removeUploadedContacts();
        mLogger.postInfo(sTag, "Removing uploaded contacts.");
    }
}
