#include "dropboxfile.h"

DropboxFile::DropboxFile(QObject *parent) :
    QIODevice(parent),
    _networkAccessManager(this)
{
    _init(NULL, "", 1024);
    connectSignals();
}

DropboxFile::DropboxFile(Dropbox *api, QObject *parent) :
    QIODevice(parent),
    _networkAccessManager(this)
{
    _init(api, "", 1024);
    obtainToken();
    connectSignals();
}

DropboxFile::DropboxFile(QString filename, Dropbox *api, QObject *parent) :
    QIODevice(parent),
    _networkAccessManager(this)
{
    _init(api, filename, 1024);
   obtainToken();
   connectSignals();
}

DropboxFile::~DropboxFile()
{
    if(_buffer != NULL)
        delete _buffer;
    if(_evLoop != NULL)
        delete _evLoop;
}

bool DropboxFile::isSequential() const
{
    return true;
}

bool DropboxFile::open(QIODevice::OpenMode mode)
{
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::open(...)" << endl;
#endif
    if(!QIODevice::open(mode))
        return false;

  /*  if(isMode(QIODevice::NotOpen))
        return true; */

    if(_buffer == NULL)
        _buffer = new QByteArray();

#ifdef QT_DEBUG
    qDebug() << "QDropboxFile: opening file" << endl;
#endif

	// clear buffer and reset position if this file was opened in write mode
	// with truncate - or if append was not set
	if(isMode(QIODevice::WriteOnly) && 
	   (isMode(QIODevice::Truncate) || !isMode(QIODevice::Append))
	  )
    {
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile: _buffer cleared." << endl;
#endif
        _buffer->clear();
		_position = 0;
    }
    else
    {
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile: reading file content" << endl;
#endif
        if(!getFileContent(_filename))
            return false;

		if(isMode(QIODevice::WriteOnly)) // write mode here means append
			_position = _buffer->size();
		else if(isMode(QIODevice::ReadOnly)) // read mode here means start at the beginning
			_position = 0;
    }

	obtainMetadata();		 

    return true;
}

void DropboxFile::close()
{
	if(isMode(QIODevice::WriteOnly))
		flush();
	QIODevice::close();
	return;
}

void DropboxFile::setApi(Dropbox *dropbox)
{
    _api = dropbox;
	return;
}

Dropbox *DropboxFile::api()
{
    return _api;
}

void DropboxFile::setFilename(QString filename)
{
    _filename = filename;
    return;
}

QString DropboxFile::filename()
{
    return _filename;
}

bool DropboxFile::flush()
{
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::flush()" << endl;
#endif

    return putFile();
}

bool DropboxFile::event(QEvent *event)
{
#ifdef QT_DEBUG
    qDebug() << "processing event: " << event->type() << endl;
#endif
    return QIODevice::event(event);
}

void DropboxFile::setFlushThreshold(qint64 num)
{
    if(num<0)
        num = 0;
    _bufferThreshold = num;
    return;
}

qint64 DropboxFile::flushThreshold()
{
    return _bufferThreshold;
}

void DropboxFile::setOverwrite(bool overwrite)
{
    _overwrite = overwrite;
    return;
}

bool DropboxFile::overwrite()
{
    return _overwrite;
}

qint64 DropboxFile::readData(char *data, qint64 maxlen)
{
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::readData(...), maxlen = " << maxlen << endl;
    QString buff_str = QString(*_buffer);
    qDebug() << "old bytes = " << _buffer->toHex() << ", str: " << buff_str <<  endl;
    qDebug() << "old size = " << _buffer->size() << endl;
#endif

	if(_buffer->size() == 0 || _position >= _buffer->size())
        return 0;

    if(_buffer->size() < maxlen)
        maxlen = _buffer->size();

	QByteArray tmp = _buffer->mid(_position, maxlen);
	const qint64 read = tmp.size();
	memcpy(data, tmp.data(), read);
   
#ifdef QT_DEBUG
    qDebug() << "new size = " << _buffer->size() << endl;
    qDebug() << "new bytes = " << _buffer->toHex() << endl;
#endif

	_position += read;

    return read;
}

qint64 DropboxFile::writeData(const char *data, qint64 len)
{
#ifdef QT_DEBUG
    qDebug() << "old content: " << _buffer->toHex() << endl;
#endif

	qint64 oldlen = _buffer->size();
    _buffer->insert(_position, data, len);

#ifdef QT_DEBUG
    qDebug() << "new content: " << _buffer->toHex() << endl;
#endif

    // flush if the threshold is reached
    _currentThreshold += len;
    if(_currentThreshold > _bufferThreshold)
        flush();

	int written_bytes = len;

	if(_buffer->size() != oldlen+len)
		written_bytes = (oldlen-_buffer->size());

	_position += written_bytes;

    return written_bytes;
}

