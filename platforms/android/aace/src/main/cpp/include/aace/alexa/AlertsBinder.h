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

// aace/alexa/AlertsBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_ALERTS_BINDER_H
#define AACE_ALEXA_ALERTS_BINDER_H

#include "AACE/Alexa/Alerts.h"
#include "aace/core/PlatformInterfaceBinder.h"

class AlertsBinder : public PlatformInterfaceBinder, public aace::alexa::Alerts {
public:
    AlertsBinder( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );

protected:
    void initialize( JNIEnv* env ) override;

public:
    void alertStateChanged( const std::string & alertToken, aace::alexa::Alerts::AlertState state, const std::string & reason ) override;

    void alertCreated( const std::string & alertToken, const std::string & detailedInfo ) override;

    void alertDeleted( const std::string & alertToken ) override;

    /*
public: // these need to be accessible
    aace::alexa::Alerts::AlertState convertAlertState( jobject obj );
*/

private:
    jobject convert( aace::alexa::Alerts::AlertState state );

private:
    jmethodID m_javaMethod_alertStateChanged_alertToken_state_reason = nullptr;
    jmethodID m_javaMethod_alertCreated_alertToken_detailedInfo = nullptr;
    jmethodID m_javaMethod_alertDeleted_alertToken = nullptr;

    // AlertState
    ObjectRef m_enum_AlertState_READY;
    ObjectRef m_enum_AlertState_STARTED;
    ObjectRef m_enum_AlertState_STOPPED;
    ObjectRef m_enum_AlertState_SNOOZED;
    ObjectRef m_enum_AlertState_COMPLETED;
    ObjectRef m_enum_AlertState_PAST_DUE;
    ObjectRef m_enum_AlertState_FOCUS_ENTERED_FOREGROUND;
    ObjectRef m_enum_AlertState_FOCUS_ENTERED_BACKGROUND;
    ObjectRef m_enum_AlertState_ERROR;
};

#endif //AACE_ALEXA_ALERTS_BINDER_H
