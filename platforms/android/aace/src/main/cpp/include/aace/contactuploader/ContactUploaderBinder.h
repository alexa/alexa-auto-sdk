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

// aace/contactuploader/ContactUploaderBinder.h

#ifndef AACE_CONTACT_UPLOAD_CONTACT_UPLOADER_BINDER_H
#define AACE_CONTACT_UPLOAD_CONTACT_UPLOADER_BINDER_H

#include "AACE/ContactUploader/ContactUploader.h"
#include "aace/core/PlatformInterfaceBinder.h"

class ContactUploaderBinder : public PlatformInterfaceBinder, public aace::contactUploader::ContactUploader {

public:
    ContactUploaderBinder() = default;

protected:
    void initialize( JNIEnv* env ) override;

public:
    void contactsUploaderStatusChanged(ContactUploaderStatus uploaderStatus, const std::string & info ) override;

private:
    jobject convertUploaderState( ContactUploaderStatus contactUploaderStatusState);

private:
    jmethodID m_javaMethod_contactsUploaderStatusChanged_uploaderState = nullptr;

    //Contact Upload Status
    ObjectRef m_enum_ContactUploadStatus_UPLOAD_CONTACTS_STARTED;
    ObjectRef m_enum_ContactUploadStatus_UPLOAD_CONTACTS_UPLOADING;
    ObjectRef m_enum_ContactUploadStatus_UPLOAD_CONTACTS_COMPLETED;
    ObjectRef m_enum_ContactUploadStatus_UPLOAD_CONTACTS_CANCELED;
    ObjectRef m_enum_ContactUploadStatus_UPLOAD_CONTACTS_ERROR;
    ObjectRef m_enum_ContactUploadStatus_REMOVE_CONTACTS_STARTED;
    ObjectRef m_enum_ContactUploadStatus_REMOVE_CONTACTS_COMPLETED;
    ObjectRef m_enum_ContactUploadStatus_REMOVE_CONTACTS_ERROR;
    ObjectRef m_enum_ContactUploadStatus_UNKNOWN_ERROR;
};

#endif //AACE_CONTACT_UPLOAD_CONTACT_UPLOADER_BINDER_H
