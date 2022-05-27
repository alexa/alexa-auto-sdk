package com.amazon.alexa.auto.apl.receiver;

import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.spy;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

import com.amazon.alexa.auto.apis.apl.APLTheme;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

@RunWith(RobolectricTestRunner.class)
public class APLThemeReceiverTest {
    @Mock
    private Context mContext;

    private APLThemeReceiver mClassUnderTest;
    private EventBus eventBus;

    private SharedPreferences mSharedPrefs;
    private SharedPreferences.Editor mEditor;

    private String receiveThemePayload;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);
        mClassUnderTest = spy(new APLThemeReceiver());

        mSharedPrefs = Mockito.mock(SharedPreferences.class);
        mEditor = Mockito.mock(SharedPreferences.Editor.class);
        Mockito.when(mContext.getSharedPreferences(anyString(), anyInt())).thenReturn(mSharedPrefs);
        Mockito.when(mSharedPrefs.edit()).thenReturn(mEditor);

        EventBus.getDefault().register(this);
        eventBus = spy(EventBus.getDefault());
    }

    @Test
    public void handleValidAPLDarkThemeUpdateTest() {
        String themeId = "gray";
        Mockito.when(mSharedPrefs.getString("com.amazon.alexa.auto.uiMode", "")).thenReturn("dark");
        Intent getAPLThemeUpdateIntent = generateIntent(themeId);
        mClassUnderTest.onReceive(mContext, getAPLThemeUpdateIntent);
        Assert.assertNotNull(mClassUnderTest.mPayload);
        Assert.assertEquals(receiveThemePayload, mClassUnderTest.generateAPLThemePayload(themeId));
    }

    @Test
    public void handleInvalidAPLDarkThemeUpdateTest() {
        Mockito.when(mSharedPrefs.getString("com.amazon.alexa.auto.uiMode", "")).thenReturn("dark");
        Intent getAPLThemeUpdateIntent = generateIntent("gray1");
        mClassUnderTest.onReceive(mContext, getAPLThemeUpdateIntent);
        Assert.assertNull(mClassUnderTest.mPayload);
    }

    @Test
    public void handleValidAPLLightThemeUpdateTest() {
        String themeId = "gray1";
        Mockito.when(mSharedPrefs.getString("com.amazon.alexa.auto.uiMode", "")).thenReturn("light");
        Intent getAPLThemeUpdateIntent = generateIntent(themeId);
        mClassUnderTest.onReceive(mContext, getAPLThemeUpdateIntent);
        Assert.assertNotNull(mClassUnderTest.mPayload);
        Assert.assertEquals(receiveThemePayload, mClassUnderTest.generateAPLThemePayload(themeId));
    }

    @Test
    public void handleInvalidAPLLightThemeUpdateTest() {
        Mockito.when(mSharedPrefs.getString("com.amazon.alexa.auto.uiMode", "")).thenReturn("light");
        Intent getAPLThemeUpdateIntent = generateIntent("black");
        mClassUnderTest.onReceive(mContext, getAPLThemeUpdateIntent);
        Assert.assertNull(mClassUnderTest.mPayload);
    }

    private Intent generateIntent(String themeId) {
        Intent intent = new Intent("com.amazon.alexa.auto.apl.UpdateAPLTheme");
        intent.putExtra("themeId", themeId);
        return intent;
    }

    @Subscribe
    public void testOnReceiveEvent(APLTheme theme) {
        receiveThemePayload = theme.getThemePayload();
    }
}
