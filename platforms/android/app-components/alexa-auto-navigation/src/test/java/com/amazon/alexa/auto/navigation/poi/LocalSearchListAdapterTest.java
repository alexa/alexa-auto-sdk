package com.amazon.alexa.auto.navigation.poi;

import android.app.Activity;
import android.content.Context;
import android.widget.LinearLayout;

import com.amazon.alexa.auto.aacs.common.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.Title;
import com.amazon.alexa.auto.navigation.handlers.LocalSearchDirectiveHandler;
import com.amazon.alexa.auto.navigation.providers.NavigationProvider;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;

import java.lang.ref.WeakReference;

@RunWith(RobolectricTestRunner.class)
public class LocalSearchListAdapterTest {
    private static final String TEST_TITLE = "TEST_TITLE";
    private static final String TEST_SUBTITLE = "TEST_SUBTITLE";
    private static final String TEST_DISTANCE = "2.2 mi";
    private static final String TEST_ADDRESS = "TEST_ADDRESS";
    private Activity mActivity;
    private Context mContext;
    private LocalSearchListAdapter mLocalSearchListAdapter;
    private NavigationProvider mNavigationProvider;
    private PointOfInterest mPointOfInterest;
    private LocalSearchDirectiveHandler mLocalSearchDirectiveHandler;

    @Before
    public void setup() {
        mActivity = Robolectric.buildActivity(Activity.class).create().resume().get();

        mContext = mActivity.getApplicationContext();
        mNavigationProvider = Mockito.mock(NavigationProvider.class);
        mLocalSearchDirectiveHandler = Mockito.mock(LocalSearchDirectiveHandler.class);
        mLocalSearchListAdapter =
                new LocalSearchListAdapter(mNavigationProvider, new WeakReference<>(mLocalSearchDirectiveHandler));

        setupTestPOI();
    }

    private void setupTestPOI() {
        mPointOfInterest = Mockito.mock(PointOfInterest.class);
        Mockito.when(mPointOfInterest.getTitle()).thenReturn(new Title(TEST_SUBTITLE, TEST_TITLE));
        Mockito.when(mPointOfInterest.getTravelDistance()).thenReturn(TEST_DISTANCE);
        Mockito.when(mPointOfInterest.getAddress()).thenReturn(TEST_ADDRESS);
    }

    @Test
    public void testAddPOI() {
        mLocalSearchListAdapter.addPOI(mPointOfInterest);

        Assert.assertEquals(mLocalSearchListAdapter.getPOIs().size(), 1);
        Assert.assertEquals(mLocalSearchListAdapter.getPOIs().get(0).getTitle().getMainTitle(), TEST_TITLE);
        Assert.assertEquals(mLocalSearchListAdapter.getPOIs().get(0).getTitle().getSubTitle(), TEST_SUBTITLE);
    }

    @Test
    public void testViewHolderProperties() {
        mLocalSearchListAdapter.addPOI(mPointOfInterest);

        LinearLayout parent = new LinearLayout(mContext);
        LocalSearchListAdapter.ViewHolder poiViewHolder =
                (LocalSearchListAdapter.ViewHolder) mLocalSearchListAdapter.onCreateViewHolder(parent, 0);

        mLocalSearchListAdapter.onBindViewHolder(poiViewHolder, 0);
        Assert.assertEquals(poiViewHolder.mPOIName.getText(), TEST_TITLE);
        Assert.assertEquals(poiViewHolder.mPOIOrdinal.getText(), String.valueOf(1));
        Assert.assertEquals(poiViewHolder.mPOIAddress.getText(), TEST_ADDRESS);

        mLocalSearchListAdapter.addPOI(mPointOfInterest);
        LocalSearchListAdapter.ViewHolder poiViewHolder2 =
                (LocalSearchListAdapter.ViewHolder) mLocalSearchListAdapter.onCreateViewHolder(parent, 0);
        mLocalSearchListAdapter.onBindViewHolder(poiViewHolder2, 1);
        Assert.assertEquals(poiViewHolder2.mPOIOrdinal.getText(), String.valueOf(2));

        // Verify clicking on poi item will dismiss the template.
        poiViewHolder.mPoiRoot.callOnClick();

        ArgumentCaptor<PointOfInterest> pointOfInterestArgumentCaptor = ArgumentCaptor.forClass(PointOfInterest.class);
        Mockito.verify(mNavigationProvider).startNavigation(pointOfInterestArgumentCaptor.capture());
        Mockito.verify(mLocalSearchDirectiveHandler).clearTemplate();

        PointOfInterest capturedPointOfInterest = pointOfInterestArgumentCaptor.getValue();
        Assert.assertEquals(TEST_ADDRESS, capturedPointOfInterest.getAddress());
        Assert.assertEquals(TEST_TITLE, capturedPointOfInterest.getTitle().getMainTitle());
    }
}
