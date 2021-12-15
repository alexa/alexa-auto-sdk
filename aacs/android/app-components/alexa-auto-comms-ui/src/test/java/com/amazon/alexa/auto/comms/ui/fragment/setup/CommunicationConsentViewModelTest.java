package com.amazon.alexa.auto.comms.ui.fragment.setup;

import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.app.Application;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.app.AlexaAppRootComponent;
import com.amazon.alexa.auto.apis.communication.ContactsController;
import com.amazon.alexa.auto.comms.ui.Constants;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class CommunicationConsentViewModelTest {
    private CommunicationConsentViewModel mClassUnderTest;

    @Mock
    Application mMockApplication;
    @Mock
    AlexaApp mMockAlexaApp;
    @Mock
    AlexaAppRootComponent mMockRootComponent;
    @Mock
    ContactsController mContactsController;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        try (MockedStatic<AlexaApp> staticMock = Mockito.mockStatic(AlexaApp.class)) {
            staticMock.when(() -> AlexaApp.from(mMockApplication)).thenReturn(mMockAlexaApp);
            when(mMockAlexaApp.getRootComponent()).thenReturn(mMockRootComponent);
            when(mMockRootComponent.getComponent(ContactsController.class))
                    .thenReturn(Optional.of(mContactsController));

            mClassUnderTest = new CommunicationConsentViewModel(mMockApplication);
        }
    }

    @Test
    public void testUploadContacts() {
        mClassUnderTest.uploadContacts("testID");
        verify(mContactsController, times(1)).uploadContacts("testID");
    }

    @Test
    public void testRemoveContacts() {
        mClassUnderTest.removeContacts("testID");
        verify(mContactsController, times(1)).removeContacts("testID");
    }

    @Test
    public void testSetContactsPermissionWithYes() {
        mClassUnderTest.setContactsUploadPermission("testID", Constants.CONTACTS_PERMISSION_YES);
        verify(mContactsController, times(1)).uploadContacts("testID");
    }

    @Test
    public void testSetContactsPermissionWithSkip() {
        mClassUnderTest.setContactsUploadPermission("testID", Constants.CONTACTS_PERMISSION_NO);
        verify(mContactsController, times(0)).uploadContacts(anyString());
    }
}
