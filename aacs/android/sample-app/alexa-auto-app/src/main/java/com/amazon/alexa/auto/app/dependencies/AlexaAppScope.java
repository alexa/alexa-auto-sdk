package com.amazon.alexa.auto.app.dependencies;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

import javax.inject.Scope;

/**
 * Dagger scope that is active for the lifetime of Alexa App.
 */
@Scope
@Retention(RetentionPolicy.RUNTIME)
public @interface AlexaAppScope {}
