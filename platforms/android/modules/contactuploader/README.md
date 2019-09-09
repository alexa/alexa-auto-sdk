# Contact Uploader Module

The Alexa Auto SDK Contact Uploader module provides a platform interface that you can implement to upload contacts from a paired connected phone to the Alexa cloud. 

**Table of Contents**

* [Overview](#overview)
* [Implementing a Contact Uploader Handler](#using-the-contact-uploader)

## Overview <a id ="overview"></a>

Alexa Communications can use contacts uploaded via the Contact Uploader module to make calls and announce the caller's name. These contacts are not available for use with any other Alexa devices.

> **Note**: The Contact Uploader module supports only phone contacts and only online use cases. It is on the deprecation path beginning with Alexa Auto SDK release 2.0.0 and is being superseded by the [Address Book module](../addressbook/README.md). All new implementations should use the Address Book module instead of the Contact Uploader module.

The platform implementation is responsible for managing the life cycle of a contacts list. This includes:

* obtaining consent from end users to allow Alexa to use their data.
* notifying the Alexa Auto SDK when end users revoke permission for Alexa to use their data (this notification ensures that the Alexa Auto SDK engine will remove the user data from the Alexa cloud).

> **Important!** Contact uploading and removal should be a separate process from the user interface process.

## Implementing a Contact Uploader Handler<a id ="using-the-contact-uploader"></a>

Extend the `ContactUploader` class to implement a custom contact uploader handler.

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
