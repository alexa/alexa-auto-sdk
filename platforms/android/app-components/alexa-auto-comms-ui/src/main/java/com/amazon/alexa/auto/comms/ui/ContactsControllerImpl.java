package com.amazon.alexa.auto.comms.ui;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
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
    private static String _AACS_CONTACTS_PACKAGE_NAME = null; // This should remain private non final

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
        String aacsContactsPackageName = getAACSContactsPackageName(mContextWk);
        if (aacsContactsPackageName != null) {
            Intent intentStartService = new Intent();
            intentStartService.setComponent(
                    new ComponentName(aacsContactsPackageName, Constants.AACS_CONTACTS_SERVICE));
            intentStartService.setAction(Constants.AACS_ACTION_UPLOAD_CONTACTS);
            intentStartService.putExtra(Constants.AACS_EXTRA_ADDRESSBOOK_ID, deviceAddress);
            intentStartService.putExtra(
                    Constants.AACS_EXTRA_ADDRESSBOOK_NAME_KEY, Constants.AACS_EXTRA_ADDRESSBOOK_NAME_VALUE);
            startAACSService(mContextWk.get(), intentStartService);
        } else {
            Log.e(TAG, "AACS contacts service is not available, skip to upload contacts.");
        }
    }

    @Override
    public void removeContacts(String deviceAddress) {
        Log.d(TAG, "Removing contacts...");
        String aacsContactsPackageName = getAACSContactsPackageName(mContextWk);
        if (aacsContactsPackageName != null) {
            Intent intentStartService = new Intent();
            intentStartService.setComponent(
                    new ComponentName(aacsContactsPackageName, Constants.AACS_CONTACTS_SERVICE));
            intentStartService.setAction(Constants.AACS_ACTION_REMOVE_CONTACTS);
            intentStartService.putExtra(Constants.AACS_EXTRA_ADDRESSBOOK_ID, deviceAddress);
            startAACSService(mContextWk.get(), intentStartService);
        } else {
            Log.e(TAG, "AACS contacts service is not available, skip to remove contacts.");
        }
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

    /**
     * This method returns the package name of AACS Contacts Service dynamically.
     * If AACS Contacts Service is running in AACS as separate application, it returns value of AACS package name.
     * Otherwise if AACS Contacts Service is included in the client app as an AAR, it returns the client app
     * package name.
     * @param contextWk Weak reference of Android context for getting a package manager
     * @return AACS Contacts Service package name
     */
    private String getAACSContactsPackageName(@NonNull WeakReference<Context> contextWk) {
        if (_AACS_CONTACTS_PACKAGE_NAME == null) {
            PackageManager packageManager = contextWk.get().getPackageManager();
            try {
                _AACS_CONTACTS_PACKAGE_NAME = AACSConstants.getAACSPackageName(contextWk);
                PackageInfo packageInfo =
                        packageManager.getPackageInfo(contextWk.get().getPackageName(), PackageManager.GET_SERVICES);
                for (ServiceInfo serviceInfo : packageInfo.services) {
                    if (serviceInfo.name.equals(Constants.AACS_CONTACTS_SERVICE)) {
                        _AACS_CONTACTS_PACKAGE_NAME = contextWk.get().getPackageName();
                        Log.d(TAG, String.format("Setting PACKAGE_NAME %s", _AACS_CONTACTS_PACKAGE_NAME));
                        break;
                    }
                }
            } catch (PackageManager.NameNotFoundException e) {
                Log.e(TAG, "Failed to find AACS contacts package information in package manager.");
                return null;
            }
        }
        return _AACS_CONTACTS_PACKAGE_NAME;
    }
}