void DropboxFile::networkRequestFinished(QNetworkReply *rply)
{
    rply->deleteLater();

#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::networkRequestFinished(...)" << endl;
#endif

    if (rply->error() != QNetworkReply::NoError)
    {
        _lastErrorCode = rply->error();
        stopEventLoop();
        return;
    }

    switch(_waitMode)
    {
    case waitForRead:
        rplyFileContent(rply);
        stopEventLoop();
        break;
    case waitForWrite:
        rplyFileWrite(rply);
        stopEventLoop();
        break;
    case notWaiting:
		break; // when we are not waiting for anything, we don't do anything - simple!
    default:
#ifdef QT_DEBUG
		// debug information only - this should not happen, but if it does we 
		// ignore replies when not waiting for anything
		qDebug() << "QDropboxFile::networkRequestFinished(...) got reply in unknown state (" << _waitMode << ")" << endl;
#endif
        break;
    }
}

void DropboxFile::obtainToken()
{
    _token       = _api->token();
    _tokenSecret = _api->tokenSecret();
    return;
}

void DropboxFile::connectSignals()
{
    connect(&_networkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(networkRequestFinished(QNetworkReply*)));
    return;
}

bool DropboxFile::isMode(QIODevice::OpenMode mode)
{
    return ( (openMode()&mode) == mode );
}

bool DropboxFile::getFileContent(QString filename)
{
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::getFileContent(...)" << endl;
#endif
    QUrl request;
    request.setUrl(DROPBOXFILE_CONTENT_URL, QUrl::StrictMode);
    request.setPath(QString("/%1/files/%2")
                    .arg(_api->apiVersion().left(1))
                    .arg(filename));

    QUrlQuery query;
    query.addQueryItem("oauth_consumer_key", _api->appKey());
    query.addQueryItem("oauth_nonce", Dropbox::generateNonce(128));
    query.addQueryItem("oauth_signature_method", _api->signatureMethodString());
    query.addQueryItem("oauth_timestamp", QString::number((int) QDateTime::currentMSecsSinceEpoch()/1000));
    query.addQueryItem("oauth_token", _api->token());
    query.addQueryItem("oauth_version", _api->apiVersion());

    QString signature = _api->oAuthSign(request);
    query.addQueryItem("oauth_signature", signature);

    request.setQuery(query);

#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::getFileContent " << request.toString() << endl;
#endif

    QNetworkRequest rq(request);
    QNetworkReply *reply = _networkAccessManager.get(rq);
    connect(this, &DropboxFile::operationAborted, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::downloadProgress, this, &DropboxFile::downloadProgress);

    _waitMode = waitForRead;
    startEventLoop();

    if(_lastErrorCode != 0)
    {
#ifdef QT_DEBUG
        qDebug() << "QDropboxFile::getFileContent ReadError: " << _lastErrorCode << _lastErrorMessage << endl;
#endif
        if(_lastErrorCode ==  DROPBOX_ERROR_FILE_NOT_FOUND)
		{
			_buffer->clear();
#ifdef QT_DEBUG
        qDebug() << "QDropboxFile::getFileContent: file does not exist" << endl;
#endif
		}
		else
			return false;
    }

    return true;
}

