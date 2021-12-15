package com.amazon.alexaautoclientservice.instrumentedtest;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.InputStream;

public class TestUtil {
    final static String ASSETS_PATH = "file-util-res/";
    final static String AACS_CONFIG_FILE = "aacs_config.json";
    final static String ENGINE_CONFIG_FILE = "auto_sdk_config.json";

    // Make sure these values are set before running the tests
    final static String CLIENT_ID = "";
    final static String PRODUCT_ID = "";
    final static String DSN = "";

    public static JSONObject constructOEMConfigMessage(String[] filepaths, String[] configStrings) throws Exception {
        String files = "";
        String configs = "";
        for (int i = 0; i < filepaths.length; i++) {
            if (i != filepaths.length - 1) {
                files += "\"" + filepaths[i] + "\",";
            } else {
                files += "\"" + filepaths[i] + "\"";
            }
        }

        for (int i = 0; i < configStrings.length; i++) {
            if (i != configStrings.length - 1) {
                configs += "\"" + configStrings[i] + "\",";
            } else {
                configs += "\"" + configStrings[i] + "\"";
            }
        }
        String configMessage = "{\n"
                + "  \"configFilepaths\" : [" + files + "],"
                + "  \"configStrings\" : [" + configs + "]"
                + "}";
        return new JSONObject(configMessage);
    }

    public static JSONObject readConfig(InputStream source) throws Exception {
        JSONObject obj = null;
        InputStream inputStream = source;

        byte[] buffer = new byte[inputStream.available()];
        inputStream.read(buffer);
        String json = new String(buffer, "UTF-8");
        obj = new JSONObject(json);

        if (inputStream != null)
            inputStream.close();

        return obj;
    }

    public static void addDeviceInfoToConfig(JSONObject config, String clientId, String productId, String dsn)
            throws Exception {
        if (config.has("aacs.alexa")) {
            config = config.getJSONObject("aacs.alexa").getJSONObject("deviceInfo");
        } else if (config.has("aace.alexa")) {
            config = config.getJSONObject("aace.alexa").getJSONObject("avsDeviceSDK").getJSONObject("deviceInfo");
        }
        config.remove("clientId");
        config.remove("productId");
        config.remove("deviceSerialNumber");
        config.put("clientId", clientId);
        config.put("productId", productId);
        config.put("deviceSerialNumber", dsn);
    }

    public static String escapeJsonString(JSONObject json) {
        return json.toString().replace("\"", "\\\"");
    }
}
