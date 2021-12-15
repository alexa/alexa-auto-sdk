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

    @Test
    public void testParseLocalSearchListTemplate() {
        Optional<String> localSearchListTemplateJson =
                TestResourceFileReader.readFileContent("aacs/LocalSearchListTemplateV2.json");
        assertTrue(localSearchListTemplateJson.isPresent());

        Optional<LocalSearchListTemplate> localSearchListTemplate =
                TemplateRuntimeMessages.parseLocalSearchListTemplate(localSearchListTemplateJson.get());
        assertTrue(localSearchListTemplate.isPresent());
    }

    @Test
    public void testParseLocalSearchDetailTemplate() {
        Optional<String> localSearchDetailTemplateJson =
                TestResourceFileReader.readFileContent("aacs/LocalSearchDetailTemplateV1.json");
        assertTrue(localSearchDetailTemplateJson.isPresent());

        Optional<LocalSearchDetailTemplate> localSearchDetailTemplate =
                TemplateRuntimeMessages.parseLocalSearchDetailTemplate(localSearchDetailTemplateJson.get());
        assertTrue(localSearchDetailTemplate.isPresent());
    }
}
