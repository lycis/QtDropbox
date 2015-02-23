#include "dropbox.h"

Dropbox::Dropbox(QObject *parent) :
    QObject(parent),
    _networkAccessManager(this)
{
#ifdef QT_DEBUG
    qDebug() << "creating dropbox api" << endl;
#endif

    _error = Dropbox::NoError;
    _errorText  = "";
    setApiVersion("1.0");
    setApiUrl("api.dropbox.com");
    setAuthMethod(Dropbox::Plaintext);

    _oauthToken = "";
    _oauthTokenSecret = "";

    _lastReply = 0;

    connect(&_networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkReplyFinished(QNetworkReply*)));

    // needed for nonce generation
    qsrand(QDateTime::currentMSecsSinceEpoch());

    _eventLoop = NULL;
}

Dropbox::Dropbox(QString key, QString sharedSecret, OAuthMethod method, QString url, QObject *parent) :
    QObject(parent),
    _networkAccessManager(this)
{
#ifdef QT_DEBUG
    qDebug() << "creating api with key, shared secret and method" << endl;
#endif

    _error      = Dropbox::NoError;
    _errorText       = "";
    setKey(key);
    setSharedSecret(sharedSecret);
    setAuthMethod(method);
    setApiVersion("1.0");
    setApiUrl(url);

    _oauthToken = "";
    _oauthTokenSecret = "";

    _lastReply = 0;

    connect(&_networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkReplyFinished(QNetworkReply*)));

    // needed for nonce generation
    qsrand(QDateTime::currentMSecsSinceEpoch());

    _eventLoop = NULL;
}

Dropbox::Error Dropbox::error()
{
    return _error;
}

QString Dropbox::errorString()
{
    return _errorText;
}

void Dropbox::setApiUrl(QString url)
{
    _apiUrl.setUrl(QString("//%1").arg(url));
    prepareApiUrl();
    return;
}

QString Dropbox::apiUrl()
{
    return _apiUrl.toString();
}

void Dropbox::setAuthMethod(OAuthMethod m)
{
    _oauthMethod = m;
    prepareApiUrl();
    return;
}

Dropbox::OAuthMethod Dropbox::authMethod()
{
    return _oauthMethod;
}

void Dropbox::setApiVersion(QString apiversion)
{
    if(apiversion.compare("1.0"))
    {
        _error = Dropbox::VersionNotSupported;
        _errorText  = "Only version 1.0 is supported.";
        emit errorOccured(Dropbox::VersionNotSupported);
        return;
    }

    _version = apiversion;
    return;
}

