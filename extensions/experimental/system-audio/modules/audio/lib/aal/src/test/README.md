# AAL Unit Tests

AAL Unit Tests are built with [Googletest](https://github.com/google/googletest).

## Running player tests

Refer to [Running Test Programs: Advanced Options](https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#running-test-programs-advanced-options) for details about specifying which tests to run.

Several new parameters are added to allow customizing tests for different setup and platforms:

    --audio-file <audio file>                 Example: file:///path/to/audio
    --device <device name>                    Optional. Example: hw:0,0
    --aal-module <AAL module>                 Optional. Example: GStreamer, OpenMAX AL, or QSA
    --iterations <iterations>                 Number of iterations

- For most tests, you need to specify an audio file with `--audio-file`.
- `--device` is optional. If omitted, the default output/input device will be used.
- `--aal-module-name` is optional. If omitted, the first supported module will be used. The supported values are:
    - `GStreamer`
    - `QSA`
    - `OpenMAX AL`  

Here is an example to run `StressTest.RepeatedStops` test with an audio file for 1000 times.

```
$ player --gtest_filter=StressTest.RepeatedStops --audio-file file:///path/to/audio/file --iterations 1000
```

## Logging

If you would like to see logs printed during testing, define `AAL_DEBUG` to enable logging. The easiest way is to add the following line to `CMakeLists.txt` of AAL: 

```
add_definitions(-DAAL_DEBUG)
```
