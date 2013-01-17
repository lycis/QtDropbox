#include "qdropbox.h"

QDropbox::QDropbox(QObject *parent) :
    QObject(parent),
    conManager(this)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "creating dropbox api" << endl;
#endif

    errorState = QDropbox::NoError;
    errorText  = "";
    setApiVersion("1.0");
    setApiUrl("api.dropbox.com");
    setAuthMethod(QDropbox::Plaintext);

    oauthToken = "";
    oauthTokenSecret = "";

    lastreply = 0;

    connect(&conManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkReplyFinished(QNetworkReply*)));

    // needed for nonce generation
    qsrand(QDateTime::currentMSecsSinceEpoch());

    _evLoop = NULL;
}

QDropbox::QDropbox(QString key, QString sharedSecret, OAuthMethod method, QString url, QObject *parent) :
    QObject(parent),
    conManager(this)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "creating api with key, shared secret and method" << endl;
#endif

    errorState      = QDropbox::NoError;
    errorText       = "";
    setKey(key);
    _appSharedSecret = sharedSecret;
    setAuthMethod(method);
    setApiVersion("1.0");
    setApiUrl(url);

    oauthToken = "";
    oauthTokenSecret = "";

    lastreply = 0;

    connect(&conManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkReplyFinished(QNetworkReply*)));

    // needed for nonce generation
    qsrand(QDateTime::currentMSecsSinceEpoch());

    _evLoop = NULL;
}

QDropbox::Error QDropbox::error()
{
    return errorState;
}

QString QDropbox::errorString()
{
    return errorText;
}

void QDropbox::setApiUrl(QString url)
{
    apiurl.setUrl(QString("//%1").arg(url));
    prepareApiUrl();
    return;
}

QString QDropbox::apiUrl()
{
    return apiurl.toString();
}

void QDropbox::setAuthMethod(OAuthMethod m)
{
    oauthMethod = m;
    prepareApiUrl();
    return;
}

QDropbox::OAuthMethod QDropbox::authMethod()
{
    return oauthMethod;
}

void QDropbox::setApiVersion(QString apiversion)
{
    if(apiversion.compare("1.0"))
    {
        errorState = QDropbox::VersionNotSupported;
        errorText  = "Only version 1.0 is supported.";
        emit errorOccured(QDropbox::VersionNotSupported);
        return;
    }

    _version = apiversion;
    return;
}

