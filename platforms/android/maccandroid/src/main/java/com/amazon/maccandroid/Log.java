package com.amazon.maccandroid;

public class Log {
    private static final String PRETAG = "MACCAndroid-";

    public static void i(String tag, String msg) {
        android.util.Log.i(PRETAG + tag, msg);
    }

    public static void d(String tag, String msg) {
        android.util.Log.d(PRETAG + tag, msg);
    }

    public static void e(String tag, String msg) {
        android.util.Log.e(PRETAG + tag, msg);
    }

    public static void e(String tag, String msg, Exception e) {
        android.util.Log.e(PRETAG + tag, msg, e);
    }

    public static void w(String tag, String msg) {
        android.util.Log.w(PRETAG + tag, msg);
    }

    public static void i(String tag, String s, Exception e) {
        android.util.Log.i(tag, s, e);
    }
}
