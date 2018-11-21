package com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager;

import android.app.Activity;

import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.ContactUploaderHandler;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

/**
 * The factory class to produce the state-manager classes based on the connectivity of the phone.
 */
public class PhoneConnectionStateManagerFactory {

    /**
     * Factory method to return a singleton instance of {@link PhoneConnectionStateManager}.
     * @param activity the {@link com.amazon.sampleapp.MainActivity}
     * @param logger the {@link LoggerHandler}
     * @param isPhoneConnected a boolean representing whether the phone is connected or not
     * @param contactUploaderHandler the {@link ContactUploaderHandler}
     * @return a singleton instance of {@link PhoneConnectionStateManager}
     */
    public static final PhoneConnectionStateManager getPhoneConnectionStateManager(
            final Activity activity,
            final LoggerHandler logger,
            final boolean isPhoneConnected,
            final ContactUploaderHandler contactUploaderHandler) {
        if (isPhoneConnected) {
            return PhoneConnectedStateManager.getInstance(activity, logger, contactUploaderHandler);
        }
        return PhoneDisconnectedStateManager.getInstance(activity, logger, contactUploaderHandler);
    }
}
