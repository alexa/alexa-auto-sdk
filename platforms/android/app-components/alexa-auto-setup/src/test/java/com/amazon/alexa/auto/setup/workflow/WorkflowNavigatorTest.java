package com.amazon.alexa.auto.setup.workflow;

import android.content.Context;

import androidx.navigation.NavController;

import com.amazon.alexa.auto.setup.workflow.event.LoginEvent;

import org.greenrobot.eventbus.EventBus;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.ref.WeakReference;
import java.util.Optional;

import io.reactivex.rxjava3.core.Single;

@Config
@RunWith(RobolectricTestRunner.class)
public class WorkflowNavigatorTest {
    private WorkflowNavigator mClassUnderTest;
    private Workflow mockWorkFlow;

    @Mock
    Context mMockContext;
    @Mock
    NavController mockNavController;
    @Mock
    WorkflowProvider mMockWorkflowProvider;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        mockWorkFlow = new Workflow();
        mockWorkFlow.addWorkflowStep("Network_Connected", "view", "navigation_fragment_network");
        mockWorkFlow.addWorkflowStep("CBL_Start", "view", "navigation_fragment_cblStart");
        mMockWorkflowProvider.mWorkflowSpecification = mockWorkFlow;
        mClassUnderTest = new WorkflowNavigator(new WeakReference<>(mMockContext), mMockWorkflowProvider);
    }

    @Test
    public void testStartNavigation() {
        Workflow workflow = new Workflow();
        Mockito.when(mMockWorkflowProvider.readWorkflowSpecificationAsync(""))
                .thenReturn(Single.just(Optional.of(workflow)));
        WorkflowStep mockFirstStep = new WorkflowStep("mockFirstStep", "view", "navigation_fragment_login");
        Mockito.when(mMockWorkflowProvider.getWorkflowStartingStep()).thenReturn(Optional.of(mockFirstStep));
        Mockito.when(mMockWorkflowProvider.getResourceIdFromWorkflowStep(mockFirstStep)).thenReturn(1);

        mClassUnderTest.startNavigation(mockNavController, "", null);
        Mockito.verify(mMockWorkflowProvider, Mockito.times(1)).getWorkflowStartingStep();
        Mockito.verify(mockNavController, Mockito.times(1)).navigate(1, mClassUnderTest.mSupportedFeatures);
    }

    @Test
    public void testStartNavigationWithStartStepOverride() {
        Workflow workflow = new Workflow();
        Mockito.when(mMockWorkflowProvider.readWorkflowSpecificationAsync(""))
                .thenReturn(Single.just(Optional.of(workflow)));
        WorkflowStep mockFirstStepOverride = new WorkflowStep("CBL_Start", "view", "navigation_fragment_cblStart");
        Mockito.when(mMockWorkflowProvider.nextWorkflowStep("CBL_Start"))
                .thenReturn(Optional.of(mockFirstStepOverride));
        Mockito.when(mMockWorkflowProvider.getResourceIdFromWorkflowStep(mockFirstStepOverride)).thenReturn(3);

        mClassUnderTest.startNavigation(mockNavController, "", "CBL_Start");
        Mockito.verify(mockNavController, Mockito.times(1)).navigate(3, mClassUnderTest.mSupportedFeatures);
    }

    @Test
    public void testNavigateToNextStepWithWorkflowEvent() {
        Workflow workflow = new Workflow();
        Mockito.when(mMockWorkflowProvider.readWorkflowSpecificationAsync(""))
                .thenReturn(Single.just(Optional.of(workflow)));
        WorkflowStep mockFirstStep = new WorkflowStep("mockFirstStep", "view", "navigation_fragment_login");
        Mockito.when(mMockWorkflowProvider.getWorkflowStartingStep()).thenReturn(Optional.of(mockFirstStep));
        Mockito.when(mMockWorkflowProvider.getResourceIdFromWorkflowStep(mockFirstStep)).thenReturn(1);

        mClassUnderTest.startNavigation(mockNavController, "", null);

        Mockito.verify(mockNavController, Mockito.times(1)).navigate(1, mClassUnderTest.mSupportedFeatures);

        Mockito.when(mMockWorkflowProvider.nextWorkflowStep("Network_Connected"))
                .thenReturn(Optional.of(mockWorkFlow.getWorkflowStep("Network_Connected")));
        Mockito.when(mMockWorkflowProvider.getResourceIdFromWorkflowStep(
                             mockWorkFlow.getWorkflowStep("Network_Connected")))
                .thenReturn(2);

        EventBus.getDefault().post(new WorkflowMessage(LoginEvent.NETWORK_CONNECTED_EVENT));

        Mockito.verify(mockNavController, Mockito.times(1)).navigate(2, mClassUnderTest.mSupportedFeatures);
    }
}
