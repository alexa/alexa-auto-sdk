package com.amazon.alexa.auto.comms.ui.db;

import androidx.annotation.NonNull;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

import java.io.Serializable;

@Entity
public class ConnectedBTDevice implements Serializable {
    @NonNull
    @PrimaryKey(autoGenerate = true)
    private int id;
    private String deviceAddress;
    private String deviceName;
    private String contactsUploadPermission;

    public ConnectedBTDevice() {
        deviceAddress = "";
        deviceName = "";
        contactsUploadPermission = "NO";
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public void setDeviceAddress(String deviceAddress) {
        this.deviceAddress = deviceAddress;
    }

    public String getDeviceAddress() {
        return deviceAddress;
    }

    public String getDeviceName() {
        return deviceName;
    }

    public void setDeviceName(String deviceName) {
        this.deviceName = deviceName;
    }

    public String getContactsUploadPermission() {
        return contactsUploadPermission;
    }

    public void setContactsUploadPermission(String contactsUploadPermission) {
        this.contactsUploadPermission = contactsUploadPermission;
    }
}
