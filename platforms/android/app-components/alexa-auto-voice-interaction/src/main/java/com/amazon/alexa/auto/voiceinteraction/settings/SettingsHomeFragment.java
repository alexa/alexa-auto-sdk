package com.amazon.alexa.auto.voiceinteraction.settings;

import android.app.AlertDialog;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.voiceinteraction.R;
import com.amazon.alexa.auto.voiceinteraction.databinding.SettingsHomeLayoutBinding;
import com.amazon.alexa.auto.voiceinteraction.dependencies.AndroidModule;
import com.amazon.alexa.auto.voiceinteraction.dependencies.DaggerSettingsComponent;

public class SettingsHomeFragment extends Fragment {
    private SettingsHomeLayoutBinding mLayoutBinding;

    AuthController mAuthController;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        mLayoutBinding = SettingsHomeLayoutBinding.inflate(inflater);

        DaggerSettingsComponent.builder().androidModule(new AndroidModule(getContext())).build().inject(this);

        AlexaApp app = AlexaApp.from(this);
        mAuthController = app.getRootComponent().getAuthController();

        return mLayoutBinding.getRoot();
    }

    @Override
    public void onViewCreated(@NonNull View v, @Nullable Bundle savedInstanceState) {
        mLayoutBinding.settingsAlexa.setOnClickListener(view -> {
            NavController navController = Navigation.findNavController(view);
            navController.navigate(R.id.navigation_fragment_alexa_settings_home);
        });

        mLayoutBinding.settingsAacs.setOnClickListener(view -> {
            NavController navController = Navigation.findNavController(view);
            navController.navigate(R.id.navigation_fragment_aacs_preferences);
        });

        mLayoutBinding.settingsLogout.setOnClickListener(view -> {
            new AlertDialog.Builder(getContext())
                    .setTitle(R.string.logout)
                    .setMessage(R.string.logout_confirm)
                    .setPositiveButton(R.string.confirm_yesno_yes, (dialog, btn) -> { mAuthController.logOut(); })
                    .setNegativeButton(R.string.confirm_yesno_no, null)
                    .show();
        });
    }
}