void QDropbox::requestFinished(int nr, QNetworkReply *rply)
{
#ifdef QTDROPBOX_DEBUG
    int resp_bytes = rply->bytesAvailable();
#endif
    QByteArray buff = rply->readAll();
    QString response = QString(buff);
#ifdef QTDROPBOX_DEBUG
    qDebug() << "request " << nr << "finished." << endl;
    qDebug() << "request was: " << rply->url().toString() << endl;
#endif
#ifdef QTDROPBOX_DEBUG
    qDebug() << "response: " << resp_bytes << "bytes" << endl;
    qDebug() << "status code: " << rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString() << endl;
    qDebug() << "== begin response ==" << endl << response << endl << "== end response ==" << endl;
    qDebug() << "req#" << nr << " is of type " << requestMap[nr].type << endl;
#endif
    // drop box error handling based on return codes
    switch(rply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
    {
    case QDROPBOX_ERROR_BAD_INPUT:
        errorState = QDropbox::BadInput;
        errorText  = "";
        emit errorOccured(errorState);
		checkReleaseEventLoop(nr);
        return;
        break;
    case QDROPBOX_ERROR_EXPIRED_TOKEN:
		errorState = QDropbox::TokenExpired;
		errorText  = "";
        emit tokenExpired();
		checkReleaseEventLoop(nr);
        return;
        break;
    case QDROPBOX_ERROR_BAD_OAUTH_REQUEST:
        errorState = QDropbox::BadOAuthRequest;
        errorText  = "";
        emit errorOccured(errorState);
		checkReleaseEventLoop(nr);
        return;
        break;
    case QDROPBOX_ERROR_FILE_NOT_FOUND:
        emit fileNotFound();
		checkReleaseEventLoop(nr);
        return;
        break;
    case QDROPBOX_ERROR_WRONG_METHOD:
        errorState = QDropbox::WrongHttpMethod;
        errorText  = "";
        emit errorOccured(errorState);
		checkReleaseEventLoop(nr);
        return;
        break;
    case QDROPBOX_ERROR_REQUEST_CAP:
        errorState = QDropbox::MaxRequestsExceeded;
        errorText = "";
        emit errorOccured(errorState);
		checkReleaseEventLoop(nr);
        return;
        break;
    case QDROPBOX_ERROR_USER_OVER_QUOTA:
        errorState = QDropbox::UserOverQuota;
        errorText = "";
        emit errorOccured(errorState);
		checkReleaseEventLoop(nr);
        return;
        break;
    default:
        break;
    }

    if(rply->error() != QNetworkReply::NoError)
    {

        errorState = QDropbox::CommunicationError;
        errorText  = QString("%1 - %2").arg(rply->error()).arg(rply->errorString());
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error " << errorState << "(" << errorText << ") in request" << endl;
#endif
        emit errorOccured(errorState);
        return;
    }

    // ignore connection requests
    if(requestMap[nr].type == QDROPBOX_REQ_CONNECT)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "- answer to connection request ignored" << endl;
#endif
        requestMap.remove(nr);
        return;
    }

    bool delayed_finish = false;
    int delayed_nr;

    if(rply->attribute(QNetworkRequest::HttpStatusCodeAttribute) == 302)
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "redirection received" << endl;
#endif
        // redirection handling
        QUrl newlocation(rply->header(QNetworkRequest::LocationHeader).toString(), QUrl::StrictMode);
#ifdef QTDROPBOX_DEBUG
        qDebug() << "new url: " << newlocation.toString() << endl;
#endif
        int oldnr = nr;
        nr = sendRequest(newlocation, requestMap[nr].method, 0, requestMap[nr].host);
        requestMap[nr].type = QDROPBOX_REQ_REDIREC;
        requestMap[nr].linked = oldnr;
        return;
    }
    else
    {
        if(requestMap[nr].type == QDROPBOX_REQ_REDIREC)
        {
            // change values if this is the answert to a redirect
            qdropbox_request redir = requestMap[nr];
            qdropbox_request orig  = requestMap[redir.linked];
            requestMap[nr] = orig;
            requestMap.remove(nr);
            nr = redir.linked;
        }

        // standard handling depending on message type
        switch(requestMap[nr].type)
        {
        case QDROPBOX_REQ_CONNECT:
            // was only a connect request - so drop it
            break;
        case QDROPBOX_REQ_RQTOKEN:
            // requested a tiken
            responseTokenRequest(response);
            break;
		case QDROPBOX_REQ_RQBTOKN:
			responseBlockedTokenRequest(response);
			break;
        case QDROPBOX_REQ_AULOGIN:
            delayed_nr = responseDropboxLogin(response, nr);
            delayed_finish = true;
            break;
        case QDROPBOX_REQ_ACCTOKN:
            responseAccessToken(response);
            break;
		case QDROPBOX_REQ_METADAT:
			parseMetadata(response);
			break;
		case QDROPBOX_REQ_BMETADA:
			parseBlockingMetadata(response);
		case QDROPBOX_REQ_BACCTOK:
			responseBlockingAccessToken(response);
			break;
        case QDROPBOX_REQ_ACCINFO:
            parseAccountInfo(response);
            break;
		case QDROPBOX_REQ_BACCINF:
			parseBlockingAccountInfo(response);
			break;
        case QDROPBOX_REQ_SHRDLNK:
            parseSharedLink(response);
            break;

        case QDROPBOX_REQ_BSHRDLN:
            parseBlockingSharedLink(response);
            break;
        default:
            errorState  = QDropbox::ResponseToUnknownRequest;
            errorText   = "Received a response to an unknown request";
            emit errorOccured(errorState);
            break;
        }
    }

    if(delayed_finish)
        delayMap[delayed_nr] = nr;
    else
    {
        if(delayMap[nr])
        {
            int drq = delayMap[nr];
            while(drq!=0)
            {
                emit operationFinished(delayMap[drq]);
                delayMap.remove(drq);
                drq = delayMap[drq];
            }
        }

        requestMap.remove(nr);
        emit operationFinished(nr);
    }

    return;
}

