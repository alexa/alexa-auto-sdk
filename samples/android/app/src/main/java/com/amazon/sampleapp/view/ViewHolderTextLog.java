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

import android.app.Fragment;
import android.graphics.Color;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.amazon.sampleapp.R;


public class ViewHolderTextLog extends RecyclerView.ViewHolder {

    private TextView log;
    private int highlight = Color.YELLOW;
    public ViewHolderTextLog(View v) {
        super(v);
        log = (TextView) v.findViewById(R.id.item_log);
        log.setTextColor(highlight);
    }

    public TextView getLog() {
        return log;
    }

    public void setLog(TextView lg) {
        this.log = lg;
    }

    public int getHighlight() {
        return highlight;
    }

    public void setHighlight(int hl) {
        this.highlight = hl;
        this.log.setTextColor( Color.parseColor( String.format( "#%06X", 0xFFFFFFFF & hl ) ) );
    }
}

