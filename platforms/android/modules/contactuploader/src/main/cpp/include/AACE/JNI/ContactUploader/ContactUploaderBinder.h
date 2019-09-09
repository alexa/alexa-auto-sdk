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

#ifndef AACE_JNI_CONTACTUPLOADER_CONTACTUPLOADER_BINDER_H
#define AACE_JNI_CONTACTUPLOADER_CONTACTUPLOADER_BINDER_H

#include <AACE/ContactUploader/ContactUploader.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace contactuploader {

    //
    // ContactUploaderHandler
    //

    class ContactUploaderHandler : public aace::contactUploader::ContactUploader {
    public:
        ContactUploaderHandler( jobject obj );

        // aace::contactUploader::ContactUploader
        void contactsUploaderStatusChanged( ContactUploaderStatus status, const std::string& info ) override;

    private:
        JObject m_obj;
    };

    //
    // ContactUploaderBinder
    //

    class ContactUploaderBinder : public aace::jni::core::PlatformInterfaceBinder {
    public:
        ContactUploaderBinder( jobject obj );

        std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
            return m_contactUploaderHandler;
        }

        std::shared_ptr<ContactUploaderHandler> getContactUploader() {
            return m_contactUploaderHandler;
        }

    private:
        std::shared_ptr<ContactUploaderHandler> m_contactUploaderHandler;
    };

    //
    // JContactUploaderStatus
    //

    class JContactUploaderStatusConfig : public EnumConfiguration<ContactUploaderHandler::ContactUploaderStatus> {
    public:
        using T = ContactUploaderHandler::ContactUploaderStatus;

        const char* getClassName() override {
            return "com/amazon/aace/contactuploader/ContactUploader$ContactUploaderStatus";
        }

        std::vector<std::pair<T, std::string>> getConfiguration() override {
            return {
                {T::UPLOAD_CONTACTS_STARTED,"UPLOAD_CONTACTS_STARTED"},
                {T::UPLOAD_CONTACTS_UPLOADING,"UPLOAD_CONTACTS_UPLOADING"},
                {T::UPLOAD_CONTACTS_COMPLETED,"UPLOAD_CONTACTS_COMPLETED"},
                {T::UPLOAD_CONTACTS_CANCELED,"UPLOAD_CONTACTS_CANCELED"},
                {T::REMOVE_CONTACTS_STARTED,"REMOVE_CONTACTS_STARTED"},
                {T::REMOVE_CONTACTS_COMPLETED,"REMOVE_CONTACTS_COMPLETED"},
                {T::REMOVE_CONTACTS_ERROR,"REMOVE_CONTACTS_ERROR"},
                {T::UNKNOWN_ERROR,"UNKNOWN_ERROR"}
            };
        }
    };

    using JContactUploaderStatus = JEnum<ContactUploaderHandler::ContactUploaderStatus, JContactUploaderStatusConfig>;

} // aace::jni::contactuploader
} // aace:jni
} // aace
#endif //AACE_JNI_CONTACTUPLOADER_CONTACTUPLOADER_BINDER_H