void Dropbox::requestFinished(int nr, QNetworkReply *rply)
{
    rply->deleteLater();
#ifdef QT_DEBUG
    int resp_bytes = rply->bytesAvailable();
#endif
    QByteArray buff = rply->readAll();
    QString response = QString(buff);
#ifdef QT_DEBUG
    qDebug() << "request " << nr << "finished." << endl;
    qDebug() << "request was: " << rply->url().toString() << endl;
#endif
#ifdef QT_DEBUG
    qDebug() << "response: " << resp_bytes << "bytes" << endl;
    qDebug() << "status code: " << rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString() << endl;
    qDebug() << "== begin response ==" << endl << response << endl << "== end response ==" << endl;
    qDebug() << "req#" << nr << " is of type " << _requestMap[nr].type << endl;
#endif
    // drop box error handling based on return codes
    switch(rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
    {
    case DROPBOX_ERROR_BAD_INPUT:
        _error = Dropbox::BadInput;
        _errorText  = "";
        emit errorOccured(_error);
        checkReleaseEventLoop(nr);
        return;
        break;
    case DROPBOX_ERROR_EXPIRED_TOKEN:
        _error = Dropbox::TokenExpired;
        _errorText  = "";
        emit tokenExpired();
        checkReleaseEventLoop(nr);
        return;
        break;
    case DROPBOX_ERROR_BAD_OAUTH_REQUEST:
        _error = Dropbox::BadOAuthRequest;
        _errorText  = "";
        emit errorOccured(_error);
        checkReleaseEventLoop(nr);
        return;
        break;
    case DROPBOX_ERROR_FILE_NOT_FOUND:
        emit fileNotFound();
        checkReleaseEventLoop(nr);
        return;
        break;
    case DROPBOX_ERROR_WRONG_METHOD:
        _error = Dropbox::WrongHttpMethod;
        _errorText  = "";
        emit errorOccured(_error);
        checkReleaseEventLoop(nr);
        return;
        break;
    case DROPBOX_ERROR_REQUEST_CAP:
        _error = Dropbox::MaxRequestsExceeded;
        _errorText = "";
        emit errorOccured(_error);
        checkReleaseEventLoop(nr);
        return;
        break;
    case DROPBOX_ERROR_USER_OVER_QUOTA:
        _error = Dropbox::UserOverQuota;
        _errorText = "";
        emit errorOccured(_error);
        checkReleaseEventLoop(nr);
        return;
        break;
    default:
        break;
    }

    if(rply->error() != QNetworkReply::NoError)
    {

        _error = Dropbox::CommunicationError;
        _errorText  = QString("%1 - %2").arg(rply->error()).arg(rply->errorString());
#ifdef QT_DEBUG
        qDebug() << "error " << _error << "(" << _errorText << ") in request" << endl;
#endif
        emit errorOccured(_error);
        return;
    }

    // ignore connection requests
    if(_requestMap[nr].type == DROPBOX_REQ_CONNECT)
    {
#ifdef QT_DEBUG
        qDebug() << "- answer to connection request ignored" << endl;
#endif
        _requestMap.remove(nr);
        return;
    }

    bool delayed_finish = false;
    int delayed_nr;

    if(rply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 302)
    {
#ifdef QT_DEBUG
        qDebug() << "redirection received" << endl;
#endif
        // redirection handling
        QUrl newlocation(rply->header(QNetworkRequest::LocationHeader).toString(), QUrl::StrictMode);
#ifdef QT_DEBUG
        qDebug() << "new url: " << newlocation.toString() << endl;
#endif
        int oldnr = nr;
        nr = sendRequest(newlocation, _requestMap[nr].method, 0, _requestMap[nr].host);
        _requestMap[nr].type = DROPBOX_REQ_REDIREC;
        _requestMap[nr].linked = oldnr;
        return;
    }
    else
    {
        if(_requestMap[nr].type == DROPBOX_REQ_REDIREC)
        {
            // change values if this is the answert to a redirect
            qdropbox_request redir = _requestMap[nr];
            qdropbox_request orig  = _requestMap[redir.linked];
            _requestMap[nr] = orig;
            _requestMap.remove(nr);
            nr = redir.linked;
        }

        // standard handling depending on message type
        switch(_requestMap[nr].type)
        {
        case DROPBOX_REQ_CONNECT:
            // was only a connect request - so drop it
            break;
        case DROPBOX_REQ_RQTOKEN:
            // requested a tiken
            responseTokenRequest(response);
            break;
        case DROPBOX_REQ_RQBTOKN:
            responseBlockedTokenRequest(response);
            break;
        case DROPBOX_REQ_AULOGIN:
            delayed_nr = responseDropboxLogin(response, nr);
            delayed_finish = true;
            break;
        case DROPBOX_REQ_ACCTOKN:
            responseAccessToken(response);
            break;
        case DROPBOX_REQ_METADAT:
            parseMetadata(response);
            break;
        case DROPBOX_REQ_BMETADA:
            parseBlockingMetadata(response);
			break;
        case DROPBOX_REQ_BACCTOK:
            responseBlockingAccessToken(response);
            break;
        case DROPBOX_REQ_ACCINFO:
            parseAccountInfo(response);
            break;
        case DROPBOX_REQ_BACCINF:
            parseBlockingAccountInfo(response);
            break;
        case DROPBOX_REQ_SHRDLNK:
            parseSharedLink(response);
            break;
        case DROPBOX_REQ_BSHRDLN:
            parseBlockingSharedLink(response);
            break;
        case DROPBOX_REQ_REVISIO:
			parseRevisions(response);
			break;
        case DROPBOX_REQ_BREVISI:
			parseBlockingRevisions(response);
			break;
        case DROPBOX_REQ_DELTA:
            parseDelta(response);
            break;
        case DROPBOX_REQ_BDELTA:
            parseBlockingDelta(response);
            break;
        default:
            _error  = Dropbox::ResponseToUnknownRequest;
            _errorText   = "Received a response to an unknown request";
            emit errorOccured(_error);
            break;
        }
    }

    if(delayed_finish)
        _delayMap[delayed_nr] = nr;
    else
    {
        if(_delayMap[nr])
        {
            int drq = _delayMap[nr];
            while(drq!=0)
            {
                emit operationFinished(_delayMap[drq]);
                _delayMap.remove(drq);
                drq = _delayMap[drq];
            }
        }

        _requestMap.remove(nr);
        emit operationFinished(nr);
    }

    return;
}

void Dropbox::networkReplyFinished(QNetworkReply *rply)
{
#ifdef QT_DEBUG
    qDebug() << "reply finished" << endl;
#endif
    int reqnr = _replynrMap[rply];
    requestFinished(reqnr, rply);
}
/*
QString QDropbox::hmacsha1(QString base, QString key)
{
    // inner pad
    QByteArray ipad;
    ipad.fill(char(0), 64);
    for(int i = 0; i < key.length(); ++i)
        ipad[i] = key[i].toLatin1();

    // outer pad
    QByteArray opad;
    opad.fill(char(0), 64);
    for(int i = 0; i < key.length(); ++i)
        opad[i] = key[i].toLatin1();

    // XOR operation for inner pad
    for(int i = 0; i < ipad.length(); ++i)
        ipad[i] = ipad[i] ^ 0x36;

    // XOR operation for outer pad
    for(int i = 0; i < opad.length(); ++i)
        opad[i] = opad[i] ^ 0x5c;

    // Hashes inner pad
    QByteArray innerSha1 = QCryptographicHash::hash(
        ipad + base.toLatin1(),
        QCryptographicHash::Sha1
        );

    // Hashes outer pad
    QByteArray outerSha1 = QCryptographicHash::hash(
        opad + innerSha1,
        QCryptographicHash::Sha1
        );

    return outerSha1.toBase64();
}
 */

/*
QString QDropbox::hmacsha1(QString base, QString key)
{
    // inner pad
    QByteArray ipad;
    ipad.fill(char(0x36), SHA1_BLOCK_SIZE);

    // outer pad
    QByteArray opad;
    opad.fill(char(0x5c), SHA1_BLOCK_SIZE);

    // SHA! Key
    QByteArray SHA1_Key;

    // STEP 1
    if (key.length() > SHA1_BLOCK_SIZE)
    {
        SHA1_Key = QCryptographicHash::hash(
                    key.toLatin1(),
                    QCryptographicHash::Sha1
                    );
    }
    else
        SHA1_Key = key.toLatin1();

    // STEP 2
    for (int i = 0; i < ipad.length(); i++)
    {
        ipad[i] = ipad[i] ^ SHA1_Key[i];
    }

    // STEP 3
    QByteArray innerSha1 = ipad;
    innerSha1.append(base.toLatin1().toBase64());

    // STEP 4
    QByteArray szReport = QCryptographicHash::hash(
                innerSha1,
                QCryptographicHash::Sha1
                );

    // STEP 5
    for (int j = 0; j < opad.length(); j++)
    {
        opad[j] = opad[j] ^ SHA1_Key[j];
    }

    // STEP 6
    QByteArray outerSha1 = opad;
    outerSha1.append(szReport);

    // STEP 7
    QByteArray digest = QCryptographicHash::hash(
                outerSha1,
                QCryptographicHash::Sha1
                );

    return digest.toBase64();
}
*/


QString Dropbox::hmacsha1(QString baseString, QString key)
{
    int blockSize = 64; // HMAC-::hmacsha1SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
        key = QCryptographicHash::hash(key.toLatin1(), QCryptographicHash::Sha1);
    }

    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "\"
    // ascii characters 0x36 ("6") and 0x5c ("\") are selected because they have large
    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)

    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.toLatin1().at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.toLatin1().at(i); // XOR operation between every byte in key and outerpadding, of key length
    }

    // result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseString ) ).toBase64
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString.toLatin1());
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}

