/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <AACE/JNI/ContactUploader/ContactUploaderBinder.h>

// String to identify log entries originating from this file.
static const char TAG[] = "aace.jni.contactuploader.ContactUploaderBinder";

// type aliases
using ContactUploaderStatus = aace::jni::contactuploader::ContactUploaderHandler::ContactUploaderStatus;

namespace aace {
namespace jni {
namespace contactuploader {

    //
    // ContactUploaderBinder
    //

    ContactUploaderBinder::ContactUploaderBinder( jobject obj ) {
        m_contactUploaderHandler = std::make_shared<ContactUploaderHandler>( obj );
    }

    //
    // ContactUploaderHandler
    //

    ContactUploaderHandler::ContactUploaderHandler( jobject obj ) : ContactUploader(), m_obj( obj, "com/amazon/aace/contactuploader/ContactUploader" ) {
    }

    void ContactUploaderHandler::contactsUploaderStatusChanged( ContactUploaderStatus status, const std::string& info )
    {
        try_with_context
        {
            jobject statusObj;

            ThrowIfNot( JContactUploaderStatus::checkType( status, &statusObj ), "invalidStatus" );
            ThrowIfNot( m_obj.invoke<void>( "contactsUploaderStatusChanged", "(Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploaderStatus;Ljava/lang/String;)V", nullptr, statusObj, JString(info).get() ), "invokeFailed" );
        }
        catch_with_ex {
            AACE_JNI_ERROR(TAG,"contactsUploaderStatusChanged",ex.what());
        }
    }

} // aace::jni::contactuploader
} // aace::jni
} // aace

#define CONTACT_UPLOADER_BINDER(ref) reinterpret_cast<aace::jni::contactuploader::ContactUploaderBinder *>( ref )

extern "C"
{
    JNIEXPORT jlong JNICALL
    Java_com_amazon_aace_contactuploader_ContactUploader_createBinder( JNIEnv* env, jobject obj )  {
        return reinterpret_cast<long>( new aace::jni::contactuploader::ContactUploaderBinder( obj ) );
    }

    JNIEXPORT void JNICALL
    Java_com_amazon_aace_contactuploader_ContactUploader_disposeBinder( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto contactUploaderBinder = CONTACT_UPLOADER_BINDER(ref);
            ThrowIfNull( contactUploaderBinder, "invalidContactUploaderBinder" );

            delete contactUploaderBinder;
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_contactuploader_ContactUploader_disposeBinder",ex.what());
        }
    }
    
    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_contactuploader_ContactUploader_addContactsBegin( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto contactUploaderBinder = CONTACT_UPLOADER_BINDER(ref);
            ThrowIfNull( contactUploaderBinder, "invalidContactUploaderBinder" );

            return contactUploaderBinder->getContactUploader()->addContactsBegin();
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_contactuploader_ContactUploader_addContactsBegin",ex.what());
            return false;
        }
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_contactuploader_ContactUploader_addContactsEnd( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto contactUploaderBinder = CONTACT_UPLOADER_BINDER(ref);
            ThrowIfNull( contactUploaderBinder, "invalidContactUploaderBinder" );

            return contactUploaderBinder->getContactUploader()->addContactsEnd();
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_contactuploader_ContactUploader_addContactsEnd",ex.what());
            return false;
        }
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_contactuploader_ContactUploader_addContactsCancel( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto contactUploaderBinder = CONTACT_UPLOADER_BINDER(ref);
            ThrowIfNull( contactUploaderBinder, "invalidContactUploaderBinder" );

            return contactUploaderBinder->getContactUploader()->addContactsCancel();
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_contactuploader_ContactUploader_addContactsCancel",ex.what());
            return false;
        }
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_contactuploader_ContactUploader_addContact( JNIEnv* env, jobject /* this */, jlong ref, jstring contact )
    {
        try
        {
            auto contactUploaderBinder = CONTACT_UPLOADER_BINDER(ref);
            ThrowIfNull( contactUploaderBinder, "invalidContactUploaderBinder" );

            return contactUploaderBinder->getContactUploader()->addContact( JString(contact).toStdStr() );
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_contactuploader_ContactUploader_addContact",ex.what());
            return false;
        }
    }

    JNIEXPORT jboolean JNICALL
    Java_com_amazon_aace_contactuploader_ContactUploader_removeUploadedContacts( JNIEnv* env, jobject /* this */, jlong ref )
    {
        try
        {
            auto contactUploaderBinder = CONTACT_UPLOADER_BINDER(ref);
            ThrowIfNull( contactUploaderBinder, "invalidContactUploaderBinder" );

            return contactUploaderBinder->getContactUploader()->removeUploadedContacts();
        }
        catch( const std::exception& ex ) {
            AACE_JNI_ERROR(TAG,"Java_com_amazon_aace_contactuploader_ContactUploader_removeUploadedContacts",ex.what());
            return false;
        }
    }
}
