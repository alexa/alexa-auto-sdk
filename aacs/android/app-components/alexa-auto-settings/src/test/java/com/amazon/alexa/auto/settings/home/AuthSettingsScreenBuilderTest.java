package com.amazon.alexa.auto.settings.home;

import android.content.Context;
import android.content.res.Resources;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentManager;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;

import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.app.common.ui.TwoChoiceDialog;
import com.amazon.alexa.auto.settings.config.PreferenceKeys;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.mockito.junit.MockitoJUnitRunner;

@RunWith(MockitoJUnitRunner.class)
public class AuthSettingsScreenBuilderTest {
    // Overriding class under test to mock statics.
    class AuthSettingsScreenBuilderOverride extends AuthSettingsScreenBuilder {
        TwoChoiceDialog.Params mLastDialogCreationParams;

        @Override
        TwoChoiceDialog createTwoChoiceDialog(@NonNull TwoChoiceDialog.Params params) {
            mLastDialogCreationParams = params;
            return mMockChoiceDialog;
        }

        @Override
        AuthController fetchAuthController(@NonNull Context context) {
            return mMockAuthController;
        }

        @Override
        AlexaSetupController fetchAlexaSetupController(@NonNull Context context) {
            return mMockAlexaSetupController;
        }

        @Override
        LoginUIEventListener fetchLoginUIEventListener(@NonNull Context context) {
            return mLoginUIEventListener;
        }
    }

    private AuthSettingsScreenBuilderOverride mClassUnderTest;

    @Mock
    AuthController mMockAuthController;
    @Mock
    AlexaSetupController mMockAlexaSetupController;
    @Mock
    LoginUIEventListener mLoginUIEventListener;
    @Mock
    PreferenceScreen mMockPrefScreen;
    @Mock
    View mMockView;
    @Mock
    AppCompatActivity mMockAppCompatActivity;
    @Mock
    FragmentManager mMockFragmentManager;
    @Mock
    Resources mMockResources;
    @Mock
    Preference mMockPrefSignIn;
    @Mock
    Preference mMockPrefDisable;
    @Mock
    Preference mMockSignout;
    @Mock
    TwoChoiceDialog mMockChoiceDialog;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNIN)).thenReturn(mMockPrefSignIn);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_DISABLE))
                .thenReturn(mMockPrefDisable);
        Mockito.when(mMockPrefScreen.findPreference(PreferenceKeys.ALEXA_SETTINGS_SIGNOUT)).thenReturn(mMockSignout);

        Mockito.when(mMockView.getContext()).thenReturn(mMockAppCompatActivity);
        Mockito.when(mMockAppCompatActivity.getSupportFragmentManager()).thenReturn(mMockFragmentManager);

        Mockito.when(mMockView.getResources()).thenReturn(mMockResources);

        Mockito.when(mMockAuthController.getAuthMode()).thenReturn(AuthMode.CBL_AUTHORIZATION);

        mClassUnderTest = new AuthSettingsScreenBuilderOverride();
    }

    @Test
    public void testPreferencesAreRemovedForNonPreviewMode() {
        mClassUnderTest.addRemovePreferences(mMockPrefScreen);

        Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefSignIn);
        Mockito.verify(mMockPrefScreen, Mockito.times(1)).removePreference(mMockPrefDisable);
    }

    @Test
    public void testSignoutPreference() {
        mClassUnderTest.installEventHandlers(mMockPrefScreen, mMockView);

        ArgumentCaptor<Preference.OnPreferenceClickListener> clickListenerCaptor =
                ArgumentCaptor.forClass(Preference.OnPreferenceClickListener.class);
        Mockito.verify(mMockSignout, Mockito.times(1)).setOnPreferenceClickListener(clickListenerCaptor.capture());

        // Invoke Signout preference by clicking on it.
        Assert.assertTrue(clickListenerCaptor.getValue().onPreferenceClick(mMockSignout));
        Mockito.verify(mMockChoiceDialog, Mockito.times(1)).show(Mockito.eq(mMockFragmentManager), Mockito.anyString());

        // Manually invoke signout handler to simulate event received from dialog.
        mClassUnderTest.onTwoChoiceDialogEvent(
                new TwoChoiceDialog.Button2Clicked(mClassUnderTest.mLastDialogCreationParams.dialogKey));
        Mockito.verify(mMockAuthController, Mockito.times(1)).logOut();
        Mockito.verify(mMockAlexaSetupController, Mockito.times(1)).setSetupCompleteStatus(false);
    }
}
