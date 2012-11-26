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

bool QDropboxFile::isSequential()
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

    if(isMode(QIODevice::Truncate))
    {
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile: _buffer cleared." << endl;
#endif
        _buffer->clear();
    }
    else
    {
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile: reading file content" << endl;
#endif
        if(!getFileContent(_filename))
            return false;
    }

    return true;
}

void QDropboxFile::close()
{
    //! \todo implement!
}

void QDropboxFile::setApi(QDropbox *dropbox)
{
    //! \todo implement!
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

    if(_buffer->size() == 0)
        return 0;

    if(_buffer->size() < maxlen)
        maxlen = _buffer->size();

    qint64 newsize = _buffer->size()-maxlen;
    //data = _buffer->left(maxlen).data();
    QByteArray tmp = _buffer->left(maxlen);
    char *d = tmp.data();
    memcpy(data, d, maxlen);
    QByteArray newbytes = _buffer->right(newsize);
    _buffer->clear();
    _buffer->append(newbytes);
#ifdef QTDROPBOX_DEBUG
    qDebug() << "new size = " << _buffer->size() << endl;
    qDebug() << "new bytes = " << _buffer->toHex() << endl;
#endif
    return maxlen;
}

qint64 QDropboxFile::writeData(const char *data, qint64 len)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "old content: " << _buffer->toHex() << endl;
#endif

    _buffer->append(data, len);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "new content: " << _buffer->toHex() << endl;
#endif

    // flush if the threshold is reached
    if(_buffer->size()%_bufferThreshold)
        flush();

    return 0;
}

void QDropboxFile::networkRequestFinished(QNetworkReply *rply)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::networkRequestFinished(...)" << endl;
#endif

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
    //! \todo implement reaction for notWaiting/default
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
    request.setPath(QString("%1/files/%2")
                    .arg(_api->apiVersion().left(1))
                    .arg(filename));
    request.addQueryItem("oauth_consumer_key", _api->appKey());
    request.addQueryItem("oauth_nonce", QDropbox::generateNonce(128));
    request.addQueryItem("oauth_signature_method", _api->signatureMethodString());
    request.addQueryItem("oauth_timestamp", QString::number((int) QDateTime::currentMSecsSinceEpoch()/1000));
    request.addQueryItem("oauth_token", _api->token());
    request.addQueryItem("oauth_version", _api->apiVersion());

    QString signature = _api->oAuthSign(request);
    request.addQueryItem("oauth_signature", signature);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::getFileContent " << request.toString() << endl;
#endif

    QNetworkRequest rq(request);
    _conManager.get(rq);

    _waitMode = waitForRead;
    startEventLoop();

    if(lastErrorCode != 0)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropboxFile::getFileContent ReadError: " << lastErrorCode << lastErrorMessage << endl;
#endif
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
        break;
    }

    // TODO interpret returned data as QDropboxFileMetadata
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
    // if we don't have write acces do not write!
	if(!isMode(QIODevice::ReadOnly))
	{
		#ifdef QTDROPBOX_DEBUG
		qDebug() << "QDropboxFile::putFile() writeonly!" << endl;
		#endif
        return false;
	}

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::putFile()" << endl;
#endif

    QUrl request;
    request.setUrl(QDROPBOXFILE_CONTENT_URL, QUrl::StrictMode);
    request.setPath(QString("%1/files_put/%2")
                    .arg(_api->apiVersion().left(1))
                    .arg(_filename));
    request.addQueryItem("oauth_consumer_key", _api->appKey());
    request.addQueryItem("oauth_nonce", QDropbox::generateNonce(128));
    request.addQueryItem("oauth_signature_method", _api->signatureMethodString());
    request.addQueryItem("oauth_timestamp", QString::number((int) QDateTime::currentMSecsSinceEpoch()/1000));
    request.addQueryItem("oauth_token", _api->token());
    request.addQueryItem("oauth_version", _api->apiVersion());
    request.addQueryItem("overwrite", (_overwrite?"true":"false"));

    QString signature = _api->oAuthSign(request);
    request.addQueryItem("oauth_signature", signature);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropboxFile::put " << request.toString() << endl;
#endif

    QNetworkRequest rq(request);
    _conManager.put(rq, *_buffer);

    _waitMode = waitForWrite;
    startEventLoop();

    if(lastErrorCode != 0)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "QDropboxFile::putFile WriteError: " << lastErrorCode << lastErrorMessage << endl;
#endif
        return false;
    }

    return true;
}

void QDropboxFile::_init(QDropbox *api, QString filename, qint64 bufferTh)
{
    _api             = api;
    _buffer          = NULL;
    _filename        = filename;
    _evLoop          = NULL;
    _waitMode        = notWaiting;
    _bufferThreshold = bufferTh;
    _overwrite       = true;
    return;
}
