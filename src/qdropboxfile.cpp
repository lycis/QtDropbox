#include "qdropboxfile.h"

QDropboxFile::QDropboxFile(QObject *parent) :
    QIODevice(parent),
    _conManager(this)
{
    _buffer = NULL;
}

QDropboxFile::QDropboxFile(QDropbox *api, QObject *parent) :
    QIODevice(parent),
    _conManager(this)
{
    _api    = api;
    _buffer = NULL;

    obtainToken();
}

QDropboxFile::QDropboxFile(QString filename, QDropbox *api, QObject *parent) :
    QIODevice(parent),
    _conManager(this)
{
   _api      = api;
   _buffer   = NULL;
   _filename = filename;

   obtainToken();
}

QDropboxFile::~QDropboxFile()
{
    if(_buffer != NULL)
        delete _buffer;
}

bool QDropboxFile::isSequential()
{
    return false;
}

bool QDropboxFile::open(QIODevice::OpenMode mode)
{
    if(!QIODevice::open(mode))
        return false;

    if(isMode(QIODevice::NotOpen))
        return true;

    _buffer = new QByteArray();

    if(isMode(QIODevice::Truncate))
    {
        _buffer->clear();
    }
    else
    {
        if(!getFileContent(_filename))
            return false;

    }

    return true;
}

void QDropboxFile::close()
{
}

void QDropboxFile::setApi(QDropbox *dropbox)
{
}

QDropbox *QDropboxFile::api()
{
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
}

qint64 QDropboxFile::readData(char *data, qint64 maxlen)
{
}

qint64 QDropboxFile::writeData(const char *data, qint64 len)
{
}

void QDropboxFile::networkRequestFinished(QNetworkReply *rply)
{
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
    QUrl request;
    request.setUrl(QDROPBOXFILE_CONTENT_URL, QUrl::StrictMode);
    request.setPath(QString("%1/files/%2")
                    .arg(_api->apiVersion().left(1))
                    .arg(filename));
    request.addQueryItem("oauth_consumer_key", _api->appKey());
    request.addQueryItem("oauth_nonce", nonce);
    request.addQueryItem("oauth_signature_method", signatureMethodString());
    request.addQueryItem("oauth_timestamp", QString::number((int) QDateTime::currentMSecsSinceEpoch()/1000));
    request.addQueryItem("oauth_token", oauthToken);
    request.addQueryItem("oauth_version", version);
}
