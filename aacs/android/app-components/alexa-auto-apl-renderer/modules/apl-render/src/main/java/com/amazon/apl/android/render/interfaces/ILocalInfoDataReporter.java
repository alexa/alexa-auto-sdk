/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.interfaces;

/**
 * Interface used for allowing platform to report changes in local info
 * that can affect the APL document.
 */
public interface ILocalInfoDataReporter {
    /**
     * Notifies the APL extension that a POI data item was selected by the platform.
     * This allows the APL document to be updated to show details for the selected
     * data item.
     *
     * @param id The identifier for the selected POI data item.
     */
    void platformDataItemSelectedById(String id);
}
