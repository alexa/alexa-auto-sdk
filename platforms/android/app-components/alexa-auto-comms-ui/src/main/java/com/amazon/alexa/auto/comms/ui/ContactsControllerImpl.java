package com.amazon.alexa.auto.comms.ui;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.comms.ui.db.BTDevice;
import com.amazon.alexa.auto.comms.ui.db.BTDeviceRepository;
import com.amazon.alexa.auto.comms.ui.db.ConnectedBTDeviceRepository;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;

import java.lang.ref.WeakReference;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

public class ContactsControllerImpl implements ContactsController {
    private static final String TAG = ContactsControllerImpl.class.getSimpleName();

    private final WeakReference<Context> mContextWk;
    private final BTDeviceRepository mBTDeviceRepository;
    private final ConnectedBTDeviceRepository mConnectedBTDeviceRepository;
    private final BehaviorSubject<Boolean> mContactsUploadConsentSubject;

    /**
     * Constructs an instance of ContactsControllerImpl.
     *
     * @param contextWk Android Context.
     */
    public ContactsControllerImpl(@NonNull WeakReference<Context> contextWk) {
        mContextWk = contextWk;
        mBTDeviceRepository = BTDeviceRepository.getInstance(mContextWk.get());
        mConnectedBTDeviceRepository = ConnectedBTDeviceRepository.getInstance(mContextWk.get());
        mContactsUploadConsentSubject = BehaviorSubject.create();

        if (!EventBus.getDefault().isRegistered(this)) {
            EventBus.getDefault().register(this);
        }
    }

    @Override
    public void uploadContacts(String deviceAddress) {
        Log.d(TAG, "Uploading contacts...");
        Intent intentStartService = new Intent();
        intentStartService.setComponent(
                new ComponentName(AACSConstants.AACS_PACKAGE_NAME, Constants.AACS_CONTACTS_SERVICE));
        intentStartService.setAction(Constants.AACS_ACTION_UPLOAD_CONTACTS);
        intentStartService.putExtra(Constants.AACS_EXTRA_ADDRESSBOOK_ID, deviceAddress);
        intentStartService.putExtra(
                Constants.AACS_EXTRA_ADDRESSBOOK_NAME_KEY, Constants.AACS_EXTRA_ADDRESSBOOK_NAME_VALUE);
        startAACSService(mContextWk.get(), intentStartService);
    }

    @Override
    public void removeContacts(String deviceAddress) {
        Log.d(TAG, "Removing contacts...");
        Intent intentStartService = new Intent();
        intentStartService.setComponent(
                new ComponentName(AACSConstants.AACS_PACKAGE_NAME, Constants.AACS_CONTACTS_SERVICE));
        intentStartService.setAction(Constants.AACS_ACTION_REMOVE_CONTACTS);
        intentStartService.putExtra(Constants.AACS_EXTRA_ADDRESSBOOK_ID, deviceAddress);
        startAACSService(mContextWk.get(), intentStartService);
    }

    @Override
    public void setContactsUploadPermission(String deviceAddress, String permission) {
        mBTDeviceRepository.getBTDeviceByAddress(deviceAddress);
        mBTDeviceRepository.updateContactsPermission(deviceAddress, permission);
        mConnectedBTDeviceRepository.updateContactsPermission(deviceAddress, permission);
    }

    @Override
    public Observable<Boolean> observeContactsConsent() {
        mBTDeviceRepository.findPrimaryBTDeviceEntry();
        return mContactsUploadConsentSubject;
    }

    @Subscribe
    public void onBTDeviceDiscoveryChange(BTDeviceRepository.BTDeviceDiscoveryMessage message) {
        if (message.isFound()) {
            Log.d(TAG, "Bluetooth device is found.");

            BTDevice device = message.getBTDevice();
            if (message.getBTDevice().getContactsUploadPermission().equals(Constants.CONTACTS_PERMISSION_NO)) {
                mContactsUploadConsentSubject.onNext(true);
            } else {
                mContactsUploadConsentSubject.onNext(false);
                uploadContacts(device.getDeviceAddress());
            }
        } else {
            Log.d(TAG, "Bluetooth device is not found, skipping contacts upload consent step.");
            mContactsUploadConsentSubject.onNext(false);
        }
    }

    @Subscribe
    public void onConnectedBTDeviceDiscoveryChange(
            ConnectedBTDeviceRepository.ConnectedBTDeviceDiscoveryMessage connectedDeviceDiscoveryMessage) {
        if (connectedDeviceDiscoveryMessage.isFound()) {
            Log.d(TAG,
                    "Device " + connectedDeviceDiscoveryMessage.getBConnectedBTDevice().getDeviceAddress()
                            + "is disconnected, remove it from connected device database.");
            mConnectedBTDeviceRepository.deleteEntry(connectedDeviceDiscoveryMessage.getBConnectedBTDevice());
        }
    }

    /**
     * Start AACS service with intent.
     * @param context Android context.
     * @param intent Android intent.
     */
    private void startAACSService(Context context, Intent intent) {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            context.startForegroundService(intent);
        } else {
            context.startService(intent);
        }
    }
}