void QDropbox::networkReplyFinished(QNetworkReply *rply)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "reply finished" << endl;
#endif
    int reqnr = replynrMap[rply];
    requestFinished(reqnr, rply);
}

QString QDropbox::hmacsha1(QString base, QString key)
{
    // inner pad
    QByteArray ipad;
    ipad.fill(char(0), 64);
    for(int i = 0; i < key.length(); ++i)
        ipad[i] = key[i].toAscii();

    // outer pad
    QByteArray opad;
    opad.fill(char(0), 64);
    for(int i = 0; i < key.length(); ++i)
        opad[i] = key[i].toAscii();

    // XOR operation for inner pad
    for(int i = 0; i < ipad.length(); ++i)
        ipad[i] = ipad[i] ^ 0x36;

    // XOR operation for outer pad
    for(int i = 0; i < opad.length(); ++i)
        opad[i] = opad[i] ^ 0x5c;

    // Hashes inner pad
    QByteArray innerSha1 = QCryptographicHash::hash(
        ipad + base.toAscii(),
        QCryptographicHash::Sha1
        );

    // Hashes outer pad
    QByteArray outerSha1 = QCryptographicHash::hash(
        opad + innerSha1,
        QCryptographicHash::Sha1
        );

    return outerSha1.toBase64();
}

/*QString QDropbox::hmacsha1(QByteArray key, QByteArray baseString)
{
    int blockSize = 64; // HMAC-::hmacsha1SHA-1 block size, defined in SHA-1 standard
    if (key.length() > blockSize) { // if key is longer than block size (64), reduce key length with SHA-1 compression
        key = QCryptographicHash::hash(key, QCryptographicHash::Sha1);
    }

    QByteArray innerPadding(blockSize, char(0x36)); // initialize inner padding with char "6"
    QByteArray outerPadding(blockSize, char(0x5c)); // initialize outer padding with char "\"
    // ascii characters 0x36 ("6") and 0x5c ("\") are selected because they have large
    // Hamming distance (http://en.wikipedia.org/wiki/Hamming_distance)

    for (int i = 0; i < key.length(); i++) {
        innerPadding[i] = innerPadding[i] ^ key.at(i); // XOR operation between every byte in key and innerpadding, of key length
        outerPadding[i] = outerPadding[i] ^ key.at(i); // XOR operation between every byte in key and outerpadding, of key length
    }

    // result = hash ( outerPadding CONCAT hash ( innerPadding CONCAT baseString ) ).toBase64
    QByteArray total = outerPadding;
    QByteArray part = innerPadding;
    part.append(baseString);
    total.append(QCryptographicHash::hash(part, QCryptographicHash::Sha1));
    QByteArray hashed = QCryptographicHash::hash(total, QCryptographicHash::Sha1);
    return hashed.toBase64();
}*/

QString QDropbox::generateNonce(qint32 length)
{
    QString clng = "";
        for(int i=0; i<length; ++i)
            clng += QString::number(int( qrand() / (RAND_MAX + 1.0) * (16 + 1 - 0) + 0 ), 16).toUpper();
        return clng;
}

QString QDropbox::oAuthSign(QUrl base, QString method)
{
    if(oauthMethod == QDropbox::Plaintext)
        return QString("%1&%2").arg(_appSharedSecret).arg(oauthTokenSecret);

    QString param   = base.toString(QUrl::RemoveAuthority|QUrl::RemovePath|QUrl::RemoveScheme).mid(1);
    param = QUrl::toPercentEncoding(param);
    QString requrl  = base.toString(QUrl::RemoveQuery);
    requrl = QUrl::toPercentEncoding(requrl);
#ifdef QTDROPBOX_DEBUG
    qDebug() << "param = " << param << endl << "requrl = " << requrl << endl;
#endif
    QString baseurl = method+"&"+requrl+"&"+param;
    QString key     = QString("%1&%2").arg(_appSharedSecret).arg(oauthTokenSecret);
#ifdef QTDROPBOX_DEBUG
    qDebug() << "baseurl = " << baseurl << " endbase";
	qDebug() << "key = " << key << " endkey";
#endif

    QString signature = "";
    if(oauthMethod == QDropbox::HMACSHA1)
        signature = hmacsha1(baseurl.toUtf8(), key.toUtf8());
    else
    {
        errorState = QDropbox::UnknownAuthMethod;
        errorText  = QString("Authentication method %1 is unknown").arg(oauthMethod);
        emit errorOccured(errorState);
        return "";
    }

#ifdef QTDROPBOX_DEBUG
    qDebug() << "key = " << key << endl;
    qDebug() << "signature = " << signature << "(base64 = " << QByteArray(signature.toUtf8()).toBase64() << endl;

#endif
    return signature.toUtf8();
}

