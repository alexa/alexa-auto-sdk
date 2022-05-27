package com.amazon.alexa.auto.apps.common.util;

import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_ID;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_VALUE_BLACK;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.THEME_VALUE_GRAY_ONE;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.UI_DARK_THEME;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.UI_LIGHT_THEME;
import static com.amazon.alexa.auto.apps.common.util.UiThemeManager.UI_MODE;

import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.spy;

import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.SensorManager;

import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.greenrobot.eventbus.EventBus;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.observers.TestObserver;

@RunWith(RobolectricTestRunner.class)
public class UiThemeManagerTest {
    private Context mockContext;
    private SensorManager mockSensorManager;
    private AACSMessageSender mockAACSMessageSender;
    private UiThemeManager uiThemeManager;
    private SharedPreferences mLightModeSharedPrefs;
    private SharedPreferences mDarkModeModeSharedPrefs;
    private SharedPreferences mUIModeSharedPrefs;
    private SharedPreferences.Editor mEditor;
    private EventBus eventBus;
    private UiThemeManager.APLThemeDirectiveReceiver mAPLThemeDirectiveReceiver;

    @Before
    public void setup() {
        mockContext = Mockito.mock(Context.class);
        mockSensorManager = Mockito.mock(SensorManager.class);
        mockAACSMessageSender = Mockito.mock(AACSMessageSender.class);
        uiThemeManager = new UiThemeManager(mockContext, mockAACSMessageSender);
        mAPLThemeDirectiveReceiver = Mockito.mock(UiThemeManager.APLThemeDirectiveReceiver.class);
        mLightModeSharedPrefs = Mockito.mock(SharedPreferences.class);
        mDarkModeModeSharedPrefs = Mockito.mock(SharedPreferences.class);
        mUIModeSharedPrefs = Mockito.mock(SharedPreferences.class);
        mEditor = Mockito.mock(SharedPreferences.Editor.class);
        Mockito.when(mockContext.getSharedPreferences(eq(UI_MODE), anyInt())).thenReturn(mUIModeSharedPrefs);
        Mockito.when(mockContext.getSharedPreferences(eq(UI_LIGHT_THEME), anyInt())).thenReturn(mLightModeSharedPrefs);
        Mockito.when(mockContext.getSharedPreferences(eq(UI_DARK_THEME), anyInt()))
                .thenReturn(mDarkModeModeSharedPrefs);
        Mockito.when(mLightModeSharedPrefs.edit()).thenReturn(mEditor);
        Mockito.when(mDarkModeModeSharedPrefs.edit()).thenReturn(mEditor);
        Mockito.when(mUIModeSharedPrefs.edit()).thenReturn(mEditor);
        EventBus.getDefault().register(mAPLThemeDirectiveReceiver);
        eventBus = spy(EventBus.getDefault());
    }

    @Test
    public void test_handle_ui_mode_light() {
        uiThemeManager.init(mockSensorManager);

        // Act
        uiThemeManager.handleAlsUpdate(UiThemeManager.UiModeType.LIGHT);

        // Verify
        Assert.assertEquals(UiThemeManager.UiModeType.LIGHT, uiThemeManager.getCurrentUIMode());
    }

    @Test
    public void test_handle_ui_mode_dark() {
        uiThemeManager.init(mockSensorManager);

        // Act
        uiThemeManager.handleAlsUpdate(UiThemeManager.UiModeType.DARK);

        // Verify
        Assert.assertEquals(UiThemeManager.UiModeType.DARK, uiThemeManager.getCurrentUIMode());
    }

    @Test
    public void test_handle_ui_theme_in_dark_mode() {
        uiThemeManager.init(mockSensorManager);
        Mockito.when(mDarkModeModeSharedPrefs.getString(THEME_ID, "")).thenReturn(THEME_VALUE_BLACK);

        // Act
        uiThemeManager.handleAlsUpdate(UiThemeManager.UiModeType.DARK);

        // Verify
        Assert.assertEquals(THEME_VALUE_BLACK, uiThemeManager.getSavedThemeId(UiThemeManager.UiModeType.DARK));
    }

    @Test
    public void test_handle_ui_theme_in_light_mode() {
        uiThemeManager.init(mockSensorManager);
        Mockito.when(mLightModeSharedPrefs.getString(THEME_ID, "")).thenReturn(THEME_VALUE_GRAY_ONE);

        // Act
        uiThemeManager.handleAlsUpdate(UiThemeManager.UiModeType.LIGHT);

        // Verify
        Assert.assertEquals(THEME_VALUE_GRAY_ONE, uiThemeManager.getSavedThemeId(UiThemeManager.UiModeType.LIGHT));
    }

    @Test
    public void test_get_ui_mode_update_observable() {
        uiThemeManager.init(mockSensorManager);

        Observable<UiThemeManager.UiModeType> uiModeUpdatedObservable = uiThemeManager.getUiModeUpdatedObservable();
        TestObserver<UiThemeManager.UiModeType> testObserver = TestObserver.create();
        uiModeUpdatedObservable.subscribe(testObserver);
        testObserver.assertValue(UiThemeManager.UiModeType.DARK);

        // Act
        uiThemeManager.handleAlsUpdate(UiThemeManager.UiModeType.LIGHT);

        // Verify
        testObserver = TestObserver.create();
        uiModeUpdatedObservable.subscribe(testObserver);
        testObserver.assertValue(UiThemeManager.UiModeType.LIGHT);
        Assert.assertEquals(UiThemeManager.UiModeType.LIGHT, uiThemeManager.getCurrentUIMode());
    }
}
