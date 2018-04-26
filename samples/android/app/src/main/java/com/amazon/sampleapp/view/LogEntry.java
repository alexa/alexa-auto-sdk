/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.view;

import org.json.JSONObject;

public class LogEntry
{
    private String m_type;
    private JSONObject m_json;

    public LogEntry( String text, JSONObject json )
    {
        m_type = text;
        m_json = json;

    }
    public String getType(){
        return m_type;
    }
    public JSONObject getJSON(){
        return m_json;
    }

    public void setType( String type ){
        m_type = type;
    }
    public void setJSON( JSONObject obj ){
        m_json = obj;
    }
}