#include "qdropboxfile.h"

QDropboxFile::QDropboxFile(QObject *parent) :
    QIODevice(parent),
    _conManager(this)
{
    _init(NULL, "", 1024);
    connectSignals();
}

QDropboxFile::QDropboxFile(QDropbox *api, QObject *parent) :
    QIODevice(parent),
    _conManager(this)
{
    _init(api, "", 1024);
    obtainToken();
    connectSignals();
}

QDropboxFile::QDropboxFile(QString filename, QDropbox *api, QObject *parent) :
    QIODevice(parent),
    _conManager(this)
{
    _init(api, filename, 1024);
   obtainToken();
   connectSignals();
}

QDropboxFile::~QDropboxFile()
{
    if(_buffer != NULL)
        delete _buffer;
    if(_evLoop != NULL)
        delete _evLoop;
}

bool QDropboxFile::isSequential() const
{
    return true;
}

bool QDropboxFile::open(QIODevice::OpenMode mode)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::open(...)" << endl;
#endif
    if(!QIODevice::open(mode))
        return false;

  /*  if(isMode(QIODevice::NotOpen))
        return true; */

    if(_buffer == NULL)
        _buffer = new QByteArray();

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile: opening file" << endl;
#endif

	// clear buffer and reset position if this file was opened in write mode
	// with truncate - or if append was not set
	if(isMode(QIODevice::WriteOnly) && 
	   (isMode(QIODevice::Truncate) || !isMode(QIODevice::Append))
	  )
    {
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile: _buffer cleared." << endl;
#endif
        _buffer->clear();
		_position = 0;
    }
    else
    {
#ifdef QTDROPBOX_DEBUG
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

void QDropboxFile::close()
{
	if(isMode(QIODevice::WriteOnly))
		flush();
	QIODevice::close();
	return;
}

void QDropboxFile::setApi(QDropbox *dropbox)
{
    _api = dropbox;
	return;
}

QDropbox *QDropboxFile::api()
{
    return _api;
}

void QDropboxFile::setFilename(QString filename)
{
    _filename = filename;
    return;
}

QString QDropboxFile::filename()
{
    return _filename;
}

bool QDropboxFile::flush()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::flush()" << endl;
#endif

    return putFile();
}

bool QDropboxFile::event(QEvent *event)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "processing event: " << event->type() << endl;
#endif
    return QIODevice::event(event);
}

void QDropboxFile::setFlushThreshold(qint64 num)
{
    if(num<0)
        num = 0;
    _bufferThreshold = num;
    return;
}

qint64 QDropboxFile::flushThreshold()
{
    return _bufferThreshold;
}

void QDropboxFile::setOverwrite(bool overwrite)
{
    _overwrite = overwrite;
    return;
}

bool QDropboxFile::overwrite()
{
    return _overwrite;
}

qint64 QDropboxFile::readData(char *data, qint64 maxlen)
{
#ifdef QTDROPBOX_DEBUG
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
   
#ifdef QTDROPBOX_DEBUG
    qDebug() << "new size = " << _buffer->size() << endl;
    qDebug() << "new bytes = " << _buffer->toHex() << endl;
#endif

	_position += read;

    return read;
}

qint64 QDropboxFile::writeData(const char *data, qint64 len)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "old content: " << _buffer->toHex() << endl;
#endif

	qint64 oldlen = _buffer->size();
    _buffer->insert(_position, data, len);

#ifdef QTDROPBOX_DEBUG
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

void QDropboxFile::networkRequestFinished(QNetworkReply *rply)
{
    rply->deleteLater();

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::networkRequestFinished(...)" << endl;
#endif

    if (rply->error() != QNetworkReply::NoError)
    {
        lastErrorCode = rply->error();
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
#ifdef QTDROPBOX_DEBUG
		// debug information only - this should not happen, but if it does we 
		// ignore replies when not waiting for anything
		qDebug() << "QDropboxFile::networkRequestFinished(...) got reply in unknown state (" << _waitMode << ")" << endl;
#endif
        break;
    }
}

void QDropboxFile::obtainToken()
{
    _token       = _api->token();
    _tokenSecret = _api->tokenSecret();
    return;
}

void QDropboxFile::connectSignals()
{
    connect(&_conManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(networkRequestFinished(QNetworkReply*)));
    return;
}

bool QDropboxFile::isMode(QIODevice::OpenMode mode)
{
    return ( (openMode()&mode) == mode );
}

