package com.amazon.alexa.auto.app.setup;

import static com.amazon.alexa.auto.app.Constants.EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN;

import android.content.Context;
import android.content.Intent;
import android.provider.Settings;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.voiceinteraction.settings.SettingsActivity;

import java.lang.ref.WeakReference;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Implementation for {@link AlexaSetupController}.
 */
public class AlexaSetupControllerImpl implements AlexaSetupController {
    private final WeakReference<Context> mContextWk;
    private final BehaviorSubject<Boolean> mAlexaSelectedVASubject;

    /**
     * Constructs an instance of AlexaSetupControllerImpl.
     *
     * @param contextWk Android Context.
     */
    public AlexaSetupControllerImpl(@NonNull WeakReference<Context> contextWk) {
        mContextWk = contextWk;
        mAlexaSelectedVASubject = BehaviorSubject.create();
        mAlexaSelectedVASubject.onNext(isAlexaCurrentlySelectedVoiceAssistant());
    }

    @Override
    public boolean isAlexaCurrentlySelectedVoiceAssistant() {
        // TODO: use RoleManager for Android Q to determine voice assistant state.
        // TODO: handshake using APIs (or other means) to determine if Alexa VIS is active.
        return true; // Assuming Alexa VA is active.
    }

    @Override
    public Observable<Boolean> observeVoiceAssistantSelection() {
        return mAlexaSelectedVASubject;
    }

    @Override
    public Intent createIntentForLaunchingVoiceAssistantSwitchUI() {
        return new Intent(Settings.ACTION_VOICE_INPUT_SETTINGS);
    }

    @Override
    public Intent createIntentForLaunchingLoginUI() {
        // Todo: should we verify that we are indeed in logged out state?
        Intent intent = new Intent(mContextWk.get(), SettingsActivity.class);
        intent.putExtra(EXTRAS_SHOULD_EXIT_ACTIVITY_AFTER_LOGIN, true);
        return intent;
    }
}