QString Dropbox::generateNonce(qint32 length)
{
    QString clng = "";
    for(int i=0; i<length; ++i)
        clng += QString::number(int( qrand() / (RAND_MAX + 1.0) * (16 + 1 - 0) + 0 ), 16).toUpper();
    return clng;
}

QString Dropbox::oAuthSign(QUrl base, QString method)
{
    if(_oauthMethod == Dropbox::Plaintext){
#ifdef QT_DEBUG
        qDebug() << "oauthMethod = Plaintext";
#endif
        return QString("%1&%2").arg(_appSharedSecret).arg(_oauthTokenSecret);
    }

    QString param   = base.toString(QUrl::RemoveAuthority|QUrl::RemovePath|QUrl::RemoveScheme).mid(1);
    param = QUrl::toPercentEncoding(param);
    QString requrl  = base.toString(QUrl::RemoveQuery);
    requrl = QUrl::toPercentEncoding(requrl);
#ifdef QT_DEBUG
    qDebug() << "param = " << param << endl << "requrl = " << requrl << endl;
#endif
    QString baseurl = method+"&"+requrl+"&"+param;
    QString key     = QString("%1&%2").arg(_appSharedSecret).arg(_oauthTokenSecret);
#ifdef QT_DEBUG
    qDebug() << "baseurl = " << baseurl << " endbase";
    qDebug() << "key = " << key << " endkey";
#endif

    QString signature = "";
    if(_oauthMethod == Dropbox::HMACSHA1)
        signature = hmacsha1(baseurl.toUtf8(), key.toUtf8());
    else
    {
        _error = Dropbox::UnknownAuthMethod;
        _errorText  = QString("Authentication method %1 is unknown").arg(_oauthMethod);
        emit errorOccured(_error);
#ifdef QT_DEBUG
        qDebug() << "Authentication method " << _oauthMethod << " is unknown";
#endif
        return "";
    }

#ifdef QT_DEBUG
    qDebug() << "key = " << key << endl;
    qDebug() << "signature = " << signature << "(base64 = " << QByteArray(signature.toUtf8()).toBase64() << endl;

#endif
    return signature.toUtf8();
}

