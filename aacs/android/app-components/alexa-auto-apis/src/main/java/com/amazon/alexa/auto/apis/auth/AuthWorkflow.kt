/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.apis.auth

enum class AuthMode {
    CBL_AUTHORIZATION,
    AUTH_PROVIDER_AUTHORIZATION
}

enum class AuthState {
    CBL_Auth_Not_Started,
    CBL_Auth_Started,
    CBL_Auth_CodePair_Received,
    CBL_Auth_Token_Saved,
    CBL_Auth_User_Identity_Saved,
    CBL_Auth_Finished,
    CBL_Auth_Start_Failed,
    Auth_Provider_Request_Authorization,
    Auth_Provider_Authorization_Error,
    Auth_Provider_Authorization_Get_Data,
    Auth_Provider_Authorization_Expired,
    Auth_Provider_Authorized,
    Auth_Provider_Not_Authorized,
    Auth_Provider_Token_Saved,
    Auth_Provider_Authorizing,
    Auth_Provider_Auth_Started,
    Auth_Provider_Logout,
    Alexa_Client_Connected,
    Alexa_Client_Disconnected,
    Alexa_Client_Auth_Unintialized,
    Auth_State_Refreshed
}

data class CodePair (
    val validationUrl : String,
    val validationCode : String
)

data class AuthWorkflowData (
        val authState : AuthState,
        val codePair: CodePair?,
        val authMsgId: String?
)