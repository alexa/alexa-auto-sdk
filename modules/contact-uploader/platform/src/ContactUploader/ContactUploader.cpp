/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AACE/ContactUploader/ContactUploader.h"

namespace aace {
namespace contactUploader {

bool ContactUploader::addContactsBegin() {
    if( m_contactUploaderEngineInterface != nullptr ) {
        return( m_contactUploaderEngineInterface->onAddContactsBegin() );
    }
    return false;
}

bool ContactUploader::addContactsEnd() {
    if( m_contactUploaderEngineInterface != nullptr ) {
        return( m_contactUploaderEngineInterface->onAddContactsEnd() );
    }
    return false;
}

bool ContactUploader::addContactsCancel() {
    if( m_contactUploaderEngineInterface != nullptr ) {
        return( m_contactUploaderEngineInterface->onAddContactsCancel() );
    }
    return false;
}

bool ContactUploader::addContact( const std::string &contact ) {
    if( m_contactUploaderEngineInterface != nullptr ) {
        return( m_contactUploaderEngineInterface->onAddContact( contact ) );
    }
    return false;
}

bool ContactUploader::removeUploadedContacts( ){
    if( m_contactUploaderEngineInterface != nullptr ) {
        return( m_contactUploaderEngineInterface->onRemoveUploadedContacts( ) );
    }
    return false;
}

void ContactUploader::setEngineInterface ( std::shared_ptr<ContactUploaderEngineInterface>  contactUploaderEngineInterface ) {
    m_contactUploaderEngineInterface = contactUploaderEngineInterface;
}

ContactUploader::~ContactUploader() = default; //key function

} // aace::contactUploader
} // aace