void Dropbox::prepareApiUrl()
{
    //if(oauthMethod == QDropbox::Plaintext)
    _apiUrl.setScheme("https");
    //else
    //  apiurl.setScheme("http");
}

int Dropbox::sendRequest(QUrl request, QString type, QByteArray postdata, QString host)
{
    if(!host.trimmed().compare(""))
        host = _apiUrl.toString(QUrl::RemoveScheme).mid(2);

#ifdef QT_DEBUG
    qDebug() << "sendRequest() host = " << host << endl;
#endif

    /*if(oauthMethod == QDropbox::Plaintext)
        reqnr = conManager.setHost(host, QHttp::ConnectionModeHttps);
    else
        reqnr = conManager.setHost(host, QHttp::ConnectionModeHttp);
    requestMap[reqnr].type   = QDROPBOX_REQ_CONNECT;
    requestMap[reqnr].method = "";*/

    QString req_str = request.toString(QUrl::RemoveAuthority|QUrl::RemoveScheme);
    if(!req_str.startsWith("/"))
        req_str = QString("/%1").arg(req_str);

    QNetworkRequest rq(request);
    QNetworkReply *rply;

    if(!type.compare("GET"))
        rply = _networkAccessManager.get(rq);
    else if(!type.compare("POST"))
    {
        rq.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
        rply = _networkAccessManager.post(rq, postdata);
    }
    else
    {
        _error = Dropbox::UnknownQueryMethod;
        _errorText  = "The provided query method is unknown.";
        emit errorOccured(_error);
#ifdef QT_DEBUG
        qDebug() << "error " << _error << "(" << _errorText << ") in request" << endl;
#endif
        return -1;
    }

    _replynrMap[rply] = ++_lastReply;

    _requestMap[_lastReply].method = type;
    _requestMap[_lastReply].host   = host;

#ifdef QT_DEBUG
    qDebug() << "sendRequest() -> request #" << _lastReply << " sent." << endl;
#endif
    return _lastReply;
}

void Dropbox::responseTokenRequest(QString response)
{
    parseToken(response);
    emit requestTokenFinished(_oauthToken, _oauthTokenSecret);
    return;
}

int Dropbox::responseDropboxLogin(QString response, int reqnr)
{
    Q_UNUSED(reqnr);

    // extract login form
    QDomDocument xml;
    QString err;
    int lnr, cnr;
    if(!xml.setContent(response, false, &err, &lnr, &cnr))
    {
#ifdef QT_DEBUG
        qDebug() << "invalid xml (" << lnr << "," << cnr << "): " << err << "dump:" << endl;
        qDebug() << xml.toString() << endl;
#endif
        return 0;
    }
    return 0;
}

void Dropbox::responseAccessToken(QString response)
{
    parseToken(response);
    emit accessTokenFinished(_oauthToken, _oauthTokenSecret);
    return;
}

QString Dropbox::signatureMethodString()
{
    QString sigmeth;
    switch(_oauthMethod)
    {
    case Dropbox::Plaintext:
        sigmeth = "PLAINTEXT";
        break;
    case Dropbox::HMACSHA1:
        sigmeth = "HMAC-SHA1";
        break;
    default:
        _error = Dropbox::UnknownAuthMethod;
        _errorText  = QString("Authentication method %1 is unknown").arg(_oauthMethod);
        emit errorOccured(_error);
        return "";
        break;
    }
    return sigmeth;
}

void Dropbox::parseToken(QString response)
{
	clearError();
#ifdef QT_DEBUG
    qDebug() << "processing token request" << endl;
#endif

    QStringList split = response.split("&");
    if(split.size() < 2)
    {
        _error = Dropbox::APIError;
        _errorText  = "The Dropbox API did not respond as expected.";
        emit errorOccured(_error);
#ifdef QT_DEBUG
        qDebug() << "error " << _error << "(" << _errorText << ") in request" << endl;
#endif
        return;
    }

    if(!split.at(0).startsWith("oauth_token_secret") ||
            !split.at(1).startsWith("oauth_token"))
    {
        _error = Dropbox::APIError;
        _errorText  = "The Dropbox API did not respond as expected.";
        emit errorOccured(_error);
#ifdef QT_DEBUG
        qDebug() << "error " << _error << "(" << _errorText << ") in request" << endl;
#endif
        return;
    }

    QStringList tokenSecretList = split.at(0).split("=");
    _oauthTokenSecret = tokenSecretList.at(1);
    QStringList tokenList = split.at(1).split("=");
    _oauthToken = tokenList.at(1);

#ifdef QT_DEBUG
    qDebug() << "token = " << _oauthToken << endl << "token_secret = " << _oauthTokenSecret << endl;
#endif

    emit tokenChanged(_oauthToken, _oauthTokenSecret);
    return;
}

