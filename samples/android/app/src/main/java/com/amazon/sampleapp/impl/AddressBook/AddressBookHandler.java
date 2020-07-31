package com.amazon.sampleapp.impl.AddressBook;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.support.v7.widget.SwitchCompat;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.amazon.aace.addressbook.AddressBook;
import com.amazon.aace.addressbook.IAddressBookEntriesFactory;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * The handler class which handles contact upload, cancel and delete.
 */
public class AddressBookHandler extends AddressBook {
    private static final String sTag = "AddressBookHandler";

    private static final String sContactsSourceId = "0001";
    private static final String sNavigationFavoritesSourceId = "0002";
    private static final String[] sSourceIds = {sContactsSourceId, sNavigationFavoritesSourceId};

    private final String mContactsDataPath;
    private final String mNavigationFavoritesDataPath;

    private View mContactsUploadView;
    private TextView mContactsUploadAccessStatus;

    private View mNavigationFavoritesUploadView;
    private TextView mNavigationFavoritesUploadAccessStatus;

    private final Activity mActivity;
    private final LoggerHandler mLogger;

    public AddressBookHandler(final Activity activity, final LoggerHandler logger, final String contactsDataPath,
            final String navigationFavoritesDataPath) {
        mActivity = activity;
        mLogger = logger;
        mContactsDataPath = contactsDataPath;
        mNavigationFavoritesDataPath = navigationFavoritesDataPath;
    }

    public void onInitialize() {
        initContactsUI();
        initNavigationFavoritesUI();
    }

