# Qt Dropbox: Unit Tests

## Introduction
This subproject builds a test application that verifies if QtDropbox is working correctly.

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

