package com.amazon.aace.metrics;

import com.amazon.aace.core.PlatformInterface;

import java.util.HashMap;

/**
 * Logger should be extended handle log events from the AAC SDK.
 */
abstract public class MetricsUploader extends PlatformInterface
{
    public boolean record(Datapoint[] datapoints, HashMap<String, String> metadata) {
        return false;
    }

    public class Datapoint {
        public DatapointType type;
        public String name;
        public String value;
        public int count;
    }

    /**
     * Datapoint types that can be added to a metric
     */
    public enum DatapointType {
        TIMER("Timer"),
        STRING("String"),
        COUNTER("Counter");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private DatapointType(String name ) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    };
}