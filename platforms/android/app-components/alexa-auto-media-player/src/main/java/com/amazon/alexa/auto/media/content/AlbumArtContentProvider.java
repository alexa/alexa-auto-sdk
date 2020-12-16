package com.amazon.alexa.auto.media.content;

import static com.amazon.alexa.auto.media.Constants.CONTENT_PROVIDER_DRAWABLE_PATH_PREFIX;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Optional;

/**
 * Content Provider to allow fetching Album Art.
 */
public class AlbumArtContentProvider extends ContentProvider {
    private static final String TAG = AlbumArtContentProvider.class.getSimpleName();

    @Override
    public boolean onCreate() {
        return true;
    }

    @Override
    @Nullable
    public ParcelFileDescriptor openFile(@NonNull Uri uri, @NonNull String mode) throws FileNotFoundException {
        Context context = getContext();
        if (context == null || uri.getPath() == null) {
            return null;
        }

        File file = new File(uri.getPath());
        if (!file.exists()) {
            throw new FileNotFoundException(uri.getPath());
        }

        // Glide stores the files in Cache so the user must be fetching
        // content from cache and no where else.
        String cachePath = context.getCacheDir().getPath();
        if (!file.getPath().startsWith(cachePath)) {
            throw new FileNotFoundException(uri.getPath());
        }

        return ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY);
    }

    @Nullable
    @Override
    public Cursor query(@NonNull Uri uri, @Nullable String[] strings, @Nullable String s, @Nullable String[] strings1,
            @Nullable String s1) {
        return null;
    }

    @Nullable
    @Override
    public String getType(@NonNull Uri uri) {
        return null;
    }

    @Nullable
    @Override
    public Uri insert(@NonNull Uri uri, @Nullable ContentValues contentValues) {
        return null;
    }

    @Override
    public int delete(@NonNull Uri uri, @Nullable String s, @Nullable String[] strings) {
        return 0;
    }

    @Override
    public int update(
            @NonNull Uri uri, @Nullable ContentValues contentValues, @Nullable String s, @Nullable String[] strings) {
        return 0;
    }
}
