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

#include <AACE/JNI/Core/NativeLib.h>

#include <AACE/JNI/AddressBook/IAddressBookEntriesFactoryBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.addressbook.IAddressBookEntriesFactoryBinder";

namespace aace {
namespace jni {
namespace addressbook {

//
// AudioStreamBinder
//

IAddressBookEntriesFactoryBinder::IAddressBookEntriesFactoryBinder(
    std::shared_ptr<aace::addressBook::AddressBook::IAddressBookEntriesFactory> factory) :
        m_factory(factory) {
}

}  // namespace addressbook
}  // namespace jni
}  // namespace aace

#define I_ADDRESS_BOOK_ENTRIES_FACTORY_BINDER(ref) \
    reinterpret_cast<aace::jni::addressbook::IAddressBookEntriesFactoryBinder*>(ref)

extern "C" {
JNIEXPORT void JNICALL
Java_com_amazon_aace_addressbook_IAddressBookEntriesFactory_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto iAddressBookEntriesFactoryBinder = I_ADDRESS_BOOK_ENTRIES_FACTORY_BINDER(ref);
        ThrowIfNull(iAddressBookEntriesFactoryBinder, "invalidIAddressBookEntriesFactoryBinder");

        delete iAddressBookEntriesFactoryBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_addressbook_IAddressBookEntriesFactory_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_addressbook_IAddressBookEntriesFactory_addName(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring entryId,
    jstring firstName,
    jstring lastName,
    jstring nickName) {
    try {
        auto iAddressBookEntriesFactoryBinder = I_ADDRESS_BOOK_ENTRIES_FACTORY_BINDER(ref);
        ThrowIfNull(iAddressBookEntriesFactoryBinder, "invalidIAddressBookEntriesFactoryBinder");

        return static_cast<jboolean>(iAddressBookEntriesFactoryBinder->getIAddressBookEntriesFactory()->addName(
            JString(entryId).toStdStr(),
            JString(firstName).toStdStr(),
            JString(lastName).toStdStr(),
            JString(nickName).toStdStr()));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_addressbook_IAddressBookEntriesFactory_addName", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_addressbook_IAddressBookEntriesFactory_addPhone(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring entryId,
    jstring label,
    jstring number) {
    try {
        auto iAddressBookEntriesFactoryBinder = I_ADDRESS_BOOK_ENTRIES_FACTORY_BINDER(ref);
        ThrowIfNull(iAddressBookEntriesFactoryBinder, "invalidIAddressBookEntriesFactoryBinder");

        return static_cast<jboolean>(iAddressBookEntriesFactoryBinder->getIAddressBookEntriesFactory()->addPhone(
            JString(entryId).toStdStr(), JString(label).toStdStr(), JString(number).toStdStr()));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_addressbook_IAddressBookEntriesFactory_addPhone", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_addressbook_IAddressBookEntriesFactory_addPostalAddress(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring entryId,
    jstring label,
    jstring addressLine1,
    jstring addressLine2,
    jstring addressLine3,
    jstring city,
    jstring stateOrRegion,
    jstring districtOrCounty,
    jstring postalCode,
    jstring country,
    jdouble latitudeInDegrees,
    jdouble longitudeInDegrees,
    jdouble accuracyInMeters)

{
    try {
        auto iAddressBookEntriesFactoryBinder = I_ADDRESS_BOOK_ENTRIES_FACTORY_BINDER(ref);
        ThrowIfNull(iAddressBookEntriesFactoryBinder, "invalidIAddressBookEntriesFactoryBinder");

        return static_cast<jboolean>(
            iAddressBookEntriesFactoryBinder->getIAddressBookEntriesFactory()->addPostalAddress(
                JString(entryId).toStdStr(),
                JString(label).toStdStr(),
                JString(addressLine1).toStdStr(),
                JString(addressLine2).toStdStr(),
                JString(addressLine3).toStdStr(),
                JString(city).toStdStr(),
                JString(stateOrRegion).toStdStr(),
                JString(districtOrCounty).toStdStr(),
                JString(postalCode).toStdStr(),
                JString(country).toStdStr(),
                latitudeInDegrees,
                longitudeInDegrees,
                accuracyInMeters));
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_addressbook_IAddressBookEntriesFactory_addPostalAddress", ex.what());
        return false;
    }
}
}
