[![Build Status](https://travis-ci.org/lycis/QtDropbox.png?branch=master)](https://travis-ci.org/lycis/QtDropbox)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/1639/badge.svg)](https://scan.coverity.com/projects/1639)

# QtDropbox

## In short
QtDropbox is an API for the well known cloud storage service [Dropbox](http://www.dropbox.com).

## A bit longer
Basically QtDropbox aims to provide an easy to use possibility to access the REST API of
Dropbox. All HTTP calls are hidden behind the curtains of neat C++/Qt classes with nice 
method names and specific uses.

## Different Qt versions
The project is targeting the most recent version of Qt and thus was ported to Qt5. In the
beginning the project was developed for Qt4 and as there are some projects based on Qt4 
out there the legacy version is still being supported.

The master branch always provides the most recent version of the project and at the moment
this is the Qt5 version.

### Checking out legacy versions
Legacy versions such as the one supporting Qt4 are provided in specific branches. Here is a
short list of which branch to check out for the specific legacy versions:

* Qt4.x -> qt4

Mind that the branch with name `qt5` is currently an unused stub!

### Support for legacy versions
The ongoing development focuses on the `master` branch first. This means that legacy versions 
are usually not further improved with new features. Bugfixes will be provided though!

This should not indicate that legacy versions won't receive important new features but they 
are rather implemented on request only. If there is a specific feature that is already 
implemented in the most recent version but you need it in a legacy version (e.g. Qt4.x) 
just open an issue.

## Development
QtDropbox is in ongoing development so not all features provided by the Dropbox API are available
right now. If you have some knowledge about C++ (with Qt framework and/or the Dropbox REST API) you
are welcome to contribute to this project. For details take a look at the 
[project webpage](http://lycis.github.com/QtDropbox/)

### Current status
QtDropbox is constantly improved and developed further to include all possible requests you could
send to Dropbox and to simplify access. Below is a list of features that are currently available and
tested:

Currently in progress:
* Complete documentation
* Examples

## Features
### General
* Connect to Dropbox
* Access user account information (quotas, user name, share links, ...)
* Parse JSON strings

### File Access
* Access files like a local QFile to read and write data
* Access file and directory metadata
* Access file revisions
* Reading file information and metadata

Postponed to next version:
* Acessing and traversing the directory structure

## Documentation
You can generate a documentation of all classes by:
    
    qmake
    make documentation

This will generate a directory called doxy/ that contains a HTML documentation. Input files
to generate a LaTeX based configuration are supplied as well.

## Further information
There are some files apart this README that may provide some useful information:

* LICENSE
  It's LGPL v3 although that is currently not mentioned in the files
* INSTALL.md
  Installation and usage instructions
* doc/
  Generally everthing inside this directory is information.
* doc/DEVELOPMENT.md
  Development guidelines. Please read if you want to contribute!
