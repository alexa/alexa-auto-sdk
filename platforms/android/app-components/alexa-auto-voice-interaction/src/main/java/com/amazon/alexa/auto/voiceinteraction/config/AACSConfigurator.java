package com.amazon.alexa.auto.voiceinteraction.config;

import android.content.ComponentName;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.alexa.auto.voiceinteraction.common.Constants;
import com.amazon.alexa.auto.voiceinteraction.util.FileUtil;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;

import io.reactivex.rxjava3.core.Single;

/**
 * A helper object to configure AACS.
 */
public class AACSConfigurator {
    private static final String TAG = AACSConfigurator.class.getSimpleName();

    @NonNull
    private final WeakReference<Context> mContextWk;
    @NonNull
    private final TargetComponent mAACSTarget;
    @NonNull
    private final AACSSender mAACSSender;
    @Nullable
    private final AACSConfigurationPreferences mConfigOverrider;

    /**
     * Constructs an instance of {@link AACSConfigurator}.
     *
     * @param contextWk Android Context.
     * @param aacsSender Helper for sending messages to AACS.
     * @param configOverrider Optional object to override AACS config before it is
     *                        sent to AACS.
     */
    public AACSConfigurator(@NonNull WeakReference<Context> contextWk, @NonNull AACSSender aacsSender,
            @Nullable AACSConfigurationPreferences configOverrider) {
        mContextWk = contextWk;
        mAACSSender = aacsSender;
        mConfigOverrider = configOverrider;

        mAACSTarget = TargetComponent.withComponent(
                new ComponentName(AACSConstants.AACS_PACKAGE_NAME, AACSConstants.AACS_CLASS_NAME),
                TargetComponent.Type.SERVICE);
    }

    /**
     * Configure AACS with default app config.
     */
    public void configureAACSUsingDefaultAppConfig() {
        Log.i(TAG, "Configuring Alexa Client Service with default app config.");

        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        FileUtil.readAACSConfigurationAsync(context).subscribe(this::sendConfigurationMessage);
    }

    /**
     * Configure AACS with preference overrides.
     */
    public void configureAACSWithPreferenceOverrides() {
        Log.i(TAG, "Configuring Alexa Client Service with default app config.");

        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        FileUtil.readAACSConfigurationAsync(context).subscribe(
                config -> { mergeConfigurationWithPreferences(config).subscribe(this::sendConfigurationMessage); });
    }

    /**
     * Send configuration message to AACS with configs JSON.
     *
     * @param configs Configuration to to be sent to AACS.
     */
    private void sendConfigurationMessage(@NonNull String configs) {
        Context context = mContextWk.get();
        Preconditions.checkNotNull(context);

        String configMessage = "{\n"
                + "  \"" + Constants.AACS_CONFIG_FILE_PATH + "\" : [],"
                + "  \"" + Constants.AACS_CONFIG_STRINGS + "\" : [" + configs + "]"
                + "}";

        mAACSSender.sendConfigMessageAnySize(configMessage, mAACSTarget, context);
        Log.i(TAG, "Alexa Client Service configured.");
    }

    private Single<String> mergeConfigurationWithPreferences(@NonNull final String config) {
        return Single.create(emitter -> {
            if (mConfigOverrider == null) {
                emitter.onSuccess(config);
                return;
            }

            Handler handler = new Handler(Looper.getMainLooper());
            Thread ioThread = new Thread(() -> {
                try {
                    JSONObject configObject = new JSONObject(config);
                    mConfigOverrider.updatePreferenceFromConfig(configObject);
                    String updatedConfig = mConfigOverrider.updateConfigFromPreference(configObject).toString();
                    handler.post(() -> emitter.onSuccess(updatedConfig));
                } catch (JSONException exception) {
                    Log.w(TAG, "Error updating configuration from preferences: " + exception);
                    handler.post(() -> emitter.onError(exception));
                }
            });

            ioThread.setDaemon(true);
            ioThread.start();
        });
    }
}
