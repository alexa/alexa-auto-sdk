package com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager;

import android.app.Activity;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;
import com.amazon.aace.contactuploader.ContactUploader;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.ContactIngestion.ContactInputSourceType;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploadAction;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.ContactUploaderHandler;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager.ContactIngestionAsyncTasks.CancelUploadContactsTask;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager.ContactIngestionAsyncTasks.RemoveUploadedContactsTask;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager.ContactIngestionAsyncTasks.UploadContactsTask;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import static com.amazon.sampleapp.impl.ContactIngestion.ContactUploadAction.*;

/**
 * The base class initializing the basic attributes of state manager classes of Phone-Connection.
 */
public final class PhoneConnectionStateManagerHelper {

    private static PhoneConnectionStateManagerHelper instance;

    private Activity mActivity;

    private View mContactUploadView;
    private TextView mContactUploadViewHeader;
    private TextView mContactUploadButton;
    private ProgressBar mContactIngestionProgressBar;
    private TextView mAccessToContactsStatus;
    private TextView mContactIngestionStatus;

    private ExecutorService mExecutorService;
    private Map<ContactUploadAction, Future> mContactUploadTaskMap;
    private ContactUploader.ContactUploadStatus mCurrentContactUploaderStatus;

    private long mUploadTimeStart;
    private long mUploadTimeEnd;
    private long mRemoveTimeStart;
    private long mRemoveTimeEnd;

    private PhoneConnectionStateManagerHelper(final Activity activity) {
        mActivity = activity;
        mContactUploadTaskMap = new ConcurrentHashMap<>();
        mExecutorService = Executors.newSingleThreadExecutor();
        mCurrentContactUploaderStatus = ContactUploader.ContactUploadStatus.UNKNOWN_ERROR;
        resetTimers();
        initializeContactUploadViewsByIds();
    }

    /**
     * Method to return singleton instance of {@link PhoneConnectionStateManagerHelper}.
     * @param activity the main activity.
     * @return the singleton instance of {@link PhoneConnectionStateManagerHelper}.
     */
    public static PhoneConnectionStateManagerHelper getInstance(final Activity activity) {
        if (instance == null) {
            instance = new PhoneConnectionStateManagerHelper(activity);
        }
        return instance;
    }

    /**
     * Initializes the UI components of Contact-Uploader.
     */
    private void initializeContactUploadViewsByIds() {
        mContactUploadView = mActivity.findViewById(R.id.phone_contact_uploader);
        mContactUploadViewHeader = mContactUploadView.findViewById(R.id.phone_contact_uploader_header);
        mContactUploadButton = mContactUploadView.findViewById(R.id.upload_cancel_or_remove_contacts);
        mContactIngestionProgressBar = mContactUploadView.findViewById(R.id.contact_upload_progress_cyclic);
        mAccessToContactsStatus = mContactUploadView.findViewById(R.id.access_to_contacts_status);
        mContactIngestionStatus = mContactUploadView.findViewById(R.id.status_of_contact_ingestion);
    }

    /**
     * Make the contact upload progressbar visible.
     */
    public void showProgressBar() {
        mContactIngestionProgressBar.setVisibility(View.VISIBLE);
    }

    /**
     * Hide the contact-upload progressbar.
     */
    public void hideProgressBar() {
        mContactIngestionProgressBar.setVisibility(View.GONE);
    }

    /**
     * Update the name of Contact-Upload button.
     * @param actionNameResId the name of the action which is one of UPLOAD or CANCEL or REMOVE.
     */
    public void updateContactIngestionButtonWithActionName(final int actionNameResId) {
        mContactUploadButton.setText(actionNameResId);
    }

    /**
     * Show the current contact ingestion status.
     * @param ingestionStatusResId the status of contact ingestion.
     */
    public void showCurrentContactIngestionStatus(final int ingestionStatusResId) {
        mContactIngestionStatus.setText(ingestionStatusResId);
        mContactIngestionStatus.setVisibility(View.VISIBLE);
    }

