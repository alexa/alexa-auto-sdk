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

// aace/alexa/SpeechRecognizerBinder.h
// This is an automatically generated file.

#ifndef AACE_ALEXA_SPEECH_RECOGNIZER_BINDER_H
#define AACE_ALEXA_SPEECH_RECOGNIZER_BINDER_H

#include "AACE/Alexa/SpeechRecognizer.h"
#include "aace/core/PlatformInterfaceBinder.h"

class SpeechRecognizerBinder : public PlatformInterfaceBinder, public aace::alexa::SpeechRecognizer {
public:
    SpeechRecognizerBinder( bool wakewordDetectionEnabled );

    aace::alexa::SpeechRecognizer::Initiator convertInitiator( JNIEnv* env, jobject obj );

protected:
    void initialize( JNIEnv* env ) override;

public:
    bool wakewordDetected( const std::string & wakeword ) override;
    void endOfSpeechDetected() override;
    bool startAudioInput() override;
    bool stopAudioInput() override;

private:
    jmethodID m_javaMethod_wakewordDetected_wakeword = nullptr;
    jmethodID m_javaMethod_endOfSpeechDetected = nullptr;
    jmethodID m_javaMethod_startAudioInput = nullptr;
    jmethodID m_javaMethod_stopAudioInput = nullptr;

    // Initiator
    ObjectRef m_enum_Initiator_HOLD_TO_TALK;
    ObjectRef m_enum_Initiator_TAP_TO_TALK;
    ObjectRef m_enum_Initiator_WAKEWORD;
};

#endif //AACE_ALEXA_SPEECH_RECOGNIZER_BINDER_H
