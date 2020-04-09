/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "AACE/AmazonLite/AmazonLiteConfiguration.h"

namespace aace {
namespace amazonLite {
namespace config {

// String to identify log entries originating from this file.
static const std::string TAG("aace.amazonLite.AmazonLiteConfiguration");

std::shared_ptr<aace::core::config::EngineConfiguration> AmazonLiteConfiguration::createAmazonLiteConfig( const std::string& rootPath, const std::vector<ModelConfig>& modelList )
{
    rapidjson::Document document;

    document.SetObject();

    rapidjson::Value aaceAmazonLiteElement;
    rapidjson::Value models;

    aaceAmazonLiteElement.SetObject();
    models.SetArray();
    
    for( auto& next : modelList ) {
        rapidjson::Value modelObject;

        modelObject.SetObject();

        modelObject.AddMember( "locale", rapidjson::Value().SetString( next.first.c_str(), next.first.length() ), document.GetAllocator() );
        modelObject.AddMember( "path", rapidjson::Value().SetString( next.second.c_str(), next.second.length() ), document.GetAllocator() );

        models.PushBack( modelObject, document.GetAllocator() );
    }

    aaceAmazonLiteElement.AddMember( "rootPath", rootPath, document.GetAllocator() );
    aaceAmazonLiteElement.AddMember( "models", models, document.GetAllocator() );

    document.AddMember( "aace.amazonLite", aaceAmazonLiteElement, document.GetAllocator() );

    // create event string
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );

    document.Accept( writer );

    return aace::core::config::StreamConfiguration::create( std::make_shared<std::stringstream>( buffer.GetString() ) );
}

} // aace::amazonLite::config
} // aace::amazonLite
} // aace