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
 * This interface allows the platform to be notified when local info data
 * is available or selected inside the APL document.
 */
public interface ILocalInfoDataConsumer {
    /**
     * Notifies the platform that the APL document contains local info data.
     *
     * @param data The list of data items containing
     * @code{.json}
     * [
     *    {
     *      "id" : "<STRING>",
     *      "name" : "<STRING>",
     *      "coordinate" : {
     *          "latitude" : <DOUBLE>,
     *          "longitude : <DOUBLE>
     *      }
     *    }
     * ]
     * @endcode
     * @li id The identifier for the POI data item.
     * @li name The name associated with the POI data item.
     * @li coordinate A JSON object containing the geocoordinates for the POI data item.
     */
    void aplDataAvailable(String data);

    /**
     * Notifies the platform that a POI data item was selected in the APL document. This
     * allows the platform to highlight the selected data item.
     *
     * @param id The identifier for the selected POI data item.
     */
    void aplDataItemSelectedById(String id);
}