void QDropbox::prepareApiUrl()
{
    //if(oauthMethod == QDropbox::Plaintext)
        apiurl.setScheme("https");
    //else
      //  apiurl.setScheme("http");
}

int QDropbox::sendRequest(QUrl request, QString type, QByteArray postdata, QString host)
{
    if(!host.trimmed().compare(""))
        host = apiurl.toString(QUrl::RemoveScheme).mid(2);

#ifdef QTDROPBOX_DEBUG
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
        rply = conManager.get(rq);
    else if(!type.compare("POST"))
    {
        rq.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
        rply = conManager.post(rq, postdata);
    }
    else
    {
        errorState = QDropbox::UnknownQueryMethod;
        errorText  = "The provided query method is unknown.";
        emit errorOccured(errorState);
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error " << errorState << "(" << errorText << ") in request" << endl;
#endif
        return -1;
    }

    replynrMap[rply] = ++lastreply;

    requestMap[lastreply].method = type;
    requestMap[lastreply].host   = host;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "sendRequest() -> request #" << lastreply << " sent." << endl;
#endif
    return lastreply;
}

void QDropbox::responseTokenRequest(QString response)
{
    parseToken(response);
    emit requestTokenFinished(oauthToken, oauthTokenSecret);
    return;
}

int QDropbox::responseDropboxLogin(QString response, int reqnr)
{
    // extract login form
    QDomDocument xml;
    QString err;
    int lnr, cnr;
    if(!xml.setContent(response, false, &err, &lnr, &cnr))
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "invalid xml (" << lnr << "," << cnr << "): " << err << "dump:" << endl;
        qDebug() << xml.toString() << endl;
#endif
        return 0;
    }
    return 0;
}

void QDropbox::responseAccessToken(QString response)
{
    parseToken(response);
    emit accessTokenFinished(oauthToken, oauthTokenSecret);
    return;
}

QString QDropbox::signatureMethodString()
{
    QString sigmeth;
    switch(oauthMethod)
    {
    case QDropbox::Plaintext:
        sigmeth = "PLAINTEXT";
        break;
    case QDropbox::HMACSHA1:
        sigmeth = "HMAC-SHA1";
        break;
    default:
        errorState = QDropbox::UnknownAuthMethod;
        errorText  = QString("Authentication method %1 is unknown").arg(oauthMethod);
        emit errorOccured(errorState);
        return "";
        break;
    }
    return sigmeth;
}

void QDropbox::parseToken(QString response)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "processing token request" << endl;
#endif

    QStringList split = response.split("&");
    if(split.size() < 2)
    {
        errorState = QDropbox::APIError;
        errorText  = "The Dropbox API did not respond as expected.";
        emit errorOccured(errorState);
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error " << errorState << "(" << errorText << ") in request" << endl;
#endif
        return;
    }

    if(!split.at(0).startsWith("oauth_token_secret") ||
       !split.at(1).startsWith("oauth_token"))
	{
        errorState = QDropbox::APIError;
        errorText  = "The Dropbox API did not respond as expected.";
        emit errorOccured(errorState);
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error " << errorState << "(" << errorText << ") in request" << endl;
#endif
        return;
    }

    QStringList tokenSecretList = split.at(0).split("=");
    oauthTokenSecret = tokenSecretList.at(1);
    QStringList tokenList = split.at(1).split("=");
    oauthToken = tokenList.at(1);