    private void initContactsUI() {
        mContactsUploadView = mActivity.findViewById(R.id.contacts_uploader);
        mContactsUploadAccessStatus = mContactsUploadView.findViewById(R.id.access_to_contacts);

        View switchItem = mActivity.findViewById(R.id.toggle_contacts_upload);
        ((TextView) switchItem.findViewById(R.id.text)).setText(R.string.permit_upload);
        SwitchCompat contactUploadSwitch = switchItem.findViewById(R.id.drawerSwitch);
        contactUploadSwitch.setChecked(false);

        // sets the listener on the phone connection toggle controller
        contactUploadSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    showPermissionDialogToAccessContacts();
                } else {
                    mContactsUploadAccessStatus.setText(R.string.denied_access_to_contacts);
                    removeAddressBook(sContactsSourceId);
                }
            }
        });
    }

    private void setToggleToUnchecked(int resNum) {
        View switchItem = mActivity.findViewById(resNum);
        SwitchCompat contactUploadSwitch = switchItem.findViewById(R.id.drawerSwitch);
        contactUploadSwitch.setChecked(false);
    }

    private void showPermissionDialogToAccessContacts() {
        final DialogInterface.OnClickListener clickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case DialogInterface.BUTTON_POSITIVE:
                        mContactsUploadAccessStatus.setText(R.string.granted_access_to_contacts);

                        addAddressBook(sContactsSourceId, "PhoneBook", AddressBookType.CONTACT);
                        break;
                    case DialogInterface.BUTTON_NEGATIVE:
                        mContactsUploadAccessStatus.setText(R.string.denied_access_to_contacts);

                        setToggleToUnchecked(R.id.toggle_contacts_upload);
                        break;
                }
            }
        };

        final DialogInterface.OnCancelListener cancelListener = new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                setToggleToUnchecked(R.id.toggle_contacts_upload);
            }
        };

        new AlertDialog.Builder(mActivity)
                .setMessage(R.string.seek_access_to_contacts)
                .setPositiveButton(R.string.confirm, clickListener)
                .setNegativeButton(R.string.deny, clickListener)
                .setOnCancelListener(cancelListener)
                .create()
                .show();
    }

    private void initNavigationFavoritesUI() {
        mNavigationFavoritesUploadView = mActivity.findViewById(R.id.navigation_favorites_uploader);
        mNavigationFavoritesUploadAccessStatus =
                mNavigationFavoritesUploadView.findViewById(R.id.access_to_navigation_favorites);
        // mContactsUploadAccessStatus.setVisibility(View.VISIBLE);

        View switchItem = mActivity.findViewById(R.id.toggle_navigation_favorites_upload);
        ((TextView) switchItem.findViewById(R.id.text)).setText(R.string.permit_upload);
        SwitchCompat navigationFavoritesUploadSwitch = switchItem.findViewById(R.id.drawerSwitch);
        navigationFavoritesUploadSwitch.setChecked(false);

        // sets the listener on the phone connection toggle controller
        navigationFavoritesUploadSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    showPermissionDialogToAccessNavigationFavorites();
                } else {
                    mNavigationFavoritesUploadAccessStatus.setText(R.string.denied_access_to_navigation_favorites);
                    removeAddressBook(sNavigationFavoritesSourceId);
                }
            }
        });
    }

    private void showPermissionDialogToAccessNavigationFavorites() {
        final DialogInterface.OnClickListener clickListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                switch (which) {
                    case DialogInterface.BUTTON_POSITIVE:
                        mNavigationFavoritesUploadAccessStatus.setText(R.string.granted_access_to_navigation_favorites);

                        addAddressBook(sNavigationFavoritesSourceId, "NavigationFavorites", AddressBookType.NAVIGATION);
                        break;
                    case DialogInterface.BUTTON_NEGATIVE:
                        mNavigationFavoritesUploadAccessStatus.setText(R.string.denied_access_to_navigation_favorites);

                        setToggleToUnchecked(R.id.toggle_navigation_favorites_upload);
                        break;
                }
            }
        };

        final DialogInterface.OnCancelListener cancelListener = new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                setToggleToUnchecked(R.id.toggle_navigation_favorites_upload);
            }
        };

        new AlertDialog.Builder(mActivity)
                .setMessage(R.string.seek_access_to_navigation_favorites)
                .setPositiveButton(R.string.confirm, clickListener)
                .setNegativeButton(R.string.deny, clickListener)
                .setOnCancelListener(cancelListener)
                .create()
                .show();
    }

    private boolean parseNameFromContact(String id, JSONObject contact, IAddressBookEntriesFactory factory)
            throws JSONException {
        JSONObject fullname = contact.getJSONObject("name");
        String firstname = fullname.getString("firstName");
        String lastname = fullname.getString("lastName");
        String nickname = fullname.getString("nickName");
        return factory.addName(id, firstname, lastname, nickname);
    }

    private boolean parsePhoneNumbers(String id, JSONObject contact, IAddressBookEntriesFactory factory)
            throws JSONException {
        if (contact.has("phoneNumbers")) {
            JSONArray phoneNumbers = contact.getJSONArray("phoneNumbers");
            for (int j = 0; j < phoneNumbers.length(); ++j) {
                JSONObject phoneNumber = phoneNumbers.getJSONObject(j);

                String label = phoneNumber.getString("label");
                String number = phoneNumber.getString("number");

                if (!factory.addPhone(id, label, number)) {
                    return false;
                }
            }
        }
        return true;
    }

    private boolean parsePostalAddress(String id, JSONObject contact, IAddressBookEntriesFactory factory)
            throws JSONException {
        JSONObject postalAddress = contact.getJSONObject("postalAddress");
        String label = postalAddress.getString("label");
        String addressLine1 = postalAddress.getString("addressLine1");
        String addressLine2 = postalAddress.getString("addressLine2");
        String addressLine3 = postalAddress.getString("addressLine3");
        String city = postalAddress.getString("city");
        String stateOrRegion = postalAddress.getString("stateOrRegion");
        String districtOrCounty = postalAddress.getString("districtOrCounty");
        String postalCode = postalAddress.getString("postalCode");
        String country = postalAddress.getString("country");
        double latitudeInDegrees = postalAddress.getDouble("latitudeInDegrees");
        double longitudeInDegrees = postalAddress.getDouble("longitudeInDegrees");
        double accuracyInMeters = postalAddress.getDouble("accuracyInMeters");
        return factory.addPostalAddress(id, label, addressLine1, addressLine2, addressLine3, city, stateOrRegion,
                districtOrCounty, postalCode, country, latitudeInDegrees, longitudeInDegrees, accuracyInMeters);
    }

    private JSONObject parseFileAsJSONObject(String filePath) throws IOException, JSONException {
        mLogger.postInfo(sTag, String.format("parsing JSON from %s", filePath));
        File file = new File(filePath);
        FileInputStream is = new FileInputStream(file);
        byte[] buffer = new byte[is.available()];
        is.read(buffer);
        String json = new String(buffer, "UTF-8");
        JSONObject obj = new JSONObject(json);
        return obj;
    }

    private boolean parseFileAsContacts(String filename, IAddressBookEntriesFactory factory) {
        try {
            JSONObject json = parseFileAsJSONObject(filename);
            JSONArray contacts = json.getJSONArray("contacts");
            for (int i = 0; i < contacts.length(); ++i) {
                JSONObject contact = contacts.getJSONObject(i);
                String id = contact.getString("id");
                if (!parseNameFromContact(id, contact, factory)) {
                    mLogger.postError(sTag, String.format("parseNameFromContact returned false"));
                    return false;
                }

                if (!parsePhoneNumbers(id, contact, factory)) {
                    mLogger.postError(sTag, String.format("parsePhoneNumbers returned false"));
                    return false;
                }
            }
        } catch (IOException e) {
            mLogger.postError(
                    sTag, String.format("Cannot read %s from assets directory. Error: %s", filename, e.getMessage()));
        } catch (JSONException e) {
            mLogger.postError(sTag, String.format("Cannot create json object. Error: %s", e.getMessage()));
        }

        return true;
    }

    private boolean parseFileAsNavigationFavorites(String filename, IAddressBookEntriesFactory factory) {
        try {
            JSONObject json = parseFileAsJSONObject(filename);
            JSONArray navigationFavorites = json.getJSONArray("navigationFavorites");
            for (int i = 0; i < navigationFavorites.length(); ++i) {
                JSONObject navigationFavorite = navigationFavorites.getJSONObject(i);
                String id = navigationFavorite.getString("id");
                if (!parseNameFromContact(id, navigationFavorite, factory)) {
                    mLogger.postError(sTag, String.format("parseNameFromNavigation returned false"));
                    return false;
                }

                if (!parsePostalAddress(id, navigationFavorite, factory)) {
                    mLogger.postError(sTag, String.format("parsePostalAddress returned false"));
                    return false;
                }
            }
        } catch (IOException e) {
            mLogger.postError(
                    sTag, String.format("Cannot read %s from assets directory. Error: %s", filename, e.getMessage()));
        } catch (JSONException e) {
            mLogger.postError(sTag, String.format("Cannot create json object. Error: %s", e.getMessage()));
        }

        return true;
    }

    @Override
    public boolean getEntries(String contactsSourceId, IAddressBookEntriesFactory factory) {
        boolean success = false;
        if (contactsSourceId.equals(sContactsSourceId)) {
            success = parseFileAsContacts(mContactsDataPath, factory);
        } else if (contactsSourceId.equals(sNavigationFavoritesSourceId)) {
            success = parseFileAsNavigationFavorites(mNavigationFavoritesDataPath, factory);
        }

        mLogger.postInfo(sTag,
                String.format("success status of adding contacts with ID: (%s): (%b)", contactsSourceId, success));
        return success;
    }

    public void removeAllAddressBooks() {
        for (String id : sSourceIds) {
            removeAddressBook(id);
        }
    }
}
