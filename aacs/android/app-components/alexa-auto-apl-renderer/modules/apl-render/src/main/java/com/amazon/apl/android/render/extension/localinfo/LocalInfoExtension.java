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

package com.amazon.apl.android.render.extension.localinfo;

import android.util.Log;

import com.amazon.apl.android.APLController;
import com.amazon.apl.android.Event;
import com.amazon.apl.android.ExtensionCommandDefinition;
import com.amazon.apl.android.ExtensionEventHandler;
import com.amazon.apl.android.RootConfig;
import com.amazon.apl.android.dependencies.IExtensionEventCallback;
import com.amazon.apl.android.providers.IExtension;
import com.amazon.apl.android.render.interfaces.ILocalInfoDataConsumer;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class implements a built in APL extension used by the local info
 * domain to expose data to the platform. This exposes a bidirectional communication
 * channel for the platfom and APL runtime to communicate.
 */
public class LocalInfoExtension implements IExtensionEventCallback, IExtension {
    public static final String TAG = LocalInfoExtension.class.getSimpleName();
    public static final String URI = "aplext:localinfo:10";
    public static final String COMMAND_SEND_POI_DATA_LIST = "SendPOIDataList";
    public static final String COMMAND_SELECT_POI = "SelectPOI";
    public static final String PROPERTY_DATA_LIST = "poiDataList";
    public static final String PROPERTY_POI_ID = "poiId";
    public static final String ON_SELECT_POI_EVENT_HANDLER = "OnSelectPOI";

    private ILocalInfoDataConsumer mDataConsumer;
    private RootConfig mRootConfig;

    //-------------------------------------------------------------------------
    // Constructor
    //-------------------------------------------------------------------------

    public LocalInfoExtension(RootConfig rootConfig) {
        mRootConfig = rootConfig;
    }

    //-------------------------------------------------------------------------
    // Events and commands supported by the extension
    //-------------------------------------------------------------------------

    private static final List<ExtensionCommandDefinition> COMMANDS = new ArrayList<>();
    private static final List<ExtensionEventHandler> EVENTS = new ArrayList<>();

    static {
        COMMANDS.add(new ExtensionCommandDefinition(URI, COMMAND_SEND_POI_DATA_LIST)
                             .allowFastMode(true)
                             .property(PROPERTY_DATA_LIST, null, true));
        COMMANDS.add(new ExtensionCommandDefinition(URI, COMMAND_SELECT_POI)
                             .allowFastMode(true)
                             .property(PROPERTY_POI_ID, "", false));
        EVENTS.add(new ExtensionEventHandler(URI, ON_SELECT_POI_EVENT_HANDLER));
    }

    //-------------------------------------------------------------------------
    // IExtension
    //-------------------------------------------------------------------------

    @Override
    public List<ExtensionCommandDefinition> getCommandDefinitions() {
        return COMMANDS;
    }

    @Override
    public List<ExtensionEventHandler> getEventHandlers() {
        return EVENTS;
    }

    @Override
    public String getUri() {
        return URI;
    }

    @Override
    public IExtensionEventCallback getCallback() {
        return this;
    }

    //-------------------------------------------------------------------------
    // IExtensionEventCallback
    //-------------------------------------------------------------------------

    @Override
    public void onExtensionEvent(String name, String uri, Event event, Map<String, Object> source,
            Map<String, Object> custom, IExtensionEventCallback.IExtensionEventCallbackResult resultCallback) {
        handleOnExtensionEvent(name, uri, event, source, custom, resultCallback);
    }

    @Override
    public void onExtensionEvent(String name, String uri, Map<String, Object> source, Map<String, Object> custom,
            IExtensionEventCallback.IExtensionEventCallbackResult resultCallback) {
        handleOnExtensionEvent(name, uri, null, source, custom, resultCallback);
    }

    private void handleOnExtensionEvent(String name, String uri, Event event, Map<String, Object> source,
            Map<String, Object> custom, IExtensionEventCallback.IExtensionEventCallbackResult resultCallback) {
        Log.v(TAG, "onExtensionEvent: " + uri + "/" + name + "/" + event + "/" + source + "/" + custom);
        switch (name) {
            case COMMAND_SEND_POI_DATA_LIST:
                sendPOIDataListHandler(custom.get(PROPERTY_DATA_LIST));
                break;
            case COMMAND_SELECT_POI:
                selectPoiHandler(custom.get(PROPERTY_POI_ID));
                break;
        }
    }

    //-------------------------------------------------------------------------
    // Public methods
    //-------------------------------------------------------------------------

    /**
     * The platform instance that will consume local info data from extension commands.
     *
     * @param consumer The consumer instance.
     */
    public void setDataConsumer(ILocalInfoDataConsumer consumer) {
        mDataConsumer = consumer;
    }

    /**
     * External selection of poi data item.
     * @param controller The APL runtime instance.
     * @param poiId Id of the selected data item.
     */
    public void poiSelected(APLController controller, String poiId) {
        Map<String, Object> map = new HashMap<>();
        map.put("poiId", poiId);
        sendEventToDoc(controller, ON_SELECT_POI_EVENT_HANDLER, map);
    }

    //-------------------------------------------------------------------------
    // Private methods
    //-------------------------------------------------------------------------

    /**
     * Handle the SendPOIDataList command.
     * @param objectDataList
     */
    private void sendPOIDataListHandler(Object objectDataList) {
        try {
            JSONArray poiDataList = new JSONArray();
            Object[] data = (Object[]) objectDataList;
            for (int i = 0; i < data.length; i++) {
                HashMap<String, String> poiItem = (HashMap<String, String>) data[i];
                JSONObject poiJson = new JSONObject(poiItem);
                Log.v(TAG, "data " + poiItem);
                poiDataList.put(poiJson);
            }
            String dataJson = poiDataList.toString();
            if (mDataConsumer != null) {
                mDataConsumer.aplDataAvailable(dataJson);
                Log.v(TAG, "aplDataAvailable: " + dataJson);
            }
        } catch (IllegalArgumentException | NullPointerException | ClassCastException e) {
            Log.w(TAG, "Invalid keys in document. Doing nothing.", e);
        }
    }

    /**
     * Notify consumer that a data item was selected through the APL document.
     *
     * @param poiId The id of the data item selected.
     */
    private void selectPoiHandler(Object poiId) {
        String poiIdString = (String) poiId;
        Log.v(TAG, "selectPoiHandler" + poiIdString);
        if (mDataConsumer != null) {
            mDataConsumer.aplDataItemSelectedById(poiIdString);
        }
    }

    /**
     * Send an APL extension event to the APL document.
     * @param controller current APL controller
     * @param handler name of the extension event handler in the doc
     * @param data map of data to send in the event
     */
    private void sendEventToDoc(APLController controller, String handler, Map<String, Object> data) {
        if (controller != null) {
            Log.v(TAG, "extension sending event to " + handler);
            controller.invokeExtensionEventHandler(URI, handler, data, false, null);
        }
    }
}
