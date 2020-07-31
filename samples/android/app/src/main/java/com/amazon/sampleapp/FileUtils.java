package com.amazon.sampleapp;

import android.content.res.AssetManager;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.util.Log;

import org.json.JSONArray;
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
    public static void copyAllAssets(AssetManager am, String subDirName, File destFile, boolean force) {
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
    public static void copyAsset(AssetManager am, String assetPath, File destFile, boolean force) {
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
    public static JSONObject getConfigFromFile(AssetManager am, String configAssetName, String configRootKey) {
        JSONObject obj = null;
        try (InputStream is = am.open(configAssetName)) {
            byte[] buffer = new byte[is.available()];
            is.read(buffer);
            String json = new String(buffer, "UTF-8");
            obj = new JSONObject(json);
        } catch (Exception e) {
            Log.w(TAG,
                    String.format("Cannot read %s from assets directory. Error: %s", configAssetName, e.getMessage()));
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

    /**
     * Returns a JSONObject representing the config object keyed by 'configRootKey' from the
     * 'configFileName' file in the SD Card directory, or null if the config cannot be read
     */
    @Nullable
    public static JSONObject getOptionalConfigFromSDCard(String configFileName, String configRootKey) {
        JSONObject config = null;
        File file = new File(Environment.getExternalStorageDirectory(), configFileName);
        if (file.exists()) {
            try (InputStream is = new FileInputStream(file)) {
                byte[] buffer = new byte[is.available()];
                is.read(buffer);
                String json = new String(buffer, "UTF-8");
                config = new JSONObject(json).optJSONObject(configRootKey);
            } catch (IOException e) {
                Log.w(TAG, String.format("Cannot read %s from sd card directory", configFileName), e);
            } catch (JSONException e) {
                Log.w(TAG, "Invalid config in " + configFileName, e);
            }
        }

        return config;
    }

    /**
     * Opens the file path specified and returns the content as a JSON object.
     *
     * @param filePath The path to the JSON data.
     * @return JSONObject The JSON object in the file.
     * @throws IOException
     * @throws JSONException
     */
    public static JSONObject parseFileAsJSONObject(String filePath) throws IOException, JSONException {
        Log.i(TAG, String.format("Parsing JSON from %s", filePath));

        // Get file buffer
        byte[] buffer = getFileBuffer(filePath);

        // Create object
        String json = new String(buffer, "UTF-8");
        JSONObject obj = new JSONObject(json);

        return obj;
    }

    /**
     * Opens the file path specified and returns the content as a JSON array.
     *
     * @param filePath The path to the JSON data.
     * @return JSONArray The JSON array in the file.
     * @throws IOException
     * @throws JSONException
     */
    public static JSONArray parseFileAsJSONArray(String filePath) throws IOException, JSONException {
        Log.i(TAG, String.format("Parsing JSON from %s", filePath));

        // Get file buffer
        byte[] buffer = getFileBuffer(filePath);

        // Create JSONArray
        String json = new String(buffer, "UTF-8");
        JSONArray arr = new JSONArray(json);

        return arr;
    }

    /**
     * Returns byte buffer with file contents.
     * @param filePath The file path.
     * @return byte[] The buffer with the file contents.
     * @throws IOException
     */
    private static byte[] getFileBuffer(String filePath) throws IOException {
        // Create file and buffer
        File file = new File(filePath);
        FileInputStream is = new FileInputStream(file);
        byte[] buffer = new byte[is.available()];

        // Read the file
        is.read(buffer);
        return buffer;
    }
}
