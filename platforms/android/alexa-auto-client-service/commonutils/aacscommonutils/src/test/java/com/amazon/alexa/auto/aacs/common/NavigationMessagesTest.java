package com.amazon.alexa.auto.aacs.common;

import static org.junit.Assert.assertTrue;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class NavigationMessagesTest {
    @Test
    public void testStartNavigation() {
        Optional<String> startNavigationJson = TestResourceFileReader.readFileContent("aacs/StartNavigation.json");
        assertTrue(startNavigationJson.isPresent());

        Optional<StartNavigation> startNavigation =
                NavigationMessages.parseStartNavigationDirective(startNavigationJson.get());
        assertTrue(startNavigation.isPresent());
    }
}
