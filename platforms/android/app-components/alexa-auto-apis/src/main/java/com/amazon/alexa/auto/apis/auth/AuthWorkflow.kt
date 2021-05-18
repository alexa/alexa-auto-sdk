package com.amazon.alexa.auto.apis.auth

enum class AuthMode {
    CBL_AUTHORIZATION,
    AUTH_PROVIDER_AUTHORIZATION
}

enum class AuthState {
    CBL_Auth_Not_Started,
    CBL_Auth_Started,
    CBL_Auth_CodePair_Received,
    CBL_Auth_Finished,
    CBL_Auth_Start_Failed,
    Auth_Provider_Request_Authorization,
    Auth_Provider_Authorization_Error,
    Auth_Provider_Authorization_Get_Data,
    Auth_Provider_Authorized,
    Auth_Provider_Not_Authorized,
    Auth_Provider_Authorizing,
    Auth_Provider_Auth_Started,
    Auth_Provider_Logout
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