package com.amazon.alexa.auto.settings.dependencies;

import com.amazon.alexa.auto.settings.AACSPreferenceFragment;
import com.amazon.alexa.auto.settings.AlexaSettingsLanguagesFragment;
import com.amazon.alexa.auto.settings.AlexaSoundPreferencesFragment;
import com.amazon.alexa.auto.settings.SettingsActivityViewModel;
import com.amazon.alexa.auto.settings.home.AlexaSettingsHomeFragment;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Alexa Settings Dependencies.
 */
@Component(modules = {AndroidModule.class, AACSModule.class, ConfigModule.class, MenuModule.class})
@Singleton
public interface SettingsComponent {
    /**
     * Inject to @{link SettingsActivityViewModel}.
     *
     * @param viewModel Object whose dependencies are resolved by dagger.
     */
    void inject(SettingsActivityViewModel viewModel);

    /**
     * Inject to @{link AlexaSettingsHomeFragment}.
     *
     * @param fragment Object whose dependencies are resolved by dagger.
     */
    void inject(AlexaSettingsHomeFragment fragment);

    /**
     * Inject to @{link AlexaSettingsLanguagesFragment}.
     *
     * @param fragment Object whose dependencies are resolved by dagger.
     */
    void inject(AlexaSettingsLanguagesFragment fragment);

    /**
     * Inject to @{link AACSPreferenceFragment}.
     *
     * @param fragment Object whose dependencies are resolved by dagger.
     */
    void inject(AACSPreferenceFragment fragment);

    /**
     * Inject to @{link AlexaSoundPreferencesFragment}.
     *
     * @param fragment Object whose dependencies are resolved by dagger.
     */
    void inject(AlexaSoundPreferencesFragment fragment);
}
