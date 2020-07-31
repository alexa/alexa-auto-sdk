/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.addressbook;

import com.amazon.aace.core.PlatformInterface;

/**
 *
 */
abstract public class AddressBook extends PlatformInterface {
    /**
     * Describes the address book type.
     * @sa @c aace::addressBook::AddressBook::AddressBookType
     */
    public enum AddressBookType {
        /**
         * Contacts
         */
        CONTACT("CONTACT"),

        /**
         * Navigation
         */
        NAVIGATION("NAVIGATION");

        /**
         * @internal
         */
        private String addressBookTypeName;

        AddressBookType(String addressBookTypeName) {
            this.addressBookTypeName = addressBookTypeName;
        }

        @Override
        public String toString() {
            return addressBookTypeName;
        }
    }

    public AddressBook() {}

    /**
     * Notifies the engine on an availability of an address book
     *
     * @param [in] addressBookSourceId A unique identifier for an address book.
     * @param [in] name Friendly name of the address book, or an empty string if not available.
     * @param [in] type Type of the address book @c AddressBookType.
     * @return @c false if address book was already added or some internal error otherwise @c true on successful.
     */
    final public boolean addAddressBook(String addressBookSourceId, String name, AddressBookType type) {
        return addAddressBook(getNativeRef(), addressBookSourceId, name, type);
    }

    /**
     * Notifies the engine on a non-availability of an already available address book.
     *
     * @param [in] addressBookSourceId A unique identifier for an address book.
     * @return @c false if address book does not already added or some internal error otherwise @c true on successful.
     */
    final public boolean removeAddressBook(String addressBookSourceId) {
        return removeAddressBook(getNativeRef(), addressBookSourceId);
    }

    /**
     * Notifies the platform implementation to ingest address book entries using factory methods. Please
     * note that engine may call this function multiple times to get the entries of the same address book.
     *
     * @param [in] addressBookSourceId A unique identifier for an address book added using @c addAddressBook
     * @param [in] factory Factory class for ingesting address book entries.
     * @return Return @c true if platform implementation has successfully ingested all the entries of the address book
     * otherwise @c false.
     */
    public boolean getEntries(String addressBookSourceId, IAddressBookEntriesFactory factory) {
        return false;
    }

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native boolean addAddressBook(
            long nativeRef, String addressBookSourceId, String name, AddressBookType type);
    private native boolean removeAddressBook(long nativeRef, String addressBookSourceId);
}
