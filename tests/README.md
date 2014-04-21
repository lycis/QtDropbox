# Qt Dropbox: Unit Tests

## Introduction
This subproject builds a test application that verifies if QtDropbox is working correctly.

## Dropbox App Keys
In order to compile and execute the tests you need to create a custom header file called keys.hpp
This file defines two macros that provide the Dropbox application key and shared secret for accessing Dropbox. These keys are needed to connect to Dropbox.

Example:
```
#define APP_KEY "myappkey"
#define APP_SECRET "mysecret"
```

## Build & Execute
You have to build QtDropbox first by using:

```
qmake
make
make install
```

Afterwards change to this subdirectory and execute:
```
cd tests # unless you've done that already
qmake
make
make install
cd ../lib
./qtdropboxtest
```

