package com.amazon.alexa.auto.app;

import android.app.Application;
import android.util.Log;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.app.dependencies.AndroidAppModule;
import com.amazon.alexa.auto.app.dependencies.AppComponent;
import com.amazon.alexa.auto.app.dependencies.DaggerAppComponent;
import com.amazon.alexa.auto.apps.common.aacs.AACSServiceController;

/**
 * Application class which also provides component registry to constituent
 * components through implementation of {@link AlexaApp}
 */
public class AutoApplication extends Application implements AlexaApp {
    private static final String TAG = AutoApplication.class.getSimpleName();

    private AppComponent mDaggerAppComponent;

    @Override
    public void onCreate() {
        super.onCreate();

        mDaggerAppComponent =
                DaggerAppComponent.builder().androidAppModule(new AndroidAppModule(getApplicationContext())).build();

        Log.i(TAG, "Alexa Auto App initialized");
    }

    /**
     * Provide Dagger Component for injection into the android modules
     * of this project.
     *
     * @return Dagger application scope object.
     */
    public AppComponent getAppComponent() {
        return mDaggerAppComponent;
    }

    @Override
    public AlexaAppRootComponent getRootComponent() {
        // This method will mostly be used by external projects which
        // would like to get their dependencies from the returned
        // component.
        return mDaggerAppComponent;
    }
}
