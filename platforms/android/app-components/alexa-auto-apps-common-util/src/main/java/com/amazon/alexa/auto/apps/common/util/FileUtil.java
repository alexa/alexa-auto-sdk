package com.amazon.alexa.auto.apps.common.util;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;

import io.reactivex.rxjava3.core.Single;

import static com.amazon.alexa.auto.apps.common.Constants.MODELS;

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
        return readAACSConfigurationAsync(
                context, resultHandler, Environment.getExternalStorageDirectory().getAbsolutePath());
    }

    /**
     * Read content of configuration file asynchronously
     *
     * @param context Android Context.
     * @param resultHandler Handler thread where result would be dispatched.
     * @param externalStorageDir Path to external storage directory.
     * @return {@link Single} of {@link String} that is resolved when file
     *          contents are available.
     */
    public static Single<String> readAACSConfigurationAsync(
            @NonNull Context context, @NonNull Handler resultHandler, @NonNull String externalStorageDir) {
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
     * Copy wake word models to files directory
     * @param context Android context.
     */
    public static void copyModelsToFilesDir(Context context) {
        File modelsDir = new File(context.getFilesDir(), MODELS);
        if (!modelsDir.exists()) {
            if (!modelsDir.mkdir()) {
                Log.e(TAG, "Error creating models directory.");
                return;
            }
            try {
                AssetManager assetManager = context.getAssets();
                String[] modelAssets = assetManager.list(MODELS);
                if (modelAssets != null) {
                    for (String next : modelAssets) {
                        if (copyFileFromAssetPath(MODELS + "/" + next, new File(modelsDir, next), false, assetManager))
                            continue;
                        return;
                    }
                }
            } catch (IOException e) {
                Log.e(TAG, String.format("Error while copying models from assets into filesDir. Error: %s", e.getMessage()));
            }
        }
    }

    /**
     * Copy file to destination path
     * @param assetPath The file path to the asset.
     * @param destFile The destination file.
     * @param force True if coping even if the file already exists. False otherwise.
     * @param assetManager The Android Asset Manager.
     * @return True if the operation succeeds. False otherwise.
     */
    private static boolean copyFileFromAssetPath(
            String assetPath, File destFile, boolean force, AssetManager assetManager) {
        if (!destFile.exists() || force) {
            if (destFile.getParentFile().exists() || destFile.getParentFile().mkdirs()) {
                // Copy the asset to the dest path
                try (InputStream is = assetManager.open(assetPath); OutputStream os = new FileOutputStream(destFile)) {
                    byte[] buf = new byte[1024];
                    int len;
                    while ((len = is.read(buf)) > 0) {
                        os.write(buf, 0, len);
                    }
                } catch (IOException e) {
                    Log.e(TAG, "Could not copy file " + assetPath);
                    return false;
                }
            } else {
                Log.e(TAG, "Could not create directory: " + destFile.getParentFile());
                return false;
            }
        } else {
            Log.w(TAG, String.format("Skipping existing file in : %s to: %s", assetPath, destFile));
        }
        return true;
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
