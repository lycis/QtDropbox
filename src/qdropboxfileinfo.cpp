#include "qdropboxfileinfo.h"

QDropboxFileInfo::QDropboxFileInfo(QObject *parent) :
    QDropboxJson(parent)
{
}

QDropboxFileInfo::QDropboxFileInfo(QString jsonStr, QObject *parent) :
    QDropboxJson(jsonStr, parent)
{
    dataFromJson();
}

QDropboxFileInfo::QDropboxFileInfo(QDropboxFileInfo &other) :
    QDropboxJson(0)
{
    copyFrom(other);
}

void QDropboxFileInfo::copyFrom(QDropboxFileInfo &other)
{
	parseString(other.strContent());
	dataFromJson();
	return;
}

QDropboxFileInfo &QDropboxFileInfo::operator =(QDropboxFileInfo &other)
{
    copyFrom(other);
    return *this;
}

void QDropboxFileInfo::dataFromJson()
{
	if(!isValid())
		return;

	_size        = getString("size");
	_revision    = getString("revision");
	_thumbExists = getBool("thumb_exists");
	_bytes       = getUInt("bytes");
	_icon        = getString("icon");
	_root        = getString("root");
	return;
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

QString QDropboxFileInfo::size()
{
	return _size;
}
