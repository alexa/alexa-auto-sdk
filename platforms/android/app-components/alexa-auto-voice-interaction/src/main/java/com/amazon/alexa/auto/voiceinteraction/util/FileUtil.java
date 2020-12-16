package com.amazon.alexa.auto.voiceinteraction.util;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.apps.common.util.Preconditions;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

import io.reactivex.rxjava3.core.Single;

/**
 * Alexa auto app file util class.
 */
public class FileUtil {
    private static final String TAG = FileUtil.class.getSimpleName();

    private static final String CONFIG_DIR = "config";
    private static final String AACS_CONFIG_FILE = "aacs_config.json";
    private static final String LOCALES_FILE = "locales.json";

    /**
     * Read content of configuration file asynchronously
     *
     * @param context Android Context.
     * @return {@link Single} of {@link String} that is resolved when file
     *          contents are available.
     */
    public static Single<String> readAACSConfigurationAsync(@NonNull Context context) {
        return readAACSConfigurationAsync(context, new Handler(Looper.getMainLooper()));
    }

    /**
     * Read content of configuration file asynchronously
     *
     * @param context Android Context.
     * @param resultHandler Handler thread where result would be dispatched.
     * @return {@link Single} of {@link String} that is resolved when file
     *          contents are available.
     */
    public static Single<String> readAACSConfigurationAsync(@NonNull Context context, @NonNull Handler resultHandler) {
        // Obtain static values early because they can't be fetched from mocked
        // Environment in new thread.
        String externalStorageDir = Environment.getExternalStorageDirectory().getAbsolutePath();

        return Single.create(emitter -> {
            Thread readThread = new Thread(() -> {
                try {
                    Preconditions.checkNotNull(context);

                    String config = FileUtil.readAACSConfiguration(context, externalStorageDir);

                    resultHandler.post(() -> emitter.onSuccess(config));
                } catch (IOException e) {
                    Log.e(TAG, "Failed to read AACS configuration. Error: " + e);
                    emitter.onError(e);
                }
            });

            readThread.setDaemon(true);
            readThread.start();
        });
    }

    /**
     * Reads the content of Alexa locales file.
     *
     * @param context Android Context.
     * @return File contents
     */
    @NonNull
    public static String readLocales(@NonNull Context context) {
        AssetManager assetManager = context.getAssets();
        try {
            return readStream(assetManager.open(LOCALES_FILE));
        } catch (IOException e) {
            Log.e(TAG, "Failed to fetch locales from locales asset file.");
            return "";
        }
    }

    /**
     * Reads the content of AACS configuration file. We will firstly read the config
     * file from external storage, if the config file is not found, we will read the
     * config from default file in assets.
     *
     * @param context Android Context.
     * @return File contents
     */
    @NonNull
    private static String readAACSConfiguration(@NonNull Context context, @NonNull String externalStorageDir)
            throws IOException {
        try {
            File fullPath = new File(externalStorageDir + "/" + AACS_CONFIG_FILE);
            return readStream(new FileInputStream(fullPath));
        } catch (Exception e) {
            Log.d(TAG,
                    String.format("Cannot read %s from external storage. Error: %s", AACS_CONFIG_FILE, e.getMessage()));
        }

        // Fallback.
        AssetManager assetManager = context.getAssets();
        return readStream(assetManager.open(CONFIG_DIR + "/" + AACS_CONFIG_FILE));
    }

    /**
     * Read the content of stream as text string.
     *
     * @param inputStream Input stream.
     * @return Content of input stream as text string.
     */
    private static String readStream(@NonNull InputStream inputStream) throws IOException {
        byte[] buffer = new byte[inputStream.available()];
        inputStream.read(buffer);
        return new String(buffer, StandardCharsets.UTF_8);
    }
}
