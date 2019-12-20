package com.amazon.sampleapp.core;

import android.app.Activity;
import android.support.v4.view.ViewPager;
import com.amazon.aace.audio.AudioOutputProvider;
import java.util.Map;

public class SampleAppContext {
    private final Activity mActivity;
    private final ViewPager mViewPager;
    private final Map<String, String> mData;
    private AudioOutputProvider mAudioOutputProvider;

    public static final String CERTS_DIR = "certsDir";
    public static final String MODEL_DIR = "modelsDir";
    public static final String PRODUCT_DSN = "productDsn";
    public static final String APPDATA_DIR = "appDataDir";
    public static final String JSON = "json";

    public SampleAppContext(Activity activity, Map<String, String> data) {
        mActivity = activity;
        mAudioOutputProvider = null;
        mViewPager = null;
        mData = data;
    }

    public SampleAppContext(Activity activity, ViewPager viewPager, Map<String, String> data) {
        mActivity = activity;
        mViewPager = viewPager;
        mData = data;
    }

    public Activity getActivity() {
        return mActivity;
    }

    public AudioOutputProvider getAudioOutputProvider() {
        return mAudioOutputProvider;
    }

    public ViewPager getViewPager() {
        return mViewPager;
    }

    public String getData( String key) {
        return mData.get(key);
    }

    public void setAudioOutputProvider(AudioOutputProvider outputProvider ) {
        mAudioOutputProvider = outputProvider;
    }
}
