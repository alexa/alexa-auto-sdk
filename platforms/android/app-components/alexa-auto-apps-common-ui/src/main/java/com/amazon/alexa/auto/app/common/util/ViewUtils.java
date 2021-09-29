package com.amazon.alexa.auto.app.common.util;

import android.view.View;

import static android.view.View.GONE;
import static android.view.View.INVISIBLE;
import static android.view.View.VISIBLE;

/**
 * Contains reusable view utilities
 */
public class ViewUtils {

    public static void toggleViewVisibility(View view, int visible) {
        switch (visible) {
            case VISIBLE:
                view.setVisibility(GONE);
                break;
            case GONE:
            case INVISIBLE:
                view.setVisibility(VISIBLE);
                break;
        }
    }
}

