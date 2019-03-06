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

// aace/cbl/CBLBinder.cpp

#include "aace/cbl/CBLBinder.h"

void CBLBinder::initialize( JNIEnv* env )
{
    m_javaMethod_cblStateChanged_state_reason_url_code = env->GetMethodID( getJavaClass(), "cblStateChanged", "(Lcom/amazon/aace/cbl/CBL$CBLState;Lcom/amazon/aace/cbl/CBL$CBLStateChangedReason;Ljava/lang/String;Ljava/lang/String;)V" );
    m_javaMethod_clearRefreshToken = env->GetMethodID( getJavaClass(), "clearRefreshToken", "()V" );
    m_javaMethod_setRefreshToken_refreshToken = env->GetMethodID( getJavaClass(), "setRefreshToken", "(Ljava/lang/String;)V" );
    m_javaMethod_getRefreshToken = env->GetMethodID( getJavaClass(), "getRefreshToken", "()Ljava/lang/String;" );

    // State
    jclass cblStateTypeEnumClass = env->FindClass( "com/amazon/aace/cbl/CBL$CBLState" );
    m_enum_CBLState_STARTING = NativeLib::FindEnum( env, cblStateTypeEnumClass, "STARTING", "Lcom/amazon/aace/cbl/CBL$CBLState;" );
    m_enum_CBLState_REQUESTING_CODE_PAIR = NativeLib::FindEnum( env, cblStateTypeEnumClass, "REQUESTING_CODE_PAIR", "Lcom/amazon/aace/cbl/CBL$CBLState;" );
    m_enum_CBLState_CODE_PAIR_RECEIVED = NativeLib::FindEnum( env, cblStateTypeEnumClass, "CODE_PAIR_RECEIVED", "Lcom/amazon/aace/cbl/CBL$CBLState;" );
    m_enum_CBLState_REFRESHING_TOKEN = NativeLib::FindEnum( env, cblStateTypeEnumClass, "REFRESHING_TOKEN", "Lcom/amazon/aace/cbl/CBL$CBLState;" );
    m_enum_CBLState_REQUESTING_TOKEN = NativeLib::FindEnum( env, cblStateTypeEnumClass, "REQUESTING_TOKEN", "Lcom/amazon/aace/cbl/CBL$CBLState;" );
    m_enum_CBLState_STOPPING = NativeLib::FindEnum( env, cblStateTypeEnumClass, "STOPPING", "Lcom/amazon/aace/cbl/CBL$CBLState;" );

    // Reason
    jclass cblReasonEnumClass = env->FindClass( "com/amazon/aace/cbl/CBL$CBLStateChangedReason" );
    m_enum_CBLStateChangedReason_SUCCESS = NativeLib::FindEnum( env, cblReasonEnumClass, "SUCCESS", "Lcom/amazon/aace/cbl/CBL$CBLStateChangedReason;" );
    m_enum_CBLStateChangedReason_ERROR = NativeLib::FindEnum( env, cblReasonEnumClass, "ERROR", "Lcom/amazon/aace/cbl/CBL$CBLStateChangedReason;" );
    m_enum_CBLStateChangedReason_TIMEOUT = NativeLib::FindEnum( env, cblReasonEnumClass, "TIMEOUT", "Lcom/amazon/aace/cbl/CBL$CBLStateChangedReason;" );
    m_enum_CBLStateChangedReason_CODE_PAIR_EXPIRED = NativeLib::FindEnum( env, cblReasonEnumClass, "CODE_PAIR_EXPIRED", "Lcom/amazon/aace/cbl/CBL$CBLStateChangedReason;" );
    m_enum_CBLStateChangedReason_NONE = NativeLib::FindEnum( env, cblReasonEnumClass, "NONE", "Lcom/amazon/aace/cbl/CBL$CBLStateChangedReason;" );
}

