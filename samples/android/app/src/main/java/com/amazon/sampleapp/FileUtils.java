package com.amazon.sampleapp;

import android.content.res.AssetManager;
import android.util.Log;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.*;

/***
 * Various utility methods for file I/O
 */
public class FileUtils {
    private static final String TAG = FileUtils.class.getSimpleName();

    /**
     * Copy all files in the given subdirectory of the assets directory to the given destination
     * @param am The AssetManager
     * @param subDirName The subdirectory of "assets" containing the files to copy
     * @param destFile The destination to populate
     * @param force Whether the destination file should be created if it does not exist
     */
    public static void copyAllAssets(AssetManager am,
                                     String subDirName,
                                     File destFile,
                                     boolean force) {
        try {
            String[] assets = am.list(subDirName);
            for (String next : assets) {
                copyAsset(am, subDirName + "/" + next, new File(destFile, next), force);
            }
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        }
    }
    /**
     * Copies the contents a file in the assets directory to the given destination file
     * @param am The AssetManager
     * @param assetPath The path of the file in the assets directory from which to copy contents
     * @param destFile The file to populate
     * @param force Whether the destination file should be created if it does not exist
     */
    public static void copyAsset(AssetManager am, String assetPath, File destFile, boolean force ) {
        Log.d(TAG, "Attempting to copy " + assetPath + " into " + destFile.getAbsolutePath());
        try {
            InputStream is = am.open(assetPath);
            copyInputStreamToFile(is, destFile, force);
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * Copies the contents of the given source file to the given destination file
     * @param srcFile The file containing contents to copy
     * @param destFile The file to populate
     * @param force Whether the destination file should be created if it does not exist
     */
    public static void copyFile(File srcFile, File destFile, boolean force) {
        Log.d(TAG, "Attempting to copy " + srcFile.getAbsolutePath() + " into " + destFile.getAbsolutePath());
        try {
            InputStream is = new FileInputStream(srcFile);
            copyInputStreamToFile(is, destFile, force);
        } catch (IOException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * Copies the given source input stream to the given destination file
     */
    public static void copyInputStreamToFile(InputStream srcInputStream, File destFile, boolean force) {
        if (!destFile.exists() || force) {
            if (destFile.getParentFile().exists() || destFile.getParentFile().mkdirs()) {
                try (OutputStream os = new FileOutputStream(destFile)) {
                    byte[] buf = new byte[1024];
                    int len;
                    while ((len = srcInputStream.read(buf)) > 0) {
                        os.write(buf, 0, len);
                    }
                } catch (IOException e) {
                    Log.e(TAG, e.getMessage());
                }
            } else {
                Log.w(TAG, "Cannot copy into destFile at " + destFile.getParentFile());
            }
        } else {
            Log.d(TAG, "Skipping copying into existing destFile " + destFile);
        }
    }

    /**
     * Returns a JSONObject representing the config object keyed by 'configRootKey' from the
     * 'configAssetName' file in the assets directory, or null if the config cannot be read
     */
    public static JSONObject getConfigFromFile(AssetManager am,
                                               String configAssetName,
                                               String configRootKey) {
        JSONObject obj = null;
        try (InputStream is = am.open(configAssetName)) {
            byte[] buffer = new byte[is.available()];
            is.read( buffer );
            String json = new String( buffer, "UTF-8" );
            obj = new JSONObject( json );
        } catch (Exception e) {
            Log.w(TAG, String.format("Cannot read %s from assets directory. Error: %s",
                    configAssetName, e.getMessage()));
        }

        JSONObject config = null;
        if (obj != null) {
            try {
                config = obj.getJSONObject(configRootKey);
            } catch (JSONException e) {
                Log.w(TAG, "No device config specified in " + configAssetName);
            }
        }
        return config;
    }
}
