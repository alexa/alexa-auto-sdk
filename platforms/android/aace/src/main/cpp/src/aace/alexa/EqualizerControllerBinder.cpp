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

#include "aace/alexa/EqualizerControllerBinder.h"

void EqualizerControllerBinder::initialize( JNIEnv* env )
{
    m_javaMethod_setBandLevels_bandLevelMap = env->GetMethodID(
            getJavaClass(),
            "setBandLevels",
            "([Lcom/amazon/aace/alexa/EqualizerController$EqualizerBandLevel;)V" );
    m_javaMethod_getBandLevels = env->GetMethodID(
            getJavaClass(),
            "getBandLevels",
            "()[Lcom/amazon/aace/alexa/EqualizerController$EqualizerBandLevel;" );

    // EqualizerBand
    jclass equalizerBandEnumClass = env->FindClass( "com/amazon/aace/alexa/EqualizerController$EqualizerBand" );
    m_enum_EqualizerBand_BASS = NativeLib::FindEnum(
            env,
            equalizerBandEnumClass,
            "BASS",
            "Lcom/amazon/aace/alexa/EqualizerController$EqualizerBand;" );
    m_enum_EqualizerBand_MIDRANGE = NativeLib::FindEnum(
            env,
            equalizerBandEnumClass,
            "MIDRANGE",
            "Lcom/amazon/aace/alexa/EqualizerController$EqualizerBand;" );
    m_enum_EqualizerBand_TREBLE = NativeLib::FindEnum(
            env,
            equalizerBandEnumClass,
            "TREBLE",
            "Lcom/amazon/aace/alexa/EqualizerController$EqualizerBand;" );

    // EqualizerBandLevel
    m_javaClass_EqualizerBandLevel = NativeLib::FindClass(
            env,
            "com/amazon/aace/alexa/EqualizerController$EqualizerBandLevel" );
    m_javaMethod_EqualizerBandLevel_init = env->GetMethodID(
            m_javaClass_EqualizerBandLevel.get(),
            "<init>",
            "(Lcom/amazon/aace/alexa/EqualizerController$EqualizerBand;I)V" );
    m_javaMethod_EqualizerBandLevel_getBand = env->GetMethodID(
            m_javaClass_EqualizerBandLevel.get(),
            "getBand",
            "()Lcom/amazon/aace/alexa/EqualizerController$EqualizerBand;" );
    m_javaMethod_EqualizerBandLevel_getLevel = env->GetMethodID(
            m_javaClass_EqualizerBandLevel.get(),
            "getLevel",
            "()I" );
}

void EqualizerControllerBinder::setBandLevels(
        const std::vector<aace::alexa::EqualizerController::EqualizerBandLevel>& bandLevels ) {
    if( getJavaObject() != nullptr
            && m_javaMethod_setBandLevels_bandLevelMap != nullptr
            && m_javaMethod_EqualizerBandLevel_init != nullptr ) {
        ThreadContext context;
        if( context.isValid() ) {
            JNIEnv* env = context.getEnv();
            int size = bandLevels.size();
            jobjectArray jBandLevels = env->NewObjectArray( size, m_javaClass_EqualizerBandLevel.get(), NULL );
            if( jBandLevels != NULL ) {
                int i = 0;
                for( const auto& bandLevel : bandLevels ) {
                    jobject jBand = convertBand( bandLevel.first );
                    if( jBand != NULL ) {
                        jobject jBandLevel = env->NewObject(
                            m_javaClass_EqualizerBandLevel.get(),
                            m_javaMethod_EqualizerBandLevel_init,
                            jBand,
                            bandLevel.second );
                        if( jBandLevel != NULL ) {
                            env->SetObjectArrayElement( jBandLevels, i, jBandLevel );
                            i++;
                        }
                    }
                }
                env->CallVoidMethod( getJavaObject(), m_javaMethod_setBandLevels_bandLevelMap, jBandLevels );
            }
        }
    }
}

std::vector<aace::alexa::EqualizerController::EqualizerBandLevel> EqualizerControllerBinder::getBandLevels() {
    std::vector<aace::alexa::EqualizerController::EqualizerBandLevel> bandLevels;
    if( getJavaObject() != nullptr
            && m_javaMethod_getBandLevels != nullptr
            && m_javaMethod_EqualizerBandLevel_getBand != nullptr
            && m_javaMethod_EqualizerBandLevel_getLevel != nullptr) {
        ThreadContext context;
        if( context.isValid() ) {
            JNIEnv *env = context.getEnv();

            jobjectArray jBandLevels
                = static_cast<jobjectArray>( env->CallObjectMethod( getJavaObject(), m_javaMethod_getBandLevels ) );
            if( jBandLevels != NULL ) {
                int size = env->GetArrayLength( jBandLevels );
                for( int i=0; i < size; i++ ) {
                    jobject jBandLevel = env->GetObjectArrayElement( jBandLevels, i );
                    if( jBandLevel != NULL ) {
                        jobject jBand = env->CallObjectMethod( jBandLevel, m_javaMethod_EqualizerBandLevel_getBand );
                        jint jLevel = env->CallIntMethod( jBandLevel, m_javaMethod_EqualizerBandLevel_getLevel );
                        if( jBand != NULL ) {
                            auto band = convertBand(env, jBand);
                            bandLevels.push_back({band, jLevel});
                        }
                    }
                }
            }
        }
    }
    return bandLevels;
}

