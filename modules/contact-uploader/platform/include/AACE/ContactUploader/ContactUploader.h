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

#ifndef AACE_CONTACTUPLOADER_CONTACTUPLOADER_H
#define AACE_CONTACTUPLOADER_CONTACTUPLOADER_H

#include "AACE/Core/PlatformInterface.h"
#include "ContactUploaderEngineInterface.h"

/** @file */

namespace aace {
namespace contactUploader {

/**
 * ContactUploader should be extended to upload the local (aka native) contacts to the Alexa Cloud.
 * ContactUploader Engine uploads the contacts to the Alexa Cloud in batches. The Engine API blocks
 * calls, so platform implementation should make sure ContactUploader is called on a separate process
 * other then the user interface thread.
 * 
 * The platform implementation is notified about the status of the upload by @c contactsUploaderStatusChanged.
 */
class ContactUploader : public aace::core::PlatformInterface {
protected:
    ContactUploader() = default;

public:
    virtual ~ContactUploader();

    /**
     * Describes the status of contact uploader.
     * @sa @c aace::contactUploader::ContactUploaderEngineInterface::ContactUploadStatus
     */
    using ContactUploaderStatus = aace::contactUploader::ContactUploaderEngineInterface::ContactUploaderStatus;
    

    /**
     * Notifies the Engine to begin the contact upload.
     *
     * After returning @c true, the Engine will remove the previously uploaded contacts, if any, and the platform implementation
     * may start calling @c addContact to upload the contacts.
     *
     * @return @c true on successful start, @c false if previous upload is in progress or due to any internal error.
     * 
     */
    bool addContactsBegin();

    /**
     * Notifies the Engine about the completion of the contact upload.
     * 
     * The platform implementation to call this when all the contacts are uploaded via @sa addContact.
     * Engine on returning @c true, will not accept any contact and shall notify the platform implementation
     * @sa contactsUploaderStatusChanged about the status of upload.
     * 
     * @return @c true on successful, @c false if no upload in progress or due to any internal error.
     */
    bool addContactsEnd();

    /**
     * Notifies the Engine to cancel the upload which is in progress.
     * 
     * The platform implementation can call this anytime to cancel the contact upload when it is in progress.
     * Engine on returning @true, will not accept any contact, and shall notify the platform implementation
     * @sa contactsUploaderStatusChanged about the cancel status and it shall also remove any partial uploaded contacts
     * from Alexa Cloud.
     * 
     * @return @c true on successful, @c false if no upload in progress or due to any internal error.
     */
    bool addContactsCancel();
    
    /**
     * Notifies the Engine to upload the contact.
     *
     * Engine on returning @c true shall queue this contact for uploading in batches.
     *
     * @param [in] contact Details of one contact to upload in structured JSON format. See the following
     * payload structure and the description of each field:
     * @code{.json})
     * {
     *      "id" : "{{STRING}}",
     *      "firstName" : "{{STRING}}",
     *      "lastName" : "{{STRING}}",
     *      "nickName" : "{{STRING}}"
     *      "company" : "{{STRING}}",
     *      "addresses" : [
     *          {
     *              "type" : "{{STRING}}",
     *              "value" : "{{STRING}}",
     *              "label" : "{{STRING}}"
     *          }
     *      ]
     * }
     * @endcode 
     * @li id (required): A unique identifier for the contact in the platform contact store.
     * @li firstName (optional): The first name of the contact. Limit is 1000 characters.
     * @li lastName (optional): The last name of the contact. Limit is 1000 characters.
     * @li nickName (optional): The nick name of the contact. Limit is 1000 characters.
     * @li company (optional): The company name of the contact. Limit is 1000 characters.
     * @li addresses (required): The address information of the contact. At least one address required to be specified.
     * @li addresses.type (required): Supported address type is "phonenumber" only. And the max supported phone number per contact is 30.
     * @li addresses.value (required): The phone number of the contact. Limit is 100 characters.
     * @li addresses.label (optional): The type value of the phone number like HOME. MOBILE, Phone1.
     *
     * @return @c true if successfully queued, @c false when contact format is not valid, upload not started or for any internal error.
     */ 
    bool addContact( const std::string& contact );

    /**
     * Notifies the Engine to remove the uploaded local contacts from the Alexa Cloud. This API to be called when the local
     * contacts become unavailable (Example use case, Phone is disconnected from the Head Unit) and contacts were already
     * successfully uploaded to Alexa Cloud.
     * 
     * Engine on returning @c true shall notify the platform implementation about the removal status of contacts
     * via @sa contactsUploaderStatusChanged.
     * 
     * @return @c true on successful, @c false if upload is in progress or due to any internal error.
     */
    bool removeUploadedContacts();

    /**
     * Notifies the platform implementation about status that occurred while uploading the batch of contacts or while
     * removing the uploaded contacts.
     * 
     * @param [in] status The @c ContactUploaderStatus
     * @param [in] info The string with additional information.
     */
    virtual void contactsUploaderStatusChanged( ContactUploaderStatus status, const std::string& info ) = 0;

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface( std::shared_ptr<ContactUploaderEngineInterface> contactUploaderEngineInterface );

private:
    std::shared_ptr<ContactUploaderEngineInterface> m_contactUploaderEngineInterface;
};

} // aace::contactUploader
} // aace

#endif // AACE_CONTACTUPLOADER_CONTACTUPLOADER_H
