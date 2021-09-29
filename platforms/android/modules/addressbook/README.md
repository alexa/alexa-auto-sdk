# Address Book Module


The Alexa Auto SDK Address Book module enables a platform implementation to augment the communication and navigation capabilities of Alexa with user data such as phone contacts or navigation favorites ("home", "work", etc.). Using this module, the user can upload their contacts or navigation favorites, or both, so that these capabilities could access them.

> **Note:** To use the Address Book contact and navigation favorites uploading functionality, your product must be placed on the allow list by Amazon. See [Requesting Additional Functionality](../../../../NEED_HELP.md#requesting-additional-functionality) for details.

**Table of Contents**

* [Overview](#overview)
* [Address Book Sequence Diagrams](#sequence-diagrams)
* [Managing Address Books](#managing-address-books)
* [Configuring the Address Book Module](#configuring-the-address-book-module)
* [Using the Address Book Module](#using-the-address-book-module)

## Overview<a id="overview"></a>

The Address Book module provides a common platform interface that you can implement to either upload user data to Alexa or allow the local communication and navigation capabilities (supported by the optional Local Voice Control (LVC) module) to access user data for offline use cases. The user can then use Alexa to call contacts by name, be informed of a caller's name, or get directions to favorite destinations.

Both the [Phone Call Controller](../phonecontrol/README.md) and Alexa-to-Alexa communications implemented via the optional Alexa Communications extension use the phone contacts uploaded via the Address Book module. However, the Contact and Navigation favorites user data are not available for use with any other Alexa devices.

Your platform implementation is responsible for managing the life cycle of an address book, including:

* obtaining consent from end users to allow Alexa to use their data.
* notifying the Alexa Auto SDK Engine when users revoke permission for Alexa to use their data. (This notification ensures that the Engine will remove the user data from Alexa).

> **Important!** Each time an address book becomes unavailable (when the phone is disconnected, for example), your platform implementation must notify the Engine to trigger the deletion of the corresponding address book from Alexa. The platform implementation will notify the Engine to upload the address book when the phone connects again.

### AddressBookType
The AddressBook API defines the type `aace.addressbook.AddressBook.AddressBookType`, which specifies the type of address book to add. The currently supported address book types are:

* `aace.addressbook.AddressBook.AddressBookType.CONTACT` for phone contacts
* `aace.addressbook.AddressBook.AddressBookType.NAVIGATION` for navigation favorites

## Sequence Diagrams<a id = "sequence-diagrams"></a>

The following sequence diagrams provide an overview of how the Address Book module handles uploading and removing contacts and navigation favorites.

### Upload Contacts

This diagram illustrates the sequence for uploading contacts and calling one of the uploaded contacts using voice.

![Contacts Upload](./assets/upload_contacts.png)

### Remove Contacts

This diagram illustrates the sequence for removing uploaded contacts.

![Contacts Remove](./assets/remove_contacts.png)

### Upload Navigation Favorites

This diagram illustrates the sequence for uploading navigation favorites and navigating to one of the uploaded destinations using voice.

![Navigation Upload](./assets/upload_navigation_fav.png)

### Remove Navigation Favorites

This diagram illustrates the sequence for removing uploaded navigation favorites.

![Navigation Remove](./assets/remove_navigation_fav.png)

## Managing Address Books<a id="managing-address-books"></a>

>**Note:** In applications that uses Local Voice Control (LVC) Extension this feature is not supported.

By default, the Auto SDK expects the platform implementation to remove uploaded address books from Alexa when the user disconnects their phone from the head unit. The Auto SDK also removes all address books at Engine start, because it might have failed to remove them previously for various reasons, such as network connection issues. Therefore, when the user connects their phone to the head unit, the platform implementation must notify the Auto SDK to re-upload the address book.

Uploading an address book might consume a significant amount of data. To reduce data usage due to repeated uploads, you can disable automatic address book removal at Engine start by configuring the Address Book module. This configuration reduces the number of address book uploads because the address book need not be uploaded every time the user connects the phone to the head unit.

>**Note:** Regardless of the configuration, Alexa periodically removes uploaded address books.

If you disable address book removal at Engine start, skip re-uploading address books if all of the following conditions are true:

- The last successful upload is less than 24-hour ago. Amazon recommends re-uploading the address books after 24 hours because of Alexa's address book retention policy.
- The user connects the same phone used for the last successful upload.
- The phone contacts and navigation favorites on the phone are the same as the last successful upload.

When the user revokes the permission to upload address books, you must implement the logic to remove uploaded address books immediately. Your implementation must ensure that they are removed successfully.

## Configuring the Address Book Module<a id="configuring-the-address-book-module"></a>

To customize the Address Book module, define the `aace.addressBook` configuration with the following field:

```
"aace.addressBook": {
    "cleanAllAddressBooksAtStart": <true/false>
}
```

The `cleanAllAddressBooksAtStart` field specifies whether to remove all uploaded address books from Alexa at Engine start. The default value is `true`.

You can specify configuration data by using a JSON file. Alternatively, use the `createAddressBookConfig` method to configure the Address Book Module programmatically. See [`com.amazon.aace.addressbook.config.AddressBookConfiguration.createAddressBookConfig`](./src/main/java/com/amazon/aace/addressbook/config/AddressBookConfiguration.java) for details.

```java
// Configure the Engine
EngineConfiguration addressBookConfiguration = AddressBookConfiguration.createAddressBookConfig(false);

mEngine.configure(new EngineConfiguration[]{
	// other config objects,
	addressBookConfiguration,
	...
});
```

Refer to the [core module](../core/README.md) documentation for steps to specify configuration data programmatically or through a JSON file.

## Using the Address Book Module<a id="using-the-address-book-module"></a>

To implement a custom Address Book handler, extend the `AddressBook` class:

```java
public class AddressBookHandler extends AddressBook {
    // The user connected a phone
    addAddressBook (CONTACT_ADDRESS_BOOK_UNIQUE_ID, "MyPhoneBook", AddressBookType.CONTACT);
    ...

    // Ingesting the navigation favorites
    addAddressBook (NAVIGATION_ADDRESS_BOOK_UNIQUE_ID, "MyCarNavFavorites", AddressBookType.NAVIGATION);
    ...

    // Alexa Auto SDK to read the contact entries
    bool getEntries( String addressBookSourceId, IAddressBookEntriesFactory factory ) {

        // For Contacts
        if (addressBookSourceId == CONTACT_ADDRESS_BOOK_UNIQUE_ID) {
            // Using factory, ingest all contacts
            foreach( contact : ContactsBook ) {
                factory.addEntry( 
                    "{
                        \"entryId\": \"UNIQUE_ENTRY_ID\",
                        \"name\": {
                            \"firstName\": \"FirstName\",
                            \"lastName\": \"LastName\",
                            \"nickName\": \"NickName\"        
                        },
                        \"phoneNumbers\": [
                            {
                                \"label\": \"Home\",
                                \"number\": \"1234567890\"
                            },
                            {
                                \"label\": \"Work\",
                                \"number\": \"1231231234\"
                            }
                        ]
                    }"
                );
                ...
            }

            // On successful ingestion return true
            return true;
        }

        //For Navigation
        else if (addressBookSourceId == NAVIGATION_ADDRESS_BOOK_UNIQUE_ID ) {
            // Using factory, ingest all navigation favorites
            foreach( navigation : NavigationBook ) {
                factory.addEntry( 
                    "{
                        \"entryId\": \"UNIQUE_ENTRY_ID\",
                        \"name\": {
                            \"firstName\": \"Home\",        
                        },
                        \"postalAddresses\": [
                            {
                                \"label\": \"Home\",
                                \"addressLine1\": \"123 Address\",
                                \"city\": \"Seattle\",
                                \"stateOrRegion\": \"WA\",          
                                \"postalCode\": \"98109\",
                                \"country\": \"US\",
                                \"latitudeInDegrees\": 47.608,
                                \"longitudeInDegrees\": -122.33,
                                \"accuracyInMeters\": 1
                            }
                        ]
                    }"
                );
                ...
            }
    
            // On successful ingestion return true
            return true;
        }
    }
    ...

    // The user disconnected a phone
    removeAddressBook(CONTACT_ADDRESS_BOOK_UNIQUE_ID);
    ...

    // Navigation Favorites is unavailable
    removeAddressBook(NAVIGATION_ADDRESS_BOOK_UNIQUE_ID);
    ...
}
```

>**Note:** The `label` parameter used for adding phone numbers denotes the phone number type (e.g., Home, Work, Mobile) assigned by the user. If a contact has multiple numbers, Alexa reads out the labels for the user to select the desired number to call. If a number is not associated with a phone number type that Alexa recognizes, Alexa reads out the last four digits of each number.

>**Note:** If a `name` field contains Kanji characters in Japanese, you must also provide the corresponding phonetic field. Without the phonetic field, Alexa may not be able to resolve the name and may respond as if the contact or navigation favorite was not available. For information about how your application provides the phonetic fields, see [IAddressBookEntriesFactory.java](./src/main/java/com/amazon/aace/addressbook/IAddressBookEntriesFactory.java).