void CBLBinder::cblStateChanged( aace::cbl::CBL::CBLState state, aace::cbl::CBL::CBLStateChangedReason reason, const std::string& url, const std::string& code ) {
    if( getJavaObject() != nullptr && m_javaMethod_cblStateChanged_state_reason_url_code != nullptr ) {
        ThreadContext context;

        if( context.isValid() ) {
            jstring urlStr = context.getEnv()->NewStringUTF( url.c_str() );
            jstring codeStr = context.getEnv()->NewStringUTF( code.c_str() );
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_cblStateChanged_state_reason_url_code, convert( state ), convert( reason ), urlStr, codeStr );
            context.getEnv()->DeleteLocalRef( urlStr );
            context.getEnv()->DeleteLocalRef( codeStr );
        }
    }
}

void CBLBinder::clearRefreshToken() {
    if( getJavaObject() != nullptr && m_javaMethod_clearRefreshToken != nullptr ) {
        ThreadContext context;

        if( context.isValid() ) {
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_clearRefreshToken );
        }
    }
}

void CBLBinder::setRefreshToken( const std::string& refreshToken ) {
    if( getJavaObject() != nullptr && m_javaMethod_setRefreshToken_refreshToken != nullptr ) {
        ThreadContext context;

        if( context.isValid() ) {
            jstring refreshTokenStr = context.getEnv()->NewStringUTF( refreshToken.c_str() );
            context.getEnv()->CallVoidMethod( getJavaObject(), m_javaMethod_setRefreshToken_refreshToken, refreshTokenStr );
            context.getEnv()->DeleteLocalRef( refreshTokenStr );
        }
    }
}

std::string CBLBinder::getRefreshToken() {
    std::string result = "";

    if( getJavaObject() != nullptr && m_javaMethod_getRefreshToken != nullptr ) {
        ThreadContext context;

        if( context.isValid() ) {
            jstring jstr = (jstring) context.getEnv()->CallObjectMethod( getJavaObject(), m_javaMethod_getRefreshToken );

            if( jstr != nullptr ) {
                result = NativeLib::convert( context.getEnv(), jstr );
            }
        }
    }

    return result;
}

jobject CBLBinder::convert( aace::cbl::CBL::CBLState state ) {
    switch( state ) {
        case aace::cbl::CBL::CBLState::STARTING:
            return m_enum_CBLState_STARTING.get();
        case aace::cbl::CBL::CBLState::REQUESTING_CODE_PAIR:
            return m_enum_CBLState_REQUESTING_CODE_PAIR.get();
        case aace::cbl::CBL::CBLState::REFRESHING_TOKEN:
            return m_enum_CBLState_REFRESHING_TOKEN.get();
        case aace::cbl::CBL::CBLState::REQUESTING_TOKEN:
            return m_enum_CBLState_REQUESTING_TOKEN.get();
        case aace::cbl::CBL::CBLState::CODE_PAIR_RECEIVED:
            return m_enum_CBLState_CODE_PAIR_RECEIVED.get();
        case aace::cbl::CBL::CBLState::STOPPING:
            return m_enum_CBLState_STOPPING.get();
    }
}

jobject CBLBinder::convert( aace::cbl::CBL::CBLStateChangedReason reason) {
    switch( reason) {
        case aace::cbl::CBL::CBLStateChangedReason::SUCCESS:
            return m_enum_CBLStateChangedReason_SUCCESS.get();
        case aace::cbl::CBL::CBLStateChangedReason::ERROR:
            return m_enum_CBLStateChangedReason_ERROR.get();
        case aace::cbl::CBL::CBLStateChangedReason::TIMEOUT:
            return m_enum_CBLStateChangedReason_TIMEOUT.get();
        case aace::cbl::CBL::CBLStateChangedReason::CODE_PAIR_EXPIRED:
            return m_enum_CBLStateChangedReason_CODE_PAIR_EXPIRED.get();
        case aace::cbl::CBL::CBLStateChangedReason::NONE:
            return m_enum_CBLStateChangedReason_NONE.get();
    }
}

// JNI
#define CBL(cptr) ((CBLBinder *) cptr)

extern "C" {

JNIEXPORT void JNICALL 
Java_com_amazon_aace_cbl_CBL_start( JNIEnv * env, jobject, jlong cptr ) {
    CBL(cptr)->start();
}

JNIEXPORT void JNICALL 
Java_com_amazon_aace_cbl_CBL_cancel(JNIEnv * env, jobject, jlong cptr) {
    CBL(cptr)->cancel();
}

}
// END OF FILE
