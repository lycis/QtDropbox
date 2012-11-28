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

	_size         = getString("size");
	_revision     = getUInt("revision");
	_thumbExists  = getBool("thumb_exists");
	_bytes        = getUInt("bytes");
	_icon         = getString("icon");
	_root         = getString("root");
	_path         = getString("path");
	_isDir        = getBool("is_path");
	_mimeType     = getString("mime_type");
	_isDeleted    = getBool("is_deleted");
	_revisionHash = getString("rev");
	_modified     = getTimestamp("modified");
	_clientModified = getTimestamp("client_modified");
	return;
}

void QDropboxFileInfo::_init()
{
    _size           = "";
    _revision       = 0;
    _thumbExists    = false;
    _bytes          = 0;
    _modified       = QDateTime::currentDateTime();
    _clientModified = QDateTime::currentDateTime();
    _icon           = "";
    _root           = "";
	_path           = "";
	_isDir          = false;
	_mimeType       = "";
	_isDeleted      = false;
	_revisionHash   = "";
    return;
}

QString QDropboxFileInfo::revisionHash()
{
	return _revisionHash;
}

bool QDropboxFileInfo::isDeleted()
{
	return _isDeleted;
}


QString QDropboxFileInfo::mimeType()
{
	return _mimeType;
}

bool QDropboxFileInfo::isDir()
{
	return _isDir;
}

QString QDropboxFileInfo::path()
{
	return _path;
}

QString QDropboxFileInfo::root()
{
	return _root;
}

QString QDropboxFileInfo::icon()
{
	return _icon;
}

QDateTime QDropboxFileInfo::clientModified()
{
	return _clientModified;
}

QDateTime QDropboxFileInfo::modified()
{
	return _modified;
}

quint64 QDropboxFileInfo::bytes()
{
	return _bytes;
}

bool QDropboxFileInfo::thumbExists()
{
	return _thumbExists;
}

quint64 QDropboxFileInfo::revision()
{
	return _revision;
}

QString QDropboxFileInfo::size()
{
	return _size;
}
