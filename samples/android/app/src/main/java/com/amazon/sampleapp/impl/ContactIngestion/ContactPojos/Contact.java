package com.amazon.sampleapp.impl.ContactIngestion.ContactPojos;

import android.support.annotation.Nullable;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

/**
 * The POJO class of a Contact.
 */
public class Contact {

    private static final String EMPTY_STRING = "";

    private String mFirstName;
    private String mNickName;
    private String mLastName;
    private String mCompany;
    private List<AddressDetails> mAddressDetails;

    public Contact(@Nullable final String firstName,
                   @Nullable final String lastName,
                   @Nullable final String nickName,
                   @Nullable final String company,
                   @Nullable final AddressDetails addressDetails) {
        mFirstName = firstName != null ? firstName : EMPTY_STRING;
        mLastName = lastName != null ? lastName : EMPTY_STRING;
        mNickName = nickName != null ? nickName : EMPTY_STRING;
        mCompany = company != null ? company : EMPTY_STRING;
        mAddressDetails = addressDetails != null ? new ArrayList<>(Arrays.asList(addressDetails)) : new ArrayList<AddressDetails>();
    }

    public String getFirstName() {
        return mFirstName;
    }

    public String getLastName() {
        return mLastName;
    }

    public String getNickName() {
        return mNickName;
    }

    public String getCompany() {
        return mCompany;
    }

    public List<AddressDetails> getAddressDetails() {
        return mAddressDetails;
    }

    public void setCompany(final String company) {
        mCompany = company;
    }

    public void addAddressDetails(final List<AddressDetails> addressDetails) {
        mAddressDetails.addAll(addressDetails);
    }

    public void addAddressDetails(final AddressDetails addressDetails) {
        mAddressDetails.add(addressDetails);
    }

    public void setFirstName(String firstName) {
        mFirstName = firstName;
    }

    public void setLastName(String lastName) {
        mLastName = lastName;
    }

    public void setNickName(String nickName) {
        mNickName = nickName;
    }

    public String toJsonString() throws JSONException {
        final JSONObject contactJsonObject = new JSONObject();
        contactJsonObject.put("id", UUID.randomUUID().toString());
        if (isNonEmptyString(mFirstName)) {
            contactJsonObject.put("firstName", mFirstName);
        }
        if (isNonEmptyString(mLastName)) {
            contactJsonObject.put("lastName", mLastName);
        }
        if (isNonEmptyString(mNickName)) {
            contactJsonObject.put("nickName", mNickName);
        }
        if (isNonEmptyString(mCompany)) {
            contactJsonObject.put("company", mCompany);
        }
        JSONArray addresses = new JSONArray();
        for (AddressDetails contactDetail : mAddressDetails) {
            addresses.put(contactDetail.toJsonObject());
        }
        contactJsonObject.put("addresses", addresses);
        return contactJsonObject.toString();
    }

    private boolean isNonEmptyString(final String string) {
        return !EMPTY_STRING.equals(string) ;
    }
}