    /**
     * Hiding the contact ingestion status.
     */
    public void hideContactIngestionStatus() {
        mContactIngestionStatus.setVisibility(View.GONE);
    }

    /**
     * Remove the futures from the Action-to-future map.
     * @param contactUploadActions the action states for which the map entry is needed to be removed.
     */
    public void removeFromFutureMapIfPresent(ContactUploadAction... contactUploadActions) {
        for (ContactUploadAction contactUploadAction : contactUploadActions) {
            if (mContactUploadTaskMap.containsKey(contactUploadAction)) {
                mContactUploadTaskMap.remove(contactUploadAction);
            }
        }
    }

    /**
     * Updates the Contact-Uploader UI on denial of access.
     */
    public void updateViewOnContactAccessDenial() {
        mContactUploadButton.setVisibility(View.GONE);
        mContactIngestionProgressBar.setVisibility(View.GONE);
        mAccessToContactsStatus.setVisibility(View.VISIBLE);
        mAccessToContactsStatus.setText(R.string.denied_access_to_contacts);
        mContactIngestionStatus.setVisibility(View.GONE);
    }

    /**
     * Updates te Contact-Uploader UI on start up.
     */
    public void showInitialViewOnContactBoard() {
        mContactUploadButton.setVisibility(View.GONE);
        mContactIngestionProgressBar.setVisibility(View.GONE);
        mAccessToContactsStatus.setVisibility(View.VISIBLE);
        mAccessToContactsStatus.setText(R.string.no_phone_is_connected);
        mContactIngestionStatus.setVisibility(View.GONE);
    }

    /**
     * Updates the view of Contact-Uploader on Confirmation of access.
     */
    public void updateViewOnContactAccessConfirmation() {
        mContactUploadButton.setVisibility(View.VISIBLE);
        mContactIngestionStatus.setVisibility(View.GONE);
        mContactUploadButton.setText(R.string.upload_contacts);
        mAccessToContactsStatus.setVisibility(View.VISIBLE);
        mAccessToContactsStatus.setText(R.string.granted_access_to_contacts);
    }

    /**
     * Interrupt the contact uploader sync task based on the action states.
     * @param contactUploadActions the {@link ContactUploadAction}s to interrupt.
     */
    public void interruptContactUploadActionTasks(final ContactUploadAction... contactUploadActions) {
        for (ContactUploadAction contactUploadAction : contactUploadActions) {
            if (mContactUploadTaskMap.containsKey(contactUploadAction)) {
                final Future future = mContactUploadTaskMap.get(contactUploadAction);
                if (!future.isDone() || !future.isCancelled()) {
                    future.cancel(true);
                }
                mContactUploadTaskMap.remove(contactUploadAction);
            }
        }
    }

    /**
     * Helper method to start uploading contacts from a specified contact-source.
     */
    private void startUploadingContacts(final LoggerHandler logger,
                                       final ContactInputSourceType contactInputSourceType,
                                       final ContactUploaderHandler contactUploaderHandler) {
        submitContactUploadTask(UPLOAD, new UploadContactsTask(mActivity, logger,
                contactInputSourceType, contactUploaderHandler));
    }

    /**
     * Helper method to start cancelling contacts.
     */
    private void cancelUploadingContacts(final LoggerHandler logger,
                                        final ContactUploaderHandler contactUploaderHandler) {
        interruptContactUploadActionTasks(UPLOAD);
        submitContactUploadTask(CANCEL, new CancelUploadContactsTask(logger, contactUploaderHandler));
    }

    /**
     * Helper method to start removing contacts.
     */
    private void removeUploadedContacts(final LoggerHandler logger,
                                        final ContactUploaderHandler contactUploaderHandler) {
        submitContactUploadTask(REMOVE, new RemoveUploadedContactsTask(logger, contactUploaderHandler));
    }

