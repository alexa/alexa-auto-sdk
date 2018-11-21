package com.amazon.sampleapp.impl.ContactIngestion.ContactProvider;

import android.app.Activity;
import com.amazon.sampleapp.impl.ContactIngestion.ContactPojos.Contact;
import com.amazon.sampleapp.impl.ContactIngestion.ContactPojos.AddressDetails;
import com.amazon.sampleapp.impl.ContactIngestion.ContactPojos.ContactSourceType;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.*;

/**
 * An implementation of a contact provider.
 */
public class FileContactProviderHandler implements ContactProvider {

    private static final String sTag = FileContactProviderHandler.class.getSimpleName();
    private static final String CONTACT_STORE_FILE_NAME = "ContactStore.txt";
    private static final String DELIMITER = ",";
    private static final String EMPTY_STRING = "";

    private final Activity mActivity;
    private final LoggerHandler mLogger;
    private BufferedReader fileReader;

    FileContactProviderHandler(final Activity activity, final LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
    }

    /**
     * Method to retrieve all contacts irrespective of {@link ContactSourceType}.
     * @return a {@link List} of {@link Contact}s.
     * @throws Exception if any exception occurs.
     */
    @Override
    public List<Contact> getAllContacts() throws Exception {
        fileReader = new BufferedReader(new InputStreamReader(mActivity.getAssets().open(CONTACT_STORE_FILE_NAME)));
        final Map<String, Contact> fullNameWithContactMap = new HashMap<>();
        String contactLineInFile = fileReader.readLine();
        while(contactLineInFile != null) {
            // format of each line
            // firstName, lastName, nickName, company, addressType, value, label
            final List<String> contactInfos = Arrays.asList(contactLineInFile.split(DELIMITER));
            if (contactInfos.size() == 7 || contactInfos.size() == 6) {
                final String firstName = contactInfos.get(0).trim();
                final String lastName = contactInfos.get(1).trim();
                final String nickName = contactInfos.get(2).trim();
                final String company = contactInfos.get(3).trim();
                final String addressType = contactInfos.get(4).trim();
                final String value = contactInfos.get(5).trim();
                final String label = (contactInfos.size() == 7) ? contactInfos.get(6).trim() : EMPTY_STRING;

                final AddressDetails addressDetails = new AddressDetails(addressType, value, label);
                final String fullName = firstName + "-" + lastName;

                if (fullNameWithContactMap.containsKey(fullName)) {
                    fullNameWithContactMap.get(fullName).addAddressDetails(addressDetails);
                } else {
                    final Contact contact = new Contact(firstName, lastName, nickName, company, addressDetails);
                    fullNameWithContactMap.put(fullName, contact);
                }
            } else {
                mLogger.postError(sTag, "This line { " + contactLineInFile + "} in contact store is not in proper format. The format should be {firstName, lastName, nickName, company, addressType, value, label}.");
            }
            contactLineInFile = fileReader.readLine();
        }
        return new ArrayList<>(fullNameWithContactMap.values());
    }

    /**
     * Method to retrieve all contacts based on a {@link ContactSourceType}.
     * @param contactSourceType the type of the contact source.
     * @return a {@link List} of {@link Contact}s.
     * @throws Exception if any exception occurs.
     */
    @Override
    public List<Contact> getContactsByType(final ContactSourceType contactSourceType) throws Exception {
        final List<Contact> contactList = getAllContacts();
        final List<Contact> contactsWithSpecificType = new ArrayList<>();
        for (final Contact contact: contactList) {
            final List<AddressDetails> addressDetailsList = contact.getAddressDetails();
            for (AddressDetails addressDetails : addressDetailsList) {
                if (addressDetails.getContactSourceType() != null &&
                        addressDetails.getContactSourceType().equals(contactSourceType)) {
                    contactsWithSpecificType.add(contact);
                }
            }
        }
        return contactsWithSpecificType;
    }
}
