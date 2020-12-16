package com.amazon.alexa.auto.login;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Application;

import androidx.arch.core.executor.testing.InstantTaskExecutorRule;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppLoggedOutScopedComponent;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.app.AlexaAppScopedComponents;
import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.CBLAuthState;
import com.amazon.alexa.auto.apis.auth.CBLAuthWorkflowData;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestRule;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.Optional;

import io.reactivex.rxjava3.subjects.PublishSubject;

public class CBLLoginViewModelTest {
    private CBLLoginViewModel mClassUnderTest;

    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaAppRootComponent mMockRootComponent;
    @Mock
    AuthController mMockAuthController;
    @Mock
    AlexaAppScopedComponents mMockScopedComponents;
    @Mock
    AlexaAppLoggedOutScopedComponent mMockLoggedOutComponent;
    @Mock
    LoginUIEventListener mMockLoginHostBinding;

    PublishSubject<CBLAuthWorkflowData> mAuthWorkflowSubject = PublishSubject.create();

    @Rule
    public TestRule rule = new InstantTaskExecutorRule();

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
            when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
            when(mMockRootComponent.getAuthController()).thenReturn(mMockAuthController);
            when(mMockRootComponent.getScopedComponents()).thenReturn(mMockScopedComponents);
            when(mMockScopedComponents.getComponent(AlexaAppLoggedOutScopedComponent.class))
                    .thenReturn(Optional.of(mMockLoggedOutComponent));
            when(mMockLoggedOutComponent.getLoginUIEventListener()).thenReturn(mMockLoginHostBinding);
            when(mMockAuthController.newAuthenticationWorkflow()).thenReturn(mAuthWorkflowSubject);

            mClassUnderTest = new CBLLoginViewModel(mMockApplication);
        }
    }

    @Test
    public void testStartLoginStartsNewAuthWorkflow() {
        mClassUnderTest.starLogin();

        verify(mMockAuthController, times(1)).newAuthenticationWorkflow();
    }

    @Test
    public void testLoginWorkflowStateChangesArePropagatedToView() {
        mClassUnderTest.starLogin();

        CBLAuthWorkflowData authStarted = new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Started, null);
        mAuthWorkflowSubject.onNext(authStarted);

        assertEquals(authStarted, mClassUnderTest.loginWorkflowState().getValue());
    }

    @Test
    public void testOnLoginFinishHostBindingIsNotified() {
        mClassUnderTest.starLogin();

        mClassUnderTest.userFinishedLogin();

        verify(mMockLoginHostBinding, times(1)).loginFinished();
    }

    @Test
    public void testAfterClearingViewModelNoNotificationsAreSent() {
        mClassUnderTest.starLogin();
        mClassUnderTest.onCleared();

        CBLAuthWorkflowData authStarted = new CBLAuthWorkflowData(CBLAuthState.CBL_Auth_Started, null);
        mAuthWorkflowSubject.onNext(authStarted);

        assertNull(mClassUnderTest.loginWorkflowState().getValue());
    }
}