void Dropbox::parseAccountInfo(QString response)
{
#ifdef QT_DEBUG
    qDebug() << "== account info ==" << response << "== account info end ==";
#endif

    DropboxJson json;
    json.parseString(response);
    _tempJson.parseString(response);
    if(!json.isValid())
    {
        _error = Dropbox::APIError;
        _errorText  = "Dropbox API did not send correct answer for account information.";
#ifdef QT_DEBUG
        qDebug() << "error: " << _errorText << endl;
#endif
        emit errorOccured(_error);
        return;
    }

    emit accountInfoReceived(response);
    return;
}

void Dropbox::parseSharedLink(QString response)
{
#ifdef QT_DEBUG
    qDebug() << "== shared link ==" << response << "== shared link end ==";
#endif

    //QDropboxJson json;
    //json.parseString(response);
    _tempJson.parseString(response);
    if(!_tempJson.isValid())
    {
        _error = Dropbox::APIError;
        _errorText  = "Dropbox API did not send correct answer for file/directory shared link.";
#ifdef QT_DEBUG
        qDebug() << "error: " << _errorText << endl;
#endif
        emit errorOccured(_error);
        stopEventLoop();
        return;
    }
    emit sharedLinkReceived(response);
}

void Dropbox::parseMetadata(QString response)
{
#ifdef QT_DEBUG
    qDebug() << "== metadata ==" << response << "== metadata end ==";
#endif

    DropboxJson json;
    json.parseString(response);
    _tempJson.parseString(response);
    if(!json.isValid())
    {
        _error = Dropbox::APIError;
        _errorText  = "Dropbox API did not send correct answer for file/directory metadata.";
#ifdef QT_DEBUG
        qDebug() << "error: " << _errorText << endl;
#endif
        emit errorOccured(_error);
        stopEventLoop();
        return;
    }

    emit metadataReceived(response);
    return;
}

void Dropbox::parseDelta(QString response)
{
#ifdef QT_DEBUG
    qDebug() << "== metadata ==" << response << "== metadata end ==";
#endif

    DropboxJson json;
    json.parseString(response);
    _tempJson.parseString(response);
    if(!json.isValid())
    {
        _error = Dropbox::APIError;
        _errorText  = "Dropbox API did not send correct answer for delta.";
#ifdef QT_DEBUG
        qDebug() << "error: " << _errorText << endl;
#endif
        emit errorOccured(_error);
        stopEventLoop();
        return;
    }

    emit deltaReceived(response);
    return;
}

void Dropbox::setKey(QString key)
{
#ifdef QT_DEBUG
    qDebug() << "appKey = " << key;
#endif
    _appKey = key;
}

QString Dropbox::key()
{
    return _appKey;
}

void Dropbox::setSharedSecret(QString sharedSecret)
{
#ifdef QT_DEBUG
    qDebug() << "appSharedSecret = " << sharedSecret;
#endif
    _appSharedSecret = sharedSecret;
}

QString Dropbox::sharedSecret()
{
    return _appSharedSecret;
}

void Dropbox::setToken(QString t)
{
    _oauthToken = t;
}

QString Dropbox::token()
{
    return _oauthToken;
}

void Dropbox::setTokenSecret(QString s)
{
#ifdef QT_DEBUG
    qDebug() << "oauthTokenSecret = " << _oauthTokenSecret;
#endif
    _oauthTokenSecret = s;
}

QString Dropbox::tokenSecret()
{
    return _oauthTokenSecret;
}

QString Dropbox::appKey()
{
    return _appKey;
}

QString Dropbox::appSharedSecret()
{
    return _appSharedSecret;
}

QString Dropbox::apiVersion()
{
    return _version;
}

