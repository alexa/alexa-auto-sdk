package com.amazon.alexa.auto.voiceinteraction.settings;

import static com.amazon.alexa.auto.voiceinteraction.common.Constants.EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN;

import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;
import androidx.appcompat.app.AppCompatActivity;
import androidx.lifecycle.ViewModelProvider;
import androidx.navigation.NavController;
import androidx.navigation.NavDestination;
import androidx.navigation.NavGraph;
import androidx.navigation.fragment.NavHostFragment;

import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.voiceinteraction.R;
import com.amazon.alexa.auto.voiceinteraction.databinding.SettingsActivityLayoutBinding;

/**
 * Activity to Alexa Auto Settings.
 */
public class SettingsActivity extends AppCompatActivity {
    private static final String TAG = SettingsActivity.class.getSimpleName();

    // View Model
    SettingsActivityViewModel mViewModel;

    SettingsActivityLayoutBinding mViewBinding;

    // State.
    private boolean mShouldExitAfterFinishingLogin = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mViewBinding = SettingsActivityLayoutBinding.inflate(getLayoutInflater());

        setContentView(mViewBinding.root);

        if (mViewModel == null) {
            mViewModel = new ViewModelProvider(this).get(SettingsActivityViewModel.class);
        }

        startObservingAuthEvents();
        startObservingNavigationEvents();

        mShouldExitAfterFinishingLogin = getIntent().getBooleanExtra(EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN, false);
    }

    @Override
    protected void onStart() {
        super.onStart();

        setupNavigationBarListener();
    }

    @Override
    protected void onStop() {
        super.onStop();

        // Finish Settings activity when user exits Settings screen
        finish();
    }

    /**
     * Start observation of auth events to switch between login and settings
     * navigation destinations.
     */
    private void startObservingAuthEvents() {
        mViewModel.getAuthState().observe(this, state -> {
            boolean currentNavDestinationIsLogin = isCurrentNavigationDestinationLogin();
            Log.d(TAG,
                    "Auth State changed. state:" + state
                            + " is-current-destination-login:" + currentNavDestinationIsLogin);

            switch (state) {
                case LOGGED_IN:
                    if (mShouldExitAfterFinishingLogin) {
                        Log.i(TAG, "Finishing the main activity after login");
                        finish();
                        return;
                    }
                    if (currentNavDestinationIsLogin) {
                        resetNavigationGraphToOriginalState();
                    }
                    break;
                case LOGGED_OUT:
                    if (!currentNavDestinationIsLogin) {
                        resetNavigationGraphWithLoginAsInitialDestination();
                    }
                    break;
            }
        });
    }

    /**
     * Start observing navigation events and change the navigation bar selection
     * to reflect current navigation destination.
     */
    private void startObservingNavigationEvents() {
        NavController navController = getNavigationController();

        navController.addOnDestinationChangedListener((controller, destination, args) -> {
            mViewBinding.navigationBar.navigationTitleTxtview.setText(destination.getLabel());
        });
    }

    /**
     * Get Navigation controller for the activity.
     *
     * @return Navigation controller.
     */
    @NonNull
    @VisibleForTesting
    private NavController getNavigationController() {
        NavHostFragment navHostFragment =
                (NavHostFragment) getSupportFragmentManager().findFragmentById(R.id.settings_nav_host_fragment);
        Preconditions.checkNotNull(navHostFragment);

        return navHostFragment.getNavController();
    }

    /**
     * Setup event listener to navigate to intended destination based on
     * user's current tab bar selection.
     */
    private void setupNavigationBarListener() {
        mViewBinding.navigationBar.navigateBackButton.setOnClickListener(view -> {
            if (getNavigationController().getCurrentDestination().getId() == R.id.navigation_fragment_settings_home) {
                startActivity(new Intent(Settings.ACTION_VOICE_INPUT_SETTINGS));
            } else {
                getNavigationController().popBackStack();
            }
        });
    }

    /**
     * Check if the current navigation destination is Login View.
     *
     * @return true if current navigation destination is Login View.
     */
    private boolean isCurrentNavigationDestinationLogin() {
        NavController navController = getNavigationController();
        NavDestination currentDestination = navController.getCurrentDestination();

        return currentDestination != null && currentDestination.getId() == R.id.navigation_fragment_login;
    }

    /**
     * Reset the navigation graph and make the initial destination as login view.
     * Also clears up the back-stack so that user cannot go back to the view when
     * logout event was detected.
     */
    private void resetNavigationGraphWithLoginAsInitialDestination() {
        Log.i(TAG, "Switching navigation graph's destination to Login");

        NavController controller = getNavigationController();
        NavGraph graph = controller.getNavInflater().inflate(R.navigation.settings_navigation);
        graph.setStartDestination(R.id.navigation_fragment_login);
        controller.setGraph(graph);
    }

    /**
     * Reset the navigation graph to original destination as recorded in
     * main_navigation.xml.
     * Also clears up the back-stack.
     */
    private void resetNavigationGraphToOriginalState() {
        Log.i(TAG, "Resetting navigation graph to original state.");

        NavController controller = getNavigationController();
        NavGraph graph = controller.getNavInflater().inflate(R.navigation.settings_navigation);
        controller.setGraph(graph);
    }
}