#ifdef QTDROPBOX_DEBUG
    qDebug() << "token = " << oauthToken << endl << "token_secret = " << oauthTokenSecret << endl;
#endif

    emit tokenChanged(oauthToken, oauthTokenSecret);
    return;
}

void QDropbox::parseAccountInfo(QString response)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "== account info ==" << response << "== account info end ==";
#endif

    QDropboxJson json;
    json.parseString(response);
    _tempJson.parseString(response);
    if(!json.isValid())
    {
        errorState = QDropbox::APIError;
        errorText  = "Dropbox API did not send correct answer for account information.";
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error: " << errorText << endl;
#endif
        emit errorOccured(errorState);
        return;
    }

    emit accountInfoReceived(response);
    return;
}

void QDropbox::parseSharedLink(QString response)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "== shared link ==" << response << "== shared link end ==";
#endif

    //QDropboxJson json;
    //json.parseString(response);
    _tempJson.parseString(response);
    if(!_tempJson.isValid())
    {
        errorState = QDropbox::APIError;
        errorText  = "Dropbox API did not send correct answer for file/directory shared link.";
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error: " << errorText << endl;
#endif
        emit errorOccured(errorState);
        stopEventLoop();
        return;
    }
    emit sharedLinkReceived(response);
}

void QDropbox::parseMetadata(QString response)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "== metadata ==" << response << "== metadata end ==";
#endif

	QDropboxJson json;
	json.parseString(response);
	_tempJson.parseString(response);
	if(!json.isValid())
	{
		errorState = QDropbox::APIError;
        errorText  = "Dropbox API did not send correct answer for file/directory metadata.";
#ifdef QTDROPBOX_DEBUG
        qDebug() << "error: " << errorText << endl;
#endif
        emit errorOccured(errorState);
        stopEventLoop();
        return;
	}

	emit metadataReceived(response);
	return;
}

void QDropbox::setKey(QString key)
{
    _appKey = key;
    return;
}

QString QDropbox::key()
{
    return _appKey;
}

void QDropbox::setSharedSecret(QString sharedSecret)
{
    _appSharedSecret = sharedSecret;
    return;
}

QString QDropbox::sharedSecret()
{
    return _appSharedSecret;
}

void QDropbox::setToken(QString t)
{
    oauthToken = t;
    return;
}

QString QDropbox::token()
{
    return oauthToken;
}

void QDropbox::setSecret(QString s)
{
    oauthTokenSecret = s;
    return;
}

QString QDropbox::tokenSecret()
{
    return oauthTokenSecret;
}

QString QDropbox::appKey()
{
    return _appKey;
}

QString QDropbox::appSharedSecret()
{
    return _appSharedSecret;
}

QString QDropbox::apiVersion()
{
    return _version;
}

int QDropbox::requestToken(bool blocking)
{
    QString sigmeth = signatureMethodString();

    timestamp = QDateTime::currentMSecsSinceEpoch()/1000;
    nonce = generateNonce(128);

    QUrl url;
    url.setUrl(apiurl.toString());
    url.addQueryItem("oauth_consumer_key",_appKey);
    url.addQueryItem("oauth_nonce", nonce);
    url.addQueryItem("oauth_signature_method", sigmeth);
    url.addQueryItem("oauth_timestamp", QString::number(timestamp));
    url.addQueryItem("oauth_version", _version);
    url.setPath(QString("%1/oauth/request_token").arg(_version.left(1)));

    QString signature = oAuthSign(url);

    url.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));
#ifdef QTDROPBOX_DEBUG
    qDebug() << "request token url: " << url.toString() << endl << "sig: " << signature << endl;
    qDebug() << "sending request " << url.toString() << " to " << apiurl.toString() << endl;
#endif

    int reqnr = sendRequest(url);
	if(blocking)
	{
		requestMap[reqnr].type = QDROPBOX_REQ_RQBTOKN;
		startEventLoop();
	}
	else
		requestMap[reqnr].type = QDROPBOX_REQ_RQTOKEN;

    return reqnr;
}

bool QDropbox::requestTokenAndWait()
{
	requestToken(true);
	return (error() == NoError);
}

