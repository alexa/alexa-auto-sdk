package com.amazon.alexa.auto.navigation.poi;

import static org.mockito.Mockito.when;

import android.app.Activity;
import android.content.Context;
import android.widget.LinearLayout;

import com.amazon.alexa.auto.aacs.common.Image;
import com.amazon.alexa.auto.aacs.common.POIImage;
import com.amazon.alexa.auto.aacs.common.Rating;
import com.amazon.alexa.auto.aacs.common.Title;
import com.amazon.alexa.auto.aacs.common.navi.PointOfInterest;
import com.amazon.alexa.auto.navigation.handlers.LocalSearchDirectiveHandler;
import com.amazon.alexa.auto.navigation.providers.NaviProvider;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.robolectric.Robolectric;
import org.robolectric.RobolectricTestRunner;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

@RunWith(RobolectricTestRunner.class)
public class LocalSearchListAdapterTest {
    private static final String TEST_TITLE = "TEST_TITLE";
    private static final String TEST_SUBTITLE = "TEST_SUBTITLE";
    private static final String TEST_DISTANCE = "2.2 mi";
    private static final String TEST_ADDRESS = "TEST_ADDRESS";
    private Activity mActivity;
    private Context mContext;
    private LocalSearchListAdapter mLocalSearchListAdapter;

    @Mock
    private NaviProvider mNaviProvider;
    @Mock
    private PointOfInterest mPointOfInterest;
    @Mock
    private LocalSearchDirectiveHandler mLocalSearchDirectiveHandler;
    @Mock
    private Rating mRating;
    @Mock
    private POIImage mPOIImage;
    @Mock
    private Image mImage;

    @Before
    public void setup() {
        MockitoAnnotations.openMocks(this);

        mActivity = Robolectric.buildActivity(Activity.class).create().resume().get();
        mContext = mActivity.getApplicationContext();
        mLocalSearchListAdapter =
                new LocalSearchListAdapter(mNaviProvider, new WeakReference<>(mLocalSearchDirectiveHandler), mContext);

        setupTestPOI();
    }

    private void setupTestPOI() {
        ArrayList<Image> images = new ArrayList<>();
        images.add(mImage);

        when(mPointOfInterest.getProvider()).thenReturn(LocalSearchListAdapter.POI_PROVIDER_YELP);
        when(mPointOfInterest.getTitle()).thenReturn(new Title(TEST_SUBTITLE, TEST_TITLE));
        when(mPointOfInterest.getTravelDistance()).thenReturn(TEST_DISTANCE);
        when(mPointOfInterest.getAddress()).thenReturn(TEST_ADDRESS);
        when(mPointOfInterest.getRating()).thenReturn(mRating);
        when(mRating.getImage()).thenReturn(mPOIImage);
        when(mImage.getUrl()).thenReturn("http://image-url");
        when(mPOIImage.getSources()).thenReturn(images);
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
        Mockito.verify(mNaviProvider).startNavigation(pointOfInterestArgumentCaptor.capture());
        Mockito.verify(mLocalSearchDirectiveHandler).clearTemplate();

        PointOfInterest capturedPointOfInterest = pointOfInterestArgumentCaptor.getValue();
        Assert.assertEquals(TEST_ADDRESS, capturedPointOfInterest.getAddress());
        Assert.assertEquals(TEST_TITLE, capturedPointOfInterest.getTitle().getMainTitle());
    }
}
