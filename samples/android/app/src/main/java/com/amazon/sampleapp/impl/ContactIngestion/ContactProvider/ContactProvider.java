package com.amazon.sampleapp.impl.ContactIngestion.ContactProvider;

import com.amazon.sampleapp.impl.ContactIngestion.ContactPojos.Contact;
import com.amazon.sampleapp.impl.ContactIngestion.ContactPojos.ContactSourceType;

import java.util.List;

/**
 * An interface defining the methods to retrieve the contacts.
 */
public interface ContactProvider {

    /**
     * Method to retrieve all contacts irrespective of {@link ContactSourceType}.
     * @return a {@link List} of {@link Contact}s.
     * @throws Exception if any exception occurs.
     */
    List<Contact> getAllContacts() throws Exception;

    /**
     * Method to retrieve all contacts based on a {@link ContactSourceType}.
     * @param contactSourceType the type of the contact source.
     * @return a {@link List} of {@link Contact}s.
     * @throws Exception if any exception occurs.
     */
    List<Contact> getContactsByType(ContactSourceType contactSourceType) throws Exception;
}