    /**
     * Sets the onClickListeners for contact-upload button.
     */
    public void setOnclickListenerForUploadOrCancelButton(final LoggerHandler logger,
                                                          final ContactInputSourceType contactInputSourceType,
                                                          final ContactUploaderHandler contactUploaderHandler) {
        mContactUploadButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switch (ContactUploadAction.valueOf(String.valueOf(mContactUploadButton.getText()))) {
                    case UPLOAD:
                        disableContactUploaderButton();
                        startUploadingContacts(logger, contactInputSourceType, contactUploaderHandler);
                        break;
                    case CANCEL:
                        disableContactUploaderButton();
                        cancelUploadingContacts(logger, contactUploaderHandler);
                        break;
                    case REMOVE:
                        disableContactUploaderButton();
                        removeUploadedContacts(logger, contactUploaderHandler);
                        break;
                }
            }
        });
    }

    /**
     * Helper method to delete uploaded contacts.
     */
    public void deleteContacts(final LoggerHandler logger,
                               final ContactUploaderHandler contactUploaderHandler) {
        if (mCurrentContactUploaderStatus != null
                && (mCurrentContactUploaderStatus == ContactUploader.ContactUploadStatus.UPLOAD_CONTACTS_STARTED
                || mCurrentContactUploaderStatus == ContactUploader.ContactUploadStatus.UPLOAD_CONTACTS_UPLOADING)) {
            cancelUploadingContacts(logger, contactUploaderHandler);
        } else {
            removeUploadedContacts(logger, contactUploaderHandler);
        }
    }

    /**
     * Submitting contact uploading tasks for execution.
     * @param contactUploadAction a {@link ContactUploadAction}
     * @param contactUploadTask the task i.e. contact upload, cancel upload or remove upload to execute
     */
    public void submitContactUploadTask(final ContactUploadAction contactUploadAction,
                                        final Runnable contactUploadTask) {
        mContactUploadTaskMap.put(contactUploadAction,
                mExecutorService.submit(contactUploadTask));
    }
    /**
     * Disabling the contact uploader button.
     */
    public void disableContactUploaderButton() {
        mContactUploadButton.setEnabled(false);
    }

    /**
     * Enabling the contact-uploader button.
     */
    public void enableContactUploaderButton() {
        mContactUploadButton.setEnabled(true);
    }

    /**
     * Tracks the current contact-upload status.
     * @param contactUploadStatus
     */
    public void updateContactUploaderState(final ContactUploader.ContactUploadStatus contactUploadStatus) {
        mCurrentContactUploaderStatus = contactUploadStatus;
    }

    public void resetTimers() {
        resetContactsUploadTimers();
        resetContactRemoveTimers();
    }

    public void resetContactsUploadTimers() {
        mUploadTimeStart = mUploadTimeEnd = -1;
    }

    public void resetContactRemoveTimers() {
        mRemoveTimeStart = mRemoveTimeEnd = -1;
    }

    public void setStartUploadingContactTimeStamp(final long timeStampInMilliSeconds) {
        mUploadTimeStart = timeStampInMilliSeconds;
    }

    public void setFinishUploadingContactTimeStamp(final long timeStampInMilliSeconds) {
        mUploadTimeEnd = timeStampInMilliSeconds;
    }

    public void setStartRemovingContactTimeStamp(final long timeStampInMilliSeconds) {
        mRemoveTimeStart = timeStampInMilliSeconds;
    }

    public void setFinishRemovingContactTimeStamp(final long timeStampInMilliSeconds) {
        mRemoveTimeEnd = timeStampInMilliSeconds;
    }

    public void logTimes(final LoggerHandler loggerHandler, final String tag) {
        if (mUploadTimeStart != -1 && mUploadTimeEnd != -1 && mUploadTimeEnd >= mUploadTimeStart) {
            loggerHandler.postInfo(tag, "Time taken to upload contacts is : " + (mUploadTimeEnd - mUploadTimeStart)/1000 + " secs.");
        }
        if (mRemoveTimeStart != -1 && mRemoveTimeEnd != -1 && mRemoveTimeEnd >= mRemoveTimeStart) {
            loggerHandler.postInfo(tag, "Time taken to remove contacts is : " + (mUploadTimeEnd - mUploadTimeStart)/1000 + " secs.");
        }
    }
}
