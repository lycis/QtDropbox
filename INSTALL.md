# QtDropbox Installation Guide

## Dependencies
To build and use QtDropbox you'll need the Qt C++ Framework with
version 4.7 or higher available for download at
[Qt Project](http://qt-project.org/).

To generate a documentation you need to have doxygen installed.

## Building
QtDropbox is built by using these commands:

    qmake
    make

If you want to generate a documentation use

    make documentation

After all binaries are compiled use

    make install

This will create the directories lib/ and qtdropbox/.

The lib/ directory contains the compiled QtDropbox library. These are
not automatically copied to your global library directory
(/usr/local/lib or /usr/lib on Linux) - you'll have to do this manually
if you wish them to be available system wide.

The qtdropbox/ directory contains all header files and the
libqtdropbox.pri project definitions file. You'll need to copy this
folder into your project that will use QtDropbox as it contains all
necessary definitions. See _Usage_ below for details.

## Usage
### Using with Qt projects
When including QtDropbox into your project you have to
include the libqtdropbox.pri project definitions file. This will add
all necessary header files to your project and link with the library.

The network module of Qt will automatically be added to your project
as it is required to run QtDropbox.

### Using with other C++ projects
QtDropbox is not intended to be used with non-Qt projects. If you
make it run - tell me :)

