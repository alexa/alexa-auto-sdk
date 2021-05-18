package com.amazon.alexa.auto.comms.ui.db;

import androidx.lifecycle.LiveData;
import androidx.room.Dao;
import androidx.room.Delete;
import androidx.room.Insert;
import androidx.room.OnConflictStrategy;
import androidx.room.Query;
import androidx.room.Update;

import java.util.List;

@Dao
public interface ConnectedBTDeviceDao {
    @Query("SELECT * FROM ConnectedBTDevice")
    LiveData<List<ConnectedBTDevice>> getConnectedDevices();

    @Query("SELECT * FROM ConnectedBTDevice ORDER BY id desc")
    LiveData<List<ConnectedBTDevice>> getDescConnectedDevices();

    @Query("SELECT * FROM ConnectedBTDevice")
    List<ConnectedBTDevice> getConnectedDevicesSync();

    @Query("SELECT * FROM ConnectedBTDevice WHERE deviceAddress = :deviceAddress")
    ConnectedBTDevice getConnectedDeviceByAddress(String deviceAddress);

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    void insertConnectedBTDevice(ConnectedBTDevice btDevice);

    @Delete
    void deleteConnectedBTDevice(ConnectedBTDevice btDevice);

    @Update
    void updateConnectedBTDevice(ConnectedBTDevice btDevice);
}
