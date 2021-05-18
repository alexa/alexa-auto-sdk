package com.amazon.sampleapp.impl.AddressBook;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Environment;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import androidx.appcompat.widget.SwitchCompat;

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

/**
 * The handler class which handles contact upload, cancel and delete.
 */
public class AddressBookHandler extends AddressBook {
    private static final String sTag = "AddressBookHandler";

    private static final String sContactsSourceId = "0001";
    private static final String sNavigationFavoritesSourceId = "0002";
    private static final String[] sSourceIds = {sContactsSourceId, sNavigationFavoritesSourceId};

    public static final String CONTACTS_FILE_NAME = "Contacts.json";
    public static final String NAVIGATION_FAVORITES_FILE_NAME = "NavigationFavorites.json";

    private View mContactsUploadView;
    private TextView mContactsUploadAccessStatus;

    private View mNavigationFavoritesUploadView;
    private TextView mNavigationFavoritesUploadAccessStatus;

    private final Activity mActivity;
    private final LoggerHandler mLogger;

    private final File mSampleDataDir;

    public AddressBookHandler(final Activity activity, final LoggerHandler logger, final File sampleDataDir) {
        mActivity = activity;
        mLogger = logger;
        mSampleDataDir = sampleDataDir;
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
                JSONObject payload = new JSONObject();

                String id = contact.getString("id");
                if (id.isEmpty()) {
                    mLogger.postError(sTag, String.format("contactsIdEmpty"));
                    return false;
                }
                payload.put("entryId", id);
                payload.put("name", contact.getJSONObject("name"));

                if (contact.has("phoneNumbers")) {
                    payload.put("phoneNumbers", contact.getJSONArray("phoneNumbers"));
                }

                factory.addEntry(payload.toString());
            }
            return true;
        } catch (IOException e) {
            mLogger.postError(
                    sTag, String.format("Cannot read %s from assets directory. Error: %s", filename, e.getMessage()));
        } catch (JSONException e) {
            mLogger.postError(sTag, String.format("Cannot create json object. Error: %s", e.getMessage()));
        }

        return false;
    }

    private boolean parseFileAsNavigationFavorites(String filename, IAddressBookEntriesFactory factory) {
        try {
            JSONObject json = parseFileAsJSONObject(filename);
            JSONArray navigationFavorites = json.getJSONArray("navigationFavorites");
            for (int i = 0; i < navigationFavorites.length(); ++i) {
                JSONObject navigationFavorite = navigationFavorites.getJSONObject(i);
                JSONObject payload = new JSONObject();

                String id = navigationFavorite.getString("id");
                if (id.isEmpty()) {
                    mLogger.postError(sTag, String.format("navigationFavoriteIdEmpty"));
                    return false;
                }

                payload.put("entryId", id);
                payload.put("name", navigationFavorite.getJSONObject("name"));

                if (navigationFavorite.has("postalAddress")) {
                    JSONArray postalAddresses = new JSONArray();
                    postalAddresses.put(navigationFavorite.getJSONObject("postalAddress"));
                    payload.put("postalAddresses", postalAddresses);
                }
                factory.addEntry(payload.toString());
            }
            return true;
        } catch (IOException e) {
            mLogger.postError(
                    sTag, String.format("Cannot read %s from assets directory. Error: %s", filename, e.getMessage()));
        } catch (JSONException e) {
            mLogger.postError(sTag, String.format("Cannot create json object. Error: %s", e.getMessage()));
        }

        return false;
    }

    @Override
    public boolean getEntries(String contactsSourceId, IAddressBookEntriesFactory factory) {
        boolean success = false;

        if (contactsSourceId.equals(sContactsSourceId)) {
            // Always use sample data from external storage if available
            File contactsFile = new File(Environment.getExternalStorageDirectory(), CONTACTS_FILE_NAME);
            if (!contactsFile.exists()) {
                // Use the default from the assets folder
                contactsFile = new File(mSampleDataDir, CONTACTS_FILE_NAME);
            }

            success = parseFileAsContacts(contactsFile.getPath(), factory);
        } else if (contactsSourceId.equals(sNavigationFavoritesSourceId)) {
            // Always use sample data from external storage if available
            File navigationFavoritesFile =
                    new File(Environment.getExternalStorageDirectory(), NAVIGATION_FAVORITES_FILE_NAME);
            if (!navigationFavoritesFile.exists()) {
                // Use the default from the assets folder
                navigationFavoritesFile = new File(mSampleDataDir, NAVIGATION_FAVORITES_FILE_NAME);
            }

            success = parseFileAsNavigationFavorites(navigationFavoritesFile.getPath(), factory);
        }

        mLogger.postInfo(sTag,
                String.format(
                        "success status of adding contacts/navigation with ID: (%s): (%b)", contactsSourceId, success));
        return success;
    }

    public void removeAllAddressBooks() {
        for (String id : sSourceIds) {
            removeAddressBook(id);
        }
    }
}