jobject EqualizerControllerBinder::convertBand( aace::alexa::EqualizerController::EqualizerBand band ) {
    switch( band ) {
        case aace::alexa::EqualizerController::EqualizerBand::BASS:
            return m_enum_EqualizerBand_BASS.get();
        case aace::alexa::EqualizerController::EqualizerBand::MIDRANGE:
            return m_enum_EqualizerBand_MIDRANGE.get();
        case aace::alexa::EqualizerController::EqualizerBand::TREBLE:
            return m_enum_EqualizerBand_TREBLE.get();
    }
}

aace::alexa::EqualizerController::EqualizerBand EqualizerControllerBinder::convertBand( JNIEnv* env, jobject jBand ) {
    if( m_enum_EqualizerBand_BASS.isSameObject( env, jBand ) ) {
        return aace::alexa::EqualizerController::EqualizerBand::BASS;
    }
    else if( m_enum_EqualizerBand_MIDRANGE.isSameObject( env, jBand ) ) {
        return aace::alexa::EqualizerController::EqualizerBand::MIDRANGE;
    }
    else if( m_enum_EqualizerBand_TREBLE.isSameObject( env, jBand ) ) {
        return aace::alexa::EqualizerController::EqualizerBand::TREBLE;
    }
}

std::vector<aace::alexa::EqualizerController::EqualizerBandLevel> EqualizerControllerBinder::convertBandLevelArray(
        JNIEnv* env,
        jobjectArray jBandLevels ) {
    std::vector<EqualizerBandLevel> bandLevels;
    if( m_javaMethod_EqualizerBandLevel_getLevel != nullptr && m_javaMethod_EqualizerBandLevel_getBand != nullptr ) {
        int count = env->GetArrayLength(jBandLevels);
        for (int i = 0; i < count; i++) {
            jobject jBandLevel = env->GetObjectArrayElement( jBandLevels, i );
            if( jBandLevel != NULL ) {
                jobject jBand = env->CallObjectMethod( jBandLevel, m_javaMethod_EqualizerBandLevel_getBand );
                if( jBand != NULL ) {
                    jint level = env->CallIntMethod( jBandLevel, m_javaMethod_EqualizerBandLevel_getLevel );
                    bandLevels.push_back( { convertBand(env, jBand), level } );
                }
            }
        }
    }
    return bandLevels;
}

// JNI
#define EQUALIZERCONTROLLER(cptr) ((EqualizerControllerBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_EqualizerController_localSetBandLevels(
        JNIEnv *env,
        jobject /* this */,
        jlong cptr,
        jobjectArray jBandLevels ) {
    auto ptr = EQUALIZERCONTROLLER(cptr);
    ptr->localSetBandLevels( ptr->convertBandLevelArray( env, jBandLevels ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_EqualizerController_localAdjustBandLevels(
        JNIEnv *env,
        jobject /* this */,
        jlong cptr,
        jobjectArray jBandAdjustments ) {
    auto ptr = EQUALIZERCONTROLLER(cptr);
    ptr->localAdjustBandLevels( ptr->convertBandLevelArray( env, jBandAdjustments ) );
}

JNIEXPORT void JNICALL
Java_com_amazon_aace_alexa_EqualizerController_localResetBands(
        JNIEnv *env,
        jobject /* this */,
        jlong cptr,
        jobjectArray jBands ) {
    auto ptr = EQUALIZERCONTROLLER(cptr);
    if( jBands == NULL ) {
        ptr->localResetBands();
        return;
    }
    int count = env->GetArrayLength( jBands );
    std::vector<aace::alexa::EqualizerController::EqualizerBand> bands;
    for( int i=0; i < count; i++ ) {
        jobject jBand = env->GetObjectArrayElement( jBands, i );
        if( jBand != NULL ) {
            aace::alexa::EqualizerController::EqualizerBand band = ptr->convertBand( env, jBand );
            bands.push_back( band );
        }
        env->DeleteLocalRef( jBand );
    }
    ptr->localResetBands( bands );
}

}

// END OF FILE
