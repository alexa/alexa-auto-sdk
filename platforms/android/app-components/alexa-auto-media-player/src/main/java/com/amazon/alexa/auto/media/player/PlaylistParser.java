package com.amazon.alexa.auto.media.player;

import android.net.Uri;

import androidx.annotation.Nullable;

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

/**
 * Parser to retrieve media URI from playlist received by Alexa service
 */
public class PlaylistParser {
    private static final String TAG = PlaylistParser.class.getSimpleName();
    private static final int RESPONSE_OK = 200;
    private static final Pattern HTTPS_PATTERN = Pattern.compile("https?:.*");
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    // Extracts Url from redirect Url. Note: not a complete playlist parser implementation
    public Uri parseUri(final Uri uri) throws IOException {
        return parsePlaylist(parseResponse(getResponse(uri)));
    }

    private InputStream getResponse(final Uri uri) throws IOException {
        Future<InputStream> response = mExecutor.submit(new Callable<InputStream>() {
            @Override
            public InputStream call() throws IOException {
                HttpURLConnection con = null;
                try {
                    URL obj = new URL(uri.toString());
                    con = (HttpURLConnection) obj.openConnection();

                    int responseCode = con.getResponseCode();
                    if (responseCode == RESPONSE_OK) {
                        return con.getInputStream();
                    } else {
                        throw new IOException(TAG + ": Unsuccessful response. Code: " + responseCode);
                    }
                } finally {
                    if (con != null)
                        con.disconnect();
                }
            }
        });

        /**
         * TODO: Replace the below get() call with an non-blocking getter
         */
        try {
            return response.get();
        } catch (Exception e) {
            throw new IOException(TAG + ": Error getting response: " + e.getMessage());
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
                throw new IOException(TAG + ": Error parsing response");
            } finally {
                inStream.close();
            }
        }
        return null;
    }

    private static Uri parsePlaylist(@Nullable final String playlist) throws IOException {
        if (playlist != null && !playlist.isEmpty()) {
            Matcher matcher = HTTPS_PATTERN.matcher(playlist);
            if (matcher.find()) {
                return Uri.parse(playlist.substring(matcher.start(), matcher.end()));
            } else {
                throw new IOException(TAG + ": Response did not contain a URL");
            }
        } else {
            throw new IOException(TAG + ": Response was empty");
        }
    }
}
