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

import com.amazon.aace.core.NativeRef;

final public class IAddressBookEntriesFactory extends NativeRef {
    /**
     * Add name.
     *
     * @param [in] entryId A unique identifier of entry in an address book.
     * @param [in] name Name of the entry, or an empty string if not available.
     * @return @c true on successful or @c false when name for the id was already added or if entryId is empty.
     */
    final public boolean addName(String entryId, String name) {
        return addName(getNativeRef(), entryId, name, "", "");
    }

    /**
     * Add first and last name.
     *
     * @param [in] entryId A unique identifier of entry in an address book.
     * @param [in] firstName First name of the entry, or an empty string if not available.
     * @param [in] lastName Last name of the entry, or an empty string if not available.
     * @return @c true on successful or @c false when name for the id was already added or if entryId is empty.
     */
    final public boolean addName(String entryId, String firstName, String lastName) {
        return addName(getNativeRef(), entryId, firstName, lastName, "");
    }

    /**
     * Add first, last and nick name.
     *
     * @param [in] entryId A unique identifier of entry in an address book.
     * @param [in] firstName First name of the entry, or an empty string if not available.
     * @param [in] lastName Last name of the entry, or an empty string if not available.
     * @param [in] nickName Nick name of the entry, or an empty string if not available.
     * @return @c true on successful or @c false when name for the id was already added or if entryId is empty.
     */
    final public boolean addName(String entryId, String firstName, String lastName, String nickName) {
        return addName(getNativeRef(), entryId, firstName, lastName, nickName);
    }

    /**
     * Add phone number(s).
     *
     * @param [in] entryId A unique identifier of entry in an address book.
     * @param [in] label Alphanumeric phone label (Example: HOME, MOBILE), or an empty string if not available.
     * @param [in] number Numeric phone number, or an empty string if not available.
     * @return @c true on successful or @c false when reached the max allowed per entryId or if entryId is empty.
     */
    final public boolean addPhone(String entryId, String label, String number) {
        return addPhone(getNativeRef(), entryId, label, number);
    }

    /**
     * Add postal address(es).
     *
     * @param [in] entryId A unique identifier of entry in an address book.
     * @param [in] label Alphanumeric postal address label (Example: HOME, WORK), or an empty string if not available.
     * @param [in] addressLine1 First line of the postal address, or an empty string if not available.
     * @param [in] addressLine2 Second line of the postal address, or an empty string if not available.
     * @param [in] addressLine3 Third line of the postal address, or an empty string if not available.
     * @param [in] city City name, or an empty string if not available.
     * @param [in] stateOrRegion State or Region name, or an empty string if not available.
     * @param [in] districtOrCounty District or County name, or an empty string if not available.
     * @param [in] postalCode Postal code or Zip code, or an empty string if not available.
     * @param [in] country Country name, or an empty string if not available.
     * @param [in] latitudeInDegrees Geo latitude in degrees.
     * @param [in] longitudeInDegrees Geo longitute in degrees.
     * @param [in] accuracyInMeters Accuracy in meters, or zero if not available.
     * @return @c true on successful or @c false when reached the max allowed per id or if entryId is empty.
     */
    final public boolean addPostalAddress(String entryId, String label, String addressLine1, String addressLine2,
            String addressLine3, String city, String stateOrRegion, String districtOrCounty, String postalCode,
            String country, double latitudeInDegrees, double longitudeInDegrees, double accuracyInMeters) {
        return addPostalAddress(getNativeRef(), entryId, label, addressLine1, addressLine2, addressLine3, city,
                stateOrRegion, districtOrCounty, postalCode, country, latitudeInDegrees, longitudeInDegrees,
                accuracyInMeters);
    }

    protected long createNativeRef() {
        return 0;
    }

    protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native void disposeBinder(long nativeRef);
    private native boolean addName(
            long nativeObject, String entryId, String firstName, String lastName, String nickName);
    private native boolean addPhone(long nativeObject, String entryId, String label, String number);
    private native boolean addPostalAddress(long nativeObject, String entryId, String label, String addressLine1,
            String addressLine2, String addressLine3, String city, String stateOrRegion, String districtOrCounty,
            String postalCode, String country, double latitudeInDegrees, double longitudeInDegrees,
            double accuracyInMeters);
}
