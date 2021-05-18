package com.amazon.alexa.auto.setup.workflow.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.Fragment;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;

import com.amazon.alexa.auto.apps.common.util.config.LocalesProvider;
import com.amazon.alexa.auto.setup.R;
import com.amazon.alexa.auto.setup.dependencies.AndroidModule;
import com.amazon.alexa.auto.setup.dependencies.DaggerSetupComponent;

import javax.inject.Inject;

/**
 * Fragment for Alexa Language Selection.
 */
public class LanguageSelectionFragment extends Fragment {
    private static final String TAG = LanguageSelectionFragment.class.getSimpleName();

    @Inject
    LocalesProvider mLocalesProvider;

    private NavController mController;

    @VisibleForTesting
    Bundle mShowContinueButton;

    /**
     * Constructs an instance of LanguageSelectionFragment.
     */
    public LanguageSelectionFragment() {}

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mShowContinueButton = new Bundle();
        mShowContinueButton.putBoolean("showContinueButton", true);

        DaggerSetupComponent.builder()
                .androidModule(new AndroidModule(getContext()))
                .build()
                .injectLanguageSettingsFragment(this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.start_language_selection, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        View fragmentView = requireView();
        mController = findNavController(fragmentView);

        String currentLocale = mLocalesProvider.getCurrentDeviceLocaleDisplayName();
        TextView languageSelectionBodyText = fragmentView.findViewById(R.id.language_selection_body_text_view);
        String format = getResources().getString(R.string.select_language_body);
        String bodyString = String.format(format, currentLocale);
        languageSelectionBodyText.setText(bodyString);

        TextView getStartedButtonText = fragmentView.findViewById(R.id.select_language_action_button);

        getStartedButtonText.setOnClickListener(
                view -> { mController.navigate(R.id.navigation_fragment_languageSelection, mShowContinueButton); });
    }

    @VisibleForTesting
    NavController findNavController(@NonNull View view) {
        return Navigation.findNavController(view);
    }
}
