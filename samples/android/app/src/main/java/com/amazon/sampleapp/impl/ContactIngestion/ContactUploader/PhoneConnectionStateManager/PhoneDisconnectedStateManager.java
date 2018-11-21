package com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager;

import android.app.Activity;
import com.amazon.aace.contactuploader.ContactUploader;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.ContactUploaderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

/**
 * The state-manager class managing the state transitions of the Contact-Uploader UI when a Phone is disconnected.
 */
public class PhoneDisconnectedStateManager implements PhoneConnectionStateManager {

    private static final String sTag = PhoneDisconnectedStateManager.class.getSimpleName();
    private static PhoneDisconnectedStateManager instance;

    private LoggerHandler mLogger;
    private ContactUploaderHandler mContactUploaderHandler;
    private PhoneConnectionStateManagerHelper mPhoneConnectionStateManagerHelper;

    private PhoneDisconnectedStateManager(final Activity activity,
                                          final LoggerHandler logger,
                                          final ContactUploaderHandler contactUploaderHandler) {
        mLogger = logger;
        mContactUploaderHandler = contactUploaderHandler;
        mPhoneConnectionStateManagerHelper = PhoneConnectionStateManagerHelper.getInstance(activity);
    }

    /**
     * Method to get the singleton instance of {@link PhoneDisconnectedStateManager}.
     * @param activity the {@link com.amazon.sampleapp.MainActivity}
     * @param logger the {@link LoggerHandler}
     * @param contactUploaderHandler the {@link ContactUploaderHandler}
     * @return the singleton instance of {@link PhoneDisconnectedStateManager}
     */
    public static PhoneDisconnectedStateManager getInstance(final Activity activity, final LoggerHandler logger, final ContactUploaderHandler contactUploaderHandler) {
        if (instance == null) {
            instance = new PhoneDisconnectedStateManager(activity, logger, contactUploaderHandler);
        }
        return instance;
    }

    /**
     * Handles states of Contact-Uploader upon connecting or disconnecting a phone.
     */
    @Override
    public void handleState() {
        mPhoneConnectionStateManagerHelper.showInitialViewOnContactBoard();
        mPhoneConnectionStateManagerHelper.deleteContacts(mLogger, mContactUploaderHandler);
    }

    /**
     * The callback function from engine upon detecting a change in contact upload status.
     * @param contactUploadStatus the current status of contact upload, {@link ContactUploader.ContactUploadStatus}
     * @param jsonReason the reason of failure
     */
    @Override
    public void onContactUploadStatusChanged(final ContactUploader.ContactUploadStatus contactUploadStatus, final String jsonReason) {
        mLogger.postInfo(sTag, "Received contact ingestion callback status as " + contactUploadStatus.toString() + " from engine ");
        mPhoneConnectionStateManagerHelper.updateContactUploaderState(contactUploadStatus);
        mPhoneConnectionStateManagerHelper.hideProgressBar();
        mPhoneConnectionStateManagerHelper.hideContactIngestionStatus();

        if (contactUploadStatus == ContactUploader.ContactUploadStatus.REMOVE_CONTACTS_STARTED) {
            mPhoneConnectionStateManagerHelper.resetTimers();
            mPhoneConnectionStateManagerHelper.setStartRemovingContactTimeStamp(System.currentTimeMillis());
        }

        if (contactUploadStatus == ContactUploader.ContactUploadStatus.REMOVE_CONTACTS_COMPLETED) {
            mPhoneConnectionStateManagerHelper.setFinishRemovingContactTimeStamp(System.currentTimeMillis());
            mPhoneConnectionStateManagerHelper.logTimes(mLogger, sTag);
        }
    }
}
