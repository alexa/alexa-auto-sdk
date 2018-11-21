package com.amazon.sampleapp.impl.ContactIngestion.ContactPojos;

import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * The POJO class defining the detailing of the contact information.
 */
public class AddressDetails {

    private static final String EMPTY_STRING = "";

    private String mLabel;
    private String mContactValue;
    private ContactSourceType mContactSourceType;

    public AddressDetails(@NonNull final String contactSourceType,
                          @NonNull final String contactValue,
                          @Nullable final String label) {
        mContactSourceType = ContactSourceType.valueOf(contactSourceType.toUpperCase());
        mContactValue = contactValue;
        mLabel = label != null ? label : EMPTY_STRING;
    }

    public String getContactValue() {
        return mContactValue;
    }

    public ContactSourceType getContactSourceType() {
        return mContactSourceType;
    }

    public String getLabel() {
        return mLabel;
    }

    public void setLabel(String label) {
        this.mLabel = label;
    }

    public JSONObject toJsonObject() throws JSONException {
        final JSONObject contactDetailsJsonObject = new JSONObject();
        contactDetailsJsonObject.put("type", mContactSourceType.toString());
        contactDetailsJsonObject.put("value", mContactValue);
        if (!EMPTY_STRING.equals(mLabel)) {
            contactDetailsJsonObject.put("label", mLabel);
        }
        return contactDetailsJsonObject;
    }
}
