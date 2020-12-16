package com.amazon.alexa.auto.voiceinteraction.dependencies;

import com.amazon.alexa.auto.voiceinteraction.service.AutoVoiceInteractionService;
import com.amazon.alexa.auto.voiceinteraction.settings.AACSPreferenceFragment;
import com.amazon.alexa.auto.voiceinteraction.settings.AlexaSettingsHomeFragment;
import com.amazon.alexa.auto.voiceinteraction.settings.AlexaSettingsLanguagesFragment;
import com.amazon.alexa.auto.voiceinteraction.settings.SettingsActivityViewModel;
import com.amazon.alexa.auto.voiceinteraction.settings.SettingsHomeFragment;

import javax.inject.Singleton;

import dagger.Component;

/**
 * Dagger Component for injecting Alexa Settings Dependencies.
 */
@Component(modules = {AndroidModule.class, AACSModule.class, ConfigModule.class})
@Singleton
public interface SettingsComponent {
    /**
     * Inject to @{link SettingsActivityViewModel}.
     *
     * @param viewModel Object whose dependencies are resolved by dagger.
     */
    void inject(SettingsActivityViewModel viewModel);

    /**
     * Inject to @{link SettingsHomeFragment}.
     *
     * @param fragment Object whose dependencies are resolved by dagger.
     */
    void inject(SettingsHomeFragment fragment);

    /**
     * Inject to @{link AlexaSettingsFragment}.
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
}
