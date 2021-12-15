package com.amazon.alexa.auto.apis.auth

data class AuthStatus (
    val loggedIn : Boolean,
    val userIdentity: UserIdentity? // Not null when logged in is true.
)

data class UserIdentity (
    val userName : String
)