void DropboxFile::rplyFileContent(QNetworkReply *rply)
{
    _lastErrorCode = 0;

    QByteArray response = rply->readAll();
    QString resp_str;
    DropboxJson json;

#ifdef QT_DEBUG
    resp_str = QString(response.toHex());
    qDebug() << "QDropboxFile::rplyFileContent response = " << resp_str << endl;

#endif

    switch(rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
    {
    case DROPBOX_ERROR_BAD_INPUT:
    case DROPBOX_ERROR_EXPIRED_TOKEN:
    case DROPBOX_ERROR_BAD_OAUTH_REQUEST:
    case DROPBOX_ERROR_FILE_NOT_FOUND:
    case DROPBOX_ERROR_WRONG_METHOD:
    case DROPBOX_ERROR_REQUEST_CAP:
    case DROPBOX_ERROR_USER_OVER_QUOTA:
        resp_str = QString(response);
        json.parseString(response.trimmed());
        _lastErrorCode = rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::rplyFileContent jason.valid = " << json.isValid() << endl;
#endif
        if(json.isValid())
            _lastErrorMessage = json.getString("error");
        else
            _lastErrorMessage = "";
        return;
        break;
    default:
        break;
    }

    _buffer->clear();
    _buffer->append(response);
    emit readyRead();
    return;
}

void DropboxFile::rplyFileWrite(QNetworkReply *rply)
{
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::rplyFileWrite(...)" << endl;
#endif

    _lastErrorCode = 0;

    QByteArray response = rply->readAll();
    QString resp_str;
    DropboxJson json;

#ifdef QT_DEBUG
    resp_str = response;
    qDebug() << "QDropboxFile::rplyFileWrite response = " << resp_str << endl;

#endif

    switch(rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
    {
    case DROPBOX_ERROR_BAD_INPUT:
    case DROPBOX_ERROR_EXPIRED_TOKEN:
    case DROPBOX_ERROR_BAD_OAUTH_REQUEST:
    case DROPBOX_ERROR_FILE_NOT_FOUND:
    case DROPBOX_ERROR_WRONG_METHOD:
    case DROPBOX_ERROR_REQUEST_CAP:
    case DROPBOX_ERROR_USER_OVER_QUOTA:
        resp_str = QString(response);
        json.parseString(response.trimmed());
        _lastErrorCode = rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::rplyFileWrite jason.valid = " << json.isValid() << endl;
#endif
        if(json.isValid())
            _lastErrorMessage = json.getString("error");
        else
            _lastErrorMessage = "";
        return;
        break;
    default:
        delete _dropboxFileInfo;

        _dropboxFileInfo = new DropboxFileInfo{QString{response}.trimmed(), this};
        if (!_dropboxFileInfo->isValid())
            _dropboxFileInfo->clear();
        break;
    }

    emit bytesWritten(_buffer->size());
    return;
}

void DropboxFile::startEventLoop()
{
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::startEventLoop()" << endl;
#endif
    if(_evLoop == NULL)
        _evLoop = new QEventLoop(this);
    _evLoop->exec();
    return;
}

void DropboxFile::stopEventLoop()
{
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::stopEventLoop()" << endl;
#endif
    if(_evLoop == NULL)
        return;
    _evLoop->exit();
    return;
}

bool DropboxFile::putFile()
{

#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::putFile()" << endl;
#endif

    QUrl request;
    request.setUrl(DROPBOXFILE_CONTENT_URL, QUrl::StrictMode);
    request.setPath(QString("/%1/files_put/%2")
                    .arg(_api->apiVersion().left(1))
                    .arg(_filename));

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("oauth_consumer_key", _api->appKey());
    urlQuery.addQueryItem("oauth_nonce", Dropbox::generateNonce(128));
    urlQuery.addQueryItem("oauth_signature_method", _api->signatureMethodString());
    urlQuery.addQueryItem("oauth_timestamp", QString::number((int) QDateTime::currentMSecsSinceEpoch()/1000));
    urlQuery.addQueryItem("oauth_token", _api->token());
    urlQuery.addQueryItem("oauth_version", _api->apiVersion());
    urlQuery.addQueryItem("overwrite", (_overwrite?"true":"false"));

    QString signature = _api->oAuthSign(request);
    urlQuery.addQueryItem("oauth_signature", signature);

    request.setQuery(urlQuery);

#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::put " << request.toString() << endl;
#endif

    QNetworkRequest rq(request);
    QNetworkReply *reply = _networkAccessManager.put(rq, *_buffer);
    connect(this, &DropboxFile::operationAborted, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::uploadProgress, this, &DropboxFile::uploadProgress);

    _waitMode = waitForWrite;	
    startEventLoop();

    if(_lastErrorCode != 0)
    {
#ifdef QT_DEBUG
        qDebug() << "QDropboxFile::putFile WriteError: " << _lastErrorCode << _lastErrorMessage << endl;
#endif
        return false;
    }

    _currentThreshold = 0;

    return true;
}

void DropboxFile::_init(Dropbox *api, QString filename, qint64 bufferTh)
{
    _api              = api;
    _buffer           = NULL;
    _filename         = filename;
    _evLoop           = NULL;
    _waitMode         = notWaiting;
    _bufferThreshold  = bufferTh;
    _overwrite        = true;
    _dropboxFileInfo         = NULL;
    _lastErrorCode     = 0;
    _lastErrorMessage  = "";
    _position         = 0;
    _currentThreshold = 0;
    return;
}


DropboxFileInfo DropboxFile::metadata()
{
    if(_dropboxFileInfo == NULL)
		obtainMetadata();

	return _api->requestMetadataAndWait(_filename);
}

bool DropboxFile::hasChanged()
{
    if(_dropboxFileInfo == NULL)
	{
		if(!metadata().isValid()) // get metadata
			return false;         // if metadata was invalid
	}

    DropboxFileInfo serverMetadata = _api->requestMetadataAndWait(_filename);
#ifdef QT_DEBUG
    qDebug() << "QDropboxFile::hasChanged() local  revision hash = " << _dropboxFileInfo->revisionHash() << endl;
    qDebug() << "QDropboxFile::hasChanged() remote revision hash = " << serverMetadata.revisionHash() << endl;
#endif
    return serverMetadata.revisionHash().compare(_dropboxFileInfo->revisionHash())!=0;
}

void DropboxFile::obtainMetadata()
{
	// get metadata of this file
    _dropboxFileInfo = new DropboxFileInfo(_api->requestMetadataAndWait(_filename).strContent(), this);
    if(!_dropboxFileInfo->isValid())
        _dropboxFileInfo->clear();
	return;
}

QList<DropboxFileInfo> DropboxFile::revisions(int max)
{
    QList<DropboxFileInfo> revisions = _api->requestRevisionsAndWait(_filename, max);
	if(_api->error() != Dropbox::NoError)
		revisions.clear();

	return revisions;
}

bool DropboxFile::seek(qint64 pos)
{
	if(pos > _buffer->size())
		return false;

	QIODevice::seek(pos);
	_position = pos;
	return true;
}

qint64 DropboxFile::pos() const
{
	return _position;
}

bool DropboxFile::reset()
{
	QIODevice::reset();
	_position = 0;
	return true;
}

void DropboxFile::abort()
{
    emit operationAborted();
}
