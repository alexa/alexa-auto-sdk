package com.amazon.alexa.auto.comms.ui;

public class Constants {
    // State
    public static final String BT_CONNECTED = "CONNECTED";
    public static final String BT_DISCONNECTED = "DISCONNECTED";
    public static final String CONTACTS_PERMISSION_YES = "YES";
    public static final String CONTACTS_PERMISSION_NO = "NO";

    // AACS Bluetooth intents
    public final static String AACS_BT_DEVICE_ADDRESS = "deviceAddress";
    public final static String AACS_BT_DEVICE_NAME = "deviceName";
    public static final String AACS_TELEPHONY_SERVICE = "com.amazon.aacstelephony";
    public static final String AACS_BT_CONNECTED = "com.amazon.aacstelephony.bluetooth.connected";
    public static final String AACS_BT_DISCONNECTED = "com.amazon.aacstelephony.bluetooth.disconnected";
    public static final String AACS_BT_CONNECTION_CHECK_COMPLETED = "com.amazon.aacstelephony.bluetooth.connectionCheckCompleted";

    // AACS Contacts intents
    public final static String AACS_CONTACTS_SERVICE = "com.amazon.aacscontacts.AACSContactsService";
    public static final String AACS_ACTION_UPLOAD_CONTACTS = "com.amazon.aacscontacts.upload";
    public static final String AACS_ACTION_REMOVE_CONTACTS = "com.amazon.aacscontacts.remove";
    public static final String AACS_EXTRA_ADDRESSBOOK_ID = "addressBookSourceId";
    public static final String AACS_EXTRA_ADDRESSBOOK_NAME_KEY = "addressBookName";
    public static final String AACS_EXTRA_ADDRESSBOOK_NAME_VALUE = "PhoneBook";

    // Alexa Auto Comms intents
    public final static String ALEXA_AUTO_COMMS = "com.amazon.alexa.auto.comms";
    public final static String ALEXA_AUTO_COMMS_PRIMARY_PHONE_CHANGED = "com.amazon.alexa.auto.comms.primaryPhoneChanged";

    // Communication
    public static final String COMMUNICATION_PERMISSION_ENABLED = "enabled";
    public static final String COMMUNICATION_PERMISSION_DISABLED = "disabled";
    public static final String COMMUNICATION_DEVICE_ADDRESS = "deviceAddress";
}
