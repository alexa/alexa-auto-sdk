package com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager.ContactIngestionAsyncTasks;

import android.app.Activity;
import com.amazon.sampleapp.impl.ContactIngestion.ContactInputSourceType;
import com.amazon.sampleapp.impl.ContactIngestion.ContactPojos.Contact;
import com.amazon.sampleapp.impl.ContactIngestion.ContactPojos.ContactSourceType;
import com.amazon.sampleapp.impl.ContactIngestion.ContactProvider.ContactProviderFactory;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.ContactUploaderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;
import org.json.JSONException;

import java.util.List;

/**
 * The asynchronous task to upload contacts from a connected phone.
 */
public class UploadContactsTask implements Runnable {

    private static final String sTag = UploadContactsTask.class.getSimpleName();

    final Activity mActivity;
    final LoggerHandler mLogger;
    final ContactUploaderHandler mContactUploaderHandler;
    final ContactInputSourceType mContactInputSourceType;

    public UploadContactsTask(final Activity activity,
                              final LoggerHandler logger,
                              final ContactInputSourceType contactInputSourceType,
                              final ContactUploaderHandler contactUploaderHandler) {
        mActivity = activity;
        mLogger = logger;
        mContactUploaderHandler = contactUploaderHandler;
        mContactInputSourceType = contactInputSourceType;
    }

    @Override
    public void run() {
        final boolean canStartUploadingContacts = mContactUploaderHandler.addContactsBegin();
        if (canStartUploadingContacts) {
            try {
                // retrieve the contacts from a contact source type.
                List<Contact> contactList = ContactProviderFactory
                        .generateContactProvider(mActivity, mLogger, mContactInputSourceType)
                        .getContactsByType(ContactSourceType.PHONENUMBER);
                mLogger.postInfo(sTag, "Retrieved a contact list of size : " + contactList.size());
                for (int i = 0; i < contactList.size(); i++) {
                    if (!Thread.currentThread().isInterrupted()) {
                        try {
                            final Contact contact = contactList.get(i);
                            final String contactInJsonFormat = contact.toJsonString();
                            final boolean isUploadedSuccessfully = mContactUploaderHandler.addContact(contactInJsonFormat);
                            if (!isUploadedSuccessfully) {
                                mLogger.postError(sTag, "Failed to upload contact : " + contactInJsonFormat);
                            }
                        } catch (final JSONException e) {
                            mLogger.postError(sTag, "Unable to convert a contact to expected JSON format, " + e);
                        }
                        if (isLastContactToUpload(i, contactList.size())) {
                            mContactUploaderHandler.addContactsEnd();
                            mLogger.postInfo(sTag, "All of the contacts are prepared to be uploaded from the connected phone.");
                        }
                    } else {
                        mLogger.postInfo(sTag, "Uploading thread is interrupted.");
                        break;
                    }
                }
            } catch (final Exception ex) {
                mLogger.postError(sTag, "Failed to retrieve contacts from contact store, " + ex);
            }
        } else {
            mLogger.postError(sTag, "Failed to start uploading contacts.");
        }
    }

    private boolean isLastContactToUpload(final int currentIndex, final int contactListSize) {
        return currentIndex == (contactListSize - 1);
    }
}
