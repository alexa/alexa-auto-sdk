package com.amazon.alexa.auto.setup.workflow;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import android.content.Context;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.lang.ref.WeakReference;
import java.util.Optional;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Config
@RunWith(RobolectricTestRunner.class)
public class WorkflowProviderTest {
    private WorkflowProvider mClassUnderTest;

    private static final String SPECIFICATION_DIR = "workflowSpecification";
    private static final String LOGIN_WORKFLOW_SPECIFICATION_FILE = "CBLLoginWorkflowSpecification.json";

    private ExecutorService executorService;

    @Mock
    private Context mMockContext;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        executorService = Executors.newSingleThreadExecutor();
        mClassUnderTest = new WorkflowProvider(new WeakReference<>(mMockContext), executorService);
    }

    @Test
    public void testFetchFirstWorkflow() {
        String loginWorkflowSpecification =
                TestResourceFileReader.readFileContent(SPECIFICATION_DIR + "/" + LOGIN_WORKFLOW_SPECIFICATION_FILE);

        Optional<Workflow> workflowOptional =
                mClassUnderTest.fetchWorkflowSpecificationFromFile(loginWorkflowSpecification);
        workflowOptional.ifPresent(workflow -> mClassUnderTest.mWorkflowSpecification = workflow);

        assertTrue(workflowOptional.isPresent());
        assertTrue(mClassUnderTest.getWorkflowStartingStep().isPresent());
        Assert.assertEquals(mClassUnderTest.getWorkflowStartingStep().get().getEvent(), "Start_Login");
    }

    @Test
    public void testFetchNextWorkflowWithState() {
        String loginWorkflowSpecification =
                TestResourceFileReader.readFileContent(SPECIFICATION_DIR + "/" + LOGIN_WORKFLOW_SPECIFICATION_FILE);

        Optional<Workflow> workflowOptional =
                mClassUnderTest.fetchWorkflowSpecificationFromFile(loginWorkflowSpecification);
        workflowOptional.ifPresent(workflow -> mClassUnderTest.mWorkflowSpecification = workflow);
        assertTrue(workflowOptional.isPresent());
        assertTrue(mClassUnderTest.nextWorkflowStep("Network_Connected").isPresent());

        WorkflowStep ExpectedStep = new WorkflowStep("Network_Connected", "view", "navigation_fragment_login");
        WorkflowStep stepToVerify = mClassUnderTest.nextWorkflowStep("Network_Connected").get();
        Assert.assertEquals(stepToVerify.getEvent(), ExpectedStep.getEvent());
        Assert.assertEquals(stepToVerify.getResource(), ExpectedStep.getResource());
        Assert.assertEquals(stepToVerify.getResource(), ExpectedStep.getResource());
    }

    @Test
    public void testFetchWorkflowSpecificationWithInvalidFile() {
        Optional<Workflow> workflowOptional = mClassUnderTest.fetchWorkflowSpecificationFromFile("This is a test file");
        assertFalse(workflowOptional.isPresent());
    }
}
