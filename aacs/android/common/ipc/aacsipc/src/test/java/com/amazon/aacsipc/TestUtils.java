package com.amazon.aacsipc;

public class TestUtils {
    public static Thread getThreadByName(String threadName) {
        for (Thread t : Thread.getAllStackTraces().keySet()) {
            if (t.getName().equals(threadName))
                return t;
        }
        return null;
    }

    public static String generateTestString(int stringSize) {
        StringBuilder stringBuilder = new StringBuilder(stringSize);
        int offset = 0;
        for (int i = 0; i < stringSize; ++i) {
            stringBuilder.append((char) ('a' + offset));
            offset = (offset + 1) % 26;
        }
        return stringBuilder.toString();
    }
}
