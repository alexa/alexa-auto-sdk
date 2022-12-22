package com.amazon.alexa.auto.setup.workflow.command;

import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.CBL_FLOW_SETUP_COMPLETED;
import static com.amazon.alexa.auto.setup.workflow.event.LoginEvent.PREVIEW_MODE_FLOW_SETUP_COMPLETED;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;

import com.amazon.alexa.auto.apis.auth.AuthController;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apis.login.LoginUIEventListener;
import com.amazon.alexa.auto.apis.setup.AlexaSetupController;
import com.amazon.alexa.auto.setup.workflow.WorkflowMessage;

import org.greenrobot.eventbus.EventBus;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;

import java.lang.reflect.Field;

@RunWith(RobolectricTestRunner.class)
public class SetupCompleteCommandTest {
    @Mock
    Context mMockContext;

    @Mock
    AlexaSetupController mMockAlexaSetupController;

    @Mock
    LoginUIEventListener mMockUIEventListener;

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
    public void testOnAuthModeCBL_publishesCBLCompletedEvent() {
        SetupCompleteCommand command = new SetupCompleteCommand(mMockContext, mMockAlexaSetupController, mMockUIEventListener);
        command.execute();
    }

    @Test
    public void testOnAuthModeAuthProvider_publishesAuthProviderCompletedEvent() {

        SetupCompleteCommand command = new SetupCompleteCommand(mMockContext, mMockAlexaSetupController, mMockUIEventListener);
        command.execute();
    }
}
