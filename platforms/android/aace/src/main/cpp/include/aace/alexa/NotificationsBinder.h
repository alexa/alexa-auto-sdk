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

// aace/alexa/NotificationsBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_NOTIFICATIONS_BINDER_H
#define AACE_ALEXA_NOTIFICATIONS_BINDER_H

#include "AACE/Alexa/Notifications.h"
#include "aace/core/PlatformInterfaceBinder.h"

class NotificationsBinder : public PlatformInterfaceBinder, public aace::alexa::Notifications {
public:
    NotificationsBinder( std::shared_ptr<aace::alexa::MediaPlayer> mediaPlayer, std::shared_ptr<aace::alexa::Speaker> speaker );

protected:
    void initialize( JNIEnv* env ) override;

public:
    void setIndicator( aace::alexa::Notifications::IndicatorState state ) override;

/*
public:
    aace::alexa::Notifications::IndicatorState convertIndicatorState( JNIEnv* env, jobject obj );
*/

private:
    jobject convert( aace::alexa::Notifications::IndicatorState state );

private:
    jmethodID m_javaMethod_setIndicator_state = nullptr;

    // IndicatorState
    ObjectRef m_enum_IndicatorState_OFF;
    ObjectRef m_enum_IndicatorState_ON;
    ObjectRef m_enum_IndicatorState_UNKNOWN;

};

#endif //AACE_ALEXA_NOTIFICATIONS_BINDER_H
