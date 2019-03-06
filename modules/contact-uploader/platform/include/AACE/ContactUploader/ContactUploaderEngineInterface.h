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

#ifndef AACE_CONTACTUPLOADER_CONTACTUPLOADER_ENGINE_INTERFACE_H
#define AACE_CONTACTUPLOADER_CONTACTUPLOADER_ENGINE_INTERFACE_H

#include <iostream>

#include "AACE/Core/PlatformInterface.h"
#include "ContactUploaderEngineInterface.h"

/** @file */

namespace aace {
namespace contactUploader {

class ContactUploaderEngineInterface : public aace::core::PlatformInterface {
public:
   /**
    * Describes the status of contact uploader to the platform implementation.
    */
    enum class ContactUploaderStatus {
        /**
         * Uploading of batched contacts to Cloud has started.
         */
        UPLOAD_CONTACTS_STARTED,

        /**
         * Uploading of subsequent batch of contacts to Cloud
         */
        UPLOAD_CONTACTS_UPLOADING,

        /**
         * Uploading of all the contacts to Cloud completed.
         *
         * The @param info would contain the list of contacts failed to upload if any.
         * The info would have the structured JSON format:
         * @code{.json})
         * {
         *      "failedContact" : [
         *           {
         *               "id" : "{{STRING}}",
         *               "reason" : "{{STRING}}",
         *           }
         *      ]
         * }
         * @endcode
         * @li failedContact : An Array, if present shall list out the failed to upload contacts.
         * @li failedContact.id : A unique identifier for the contact in the platform contact store.
         * @li failedContact.reason : The reason for the failure.
        */
        UPLOAD_CONTACTS_COMPLETED,
        
        /**
         * Uploading of contacts is Canceled.
         */
        UPLOAD_CONTACTS_CANCELED,

        /**
         * Error while uploading the contacts.
         */
        UPLOAD_CONTACTS_ERROR,

        /**
         * Removing the contacts from Cloud has started.
         */
        REMOVE_CONTACTS_STARTED,
        
        /**
         * Removed the contacts from Cloud.
         */
        REMOVE_CONTACTS_COMPLETED,
        
        /**
         * Error while removing contacts from Cloud.
         */
        REMOVE_CONTACTS_ERROR,

        /**
         * Unknown Error. 
         */
        UNKNOWN_ERROR

    };

    virtual bool onAddContactsBegin() = 0;
    virtual bool onAddContactsEnd() = 0;
    virtual bool onAddContactsCancel() = 0;
    virtual bool onAddContact( const std::string& contact ) = 0;
    virtual bool onRemoveUploadedContacts() = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const ContactUploaderEngineInterface::ContactUploaderStatus& status) {
    switch (status) {
        case ContactUploaderEngineInterface::ContactUploaderStatus::UPLOAD_CONTACTS_STARTED:
            stream << "UPLOAD_CONTACTS_STARTED";
            break;
        case ContactUploaderEngineInterface::ContactUploaderStatus::UPLOAD_CONTACTS_UPLOADING:
            stream << "UPLOAD_CONTACTS_UPLOADING";
            break;
        case ContactUploaderEngineInterface::ContactUploaderStatus::UPLOAD_CONTACTS_COMPLETED:
            stream << "UPLOAD_CONTACTS_COMPLETED";
            break;
        case ContactUploaderEngineInterface::ContactUploaderStatus::UPLOAD_CONTACTS_CANCELED:
            stream << "UPLOAD_CONTACTS_CANCELED";
            break;
        case ContactUploaderEngineInterface::ContactUploaderStatus::UPLOAD_CONTACTS_ERROR:
            stream << "UPLOAD_CONTACTS_ERROR";
            break;
        case ContactUploaderEngineInterface::ContactUploaderStatus::REMOVE_CONTACTS_STARTED:
            stream << "REMOVE_CONTACTS_STARTED";
            break;
        case ContactUploaderEngineInterface::ContactUploaderStatus::REMOVE_CONTACTS_COMPLETED:
            stream << "REMOVE_CONTACTS_COMPLETED";
            break;
        case ContactUploaderEngineInterface::ContactUploaderStatus::REMOVE_CONTACTS_ERROR:
            stream << "REMOVE_CONTACTS_ERROR";
            break;
        case ContactUploaderEngineInterface::ContactUploaderStatus::UNKNOWN_ERROR:
            stream << "UNKNOWN_ERROR";
            break;
    }
    return stream;
}

} // aace::contactUploader
} // aace

#endif // AACE_CONTACTUPLOADER_CONTACTUPLOADER_ENGINE_INTERFACE_H