int QDropbox::authorize(QString email, QString pwd)
{
    QUrl dropbox_authorize;
    dropbox_authorize.setPath(QString("%1/oauth/authorize")
                           .arg(_version.left(1)));
    qDebug() << "oauthToken = " << oauthToken << endl;
    dropbox_authorize.addQueryItem("oauth_token", oauthToken);
    int reqnr = sendRequest(dropbox_authorize, "GET", 0, "www.dropbox.com");
    requestMap[reqnr].type = QDROPBOX_REQ_AULOGIN;
    mail     = email;
    password = pwd;
    return reqnr;
}

QUrl QDropbox::authorizeLink()
{
    QUrl link;
    link.setScheme("https");
    link.setHost("www.dropbox.com");
    link.setPath(QString("%1/oauth/authorize")
                 .arg(_version.left(1)));
    link.addQueryItem("oauth_token", oauthToken);
    return link;
}

int QDropbox::requestAccessToken(bool blocking)
{
    QUrl url;
    url.setUrl(apiurl.toString());
    url.addQueryItem("oauth_consumer_key",_appKey);
    url.addQueryItem("oauth_nonce", nonce);
    url.addQueryItem("oauth_signature_method", signatureMethodString());
    url.addQueryItem("oauth_timestamp", QString::number(timestamp));
    url.addQueryItem("oauth_token", oauthToken);
    url.addQueryItem("oauth_version", _version);
    url.setPath(QString("%1/oauth/access_token").
                arg(_version.left(1)));

#ifdef QTDROPBOX_DEBUG
	qDebug() << "requestToken = " << url.queryItemValue("oauth_token");
#endif

    QString signature = oAuthSign(url);
    url.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    QString dataString = url.toString(QUrl::RemoveScheme|QUrl::RemoveAuthority|
                                      QUrl::RemovePath).mid(1);
#ifdef QTDROPBOX_DEBUG
    qDebug() << "dataString = " << dataString << endl;
#endif

    QByteArray postData;
    postData.append(dataString.toUtf8());

    QUrl query(url.toString(QUrl::RemoveQuery));
    int reqnr = sendRequest(query, "POST", postData);

	if(blocking)
	{
		requestMap[reqnr].type = QDROPBOX_REQ_BACCTOK;
		startEventLoop();
	}
	else
		requestMap[reqnr].type = QDROPBOX_REQ_ACCTOKN;

    return reqnr;
}

bool QDropbox::requestAccessTokenAndWait()
{
	requestAccessToken(true);
#ifdef QTDROPBOX_DEBUG
	qDebug() << "requestTokenAndWait() finished: error = " << error() << endl;
#endif
	return (error() == NoError);
}

void QDropbox::requestAccountInfo(bool blocking)
{
	clearError();

	timestamp = QDateTime::currentMSecsSinceEpoch()/1000;

    QUrl url;
    url.setUrl(apiurl.toString());
    url.addQueryItem("oauth_consumer_key",_appKey);
    url.addQueryItem("oauth_nonce", nonce);
    url.addQueryItem("oauth_signature_method", signatureMethodString());
    url.addQueryItem("oauth_timestamp", QString::number(timestamp));
    url.addQueryItem("oauth_token", oauthToken);
    url.addQueryItem("oauth_version", _version);
    url.setPath(QString("%1/account/info").arg(_version.left(1)));

    QString signature = oAuthSign(url);
    url.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    int reqnr = sendRequest(url);
	if(blocking)
	{
		requestMap[reqnr].type = QDROPBOX_REQ_BACCINF;
		startEventLoop();
	}
	else
		requestMap[reqnr].type = QDROPBOX_REQ_ACCINFO;
    return;
}

QDropboxAccount QDropbox::requestAccountInfoAndWait()
{
	requestAccountInfo(true);
	QDropboxAccount a(_tempJson.strContent(), this);
    _account = a;
	return _account;
}

void QDropbox::parseBlockingAccountInfo(QString response)
{
	clearError();
	parseAccountInfo(response);
	stopEventLoop();
	return;
}

