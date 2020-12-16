package com.amazon.alexa.auto.navigation.providers.google;

import static com.amazon.alexa.auto.navigation.providers.google.GoogleMapsNavigationProvider.GOOGLE_MAPS_CANCEL_NAVIGATION_URI;
import static com.amazon.alexa.auto.navigation.providers.google.GoogleMapsNavigationProvider.GOOGLE_MAPS_PACKAGE;
import static com.amazon.alexa.auto.navigation.providers.google.GoogleMapsNavigationProvider.GOOGLE_MAPS_PREVIEW_ROUTE;
import static com.amazon.alexa.auto.navigation.providers.google.GoogleMapsNavigationProvider.GOOGLE_MAPS_START_NAV_URI;
import static com.amazon.alexa.auto.navigation.providers.google.GoogleMapsNavigationProvider.GOOGLE_MAPS_ZOOM_TO_POI;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import com.amazon.alexa.auto.aacs.common.Address;
import com.amazon.alexa.auto.aacs.common.AlexaWaypoint;
import com.amazon.alexa.auto.aacs.common.Coordinate;
import com.amazon.alexa.auto.aacs.common.PointOfInterest;
import com.amazon.alexa.auto.aacs.common.StartNavigation;
import com.amazon.alexa.auto.aacs.common.Title;
import com.amazon.alexa.auto.navigation.providers.google.GoogleMapsNavigationProvider;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;
import org.robolectric.RobolectricTestRunner;

import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

@RunWith(RobolectricTestRunner.class)
public class GoogleMapsNavigationProviderTest {
    public static final String TEST_ADDRESS_LINE_1 = "test address line 1";
    public static final String TEST_CITY = "testCity";
    public static final String TEST_STATE = "testState";
    public static final String TEST_COUNTY = "testCounty";
    public static final String TEST_POSTAL_CODE = "testPostalCode";
    public static final String TEST_COUNTRY_CODE = "testCountryCode";
    public static final String TEST_TITLE = "testTitle";
    public static final double LATITUDE = 21.23;
    public static final double LONGITUDE = 31.23;
    public static final String TEST_DEST_TYPE = "DESTINATION";
    public static final String TEST_DEST_NAME = "HOME";
    private GoogleMapsNavigationProvider mClassUnderTest;
    private Context mContext;
    private StartNavigation startNavigation;
    private PointOfInterest mPointOfInterest;

    @Before
    public void setup() {
        mContext = Mockito.mock(Context.class);
        mClassUnderTest = new GoogleMapsNavigationProvider(new WeakReference<>(mContext));
        createTestStartNavigationDirective();
        createTestPOI();
    }

    private void createTestPOI() {
        mPointOfInterest = Mockito.mock(PointOfInterest.class);
        Mockito.when(mPointOfInterest.getCoordinate()).thenReturn(new Coordinate(LONGITUDE, LATITUDE));
        Mockito.when(mPointOfInterest.getTitle()).thenReturn(new Title(TEST_TITLE, TEST_TITLE));
    }

    private void createTestStartNavigationDirective() {
        Address address = new Address(
                TEST_ADDRESS_LINE_1, "", "", TEST_CITY, TEST_STATE, TEST_COUNTY, TEST_POSTAL_CODE, TEST_COUNTRY_CODE);
        List<Double> latLongCoordinate = Arrays.asList(LATITUDE, LONGITUDE);
        AlexaWaypoint alexaWaypoint = new AlexaWaypoint(TEST_DEST_TYPE, latLongCoordinate, address, TEST_DEST_NAME);
        startNavigation = new StartNavigation(Collections.singletonList(alexaWaypoint), "DRIVING");
    }

    @Test
    public void startNavigationTest() {
        mClassUnderTest.startNavigation(startNavigation);

        ArgumentCaptor<Intent> intentArgumentCaptor = ArgumentCaptor.forClass(Intent.class);

        Mockito.verify(mContext).startActivity(intentArgumentCaptor.capture());

        Intent intent = intentArgumentCaptor.getValue();

        Assert.assertEquals(intent.getAction(), Intent.ACTION_VIEW);
        Assert.assertEquals(intent.getPackage(), GOOGLE_MAPS_PACKAGE);
        Assert.assertEquals(intent.getData(), Uri.parse(String.format(GOOGLE_MAPS_START_NAV_URI, LATITUDE, LONGITUDE)));
    }

    @Test
    public void cancelNavigationTest() {
        mClassUnderTest.cancelNavigation();

        ArgumentCaptor<Intent> intentArgumentCaptor = ArgumentCaptor.forClass(Intent.class);
        Mockito.verify(mContext).startActivity(intentArgumentCaptor.capture());

        Intent intent = intentArgumentCaptor.getValue();
        Assert.assertEquals(intent.getAction(), Intent.ACTION_VIEW);
        Assert.assertEquals(intent.getData(), Uri.parse(GOOGLE_MAPS_CANCEL_NAVIGATION_URI));
    }

    @Test
    public void previewRouteTest() {
        mClassUnderTest.previewRoute(mPointOfInterest);

        ArgumentCaptor<Intent> intentArgumentCaptor = ArgumentCaptor.forClass(Intent.class);
        Mockito.verify(mContext).startActivity(intentArgumentCaptor.capture());

        Intent intent = intentArgumentCaptor.getValue();
        Assert.assertEquals(intent.getAction(), Intent.ACTION_VIEW);
        Assert.assertEquals(
                intent.getData(), Uri.parse(String.format(GOOGLE_MAPS_PREVIEW_ROUTE, LATITUDE, LONGITUDE, TEST_TITLE)));
    }

    @Test
    public void startNavigationToPOITest() {
        mClassUnderTest.startNavigation(mPointOfInterest);

        ArgumentCaptor<Intent> intentArgumentCaptor = ArgumentCaptor.forClass(Intent.class);
        Mockito.verify(mContext).startActivity(intentArgumentCaptor.capture());

        Intent intent = intentArgumentCaptor.getValue();
        Assert.assertEquals(intent.getAction(), Intent.ACTION_VIEW);
        Assert.assertEquals(intent.getData(), Uri.parse(String.format(GOOGLE_MAPS_START_NAV_URI, LATITUDE, LONGITUDE)));
    }

    @Test
    public void zoomToPOITest() {
        mClassUnderTest.zoomToPOI(mPointOfInterest);

        ArgumentCaptor<Intent> intentArgumentCaptor = ArgumentCaptor.forClass(Intent.class);
        Mockito.verify(mContext).startActivity(intentArgumentCaptor.capture());

        Intent intent = intentArgumentCaptor.getValue();
        Assert.assertEquals(intent.getAction(), Intent.ACTION_VIEW);
        Assert.assertEquals(intent.getData(), Uri.parse(String.format(GOOGLE_MAPS_ZOOM_TO_POI, LATITUDE, LONGITUDE)));
    }
}
