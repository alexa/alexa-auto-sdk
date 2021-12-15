# Bluetooth Extension

The Bluetooth extension allows the Alexa Auto SDK to connect to devices through the Bluetooth Classic or Bluetooth Low Energy (BLE) protocol. Using these protocols, the Auto SDK can offer Bluetooth-based features, such as Alexa Mobile Accessory (AMA) or Login with Phone, to users of Android or iOS smartphones.

<!-- omit in toc -->
## Table of Contents
- [Requirements for Bluetooth Classic or BLE](#requirements-for-bluetooth-classic-or-ble)
  - [Bluetooth Classic](#bluetooth-classic)
  - [BLE](#ble)
- [Choosing a Transport Protocol](#choosing-a-transport-protocol)
  - [Supporting Bluetooth Classic](#supporting-bluetooth-classic)
  - [Supporting BLE](#supporting-ble)
- [Registering the BluetoothProvider Platform Interface](#registering-the-bluetoothprovider-platform-interface)
- [Sequence Diagrams](#sequence-diagrams)
  - [Bluetooth Classic Connection](#bluetooth-classic-connection)
  - [BLE Connection](#ble-connection)
- [Examples of Implementation](#examples-of-implementation)
  - [Implementation for BluetoothProviderHandler](#implementation-for-bluetoothproviderhandler)
  - [Implementation for Bluetooth Classic](#implementation-for-bluetooth-classic)
  - [Implementation for BLE](#implementation-for-ble)
- [Requirement for Accepting Connections from Another Device](#requirement-for-accepting-connections-from-another-device)

## Requirements for Bluetooth Classic or BLE
The variant of the Bluetooth standard determines the tasks required to support the client.

### Bluetooth Classic
Complete the following tasks to support different clients:

* For Android phones, create a RFCOMM socket with the corresponding Service Discovery Protocol (SDP) record. (RFCOMM is a Bluetooth transport protocol.)
* For iOS phones, create an iOS Accessory Protocol 2 (iAP2) communication channel.

For information about these tasks, see [Supporting Bluetooth Classic](#supporting-bluetooth-classic-transport).

### BLE
Add a new Generic Attribute Profile (GATT) service and advertise the service. For information about this task, see [Supporting BLE](#supporting-ble-transport).

## Choosing a Transport Protocol

Consider the following factors when choosing the protocol to use with Bluetooth:

* Capabilities and limitations of remote devices (e.g., Android or iOS phones)
* Capabilities of the Bluetooth software stack, which includes the system service, driver, and firmware (e.g., whether the software supports RFCOMM or iAP2)
* Capabilities of the Bluetooth chipset used on the head unit (e.g., single-mode or dual-mode)

The following table shows the transport protocol to use based on the head unit chipset and the type of smartphone. 

>**Note:** To use Bluetooth for Login With Phone, you must use RFCOMM as the transport protocol.

|                   | Single-mode chipset | Dual-mode chipset |
| ----------------- | --------------------- | ------------------- |
| **iOS phone**     | iAP2                  | iAP2 or GATT        |
| **Android phone** | RFCOMM                | RFCOMM or GATT      |

### Supporting Bluetooth Classic
Follow one of these steps, depending on the transport protocol, to create a communication channel between the head unit and the phone:

* For RFCOMM communication, assign an unused RFCOMM channel to the head unit to listen on. The implementation must register an SDP record with the local SDP server, which is part of the Bluetooth software stack. The server contains the specified UUID, service name, and auto-assigned channel. Remote Bluetooth devices can use the same UUID to query the SDP server and discover the channel to connect to. 
  
    The SDP record is removed when the socket is closed or if the application closes unexpectedly. Android clients discover the head unit by using the method that is described in [BluetoothDevice.getUuids](https://developer.android.com/reference/android/bluetooth/BluetoothDevice#getUuids()).

* For iAP2, allocate a communication channel with the specified protocol identifier. For information about iAP2, see the Accessory Interface Specification for Apple Devices.

### Supporting BLE
In your implementation, create a new GATT service according to the JSON configuration specified in the `GATTServer.start` call. The following JSON shows a sample configuration:

```json
{
  "characteristics": [
    {
      "id": "A49921F7-9E7D-46F6-8832-9F44658892AC",
      "mtu": 104,
      "name": "Alexa Characteristic TX",
      "permissions": [
        "write"
      ],
      "properties": [
        "write"
      ]
    },
    {
      "descriptors": [
        {
          "id": "00002902-0000-1000-8000-00805f9b34fb",
          "name": "Configuration",
          "permissions": [
            "read",
            "write"
          ]
        }
      ],
      "id": "34D7A574-5298-4C35-8109-1EAA2E9476E8",
      "mtu": 104,
      "name": "Alexa Characteristic RX",
      "permissions": [
        "read"
      ],
      "properties": [
        "notify",
        "read"
      ]
    }
  ]
}
```

## Registering the BluetoothProvider Platform Interface

For Linux and QNX, register the following C++ platform interface with the Auto SDK:

```cpp
class BluetoothProvider : public aace::core::PlatformInterface {
    /**
     * Create a GATT Server.
     *
     * @return the created GATT server. nullptr if GATT is not supported.
     */
    virtual std::shared_ptr<GATTServer> createGATTServer();

    /**
     * Create an RFCOMM server socket and register the corresponding SDP record.
     *
     * @param name service name for SDP record
     * @param uuid uuid for SDP record
     * @return the created server socket. nullptr if any error occurs.
     */
    virtual std::shared_ptr<BluetoothServerSocket> listenUsingRfcomm(const std::string& name, const std::string& uuid);

    /**
     * Create an iAP2 server socket with specified protocol.
     *
     * @param protocol the protocol to use when communicating with the device
     * @return the created server socket. nullptr if any error occurs.
     */
    virtual std::shared_ptr<BluetoothServerSocket> listenUsingiAP2(const std::string& protocol);
};
```

For Android, register the following Java platform interface with the Auto SDK:

```java
public abstract class BluetoothProvider extends PlatformInterface {
    public GATTServer createGATTServer() {...}
    public BluetoothServerSocket listenUsingRfcomm(String name, String uuid) {...}
    public BluetoothServerSocket listenUsingiAP2(String protocol) {...}
}
```

## Sequence Diagrams
The sequence diagrams illustrate the flow for a Bluetooth Classic connection and the flow for a BLE connection.

### Bluetooth Classic Connection
<img src="./assets/bluetooth_classic.png" />
</p>

### BLE Connection

<img src="./assets/ble.png" />
</p>

## Examples of Implementation
This section provides examples of implementation to support Bluetooth Classic and BLE on Android.

>**Note:** Amazon does not provide reference implementation for Linux and QNX.

### Implementation for BluetoothProviderHandler

The following example shows the implementation for `BluetoothProviderHandler`, which is needed for either Bluetooth Classic or BLE connections:

```java
public class BluetoothProviderHandler extends BluetoothProvider {
    private final Activity mActivity;
    private final LoggerHandler mLogger;

    public BluetoothProviderHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
    }

    @Override
    public GATTServer createGATTServer() {
        return new GATTServerHandler(mActivity, mLogger);
    }

    @Override
    public BluetoothServerSocket listenUsingRfcomm(String name, String uuid) {
        return new BluetoothServerSocketHandler(mActivity, name, uuid);
    }
}
```

### Implementation for Bluetooth Classic

The code samples in this section illustrate how to use  `BluetoothServerSocketHandler` and `BluetoothSocketHandler` for Bluetooth Classic connections:

```java
public class BluetoothServerSocketHandler extends com.amazon.aace.bluetooth.BluetoothServerSocket {
    private static final String TAG = "BluetoothSSHandler";
    private BluetoothServerSocket mServerSocket;

    BluetoothServerSocketHandler(Context context, String name, String uuid) throws IOException {
        BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        BluetoothAdapter bluetoothAdapter = bluetoothManager.getAdapter();
        mServerSocket = bluetoothAdapter.listenUsingRfcommWithServiceRecord(name, UUID.fromString(uuid));
    }

    @Override
    public com.amazon.aace.bluetooth.BluetoothSocket accept() {
        try {
            return new BluetoothSocketHandler(mServerSocket.accept());
        } catch (IOException e) {
            Log.e(TAG, "failed to accept incoming connection", e);
        }
        return null;
    }
}
```

>**Note:** To create a listening, secure RFCOMM Bluetooth socket with an SDP record of a specified UUID, use `listenUsingRfcommWithServiceRecord` as described in the
[BluetoothAdapter documentation](https://developer.android.com/reference/android/bluetooth/BluetoothAdapter#listenUsingRfcommWithServiceRecord(java.lang.String,%20java.util.UUID)). The remote device connecting to this socket should be paired with the head unit first.

```java
public class BluetoothSocketHandler extends com.amazon.aace.bluetooth.BluetoothSocket {
    private static final String TAG = "BluetoothSocketHandler";
    BluetoothSocket mSocket;

    BluetoothSocketHandler(BluetoothSocket socket) {
        mSocket = socket;
    }

    @Override
    public int read(byte[] data, int off, int len) {
        try {
            return mSocket.getInputStream().read(data, off, len);
        } catch (IOException e) {
            Log.e(TAG, "failed to read", e);
        }
        return -1;
    }

    @Override
    public void write(byte[] data, int off, int len) {
        try {
            mSocket.getOutputStream().write(data, off, len);
        } catch (IOException e) {
            Log.e(TAG, "failed to write", e);
        }
    }

    @Override
    public void close() {
        try {
            mSocket.close();
        } catch (IOException e) {
            Log.e(TAG, "failed to close", e);
        }
    }
}
```

### Implementation for BLE

The following example supports BLE connections:

```java
public class GATTServerHandler extends GATTServer {
    private static final String TAG = GATTServerHandler.class.getSimpleName();

    private final Activity mActivity;
    private final LoggerHandler mLogger;

    private BluetoothManager mBluetoothManager;
    private BluetoothGattServer mGattServer;
    private BluetoothLeAdvertiser mAdvertiser;
    private BluetoothDevice mBluetoothDevice;

    public GATTServerHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        mBluetoothManager = (BluetoothManager) activity.getSystemService(Context.BLUETOOTH_SERVICE);
    }

    @Override
    public boolean start(String configuration) {
        // Open the GATT server
        mGattServer = mBluetoothManager.openGattServer(mActivity, mGattServerCallback);

        // Parse the json services specified in the config
        // ...

        // Add the services 
        mGattServer.addService(gattService);

        // Start advertising the service
        mAdvertiser = BluetoothAdapter.getDefaultAdapter().getBluetoothLeAdvertiser();
        mAdvertiser.startAdvertising(advertiseSettings.build(), advertiseData.build(), mAdvertisingCallback);

        return true;
    }

    @Override
    public boolean setCharacteristicValue(String serviceId, String characteristicId, byte[] data) {
        BluetoothGattCharacteristic characteristic = mGattServer
                .getService(UUID.fromString(serviceId))
                .getCharacteristic(UUID.fromString(characteristicId));
        characteristic.setValue(data);
        mGattServer.notifyCharacteristicChanged(mBluetoothDevice, characteristic, false);
    }

    @Override
    public boolean stop() {...}

    private final BluetoothGattServerCallback mGattServerCallback = new BluetoothGattServerCallback() {
        @Override
        public void onConnectionStateChange(BluetoothDevice device, int status, int newState) {
            switch (newState) {
                case BluetoothProfile.STATE_CONNECTED:
                    mBluetoothDevice = device;
                    connectionStateChanged(device.getAddress(), ConnectionState.CONNECTED);
                    break;
                case BluetoothProfile.STATE_DISCONNECTED:
                    mBluetoothDevice = null;
                    connectionStateChanged(device.getAddress(), ConnectionState.DISCONNECTED);
                    break;
            }
        }

        @Override
        public void onCharacteristicReadRequest(BluetoothDevice device, int requestId, int offset, BluetoothGattCharacteristic characteristic) {
            mGattServer.sendResponse(device, requestId, BluetoothGatt.GATT_SUCCESS, offset, characteristic.getValue());
        }

        @Override
        public void onCharacteristicWriteRequest(BluetoothDevice device, int requestId, BluetoothGattCharacteristic characteristic, boolean preparedWrite, boolean responseNeeded, int offset, byte[] value) {
            // let the engine impl handle the request
            requestCharacteristic(device.getAddress(), requestId, service.getUuid().toString(), characteristic.getUuid().toString(), value);
        }
    }
}
```

You can determine the BLE advertising power level and frequency. The following values are recommended:

* `AdvertiseSettings.ADVERTISE_MODE_BALANCED`
* `AdvertiseSettings.ADVERTISE_TX_POWER_MEDIUM`

## Requirement for Accepting Connections from Another Device
The Alexa app hosting either the GATT service or RFCOMM server socket must run in the background to accept connections from another device.