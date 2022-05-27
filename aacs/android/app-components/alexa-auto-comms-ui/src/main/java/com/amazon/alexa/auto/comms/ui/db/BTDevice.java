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

import com.amazon.alexa.auto.comms.ui.Constants;

import org.jetbrains.annotations.NotNull;

import java.io.Serializable;

@Entity
public class BTDevice implements Serializable {
    @NonNull
    @PrimaryKey
    public String deviceAddress;
    private String deviceName;
    private String contactsUploadPermission;
    private Boolean firstPair;

    public BTDevice() {
        deviceAddress = "";
        deviceName = "";
        contactsUploadPermission = "NO";
        firstPair = false;
    }

    @NotNull
    public String getDeviceAddress() {
        return deviceAddress;
    }

    public void setDeviceAddress(@NotNull String deviceAddress) {
        this.deviceAddress = deviceAddress;
    }

    public Boolean getFirstPair() {
        return firstPair;
    }

    public void setFirstPair(Boolean firstPair) {
        this.firstPair = firstPair;
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
