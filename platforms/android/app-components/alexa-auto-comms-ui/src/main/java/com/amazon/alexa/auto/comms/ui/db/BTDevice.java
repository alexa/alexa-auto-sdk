package com.amazon.alexa.auto.comms.ui.db;

import androidx.annotation.NonNull;
import androidx.room.Entity;
import androidx.room.PrimaryKey;

import org.jetbrains.annotations.NotNull;

import java.io.Serializable;

@Entity
public class BTDevice implements Serializable {
    @NonNull
    @PrimaryKey
    public String deviceAddress;
    private String deviceName;
    private String contactsUploadPermission;

    public BTDevice() {
        deviceAddress = "";
        deviceName = "";
        contactsUploadPermission = "NO";
    }

    @NotNull
    public String getDeviceAddress() {
        return deviceAddress;
    }

    public void setDeviceAddress(@NotNull String deviceAddress) {
        this.deviceAddress = deviceAddress;
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
