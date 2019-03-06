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

#include "AACE/Engine/ContactUploader/ContactUploaderEngineService.h"
#include "AACE/Engine/Alexa/AlexaEngineService.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace contactUploader {

// String to identify log entries originating from this file.
static const std::string TAG("aace.contactuploader.ContactUploaderEngineService");

// register the service
REGISTER_SERVICE(ContactUploaderEngineService);

ContactUploaderEngineService::ContactUploaderEngineService( const aace::engine::core::ServiceDescription& description ) : aace::engine::core::EngineService( description ) {
}

bool ContactUploaderEngineService::configure( const std::vector< std::shared_ptr<std::istream>>& configuration) {
    try {
        //TBD Add Configure support.    
        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"configure").d("reason", ex.what() ) );
        return false;
    }
}

bool ContactUploaderEngineService::start() {
    return true;
}

bool ContactUploaderEngineService::stop() {
    return true;
}

bool ContactUploaderEngineService::shutdown() {
    if( m_contactUploaderEngineImpl != nullptr ) {
        m_contactUploaderEngineImpl->shutdown();
        m_contactUploaderEngineImpl.reset();
    } 
    return true;
}

bool ContactUploaderEngineService::registerPlatformInterface( std::shared_ptr<aace::core::PlatformInterface> platformInterface ) {
    try {
        ReturnIf( registerPlatformInterfaceType<aace::contactUploader::ContactUploader>( platformInterface ), true );
        return false;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterface").d("reason", ex.what() ) );
        return false;
    }
}

bool ContactUploaderEngineService::registerPlatformInterfaceType( std::shared_ptr<aace::contactUploader::ContactUploader> contactUploader ) {
    std::shared_ptr<alexaClientSDK::avsCommon::utils::DeviceInfo> deviceInfo;
    try {
        ThrowIfNotNull( m_contactUploaderEngineImpl, "platformInterfaceAlreadyRegistered" );

        // Get the Alexa component interface which exposes all service interfaces
        auto alexaComponentInterface = getContext()->getServiceInterface<aace::engine::alexa::AlexaComponentInterface>( "aace.alexa" );
        ThrowIfNull( alexaComponentInterface, "alexaComponentInterfaceInValid" );

        auto authDelegate = alexaComponentInterface->getAuthDelegate();
        ThrowIfNull( authDelegate, "authDeleteInterfaceInValid" );

        auto config = alexaClientSDK::avsCommon::utils::configuration::ConfigurationNode::getRoot();

        // create device info
        deviceInfo = alexaClientSDK::avsCommon::utils::DeviceInfo::create( config );
        ThrowIfNull( deviceInfo, "createDeviceInfoFailed" );

        m_contactUploaderEngineImpl = aace::engine::contactUploader::ContactUploaderEngineImpl::create( contactUploader, authDelegate,  deviceInfo );
        ThrowIfNull( m_contactUploaderEngineImpl, "createContactUploaderEngineImplFailed" );

        return true;
    }
    catch( std::exception& ex ) {
        AACE_ERROR(LX(TAG,"registerPlatformInterfaceType<ContactUploader>").d("reason", ex.what() ) );
        return false;
    }
}

} // aace::engine::contactUploader
} // aace::engine
} // aace
