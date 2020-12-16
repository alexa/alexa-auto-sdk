package com.amazon.alexa.auto.voiceinteraction.session;

import android.view.ViewGroup;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.observers.TestObserver;

public class SessionViewControllerImplTest {
    private SessionViewControllerImpl mClassUnderTest;

    @Before
    public void setup() {
        mClassUnderTest = new SessionViewControllerImpl();
    }

    @Test
    public void testAddViewGroup() {
        ViewGroup viewGroup = Mockito.mock(ViewGroup.class);
        mClassUnderTest.setSessionView(viewGroup);

        ViewGroup returnViewGroup = mClassUnderTest.getTemplateRuntimeViewContainer().get();
        Assert.assertEquals(viewGroup, returnViewGroup);
    }

    @Test
    public void testTemplateRuntimeDisplayedRunnable() {
        ViewGroup viewGroup = Mockito.mock(ViewGroup.class);
        mClassUnderTest.setSessionView(viewGroup);

        Observable<Boolean> templateDisplayedObservable = mClassUnderTest.getTemplateDisplayedObservable();
        TestObserver<Boolean> testObserver = TestObserver.create();
        templateDisplayedObservable.subscribe(testObserver);

        testObserver.assertValue(false);

        mClassUnderTest.getTemplateRuntimeViewContainer();

        testObserver = TestObserver.create();
        templateDisplayedObservable.subscribe(testObserver);

        testObserver.assertValue(true);

        mClassUnderTest.clearTemplate();

        testObserver = TestObserver.create();
        templateDisplayedObservable.subscribe(testObserver);

        testObserver.assertValue(false);
    }

    @Test
    public void testIsSessionActive() {
        Assert.assertFalse(mClassUnderTest.isSessionActive());

        ViewGroup viewGroup = Mockito.mock(ViewGroup.class);
        mClassUnderTest.setSessionView(viewGroup);

        Assert.assertTrue(mClassUnderTest.isSessionActive());

        mClassUnderTest.setSessionView(null);

        Assert.assertFalse(mClassUnderTest.isSessionActive());
    }
}
