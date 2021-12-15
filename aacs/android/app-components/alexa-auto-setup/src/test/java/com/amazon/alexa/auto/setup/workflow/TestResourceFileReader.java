package com.amazon.alexa.auto.setup.workflow;

import java.net.URL;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Optional;

public class TestResourceFileReader {
    /**
     * Read contents of a file from test resources folder under test.
     *
     * @param fileName Name of the file.
     * @return content of the file as String.
     */
    public static String readFileContent(String fileName) {
        URL fileURL = ClassLoader.getSystemResource(fileName);

        try {
            return new String(Files.readAllBytes(Paths.get(fileURL.getPath())));
        } catch (Exception e) {
            return "";
        }
    }
}
