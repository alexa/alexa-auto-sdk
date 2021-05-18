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
public interface BTDeviceDao {
    @Query("SELECT * FROM BTDevice WHERE deviceAddress = :deviceAddress")
    LiveData<BTDevice> getBTDeviceByAddress(String deviceAddress);

    @Query("SELECT * FROM BTDevice WHERE deviceAddress = :deviceAddress")
    BTDevice getBTDeviceByAddressSync(String deviceAddress);

    @Query("SELECT * FROM BTDevice")
    List<BTDevice> getDevicesSync();

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    void insertBTDevice(BTDevice btDevice);

    @Delete
    void deleteBTDevice(BTDevice btDevice);

    @Update
    void updateBTDevice(BTDevice btDevice);
}
