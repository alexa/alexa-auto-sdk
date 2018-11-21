package com.amazon.sampleapp.impl.ContactIngestion.ContactUploader;

import android.app.Activity;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;
import com.amazon.aace.contactuploader.ContactUploader;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager.PhoneConnectionStateManager;
import com.amazon.sampleapp.impl.ContactIngestion.ContactUploader.PhoneConnectionStateManager.PhoneConnectionStateManagerFactory;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

/**
 * The handler class which handles contact upload, cancel and delete.
 */
public class ContactUploaderHandler extends ContactUploader {

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private PhoneConnectionStateManager mPhoneConnectionStateHandler;

    public ContactUploaderHandler(final Activity activity,
                                  final LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
    }

    /**
     * Initializes the Contact-Uploader.
     */
    public void onInitialize() {
        setupContactUploader();
    }

    /**
     * Performs boot up operations of Contact-Uploader.
     * (1) Renders the initial view of Contact-Uploader
     * (2) Sets listeners on phone connection toggle controller
     * (3) Invokes a state manager to delete previously uploaded address book
     */
    private void setupContactUploader() {
        // initial UI setup when the app boots up

        // Switch to toggle phone call connection state
        View switchItem = mActivity.findViewById( R.id.togglePhoneConnection );
        ( (TextView) switchItem.findViewById( R.id.text ) ).setText( R.string.contact_upload_switch);
        SwitchCompat contactUploadSwitch = switchItem.findViewById( R.id.drawerSwitch );
        contactUploadSwitch.setChecked( false );

        // sets the listener on the phone connection toggle controller
        contactUploadSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                handlePhoneConnectivityStates(isChecked);
            }
        });
    }

    /**
     * Invokes a state-manager class according to phone connection.
     * @param isChecked a boolean representing whether a phone is connected or not
     */
    private void handlePhoneConnectivityStates(final boolean isChecked) {
        mPhoneConnectionStateHandler = PhoneConnectionStateManagerFactory.getPhoneConnectionStateManager(mActivity,
                mLogger, isChecked, this);
        mPhoneConnectionStateHandler.handleState();
    }

    /**
     * The callback function from engine upon detecting a change in contact upload status.
     * @param contactUploadStatus current status of contact upload, {@link ContactUploadStatus}
     * @param info the reason of failure
     */
    @Override
    public void contactsUploaderStatusChanged(final ContactUploadStatus contactUploadStatus, final String info) {
        mPhoneConnectionStateHandler.onContactUploadStatusChanged(contactUploadStatus, info);
    }
}