int Dropbox::requestToken(bool blocking)
{
	clearError();
    QString sigmeth = signatureMethodString();

    _timestamp = QDateTime::currentMSecsSinceEpoch()/1000;
    _nonce = generateNonce(128);

    QUrl url;
    url.setUrl(_apiUrl.toString());
    url.setPath(QString("/%1/oauth/request_token").arg(_version.left(1)));

    QUrlQuery query;
    query.addQueryItem("oauth_consumer_key",_appKey);
    query.addQueryItem("oauth_nonce", _nonce);
    query.addQueryItem("oauth_signature_method", sigmeth);
    query.addQueryItem("oauth_timestamp", QString::number(_timestamp));
    query.addQueryItem("oauth_version", _version);

    QString signature = oAuthSign(url);
    query.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    url.setQuery(query);
#ifdef QT_DEBUG
    qDebug() << "request token url: " << url.toString() << endl << "sig: " << signature << endl;
    qDebug() << "sending request " << url.toString() << " to " << _apiUrl.toString() << endl;
#endif

    int reqnr = sendRequest(url);
    if(blocking)
    {
        _requestMap[reqnr].type = DROPBOX_REQ_RQBTOKN;
        startEventLoop();
    }
    else
        _requestMap[reqnr].type = DROPBOX_REQ_RQTOKEN;

    return reqnr;
}

bool Dropbox::requestTokenAndWait()
{
    requestToken(true);
    return (error() == NoError);
}

int Dropbox::authorize(QString mail, QString password)
{
    QUrl dropbox_authorize;
    dropbox_authorize.setPath(QString("/%1/oauth/authorize")
                              .arg(_version.left(1)));
#ifdef QT_DEBUG
    qDebug() << "oauthToken = " << _oauthToken << endl;
#endif

    QUrlQuery query;
    query.addQueryItem("oauth_token", _oauthToken);
    dropbox_authorize.setQuery(query);
    int reqnr = sendRequest(dropbox_authorize, "GET", 0, "www.dropbox.com");
    _requestMap[reqnr].type = DROPBOX_REQ_AULOGIN;
    _mail     = mail;
    _password = password;
    return reqnr;
}

QUrl Dropbox::authorizeLink()
{
    QUrl link;
    link.setScheme("https");
    link.setHost("www.dropbox.com");
    link.setPath(QString("/%1/oauth/authorize")
                 .arg(_version.left(1)));

    QUrlQuery query;
    query.addQueryItem("oauth_token", _oauthToken);
    link.setQuery(query);
    return link;
}

int Dropbox::requestAccessToken(bool blocking)
{
	clearError();

    QUrl url;
    url.setUrl(_apiUrl.toString());

    QUrlQuery query;
    query.addQueryItem("oauth_consumer_key",_appKey);
    query.addQueryItem("oauth_nonce", _nonce);
    query.addQueryItem("oauth_signature_method", signatureMethodString());
    query.addQueryItem("oauth_timestamp", QString::number(_timestamp));
    query.addQueryItem("oauth_token", _oauthToken);
    query.addQueryItem("oauth_version", _version);

    url.setPath(QString("/%1/oauth/access_token").
                arg(_version.left(1)));

#ifdef QT_DEBUG
    qDebug() << "requestToken = " << query.queryItemValue("oauth_token");
#endif

    QString signature = oAuthSign(url);
    query.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    url.setQuery(query);

    QString dataString = url.toString(QUrl::RemoveScheme|QUrl::RemoveAuthority|
                                      QUrl::RemovePath).mid(1);
#ifdef QT_DEBUG
    qDebug() << "dataString = " << dataString << endl;
#endif

    QByteArray postData;
    postData.append(dataString.toUtf8());

    QUrl xQuery(url.toString(QUrl::RemoveQuery));
    int reqnr = sendRequest(xQuery, "POST", postData);

    if(blocking)
    {
        _requestMap[reqnr].type = DROPBOX_REQ_BACCTOK;
        startEventLoop();
    }
    else
        _requestMap[reqnr].type = DROPBOX_REQ_ACCTOKN;

    return reqnr;
}

bool Dropbox::requestAccessTokenAndWait()
{
    requestAccessToken(true);
#ifdef QT_DEBUG
    qDebug() << "requestTokenAndWait() finished: error = " << error() << endl;
#endif
    return (error() == NoError);
}

void Dropbox::requestAccountInfo(bool blocking)
{
    clearError();

    _timestamp = QDateTime::currentMSecsSinceEpoch()/1000;

    QUrl url;
    url.setUrl(_apiUrl.toString());

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("oauth_consumer_key",_appKey);
    urlQuery.addQueryItem("oauth_nonce", _nonce);
    urlQuery.addQueryItem("oauth_signature_method", signatureMethodString());
    urlQuery.addQueryItem("oauth_timestamp", QString::number(_timestamp));
    urlQuery.addQueryItem("oauth_token", _oauthToken);
    urlQuery.addQueryItem("oauth_version", _version);

    QString signature = oAuthSign(url);
    urlQuery.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    url.setPath(QString("/%1/account/info").arg(_version.left(1)));
    url.setQuery(urlQuery);

    int reqnr = sendRequest(url);
    if(blocking)
    {
        _requestMap[reqnr].type = DROPBOX_REQ_BACCINF;
        startEventLoop();
    }
    else
        _requestMap[reqnr].type = DROPBOX_REQ_ACCINFO;
    return;
}