void QDropbox::requestMetadata(QString file, bool blocking)
{
	clearError();

	timestamp = QDateTime::currentMSecsSinceEpoch()/1000;

	QUrl url;
	url.setUrl(apiurl.toString());
	 url.addQueryItem("oauth_consumer_key",_appKey);
    url.addQueryItem("oauth_nonce", nonce);
    url.addQueryItem("oauth_signature_method", signatureMethodString());
    url.addQueryItem("oauth_timestamp", QString::number(timestamp));
    url.addQueryItem("oauth_token", oauthToken);
    url.addQueryItem("oauth_version", _version);
	url.setPath(QString("%1/metadata/%2").arg(_version.left(1), file));

	QString signature = oAuthSign(url);
	url.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));
	
	int reqnr = sendRequest(url);
	if(blocking)
	{
		requestMap[reqnr].type = QDROPBOX_REQ_BMETADA;
		startEventLoop();
	}
	else
		requestMap[reqnr].type = QDROPBOX_REQ_METADAT;
	//QDropboxFileInfo fi(_tempJson.strContent(), this);
	return;
}

QDropboxFileInfo QDropbox::requestMetadataAndWait(QString file)
{
	requestMetadata(file, true);
	QDropboxFileInfo fi(_tempJson.strContent(), this);
	return fi;
}

void QDropbox::requestSharedLink(QString file, bool blocking)
{
    QUrl url;
    url.setUrl(apiurl.toString());
    url.addQueryItem("oauth_consumer_key",_appKey);
    url.addQueryItem("oauth_nonce", nonce);
    url.addQueryItem("oauth_signature_method", signatureMethodString());
    url.addQueryItem("oauth_timestamp", QString::number(timestamp));
    url.addQueryItem("oauth_token", oauthToken);
    url.addQueryItem("oauth_version", _version);
    url.setPath(QString("%1/shares/%2").arg(_version.left(1), file));

    QString signature = oAuthSign(url);
    url.addQueryItem("oauth_signature", QUrl::toPercentEncoding(signature));

    int reqnr = sendRequest(url);
    if(blocking)
    {
        requestMap[reqnr].type = QDROPBOX_REQ_BSHRDLN;
        startEventLoop();
    }
    else
        requestMap[reqnr].type = QDROPBOX_REQ_SHRDLNK;

    return;
}

QUrl QDropbox::requestSharedLinkAndWait(QString file)
{
    requestSharedLink(file,true);
    QDropboxJson json(_tempJson.strContent());
    QString urlString = json.getString("url");
    return QUrl(urlString);
}

void QDropbox::startEventLoop()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox::startEventLoop()" << endl;
#endif
    if(_evLoop == NULL)
        _evLoop = new QEventLoop(this);
    _evLoop->exec();
    return;
}

void QDropbox::stopEventLoop()
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "QDropbox::stopEventLoop()" << endl;
#endif
    if(_evLoop == NULL)
        return;
#ifdef QTDROPBOX_DEBUG
    qDebug() << "loop ended" << endl;
#endif
    _evLoop->exit();
    return;
}

void QDropbox::responseBlockedTokenRequest(QString response)
{
	clearError();
	responseTokenRequest(response);
	stopEventLoop();
	return;
}

void QDropbox::responseBlockingAccessToken(QString response)
{
	clearError();
	responseAccessToken(response);
	stopEventLoop();
	return;
}

void QDropbox::parseBlockingMetadata(QString response)
{
	clearError();
	parseMetadata(response);
	stopEventLoop();
	return;
}

void QDropbox::parseBlockingSharedLink(QString response)
{
    clearError();
    parseSharedLink(response);
    stopEventLoop();
}

// check if the event loop has to be stopped after a blocking request was sent
void QDropbox::checkReleaseEventLoop(int reqnr)
{
	switch(requestMap[reqnr].type)
	{
	case QDROPBOX_REQ_RQBTOKN:
	case QDROPBOX_REQ_BACCTOK:
	case QDROPBOX_REQ_BACCINF:
    case QDROPBOX_REQ_BMETADA:
		stopEventLoop(); // release local event loop
		break;
	default:
		break;
	}
	return;
}

void QDropbox::clearError()
{
	errorState  = QDropbox::NoError;
	errorText  = "";
	return;
}

