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

QDropboxFileInfo::QDropboxFileInfo(QDropboxFileInfo &other)
{
    copyFrom(other);
}

void QDropboxFileInfo::copyFrom(QDropboxFileInfo &other)
{
}

QDropboxFileInfo &QDropboxFileInfo::operator =(QDropboxFileInfo &other)
{
    copyFrom(other);
}

void QDropboxFileInfo::dataFromJson(QDropboxJson json)
{
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
