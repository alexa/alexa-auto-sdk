package com.amazon.alexa.auto.apis.auth

enum class CBLAuthState {
    CBL_Auth_Not_Started,
    CBL_Auth_Started,
    CBL_Auth_CodePair_Received,
    CBL_Auth_Finished
}

data class CodePair (
    val validationUrl : String,
    val validationCode : String
)

data class CBLAuthWorkflowData (
    val authState : CBLAuthState,
    val codePair: CodePair?
)