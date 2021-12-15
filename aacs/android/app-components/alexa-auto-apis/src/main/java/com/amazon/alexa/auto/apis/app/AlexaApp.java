package com.amazon.alexa.auto.apis.app;

import android.app.Activity;
import android.app.Application;
import android.content.Context;

import androidx.fragment.app.Fragment;

import java.util.Optional;

import io.reactivex.rxjava3.annotations.NonNull;

/**
 * Glue interface that provides dependencies to different components
 * participating in making of Alexa App.
 * This interface must be implemented by Application class of Main
 * Alexa App that is packaging all other components.
 */
public interface AlexaApp {
    /**
     * Provides the AlexaApp Root Component from where other
     * components can fetch their dependencies.
     *
     * @return Root Component.
     */
    AlexaAppRootComponent getRootComponent();

    /**
     * Cast Application to AlexaApp
     *
     * @param application Android Application Object.
     * @return AlexaApp object.
     */
    static AlexaApp from(@NonNull Application application) {
        return (AlexaApp) application;
    }

    /**
     * Obtain AlexaApp from Context.
     *
     * @param context Android Context.
     * @return AlexaApp object.
     */
    static AlexaApp from(@NonNull Context context) {
        return (AlexaApp) context.getApplicationContext();
    }

    /**
     * Obtain AlexaApp from Activity.
     *
     * @param activity Android Activity.
     * @return AlexaApp object.
     */
    static AlexaApp from(@NonNull Activity activity) {
        return (AlexaApp) activity.getApplication();
    }

    /**
     * Obtain AlexaApp from Fragment.
     *
     * @param fragment Android Fragment.
     * @return AlexaApp object.
     */
    static AlexaApp from(@NonNull Fragment fragment) {
        return (AlexaApp) fragment.getActivity().getApplication();
    }
}
