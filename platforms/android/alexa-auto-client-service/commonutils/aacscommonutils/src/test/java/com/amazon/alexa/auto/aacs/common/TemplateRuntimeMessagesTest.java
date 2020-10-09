package com.amazon.alexa.auto.aacs.common;

import static org.junit.Assert.assertTrue;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

import java.util.Optional;

@RunWith(RobolectricTestRunner.class)
public class TemplateRuntimeMessagesTest {
    @Test
    public void testParseRenderPlayerInfo() {
        Optional<String> renderInfoJson =
                TestResourceFileReader.readFileContent("aacs/TemplateRuntimePlayerRenderInfo.json");
        assertTrue(renderInfoJson.isPresent());

        Optional<RenderPlayerInfo> playerMetadata = TemplateRuntimeMessages.parseRenderPlayerInfo(renderInfoJson.get());
        assertTrue(playerMetadata.isPresent());
    }
}
