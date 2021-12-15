/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package com.amazon.aace.audio;

final public class AudioFormat {
    private Encoding mEncoding;
    private SampleFormat mSampleFormat;
    private Layout mLayout;
    private Endianness mEndianness;
    private int mSampleRate;
    private int mSampleSize;
    private int mChannels;

    /**
     * Describes the playback state of the platform media player
     */
    public enum Encoding {
        /**
         * @hideinitializer
         */
        UNKNOWN("UNKNOWN"),
        /**
         * @hideinitializer
         */
        LPCM("LPCM"),
        /**
         * @hideinitializer
         */
        MP3("MP3"),
        /**
         * @hideinitializer
         */
        OPUS("OPUS");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Encoding(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public enum SampleFormat {
        UNKNOWN("UNKNOWN"),
        SIGNED("SIGNED"),
        UNSIGNED("UNSIGNED"),
        FLOAT("FLOAT");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private SampleFormat(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public enum Layout {
        UNKNOWN("UNKNOWN"),
        NON_INTERLEAVED("NON_INTERLEAVED"),
        INTERLEAVED("INTERLEAVED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Layout(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public enum Endianness {
        UNKNOWN("UNKNOWN"),
        LITTLE("LITTLE"),
        BIG("BIG");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private Endianness(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    public AudioFormat(Encoding encoding, SampleFormat sampleFormat, Layout layout, Endianness endianness,
            int sampleRate, int sampleSize, int channels) {
        mEncoding = encoding;
        mSampleFormat = sampleFormat;
        mLayout = layout;
        mEndianness = endianness;
        mSampleRate = sampleRate;
        mSampleSize = sampleSize;
        mChannels = channels;
    }

    /**
     * @return The @c Encoding type of the data returned by this @c AudioStream.
     */
    final public Encoding getEncoding() {
        return mEncoding;
    }

    final public SampleFormat getSampleFormat() {
        return mSampleFormat;
    }

    final public Layout getLayout() {
        return mLayout;
    }

    final public Endianness getEndianness() {
        return mEndianness;
    }

    final public int getSampleRate() {
        return mSampleRate;
    }

    final public int getSampleSize() {
        return mSampleSize;
    }

    final public int getNumChannels() {
        return mChannels;
    }
}
