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

// aace/contactuploader/ContactUploaderBinder.cpp

#include "aace/contactuploader/ContactUploaderBinder.h"

void ContactUploaderBinder::initialize(JNIEnv *env)
{
    m_javaMethod_contactsUploaderStatusChanged_uploaderState =
            env->GetMethodID( getJavaClass(),
                              "contactsUploaderStatusChanged",
                              "(Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;Ljava/lang/String;)V");

    // contact upload status
    jclass contactUploadStatusEnumClass =
            env->FindClass( "com/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus" );
    m_enum_ContactUploadStatus_UPLOAD_CONTACTS_STARTED = NativeLib::FindEnum( env,
                                                                              contactUploadStatusEnumClass,
                                                                              "UPLOAD_CONTACTS_STARTED",
                                                                              "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );
    m_enum_ContactUploadStatus_UPLOAD_CONTACTS_UPLOADING = NativeLib::FindEnum( env,
                                                                                contactUploadStatusEnumClass,
                                                                                "UPLOAD_CONTACTS_UPLOADING",
                                                                                "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );
    m_enum_ContactUploadStatus_UPLOAD_CONTACTS_COMPLETED = NativeLib::FindEnum( env,
                                                                                contactUploadStatusEnumClass,
                                                                                "UPLOAD_CONTACTS_COMPLETED",
                                                                                "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );
    m_enum_ContactUploadStatus_UPLOAD_CONTACTS_CANCELED = NativeLib::FindEnum( env,
                                                                               contactUploadStatusEnumClass,
                                                                               "UPLOAD_CONTACTS_CANCELED",
                                                                               "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );
    m_enum_ContactUploadStatus_UPLOAD_CONTACTS_ERROR = NativeLib::FindEnum( env,
                                                                            contactUploadStatusEnumClass,
                                                                            "UPLOAD_CONTACTS_ERROR",
                                                                            "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );

    m_enum_ContactUploadStatus_REMOVE_CONTACTS_STARTED = NativeLib::FindEnum( env,
                                                                            contactUploadStatusEnumClass,
                                                                            "REMOVE_CONTACTS_STARTED",
                                                                            "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );

    m_enum_ContactUploadStatus_REMOVE_CONTACTS_COMPLETED = NativeLib::FindEnum( env,
                                                                            contactUploadStatusEnumClass,
                                                                            "REMOVE_CONTACTS_COMPLETED",
                                                                            "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );

    m_enum_ContactUploadStatus_REMOVE_CONTACTS_ERROR = NativeLib::FindEnum( env,
                                                                            contactUploadStatusEnumClass,
                                                                            "REMOVE_CONTACTS_ERROR",
                                                                            "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );

    m_enum_ContactUploadStatus_UNKNOWN_ERROR = NativeLib::FindEnum( env,
                                                                    contactUploadStatusEnumClass,
                                                                    "UNKNOWN_ERROR",
                                                                    "Lcom/amazon/aace/contactuploader/ContactUploader$ContactUploadStatus;" );
}

void ContactUploaderBinder::contactsUploaderStatusChanged( ContactUploaderStatus uploaderStatus, const std::string & info) {
    if( getJavaObject() != nullptr && m_javaMethod_contactsUploaderStatusChanged_uploaderState != nullptr )
    {
        ThreadContext context;

        if( context.isValid() )
        {
            jstring infos = context.getEnv()->NewStringUTF(info.c_str());
            context.getEnv()->CallVoidMethod(getJavaObject(),
                                             m_javaMethod_contactsUploaderStatusChanged_uploaderState,
                                             convertUploaderState(uploaderStatus), infos);
        }
    }
}

jobject ContactUploaderBinder::convertUploaderState( ContactUploaderStatus uploaderStatus ) {
    switch (uploaderStatus) {
        case ContactUploaderStatus::UPLOAD_CONTACTS_STARTED:
            return m_enum_ContactUploadStatus_UPLOAD_CONTACTS_STARTED.get();
        case ContactUploaderStatus::UPLOAD_CONTACTS_UPLOADING:
            return m_enum_ContactUploadStatus_UPLOAD_CONTACTS_UPLOADING.get();
        case ContactUploaderStatus::UPLOAD_CONTACTS_COMPLETED:
            return m_enum_ContactUploadStatus_UPLOAD_CONTACTS_COMPLETED.get();
        case ContactUploaderStatus::UPLOAD_CONTACTS_CANCELED:
            return m_enum_ContactUploadStatus_UPLOAD_CONTACTS_CANCELED.get();
        case ContactUploaderStatus::UPLOAD_CONTACTS_ERROR:
            return m_enum_ContactUploadStatus_UPLOAD_CONTACTS_ERROR.get();
        case ContactUploaderStatus::REMOVE_CONTACTS_STARTED:
            return m_enum_ContactUploadStatus_REMOVE_CONTACTS_STARTED.get();
        case ContactUploaderStatus ::REMOVE_CONTACTS_COMPLETED:
            return m_enum_ContactUploadStatus_REMOVE_CONTACTS_COMPLETED.get();
        case ContactUploaderStatus::REMOVE_CONTACTS_ERROR:
            return m_enum_ContactUploadStatus_REMOVE_CONTACTS_ERROR.get();
        case ContactUploaderStatus::UNKNOWN_ERROR:
            return m_enum_ContactUploadStatus_UNKNOWN_ERROR.get();
    }
}

#define CONTACTUPLOADER(cptr) ((ContactUploaderBinder *) cptr)

extern "C" {
JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_contactuploader_ContactUploader_addContactsBegin(JNIEnv *env,
                                                                      jobject instance,
                                                                      jlong cptr) {
    return CONTACTUPLOADER(cptr)->addContactsBegin();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_contactuploader_ContactUploader_addContactsEnd(JNIEnv *env,
                                                                    jobject instance,
                                                                    jlong cptr) {
    return CONTACTUPLOADER(cptr)->addContactsEnd();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_contactuploader_ContactUploader_addContactsCancel(JNIEnv *env,
                                                                       jobject instance,
                                                                       jlong cptr) {
    return CONTACTUPLOADER(cptr)->addContactsCancel();
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_contactuploader_ContactUploader_addContact(JNIEnv *env,
                                                                jobject instance,
                                                                jlong cptr,
                                                                jstring jsonContactInfo) {
    return CONTACTUPLOADER(cptr)->addContact(NativeLib::convert(env, jsonContactInfo));
}

JNIEXPORT jboolean JNICALL
Java_com_amazon_aace_contactuploader_ContactUploader_removeUploadedContacts(JNIEnv *env,
                                                                            jobject instance,
                                                                            jlong cptr) {
    return CONTACTUPLOADER(cptr)->removeUploadedContacts();
}
}