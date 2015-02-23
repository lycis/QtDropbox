#include "dropboxfileinfo.h"

DropboxFileInfo::DropboxFileInfo(QObject *parent) :
    DropboxJson(parent)
{
  _init();
}

DropboxFileInfo::DropboxFileInfo(QString jsonStr, QObject *parent) :
    DropboxJson(jsonStr, parent)
{
    _init();
    dataFromJson();
}

DropboxFileInfo::DropboxFileInfo(const DropboxFileInfo &other) :
    DropboxJson(0)
{
    _init();
    copyFrom(other);
}

DropboxFileInfo::~DropboxFileInfo()
{
  if(_content != NULL)
    delete _content;
}

void DropboxFileInfo::copyFrom(const DropboxFileInfo &other)
{
	parseString(other.strContent());
	dataFromJson();
	setParent(other.parent());
	return;
}

DropboxFileInfo &DropboxFileInfo::operator=(const DropboxFileInfo &other)
{
    copyFrom(other);
    return *this;
}

void DropboxFileInfo::dataFromJson()
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
#ifdef QT_DEBUG
	  qDebug() << "fileinfo: generating contents list";
#endif
      _content = new QList<DropboxFileInfo>();
	  QStringList contentsArray = getArray("contents");
	  for(qint32 i = 0; i<contentsArray.size(); ++i)
	  {
        DropboxFileInfo contentInfo(contentsArray.at(i));
		if(!contentInfo.isValid())
		  continue;
		
		_content->append(contentInfo);
	  }
	}

	return;
}

void DropboxFileInfo::_init()
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

QString DropboxFileInfo::revisionHash()  const
{
	return _revisionHash;
}

bool DropboxFileInfo::isDeleted()  const
{
	return _isDeleted;
}


QString DropboxFileInfo::mimeType()  const
{
	return _mimeType;
}

bool DropboxFileInfo::isDir()  const
{
	return _isDir;
}

QString DropboxFileInfo::path()  const
{
	return _path;
}

QString DropboxFileInfo::root()  const
{
	return _root;
}

QString DropboxFileInfo::icon()  const
{
	return _icon;
}

QDateTime DropboxFileInfo::clientModified()
{
	return _clientModified;
}

QDateTime DropboxFileInfo::modified()
{
	return _modified;
}

quint64 DropboxFileInfo::bytes()  const
{
	return _bytes;
}

bool DropboxFileInfo::thumbExists()  const
{
	return _thumbExists;
}

quint64 DropboxFileInfo::revision() const
{
	return _revision;
}

QString DropboxFileInfo::size() const
{
	return _size;
}

QList<DropboxFileInfo> DropboxFileInfo::contents() const
{
   if(_content == NULL || !isDir())
   {
     QList<DropboxFileInfo> l;
	 l.clear();
     return l;
   }
   
   return *_content;
}