DropboxAccount Dropbox::requestAccountInfoAndWait()
{
    requestAccountInfo(true);
    DropboxAccount a(_tempJson.strContent(), this);
    _account = a;
    return _account;
}

void Dropbox::parseBlockingAccountInfo(QString response)
{
    clearError();
    parseAccountInfo(response);
    stopEventLoop();
    return;
}

void Dropbox::requestMetadata(QString file, bool blocking)
{
    clearError();

    _timestamp = QDateTime::currentMSecsSinceEpoch()/1000;

    QUrl url;
    url.setUrl(_apiUrl.toString());

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("oauth_consumer_key",_appKey);
    urlQuery.addQueryItem("oauth_nonce", _nonce);
    urlQuery.addQueryItem("oauth_signature_method", signatureMethodString());
    urlQuery.addQueryItem("oauth_timestamp", QString::number(_timestamp));
    urlQuery.addQueryItem("oauth_token", _oauthToken);
    urlQuery.addQueryItem("oauth_version", _version);

    QString signature = oAuthSign(url);
    urlQuery.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    url.setQuery(urlQuery);
    url.setPath(QString("/%1/metadata/%2").arg(_version.left(1), file));

    int reqnr = sendRequest(url);
    if(blocking)
    {
        _requestMap[reqnr].type = DROPBOX_REQ_BMETADA;
        startEventLoop();
    }
    else
        _requestMap[reqnr].type = DROPBOX_REQ_METADAT;
    //QDropboxFileInfo fi(_tempJson.strContent(), this);
    return;
}

DropboxFileInfo Dropbox::requestMetadataAndWait(QString file)
{
    requestMetadata(file, true);
    DropboxFileInfo fi(_tempJson.strContent(), this);
    return fi;
}

void Dropbox::requestSharedLink(QString file, bool blocking)
{
	clearError();

    QUrl url;
    url.setUrl(_apiUrl.toString());

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("oauth_consumer_key",_appKey);
    urlQuery.addQueryItem("oauth_nonce", _nonce);
    urlQuery.addQueryItem("oauth_signature_method", signatureMethodString());
    urlQuery.addQueryItem("oauth_timestamp", QString::number(_timestamp));
    urlQuery.addQueryItem("oauth_token", _oauthToken);
    urlQuery.addQueryItem("oauth_version", _version);

    QString signature = oAuthSign(url);
    urlQuery.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    url.setPath(QString("/%1/shares/%2").arg(_version.left(1), file));
    url.setQuery(urlQuery);

    int reqnr = sendRequest(url);
    if(blocking)
    {
        _requestMap[reqnr].type = DROPBOX_REQ_BSHRDLN;
        startEventLoop();
    }
    else
        _requestMap[reqnr].type = DROPBOX_REQ_SHRDLNK;

    return;
}

QUrl Dropbox::requestSharedLinkAndWait(QString file)
{
    requestSharedLink(file,true);
    DropboxJson json(_tempJson.strContent());
    QString urlString = json.getString("url");
    return QUrl(urlString);
}

void Dropbox::requestDelta(QString cursor, QString path_prefix, bool blocking)
{
    clearError();

    _timestamp = QDateTime::currentMSecsSinceEpoch()/1000;

    QUrl url;
    url.setUrl(_apiUrl.toString());

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("oauth_consumer_key",_appKey);
    urlQuery.addQueryItem("oauth_nonce", _nonce);
    urlQuery.addQueryItem("oauth_signature_method", signatureMethodString());
    urlQuery.addQueryItem("oauth_timestamp", QString::number(_timestamp));
    urlQuery.addQueryItem("oauth_token", _oauthToken);
    urlQuery.addQueryItem("oauth_version", _version);
    if(cursor.length() > 0)
    {
        urlQuery.addQueryItem("cursor", cursor);
    }
    if(path_prefix.length() > 0)
    {
        urlQuery.addQueryItem("path_prefix", path_prefix);
    }

    QString signature = oAuthSign(url);
    urlQuery.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    url.setQuery(urlQuery);
    url.setPath(QString("/%1/delta").arg(_version.left(1)));

    QString dataString = url.toString(QUrl::RemoveScheme|QUrl::RemoveAuthority|
                                      QUrl::RemovePath).mid(1);
#ifdef QT_DEBUG
    qDebug() << "dataString = " << dataString << endl;
#endif

    QByteArray postData;
    postData.append(dataString.toUtf8());

    QUrl xQuery(url.toString(QUrl::RemoveQuery));
    int reqnr = sendRequest(xQuery, "POST", postData);

    if(blocking)
    {
        _requestMap[reqnr].type = DROPBOX_REQ_BDELTA;
        startEventLoop();
    }
    else
        _requestMap[reqnr].type = DROPBOX_REQ_DELTA;
    return;
}

