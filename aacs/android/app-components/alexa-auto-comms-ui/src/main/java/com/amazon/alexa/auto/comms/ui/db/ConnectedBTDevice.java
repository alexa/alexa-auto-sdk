/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
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
