package com.amazon.alexa.auto.voice.ui.session;

import androidx.fragment.app.Fragment;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.observers.TestObserver;

public class SessionActivityControllerImplTest {
    private SessionActivityControllerImpl mClassUnderTest;

    @Before
    public void setup() {
        mClassUnderTest = new SessionActivityControllerImpl();
    }

    @Test
    public void testAddFragment() {
        Fragment fragment = Mockito.mock(Fragment.class);
        mClassUnderTest.addFragment(fragment);

        Fragment returnFragment = mClassUnderTest.getFragment();
        Assert.assertEquals(fragment, returnFragment);
    }

    @Test
    public void testVoiceFragmentDisplayedRunnable() {
        Fragment fragment = Mockito.mock(Fragment.class);

        Observable<Boolean> fragmentDisplayedObservable = mClassUnderTest.getFragmentAddedObservable();
        TestObserver<Boolean> testObserver = TestObserver.create();
        fragmentDisplayedObservable.subscribe(testObserver);

        testObserver.assertValue(false);

        mClassUnderTest.addFragment(fragment);

        testObserver = TestObserver.create();
        fragmentDisplayedObservable.subscribe(testObserver);

        testObserver.assertValue(true);

        mClassUnderTest.removeFragment();

        testObserver = TestObserver.create();
        fragmentDisplayedObservable.subscribe(testObserver);

        testObserver.assertValue(false);

        mClassUnderTest.addFragment(fragment);

        testObserver = TestObserver.create();
        fragmentDisplayedObservable.subscribe(testObserver);

        testObserver.assertValue(true);

        mClassUnderTest.removeFragment();

        testObserver = TestObserver.create();
        fragmentDisplayedObservable.subscribe(testObserver);

        testObserver.assertValue(false);
    }
}
