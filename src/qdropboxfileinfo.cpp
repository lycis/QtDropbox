#include "qdropboxfileinfo.h"

QDropboxFileInfo::QDropboxFileInfo(QObject *parent) :
    QObject(parent)
{
}

QDropboxFileInfo::QDropboxFileInfo(QDropboxJson json, QObject *parent) :
    QObject(parent)
{
    dataFromJson(json);
}

QDropboxFileInfo::QDropboxFileInfo(QDropboxFileInfo &other) :
    QObject(0)
{
    copyFrom(other);
}

void QDropboxFileInfo::copyFrom(QDropboxFileInfo &other)
{
}

QDropboxFileInfo &QDropboxFileInfo::operator =(QDropboxFileInfo &other)
{
    copyFrom(other);
    return *this;
}

void QDropboxFileInfo::dataFromJson(QDropboxJson json)
{
    //! \todo implement!
}

void QDropboxFileInfo::_init()
{
    _size           = "";
    _revision       = "";
    _thumbExists    = false;
    _bytes          = 0;
    _modified       = QDateTime::currentDateTime();
    _clientModified = QDateTime::currentDateTime();
    _icon           = "";
    _root           = "";
    return;
}
