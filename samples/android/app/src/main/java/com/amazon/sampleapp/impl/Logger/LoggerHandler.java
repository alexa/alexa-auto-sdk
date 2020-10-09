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

package com.amazon.sampleapp.impl.Logger;

import android.graphics.Color;
import android.util.Log;

import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.core.LoggerControllerInterface;
import com.amazon.sampleapp.logView.LogEntry;
import com.amazon.sampleapp.logView.LogRecyclerViewAdapter;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.Locale;
import java.util.Observable;
import java.util.Observer;
import java.util.Queue;

public class LoggerHandler extends Logger implements LoggerControllerInterface {
    /* Log colors */
    private static final int sColorVerbose = Color.parseColor("#B3E5FC"); // Light Blue
    private static final int sColorInfo = Color.parseColor("#FFFFFF"); // White
    private static final int sColorMetric = Color.parseColor("#73C54C"); // Light Green
    private static final int sColorWarn = Color.parseColor("#F57F17"); // Orange
    private static final int sColorError = Color.parseColor("#D50000"); // Red
    private static final int sColorJsonTemplate = Color.parseColor("#F9B702"); // Gold

    private static final SimpleDateFormat sTimeFormat // Note: not thread safe
            = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.getDefault());
    private static final Level sLevel = Level.VERBOSE;
    private static final String sClientSourceTag = "CLI";

    private LoggerObservable mObservable;

    private Queue<LogEntry> mDisplayLogsQueue = new LinkedList<LogEntry>();
    private boolean mIsEnqueuingLogs = false;

    public LoggerHandler() {
        mObservable = new LoggerObservable();
    }

    // Handle log from Auto SDK
    @Override
    public boolean logEvent(Level level, long time, String source, String message) {
        if (level.ordinal() >= sLevel.ordinal()) {
            int color;
            switch (level) {
                case VERBOSE:
                    color = sColorVerbose;
                    break;
                case WARN:
                    color = sColorWarn;
                    break;
                case CRITICAL:
                case ERROR:
                    color = sColorError;
                    break;
                case INFO:
                    color = sColorInfo;
                    break;
                case METRIC:
                    color = sColorMetric;
                    break;
                default:
                    color = sColorInfo;
            }

            // Configure log for GUI log view
            JSONObject json = new JSONObject();
            try {
                json.put("text",
                        String.format("%-25s [%s] %3c%n%s", sTimeFormat.format(time), source, level.toChar(), message));
                json.put("textColor", color);
                json.put("source", source); // For log view filtering
                json.put("level", level.toString()); // For log view filtering
            } catch (JSONException e) {
                Log.e(sClientSourceTag, "Error: ", e);
                return true;
            }
            mObservable.log(json);
        }
        return true;
    }

    /* Client level log methods. Will use Auto SDK Logger */

    public void postVerbose(String tag, String message) {
        log(Level.VERBOSE, tag, message);
    }
    public void postInfo(String tag, String message) {
        log(Level.INFO, tag, message);
    }
    public void postWarn(String tag, String message) {
        log(Level.WARN, tag, message);
    }
    public void postError(String tag, String message) {
        log(Level.ERROR, tag, message);
    }
    public void postError(String tag, Throwable thr) {
        try (ByteArrayOutputStream os = new ByteArrayOutputStream()) {
            PrintStream ps = new PrintStream(os);
            thr.printStackTrace(ps);
            String str = os.toString();
            log(Level.ERROR, tag, str);

        } catch (IOException e) {
            Log.e(sClientSourceTag, "Error: ", e);
        }
    }

    /* Additional client logs. Will insert log into GUI log view but not use Auto SDK Logger */

    // Client log for JSON Templates
    public void postJSONTemplate(String tag, String message) {
        Date currentTime = Calendar.getInstance().getTime();
        Level level = Level.INFO;

        JSONObject json = new JSONObject();
        try {
            json.put("text",
                    String.format("%-25s [%s] %3c%n%s:%n%s", sTimeFormat.format(currentTime), sClientSourceTag,
                            level.toChar(), tag, message));
            json.put("textColor", sColorJsonTemplate);
            json.put("source", sClientSourceTag); // For log view filtering
            json.put("level", level.toString()); // For log view filtering

        } catch (JSONException e) {
            Log.e(sClientSourceTag, "Error: ", e);
            return;
        }

        synchronized (this) {
            if (mIsEnqueuingLogs) {
                mDisplayLogsQueue.add(new LogEntry(LogRecyclerViewAdapter.JSON_TEXT, json));
            } else {
                mObservable.log(json, LogRecyclerViewAdapter.JSON_TEXT);
            }
        }
    }

    // Client log for display cards
    public void postDisplayCard(JSONObject template, int logType) {
        Level level = Level.INFO;
        JSONObject json = new JSONObject();
        try {
            json.put("template", template);
            json.put("source", sClientSourceTag); // For log view filtering
            json.put("level", level.toString()); // For log view filtering

        } catch (JSONException e) {
            Log.e(sClientSourceTag, "Error: ", e);
        }

        synchronized (this) {
            if (mIsEnqueuingLogs) {
                mDisplayLogsQueue.add(new LogEntry(logType, json));
            } else {
                mObservable.log(json, logType);
            }
        }
    }

    public void pause() {
        synchronized (this) {
            mIsEnqueuingLogs = true;
        }
    }

    public void resume() {
        synchronized (this) {
            mIsEnqueuingLogs = false;

            while (!mDisplayLogsQueue.isEmpty()) {
                LogEntry entry = mDisplayLogsQueue.remove();
                int logType = entry.getType();
                JSONObject json = entry.getJSON();
                mObservable.log(json, logType);
            }
        }
    }

    /* Logger Observable for inserting logs into GUI log view */

    public static class LoggerObservable extends Observable {
        public void log(String message) {
            setChanged();
            notifyObservers(message);
        }

        public void log(JSONObject obj) {
            setChanged();
            notifyObservers(new LogEntry(LogRecyclerViewAdapter.TEXT_LOG, obj));
        }

        public void log(JSONObject obj, int logType) {
            setChanged();
            notifyObservers(new LogEntry(logType, obj));
        }
    }

    public void addLogObserver(Observer observer) {
        mObservable.addObserver(observer);
    }
}
