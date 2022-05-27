# DeviceUsage Interface

Periodically publish the [`DeviceUsage.ReportNetworkDataUsage`](https://alexa.github.io/alexa-auto-sdk/docs/aasb/core/DeviceUsage/index.html#reportnetworkdatausage) message (for example, at five minute intervals) to report network data usage to the Engine. If your application uses the `Device Client Metrics (DCM)` extension, the Engine records metrics with this information.

The `usage` field in the payload is a JSON object as a string. The format of the JSON is the following:

```
{
  "startTimeStamp" : {{LONG}},
  "endTimeStamp" : {{LONG}},
  "networkInterfaceType": "{{STRING}}",
  "dataPlanType" : "{{STRING}}",
  "bytesUsage" :{
      "rxBytes" : {{LONG}},
      "txBytes" : {{LONG}}
   }
}
```

The following table describes the properties in the JSON:

| Property               | Type   | Required | Description                                                                                                                                                                  | Example                                   |
| ---------------------- | ------ | -------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------- |
| startTimeStamp         | Long   | Yes      | The starting timestamp in milliseconds for this network usage datapoint                                                                                         | —                                         |
| endTimeStamp           | Long   | Yes     | The ending timestamp in milliseconds for this network usage datapoint                                                       | —                                         |
| networkInterfaceType   | String | Yes      | The name of the network interface over which the data is recorded                                                                                                                   | "WIFI",<br>"MOBILE"                       |
| dataPlanType           | String | No       | The type of data plan the device is subscribed to. This is an optional field and should be provided if your application uses the `AlexaConnectivity` module. | See [`AlexaConnectivity`](https://alexa.github.io/alexa-auto-sdk/docs/explore/features/connectivity/) |
| bytesUsage<br>.rxBytes | Long   | Yes      | The number of bytes received over the network interface during the time range represented by this datapoint                                                                                                                               | —                                         |
| bytesUsage<br>.txBytes | Long   | Yes      | The number of bytes transmitted over the network interface during the time range represented by this datapoint                                                                                                                             | —                                         |