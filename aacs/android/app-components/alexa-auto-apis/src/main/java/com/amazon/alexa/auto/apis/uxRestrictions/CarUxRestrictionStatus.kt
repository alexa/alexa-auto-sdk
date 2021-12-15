package com.amazon.alexa.auto.apis.uxRestrictions

data class CarUxRestrictionStatus (
    val isRequiredUXRestriction : Boolean,
    val actionRestriction: CarUxRestriction?
)

data class CarUxRestriction (
    val uxRestrictionName : String
)