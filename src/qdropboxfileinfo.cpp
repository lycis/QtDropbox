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

QDropboxFileInfo::QDropboxFileInfo(const QDropboxFileInfo &other) :
    QDropboxJson(0)
{
    copyFrom(other);
}

void QDropboxFileInfo::copyFrom(const QDropboxFileInfo &other)
{
	parseString(other.strContent());
	dataFromJson();
	setParent(other.parent());
	return;
}

QDropboxFileInfo &QDropboxFileInfo::operator=(const QDropboxFileInfo &other)
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

QString QDropboxFileInfo::revisionHash()  const
{
	return _revisionHash;
}

bool QDropboxFileInfo::isDeleted()  const
{
	return _isDeleted;
}


QString QDropboxFileInfo::mimeType()  const
{
	return _mimeType;
}

bool QDropboxFileInfo::isDir()  const
{
	return _isDir;
}

QString QDropboxFileInfo::path()  const
{
	return _path;
}

QString QDropboxFileInfo::root()  const
{
	return _root;
}

QString QDropboxFileInfo::icon()  const
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

quint64 QDropboxFileInfo::bytes()  const
{
	return _bytes;
}

bool QDropboxFileInfo::thumbExists()  const
{
	return _thumbExists;
}

quint64 QDropboxFileInfo::revision() const
{
	return _revision;
}

QString QDropboxFileInfo::size() const
{
	return _size;
}
