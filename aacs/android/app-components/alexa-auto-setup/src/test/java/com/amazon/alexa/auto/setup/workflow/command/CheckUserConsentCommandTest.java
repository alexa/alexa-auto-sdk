package com.amazon.alexa.auto.setup.workflow.command;

import static com.amazon.alexa.auto.apps.common.util.ModuleProvider.ModuleName.GEOLOCATION;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.LOCATION_CONSENT_COMPLETED;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.verify;

import android.content.Context;

import com.amazon.alexa.auto.apps.common.util.ModuleProvider;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockedStatic;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.lang.reflect.Field;

@RunWith(RobolectricTestRunner.class)
public class CheckUserConsentCommandTest {
    @Mock
    Context mMockContext;
    @Captor
    ArgumentCaptor<WorkflowMessage> workflowMessageArgumentCaptor;

    @Before
    public void setup() throws Exception {
        MockitoAnnotations.openMocks(this);

        Field field = EventBus.class.getDeclaredField("defaultInstance");
        if (!field.isAccessible())
            field.setAccessible(true);
        field.set(null, Mockito.mock(EventBus.class));
    }

    @Test
    public void testOnGeolocationModuleDoesNotExist_publishSkipEvent() {
        CheckLocationConsentCommand command = new CheckLocationConsentCommand(mMockContext);
        command.execute();

        verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
        assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), LOCATION_CONSENT_COMPLETED);
    }

    @Test
    public void testOnGeolocationModuleExists_publishEventToShowLocationConsentScreen() {
        try (MockedStatic<ModuleProvider> staticMock = Mockito.mockStatic(ModuleProvider.class)) {
            staticMock.when(() -> ModuleProvider.getModules(mMockContext)).thenReturn(GEOLOCATION.name());

            CheckLocationConsentCommand command = new CheckLocationConsentCommand(mMockContext);
            command.execute();

            verify(EventBus.getDefault()).post(workflowMessageArgumentCaptor.capture());
            assertEquals(workflowMessageArgumentCaptor.getValue().getWorkflowEvent(), "Location_Consent");
        }
    }
}