bool QDropboxFile::getFileContent(QString filename)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::getFileContent(...)" << endl;
#endif
    QUrl request;
    request.setUrl(QDROPBOXFILE_CONTENT_URL, QUrl::StrictMode);
    request.setPath(QString("/%1/files/%2")
                    .arg(_api->apiVersion().left(1))
                    .arg(filename));

    QUrlQuery query;
    query.addQueryItem("oauth_consumer_key", _api->appKey());
    query.addQueryItem("oauth_nonce", QDropbox::generateNonce(128));
    query.addQueryItem("oauth_signature_method", _api->signatureMethodString());
    query.addQueryItem("oauth_timestamp", QString::number((int) QDateTime::currentMSecsSinceEpoch()/1000));
    query.addQueryItem("oauth_token", _api->token());
    query.addQueryItem("oauth_version", _api->apiVersion());

    QString signature = _api->oAuthSign(request);
    query.addQueryItem("oauth_signature", signature);

    request.setQuery(query);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::getFileContent " << request.toString() << endl;
#endif

    QNetworkRequest rq(request);
    QNetworkReply *reply = _conManager.get(rq);
    connect(this, &QDropboxFile::operationAborted, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::downloadProgress, this, &QDropboxFile::downloadProgress);

    _waitMode = waitForRead;
    startEventLoop();

    if(lastErrorCode != 0)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropboxFile::getFileContent ReadError: " << lastErrorCode << lastErrorMessage << endl;
#endif
		if(lastErrorCode ==  QDROPBOX_ERROR_FILE_NOT_FOUND)
		{
			_buffer->clear();
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropboxFile::getFileContent: file does not exist" << endl;
#endif
		}
		else
			return false;
    }

    return true;
}

void QDropboxFile::rplyFileContent(QNetworkReply *rply)
{
    lastErrorCode = 0;

    QByteArray response = rply->readAll();
    QString resp_str;
    QDropboxJson json;

#ifdef QTDROPBOX_DEBUG
    resp_str = QString(response.toHex());
    qDebug() << "QDropboxFile::rplyFileContent response = " << resp_str << endl;

#endif

    switch(rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
    {
    case QDROPBOX_ERROR_BAD_INPUT:
    case QDROPBOX_ERROR_EXPIRED_TOKEN:
    case QDROPBOX_ERROR_BAD_OAUTH_REQUEST:
    case QDROPBOX_ERROR_FILE_NOT_FOUND:
    case QDROPBOX_ERROR_WRONG_METHOD:
    case QDROPBOX_ERROR_REQUEST_CAP:
    case QDROPBOX_ERROR_USER_OVER_QUOTA:
        resp_str = QString(response);
        json.parseString(response.trimmed());
        lastErrorCode = rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::rplyFileContent jason.valid = " << json.isValid() << endl;
#endif
        if(json.isValid())
            lastErrorMessage = json.getString("error");
        else
            lastErrorMessage = "";
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

void QDropboxFile::rplyFileWrite(QNetworkReply *rply)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::rplyFileWrite(...)" << endl;
#endif

    lastErrorCode = 0;

    QByteArray response = rply->readAll();
    QString resp_str;
    QDropboxJson json;

#ifdef QTDROPBOX_DEBUG
    resp_str = response;
    qDebug() << "QDropboxFile::rplyFileWrite response = " << resp_str << endl;

#endif

    switch(rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
    {
    case QDROPBOX_ERROR_BAD_INPUT:
    case QDROPBOX_ERROR_EXPIRED_TOKEN:
    case QDROPBOX_ERROR_BAD_OAUTH_REQUEST:
    case QDROPBOX_ERROR_FILE_NOT_FOUND:
    case QDROPBOX_ERROR_WRONG_METHOD:
    case QDROPBOX_ERROR_REQUEST_CAP:
    case QDROPBOX_ERROR_USER_OVER_QUOTA:
        resp_str = QString(response);
        json.parseString(response.trimmed());
        lastErrorCode = rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::rplyFileWrite jason.valid = " << json.isValid() << endl;
#endif
        if(json.isValid())
            lastErrorMessage = json.getString("error");
        else
            lastErrorMessage = "";
        return;
        break;
    default:
        delete _metadata;

        _metadata = new QDropboxFileInfo{QString{response}.trimmed(), this};
        if (!_metadata->isValid())
            _metadata->clear();
        break;
    }

    emit bytesWritten(_buffer->size());
    return;
}

void QDropboxFile::startEventLoop()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::startEventLoop()" << endl;
#endif
    if(_evLoop == NULL)
        _evLoop = new QEventLoop(this);
    _evLoop->exec();
    return;
}

void QDropboxFile::stopEventLoop()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::stopEventLoop()" << endl;
#endif
    if(_evLoop == NULL)
        return;
    _evLoop->exit();
    return;
}

bool QDropboxFile::putFile()
{

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::putFile()" << endl;
#endif

    QUrl request;
    request.setUrl(QDROPBOXFILE_CONTENT_URL, QUrl::StrictMode);
    request.setPath(QString("/%1/files_put/%2")
                    .arg(_api->apiVersion().left(1))
                    .arg(_filename));

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("oauth_consumer_key", _api->appKey());
    urlQuery.addQueryItem("oauth_nonce", QDropbox::generateNonce(128));
    urlQuery.addQueryItem("oauth_signature_method", _api->signatureMethodString());
    urlQuery.addQueryItem("oauth_timestamp", QString::number((int) QDateTime::currentMSecsSinceEpoch()/1000));
    urlQuery.addQueryItem("oauth_token", _api->token());
    urlQuery.addQueryItem("oauth_version", _api->apiVersion());
    urlQuery.addQueryItem("overwrite", (_overwrite?"true":"false"));

    QString signature = _api->oAuthSign(request);
    urlQuery.addQueryItem("oauth_signature", signature);

    request.setQuery(urlQuery);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::put " << request.toString() << endl;
#endif

    QNetworkRequest rq(request);
    QNetworkReply *reply = _conManager.put(rq, *_buffer);
    connect(this, &QDropboxFile::operationAborted, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::uploadProgress, this, &QDropboxFile::uploadProgress);

    _waitMode = waitForWrite;	
    startEventLoop();

    if(lastErrorCode != 0)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropboxFile::putFile WriteError: " << lastErrorCode << lastErrorMessage << endl;
#endif
        return false;
    }

    _currentThreshold = 0;

    return true;
}

