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
package com.amazon.alexa.auto.apis.communication;

import com.amazon.alexa.auto.apis.app.ScopedComponent;

import io.reactivex.rxjava3.core.Observable;

/**
 * API interface to let Alexa App update/remove contacts, set contacts upload permission
 * and observe contacts consent.
 */
public interface ContactsController extends ScopedComponent {
    /**
     * Upload contacts with device address.
     * @param deviceAddress device unique address
     */
    void uploadContacts(String deviceAddress);

    /**
     * Remove contacts with device address.
     * @param deviceAddress device unique address
     */
    void removeContacts(String deviceAddress);

    /**
     * Set contacts upload permission.
     * @param deviceAddress device unique address
     * @param permission contacts permission
     */
    void setContactsUploadPermission(String deviceAddress, String permission);

    /**
     * Observe contacts upload consent.
     * @return contacts consent observable, returning true if contacts consent is needed.
     */
    Observable<Boolean> observeContactsConsent();
}
