package com.amazon.maccandroid;

import android.net.Uri;

import org.mockito.Mockito;
import org.robolectric.annotation.Implementation;
import org.robolectric.annotation.Implements;
import org.robolectric.annotation.RealObject;

@Implements(Uri.Builder.class)
public class ShadowUri {
    @RealObject
    protected Uri.Builder uriBuilder;

    public void __constructor__() {}

    @Implementation
    public Uri.Builder scheme(String scheme) {
        return uriBuilder;
    }

    @Implementation
    public Uri.Builder path(String path) {
        return uriBuilder;
    }

    @Implementation
    public Uri.Builder appendQueryParameter(String key, String value) {
        return uriBuilder;
    }

    @Implementation
    public Uri build() {
        return Mockito.mock(Uri.class);
    }
}