void QDropboxFile::_init(QDropbox *api, QString filename, qint64 bufferTh)
{
    _api              = api;
    _buffer           = NULL;
    _filename         = filename;
    _evLoop           = NULL;
    _waitMode         = notWaiting;
    _bufferThreshold  = bufferTh;
    _overwrite        = true;
    _metadata         = NULL;
    lastErrorCode     = 0;
    lastErrorMessage  = "";
    _position         = 0;
    _currentThreshold = 0;
    return;
}


QDropboxFileInfo QDropboxFile::metadata()
{
	if(_metadata == NULL)
		obtainMetadata();

	return _api->requestMetadataAndWait(_filename);
}

bool QDropboxFile::hasChanged()
{
	if(_metadata == NULL)
	{
		if(!metadata().isValid()) // get metadata
			return false;         // if metadata was invalid
	}

	QDropboxFileInfo serverMetadata = _api->requestMetadataAndWait(_filename);
#ifdef QTDROPBOX_DEBUG
	qDebug() << "QDropboxFile::hasChanged() local  revision hash = " << _metadata->revisionHash() << endl;
	qDebug() << "QDropboxFile::hasChanged() remote revision hash = " << serverMetadata.revisionHash() << endl;
#endif
	return serverMetadata.revisionHash().compare(_metadata->revisionHash())!=0;
}

void QDropboxFile::obtainMetadata()
{
	// get metadata of this file
	_metadata = new QDropboxFileInfo(_api->requestMetadataAndWait(_filename).strContent(), this);
	if(!_metadata->isValid())
		_metadata->clear();
	return;
}

QList<QDropboxFileInfo> QDropboxFile::revisions(int max)
{
	QList<QDropboxFileInfo> revisions = _api->requestRevisionsAndWait(_filename, max);
	if(_api->error() != QDropbox::NoError)
		revisions.clear();

	return revisions;
}

bool QDropboxFile::seek(qint64 pos)
{
	if(pos > _buffer->size())
		return false;

	QIODevice::seek(pos);
	_position = pos;
	return true;
}

qint64 QDropboxFile::pos() const
{
	return _position;
}

bool QDropboxFile::reset()
{
	QIODevice::reset();
	_position = 0;
	return true;
}

void QDropboxFile::abort()
{
    emit operationAborted();
}
