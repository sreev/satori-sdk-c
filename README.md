C SDK for Satori Platform
=========


This C SDK is a lightweight SDK for POSIX systems (tested on MacOS and Linux)
It has no external dependencies, does not require a JSON library (but can work with any),
and supports running with any async event loop (e.g., libev or libevent).
The library does not have any dynamic memory allocation aside from `rtm_connect()` which allocates the single connection object. All message processing is done in-place.

Build
=====

For desktop/server
------------------

The build system is using [cmake](https://cmake.org/).

To build, just execute:
```sh
$ cmake .
$ cmake --build .
```

To build with tests and benchmarks, execute:
```sh
$ cmake . -Dtest=1 -Dbench=1
$ cmake --build .
```

All artifacts will be produced in the `target` directory.
To run the unit tests, first you need to create a credentials.json file:
```sh
$ cat credentials.json
{
  "endpoint": "ws://<SATORI_HOST>/",
  "appkey": "<APP KEY>"
}
```
after that, execute:
```sh
$ ./core/test/rtm_unit_tests
```
Similarly, you can run the benchmarks by running:
```sh
$ cmake . -Dbench=1 -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
$ ./core/bench/rtm_benchmark
```

TLS support
-----------

The SDK can take advantage of OpenSSL, GNUTLS or Apple SSL API for supporting secure (wss://) connections.
To enable that, pass one of "-DUSE_OPENSSL=ON", "-DUSE_GNUTLS=ON" or "-DUSE_APPLE_SSL=ON" respectively to CMake.

## RTM Framework for iOS

The SatoriSDK framework for iOS helps you easily integrate your iOS apps with Satori. Using the framework, you can publish and subscribe messages to RTM.

### Step 1: Build framework

There are two options to build the RTM framework for iOS:

##### Option 1 - Build directly from command-line

```sh
$ cd ios-framework/SatoriSDK
$ xcodebuild -project SatoriSDK.xcodeproj -scheme SatoriSDK-Universal -config <config-name> # where <config-name> can be Debug or Release. Default is Debug if -config option is not specified.
```
##### Option 2 - Build in Xcode IDE
```sh
$ open SatoriSDK.xcodeproj
$ # Select SatoriSDK-Universal target and build.
```
The SatoriSDK.framework will be built under ios-framework/build directory.

### Step 2: Add framework to your project

Once you build the framework, open your app's Xcode project and drag-and-drop the framework under "Embedded Binaries" section under the app's target. Choose "Copy items if needed" and "Create groups" in the dialog box.

### Step 3: Use the framework APIs

The SatoriSDK.framework provides you with both Objective-C and C APIs to integrate within your app. Use ```#import <SatoriSDK/SatoriSDK.h>``` in your application class to make use of these APIs. The Objective-C specific APIs are located in ```SatoriRtmConnection.h``` and C APIs can be found in ```rtm.h```

##### Objective-C Sample Code

```Objective-C

// create a new rtm instance with url and appKey
SatoriRtmConnection *rtm = [[SatoriRtmConnection alloc] initWithUrl:"url" andAppkey:"appkey"];

// connect to rtm and provide pdu data handler block
rtm_status status = [rtm connectWithPduHandler:^(SatoriPdu * _Nonnull pdu) {
        //Use pdu
    }];

// subscribe to a channel
unsigned int reqId;
[rtm subscribe:@"channel-name" andRequestId:&reqId];

// publish a string or json to a channel
unsigned int reqId;
[rtm publishString:@"Hello world" toChannel:@"channel-name" andRequestId:&reqId];
[rtm publishJson:@"{\"key\":\"value\"}" toChannel:@"channel-name" andRequestId:&reqId];

// Enable or disable verbose logging of all incoming and outgoing PDUs
rtm.enableVerboseLogging = YES;
rtm.enableVerboseLogging = NO;

// Use wait or waitWithTimeout methods to block until at least one data message gets processed
[rtm wait];
[rtm waitWithTimeout:15];

// For non-blocking wait, use poll
while([rtm poll] >= 0) { sleep(1); }

// Make sure to disconnect when rtm connection is no longer needed
[rtm disconnect];

```


For Windows (Visual Studio)
---------------------------

Assuming Visual Studio and cmake are installed, open Developer Command Prompt for Visual Studio.

```sh
C:\satori-sdk-c> mkdir vsprj
C:\satori-sdk-c> cd vsprj
C:\satori-sdk-c\vsprj> cmake -DBUILD_SHARED_LIBS=ON -G "Visual Studio 14 2015" ..
C:\satori-sdk-c\vsprj> msbuild satori-sdk-c.sln
```

Adjust the generator name from "Visual Studio 14 2015" to another if necessary.

For TLS support only "-DUSE_OPENSSL=ON" is supported at this time.

Additionally, WinSock subsystem must be initialized prior to connecting to RTM.

```C
    #include <WinSock2.h>
    #include <Windows.h>
    #include <WS2tcpip.h>

    ...

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        fprintf(stderr, "WSAStartup failed with code %d", err);
    }
```

Usage
=====
```C
rtm_client_t *rtm = (rtm_client_t*) malloc (rtm_client_size);
rtm_connect(rtm, "ws://xxx.api.satori.com/", "<APPKEY>", rtm_default_pdu_handler, NULL);
rtm_subscribe(rtm, "channel_a", NULL);
rtm_publish_string(rtm, "channel_a", "Hello, world", NULL);
rtm_publish_json(rtm, "channel_a", "{\"key\":\"value\", \"k2\":123}", NULL);
while (rtm_poll(rtm)>=0) { sleep(1); }
rtm_close(rtm);
free(rtm);
```

`rtm_poll(rtm)` can be substitued for `rtm_wait(rtm)` to avoid the sleep when there is nothing better to do.
```while (rtm_wait(rtm)>=0) {}```

All channel data events go to the `message_handler` specified in the call to `rtm_connect`, other notifications, such as acknowledgements go to the `event_handler`. The default handlers simply print out to stdout.

In order to use RTM receipts/acknowledgements, simply provide the last argument to the `subscribe`/`unsubscribe`/`publish` functions instead of `NULL`. This argument is of type `unsigned int` and will be generated by the SDK on every function call. The generated id will be stored in the provided pointer and will eventually show in one of the `event_handler` callbacks when the acknowledgement arrives on the wire. When passing `NULL` no receipts will be generated.

You can use `rtm_get_fd(rtm)` to get the underlying file descriptor in order to connect to a message loop / select / poll.
In such a case, simply call `rtm_poll(rtm)` whenever there is data that can be read from the socket.
`rtm_poll()` never blocks.
`rtm_wait()` is a blocking alternative to `rtm_poll()` which blocks until at least one data message gets processed. Use it if you have nothing better to do.
`rtm_publish_*()` may block if the network blocks.
`rtm_connect()` will block until the connection handshake is complete.
You can set the global `rtm_connect_timeout` to the maximum number of seconds to wait for the connection handshake to complete.

A global error handler can be set by setting `rtm_error_logger` to any function that takes a `const char* msg`. The default one prints errors to stderr.

Verbose logging of all incoming and outcoming PDUs
==================================================

You can enable dumping of all PDUs to stderr either from your code::

  rtm_connect(rtm, ...);
  rtm_enable_verbose_logging(rtm);

or by setting DEBUG_SATORI_SDK environment variable prior to running your application::

  export DEBUG_SATORI_SDK=1
  ./my_program


Missing functionality
=====================
- Handling of network errors is left to the user
- Providing access to the `position` in RTM messages (and during subscription)
