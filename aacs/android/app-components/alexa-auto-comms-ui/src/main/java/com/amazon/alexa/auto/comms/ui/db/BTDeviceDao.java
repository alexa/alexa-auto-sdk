/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
