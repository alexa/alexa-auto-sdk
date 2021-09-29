package com.amazon.alexa.auto.setup.workflow.fragment;

import android.app.Application;
import android.content.Intent;
import android.provider.Settings;

import androidx.annotation.NonNull;
import androidx.lifecycle.AndroidViewModel;

/**
 * ViewModel for @{link NetworkFragment}
 */
public class NetworkViewModel extends AndroidViewModel {
    private static final String TAG = NetworkViewModel.class.getSimpleName();

    /**
     * Constructor for NetworkViewModel.
     *
     * @param application Application object from where the view model will
     *                    fetch dependencies.
     */
    public NetworkViewModel(@NonNull Application application) {
        super(application);
    }

    public void goToNetworkSettings() {
        Intent intent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        getApplication().getApplicationContext().startActivity(intent);
    }

}
