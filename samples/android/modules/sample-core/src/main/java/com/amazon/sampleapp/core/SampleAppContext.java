package com.amazon.sampleapp.core;

import android.app.Activity;
import android.support.v4.view.ViewPager;

import com.amazon.aace.audio.AudioOutputProvider;

import org.json.JSONObject;

public interface SampleAppContext {
    String CERTS_DIR = "certsDir";
    String MODEL_DIR = "modelsDir";
    String PRODUCT_DSN = "productDsn";
    String APPDATA_DIR = "appDataDir";
    String JSON = "json";

    Activity getActivity();

    AudioOutputProvider getAudioOutputProvider();

    ViewPager getViewPager();

    String getData(String key);

    JSONObject getConfigFromFile(String configAssetName, String configRootKey);
}
