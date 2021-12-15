package com.amazon.alexa.auto.comms.ui.db;

import androidx.room.Database;
import androidx.room.RoomDatabase;

@Database(entities = {ConnectedBTDevice.class}, version = 1, exportSchema = false)
public abstract class ConnectedBTDeviceDatabase extends RoomDatabase {
    public abstract ConnectedBTDeviceDao connectedBTDeviceDao();
}
