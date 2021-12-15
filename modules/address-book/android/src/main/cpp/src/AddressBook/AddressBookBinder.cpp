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

#include <AACE/JNI/AddressBook/AddressBookBinder.h>
#include <AACE/JNI/AddressBook/IAddressBookEntriesFactoryBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.addressbook.AddressBookBinder";

// type aliases
using AddressBookType = aace::jni::addressbook::AddressBookHandler::AddressBookType;

namespace aace {
namespace jni {
namespace addressbook {

//
// AddressBookBinder
//

AddressBookBinder::AddressBookBinder(jobject obj) {
    m_addressBookHandler = std::make_shared<AddressBookHandler>(obj);
}

//
// AddressBookHandler
//

AddressBookHandler::AddressBookHandler(jobject obj) :
        AddressBook(), m_obj(obj, "com/amazon/aace/addressbook/AddressBook") {
}

bool AddressBookHandler::getEntries(
    const std::string& addressBookSourceId,
    std::weak_ptr<IAddressBookEntriesFactory> factory) {
    try_with_context {
        JObject javaIAddressBookEntriesFactory("com/amazon/aace/addressbook/IAddressBookEntriesFactory");
        ThrowIfJavaEx(env, "createIAddressBookEntriesFactoryFailed");

        // create the IAddressBookEntriesFactory binder
        auto factorySharedRef = factory.lock();
        if (factorySharedRef) {
            long iAddressBookEntriesFactoryBinder =
                reinterpret_cast<long>(new IAddressBookEntriesFactoryBinder(factorySharedRef));

            // set the java audio stream object native ref to the audio stream binder
            javaIAddressBookEntriesFactory.invoke<void>(
                "setNativeRef", "(J)V", nullptr, iAddressBookEntriesFactoryBinder);
        } else {
            Throw("invalidFactory");
        }

        jboolean result;
        ThrowIfNot(
            m_obj.invoke(
                "getEntries",
                "(Ljava/lang/String;Lcom/amazon/aace/addressbook/IAddressBookEntriesFactory;)Z",
                &result,
                JString(addressBookSourceId).get(),
                javaIAddressBookEntriesFactory.get()),
            "invokeFailed");
        return result;
    }
    catch_with_ex {
        AACE_JNI_ERROR(TAG, "getEntries", ex.what());
        return false;
    }
}

}  // namespace addressbook
}  // namespace jni
}  // namespace aace

#define ADDRESS_BOOK_BINDER(ref) reinterpret_cast<aace::jni::addressbook::AddressBookBinder*>(ref)

extern "C" {
JNIEXPORT jlong JNICALL Java_com_amazon_aace_addressbook_AddressBook_createBinder(JNIEnv* env, jobject obj) {
    return reinterpret_cast<long>(new aace::jni::addressbook::AddressBookBinder(obj));
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_addressbook_AddressBook_disposeBinder(JNIEnv* env, jobject /* this */, jlong ref) {
    try {
        auto addressBookBinder = ADDRESS_BOOK_BINDER(ref);
        ThrowIfNull(addressBookBinder, "invalidAddressBookBinder");

        delete addressBookBinder;
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_addressbook_AddressBook_disposeBinder", ex.what());
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_addressbook_AddressBook_addAddressBook(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring addressBookSourceId,
    jstring name,
    jobject type) {
    try {
        auto addressBookBinder = ADDRESS_BOOK_BINDER(ref);
        ThrowIfNull(addressBookBinder, "invalidAddressBookBinder");

        AddressBookType addressBookType;
        ThrowIfNot(
            aace::jni::addressbook::JAddressBookType::checkType(type, &addressBookType), "invalidAddressBookType");

        return addressBookBinder->getAddressBook()->addAddressBook(
            JString(addressBookSourceId).toStdStr(), JString(name).toStdStr(), addressBookType);
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_addressbook_AddressBook_addAddressBook", ex.what());
        return false;
    }
}

JNIEXPORT jboolean JNICALL Java_com_amazon_aace_addressbook_AddressBook_removeAddressBook(
    JNIEnv* env,
    jobject /* this */,
    jlong ref,
    jstring addressBookSourceId) {
    try {
        auto addressBookBinder = ADDRESS_BOOK_BINDER(ref);
        ThrowIfNull(addressBookBinder, "invalidAddressBookBinder");

        return addressBookBinder->getAddressBook()->removeAddressBook(JString(addressBookSourceId).toStdStr());
    } catch (const std::exception& ex) {
        AACE_JNI_ERROR(TAG, "Java_com_amazon_aace_addressbook_AddressBook_removeAddressBook", ex.what());
        return false;
    }
}
}
