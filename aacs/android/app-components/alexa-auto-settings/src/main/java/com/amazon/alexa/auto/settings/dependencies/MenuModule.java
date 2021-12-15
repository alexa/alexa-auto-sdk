package com.amazon.alexa.auto.settings.dependencies;

import com.amazon.alexa.auto.settings.home.AlexaSettingsScreenBuilder;
import com.amazon.alexa.auto.settings.home.AuthSettingsScreenBuilder;
import com.amazon.alexa.auto.settings.home.DebugSettingsScreenBuilder;
import com.amazon.alexa.auto.settings.home.VoiceAssistanceSettingsScreenBuilder;

import java.util.ArrayList;
import java.util.List;

import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module for Menu classes.
 */
@Module
public class MenuModule {
    /**
     * Provides list of {@link AlexaSettingsScreenBuilder}.
     *
     */
    @Provides
    @Singleton
    public List<AlexaSettingsScreenBuilder> provideMenuScreenBuilders() {
        // The order is important, later the item, final a say it would have in determining
        // what menu items would be presented on Alexa Settings screen.
        List<AlexaSettingsScreenBuilder> builders = new ArrayList<>();
        builders.add(new AuthSettingsScreenBuilder());
        builders.add(new VoiceAssistanceSettingsScreenBuilder());
        builders.add(new DebugSettingsScreenBuilder());

        return builders;
    }
}