DropboxDeltaResponse Dropbox::requestDeltaAndWait(QString cursor, QString path_prefix)
{
    requestDelta(cursor, path_prefix, true);
    DropboxDeltaResponse r(_tempJson.strContent());

    return r;
}

void Dropbox::startEventLoop()
{
#ifdef QT_DEBUG
    qDebug() << "QDropbox::startEventLoop()" << endl;
#endif
    if(_eventLoop == NULL)
        _eventLoop = new QEventLoop(this);
    _eventLoop->exec();
    return;
}

void Dropbox::stopEventLoop()
{
#ifdef QT_DEBUG
    qDebug() << "QDropbox::stopEventLoop()" << endl;
#endif
    if(_eventLoop == NULL)
        return;
#ifdef QT_DEBUG
    qDebug() << "loop ended" << endl;
#endif
    _eventLoop->exit();
    return;
}

void Dropbox::responseBlockedTokenRequest(QString response)
{
    clearError();
    responseTokenRequest(response);
    stopEventLoop();
    return;
}

void Dropbox::responseBlockingAccessToken(QString response)
{
    clearError();
    responseAccessToken(response);
    stopEventLoop();
    return;
}

void Dropbox::parseBlockingMetadata(QString response)
{
    clearError();
    parseMetadata(response);
    stopEventLoop();
    return;
}

void Dropbox::parseBlockingDelta(QString response)
{
    clearError();
    parseDelta(response);
    stopEventLoop();
    return;
}

void Dropbox::parseBlockingSharedLink(QString response)
{
    clearError();
    parseSharedLink(response);
    stopEventLoop();
	return;
}

// check if the event loop has to be stopped after a blocking request was sent
void Dropbox::checkReleaseEventLoop(int reqnr)
{
    switch(_requestMap[reqnr].type)
    {
    case DROPBOX_REQ_RQBTOKN:
    case DROPBOX_REQ_BACCTOK:
    case DROPBOX_REQ_BACCINF:
    case DROPBOX_REQ_BMETADA:
    case DROPBOX_REQ_BREVISI:
        stopEventLoop(); // release local event loop
        break;
    default:
        break;
    }
    return;
}

void Dropbox::requestRevisions(QString file, int max, bool blocking)
{
	clearError();

	QUrl url;
    url.setUrl(_apiUrl.toString());

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("oauth_consumer_key",_appKey);
    urlQuery.addQueryItem("oauth_nonce", _nonce);
    urlQuery.addQueryItem("oauth_signature_method", signatureMethodString());
    urlQuery.addQueryItem("oauth_timestamp", QString::number(_timestamp));
    urlQuery.addQueryItem("oauth_token", _oauthToken);
    urlQuery.addQueryItem("oauth_version", _version);
    urlQuery.addQueryItem("rev_limit", QString::number(max));

    QString signature = oAuthSign(url);
    urlQuery.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    url.setPath(QString("/%1/revisions/%2").arg(_version.left(1), file));
    url.setQuery(urlQuery);

    int reqnr = sendRequest(url);
    if(blocking)
    {
        _requestMap[reqnr].type = DROPBOX_REQ_BREVISI;
        startEventLoop();
    }
    else
        _requestMap[reqnr].type = DROPBOX_REQ_REVISIO;

    return;
}

QList<DropboxFileInfo> Dropbox::requestRevisionsAndWait(QString file, int max)
{
	clearError();
	requestRevisions(file, max, true);
    QList<DropboxFileInfo> revisionList;

    if(_error != Dropbox::NoError || !_tempJson.isValid())
		return revisionList;

	QStringList responseList = _tempJson.getArray();
	for(int i=0; i<responseList.size(); ++i)
	{
		QString revData = responseList.at(i);
        DropboxFileInfo revision(revData);
		revisionList.append(revision);
	}

	return revisionList;
}

void Dropbox::parseRevisions(QString response)
{
    DropboxJson json;
    _tempJson.parseString(response);
    if(!_tempJson.isValid())
    {
        _error = Dropbox::APIError;
        _errorText  = "Dropbox API did not send correct answer for file/directory metadata.";
        emit errorOccured(_error);
        stopEventLoop();
        return;
    }

    emit revisionsReceived(response);
    return;
}

void Dropbox::parseBlockingRevisions(QString response)
{
	clearError();
	parseRevisions(response);
	stopEventLoop();
	return;
}

void Dropbox::clearError()
{
    _error  = Dropbox::NoError;
    _errorText  = "";
    return;
}

