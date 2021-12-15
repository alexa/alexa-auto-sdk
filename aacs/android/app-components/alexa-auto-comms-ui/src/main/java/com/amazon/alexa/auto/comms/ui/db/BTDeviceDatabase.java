package com.amazon.alexa.auto.comms.ui.db;

import androidx.room.Database;
import androidx.room.RoomDatabase;
;

@Database(entities = {BTDevice.class}, version = 1, exportSchema = false)
public abstract class BTDeviceDatabase extends RoomDatabase {
    public abstract BTDeviceDao btDeviceDao();
}
