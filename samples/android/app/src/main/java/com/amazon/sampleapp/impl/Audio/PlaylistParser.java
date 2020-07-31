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

package com.amazon.sampleapp.impl.Audio;

import android.net.Uri;
import android.support.annotation.Nullable;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

class PlaylistParser {
    private static final String sTag = "PlaylistParser";
    private static final int sResponseOk = 200;
    private static final Pattern sPattern = Pattern.compile("https?:.*");
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    // Extracts Url from redirect Url. Note: not a complete playlist parser implementation
    Uri parseUri(final Uri uri) throws IOException {
        return parsePlaylist(getResponse(uri));
    }

    private String getResponse(final Uri uri) throws IOException {
        Future<String> response = mExecutor.submit(new Callable<String>() {
            @Override
            public String call() throws IOException {
                HttpURLConnection con = null;
                try {
                    URL obj = new URL(uri.toString());
                    con = (HttpURLConnection) obj.openConnection();

                    int responseCode = con.getResponseCode();
                    if (responseCode == sResponseOk) {
                        return parseResponse(con.getInputStream());
                    } else {
                        throw new IOException(sTag + ": Unsuccessful response. Code: " + responseCode);
                    }
                } finally {
                    if (con != null)
                        con.disconnect();
                }
            }
        });

        try {
            return response.get();
        } catch (Exception e) {
            throw new IOException(sTag + ": Error getting response: " + e.getMessage());
        }
    }

    private String parseResponse(InputStream inStream) throws IOException {
        if (inStream != null) {
            BufferedReader in = new BufferedReader(new InputStreamReader(inStream));
            String inputLine;
            StringBuilder response = new StringBuilder();

            try {
                while ((inputLine = in.readLine()) != null) {
                    response.append(inputLine);
                }
                return response.toString();
            } catch (Exception e) {
                throw new IOException(sTag + ": Error parsing response: " + e.toString());
            } finally {
                inStream.close();
            }
        }
        return null;
    }

    private static Uri parsePlaylist(@Nullable final String playlist) throws IOException {
        if (playlist != null && !playlist.isEmpty()) {
            Matcher matcher = sPattern.matcher(playlist);
            if (matcher.find()) {
                return Uri.parse(playlist.substring(matcher.start(), matcher.end()));
            } else {
                throw new IOException(sTag + ": Response did not contain a URL");
            }
        } else {
            throw new IOException(sTag + ": Response was empty");
        }
    }
}
