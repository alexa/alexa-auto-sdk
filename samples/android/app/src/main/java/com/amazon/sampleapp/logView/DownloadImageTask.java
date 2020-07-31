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

package com.amazon.sampleapp.logView;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.ImageView;

import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.WeakReference;

public class DownloadImageTask extends AsyncTask<String, Void, Bitmap> {
    private static final String sTag = "CLI";
    private final WeakReference<ImageView> mImage;

    public DownloadImageTask(ImageView image) {
        mImage = new WeakReference<>(image);
    }

    protected Bitmap doInBackground(String... urls) {
        String urlDisplay = urls[0];
        Bitmap mIcon11 = null;

        try (InputStream in = new java.net.URL(urlDisplay).openStream()) {
            mIcon11 = BitmapFactory.decodeStream(in);
        } catch (IOException e) {
            String exceptionMessage = "";
            if (e.getMessage() == null) {
                exceptionMessage = "Exception occured. Cannot display message";
            } else {
                exceptionMessage = e.getMessage();
            }
            Log.e(sTag, exceptionMessage);
        }

        return mIcon11;
    }

    protected void onPostExecute(Bitmap result) {
        try {
            mImage.get().setImageBitmap(result);
        } catch (Exception e) {
            Log.e(sTag, e.getMessage());
        }
    }
}
