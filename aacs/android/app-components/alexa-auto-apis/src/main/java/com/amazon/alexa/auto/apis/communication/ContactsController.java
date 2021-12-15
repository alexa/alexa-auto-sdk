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
