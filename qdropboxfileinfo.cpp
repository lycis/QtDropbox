#include "qdropboxfileinfo.h"

QDropboxFileInfo::QDropboxFileInfo(QObject *parent) :
    QDropboxJson(parent)
{
  _init();
}

QDropboxFileInfo::QDropboxFileInfo(QString jsonStr, QObject *parent) :
    QDropboxJson(jsonStr, parent)
{
    _init();
    dataFromJson();
}

QDropboxFileInfo::QDropboxFileInfo(const QDropboxFileInfo &other) :
    QDropboxJson(0)
{
    _init();
    copyFrom(other);
}

QDropboxFileInfo::~QDropboxFileInfo()
{
  if(_content != NULL)
    delete _content;
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
	_isDir        = getBool("is_dir");
	_mimeType     = getString("mime_type");
	_isDeleted    = getBool("is_deleted");
	_revisionHash = getString("rev");
	_modified     = getTimestamp("modified");
    _clientModified = getTimestamp("client_mtime");
	
	// create content list
	if(_isDir)
	{
#ifdef QTDROPBOX_DEBUG
	  qDebug() << "fileinfo: generating contents list";
#endif
	  _content = new QList<QDropboxFileInfo>();
	  QStringList contentsArray = getArray("contents");
	  for(qint32 i = 0; i<contentsArray.size(); ++i)
	  {
	    QDropboxFileInfo contentInfo(contentsArray.at(i));
		if(!contentInfo.isValid())
		  continue;
		
		_content->append(contentInfo);
	  }
	}

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
	_content        = NULL;
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

QList<QDropboxFileInfo> QDropboxFileInfo::contents() const
{
   if(_content == NULL || !isDir())
   {
     QList<QDropboxFileInfo> l;
	 l.clear();
     return l;
   }
   
   return *_content;
}
