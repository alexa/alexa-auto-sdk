# Contact Uploader API

## Overview

The Contact Uploader API enables the OEMs to upload contacts from a paired connected phone to the Alexa cloud. Alexa Communications can use the uploaded contacts to make calls, announce the caller's name, send messages and more. These contacts are not available for use with any other Alexa devices.

## Handling Contact Uploader API

It is the platform implementation's responsibility to monitor phone connectivity with the head unit and call the Contact Uploader API. The Contact Uploader Engine handles the platform implementation for uploading and removing contacts from the Alexa cloud.

It is the responsibility of the platform implementation to prompt users for consent to upload the contacts from a connected phone.

> ***Important!*** Contact uploading and removal should be a separate process from the user interface thread.

Extend the `contactUploader` class to implement a custom contact uploader handler.

```
public class ContactUploaderHandler extends ContactUploader {

    // Begin contact upload on phone connect.
    addContactsBegin();

    // Include details for each contact being added.
    addContact( contact );
    ...

    // End contact upload after last contact is uploaded.
    addContactsEnd();
    ...

    // Remove all uploaded contacts from Alexa cloud when phone is no longer connected.
    removeUploadedContacts();
    ...

    // Cancel contact upload if phone is disconnected in the middle of uploading contacts.
    addContactsCancel();
    ...

    // Provide contact upload status change notifications to the platform.
    @Override
    public void contactsUploaderStatusChanged(ContactUploadStatus contactUploadStatus, String info) {
        // Handle the contact uploader status
        ...
    }
    ...
}

```
