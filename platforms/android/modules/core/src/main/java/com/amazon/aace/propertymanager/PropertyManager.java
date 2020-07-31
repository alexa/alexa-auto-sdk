/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.propertyManager;

import com.amazon.aace.core.PlatformInterface;

/**
 * PropertyManager should be extended to set and retrieve Engine property
 * settings and be notified of property changes.
 *
 * @sa com.amazon.aace.alexa.AlexaProperties.java
 * @sa com.amazon.aace.alexa.CoreProperties.java
 * @sa com.amazon.aace.alexa.NetworkProperties.java
 * @sa com.amazon.aace.alexa.VehicleProperties.java
 */
abstract public class PropertyManager extends PlatformInterface {
    /**
     * Describes the state of a property change.
     */
    public enum PropertyState {
        /**
         * The property change was successful.
         * @hideinitializer
         */
        SUCCEEDED("SUCCEEDED"),

        /**
         * The property change failed.
         * @hideinitializer
         */
        FAILED("FAILED");

        /**
         * @internal
         */
        private String m_name;

        /**
         * @internal
         */
        private PropertyState(String name) {
            m_name = name;
        }

        /**
         * @internal
         */
        public String toString() {
            return m_name;
        }
    }

    /**
     * Sets a property value in the Engine. setProperty() is an
     * asynchronous operation and the Engine will call propertyStateChanged()
     * with the status when it is completed.
     *
     * @param name The name used by the Engine to identify the property.
     *        The property name must be one of the property constants recognized
     *        by the Engine, e.g. the properties in
     *        @c com.amazon.aace.alexa.AlexaProperties.java
     * @param value The property setting
     * @return @c true if the property value was updated or set to the current
     *         setting, else @c false if an error occured.
     */
    public final boolean setProperty(String name, String value) {
        return setProperty(getNativeRef(), name, value);
    }

    /**
     * Notifies the platform implementation of the status of a property change
     * after a call to setProperty().
     *
     * @param name The name used by the Engine to identify the property.
     * @param value The property value.
     * @param state The state of the property change.
     */
    public void propertyStateChanged(String name, String value, PropertyState state) {}

    /**
     * Retrieves the setting for the property identified by
     * @c name from the Engine
     *
     * @param name The name used by the Engine to identify the property.
     *        The property name must be one of the property constants recognized
     *        by the Engine, e.g. the properties in
     *        @c com.amazon.aace.alexa.AlexaProperties.java
     * @return The property value as a string, or an empty string if the
     *        property value was not found
     */
    public final String getProperty(String name) {
        return getProperty(getNativeRef(), name);
    }

    /**
     * Notifies the platform implementation of a property setting change in the
     * Engine.
     * @note This will not be called if the property setting change was
     * initiated by @c PropertyManager.setProperty()
     *
     * @param name The name used by the Engine to identify the property.
     * @param newValue The new value of the property
     */
    public void propertyChanged(String name, String newValue) {}

    // NativeRef implementation
    final protected long createNativeRef() {
        return createBinder();
    }

    final protected void disposeNativeRef(long nativeRef) {
        disposeBinder(nativeRef);
    }

    // Native Engine JNI methods
    private native long createBinder();
    private native void disposeBinder(long nativeRef);
    private native boolean setProperty(long nativeRef, String name, String value);
    private native String getProperty(long nativeRef, String name);
}