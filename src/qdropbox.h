#ifndef QDROPBOX_H
#define QDROPBOX_H

#include "qtdropbox_global.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QHttpRequestHeader>
#include <QHttpResponseHeader>
#include <QCryptographicHash>
#include <QDateTime>
#include <QUrl>
#include <QDomDocument>

#ifdef QTDROPBOX_DEBUG
#include <QDebug>
#endif

#include "qdropboxjson.h"
#include "qdropboxaccount.h"

typedef int qdropbox_request_type;

const qdropbox_request_type QDROPBOX_REQ_CONNECT = 0x01;
const qdropbox_request_type QDROPBOX_REQ_RQTOKEN = 0x02;
const qdropbox_request_type QDROPBOX_REQ_AULOGIN = 0x03;
const qdropbox_request_type QDROPBOX_REQ_REDIREC = 0x04;
const qdropbox_request_type QDROPBOX_REQ_ACCTOKN = 0x05;
const qdropbox_request_type QDROPBOX_REQ_ACCINFO = 0x06;

const qint32 QDROPBOX_ERROR_BAD_INPUT           = 400;
const qint32 QDROPBOX_ERROR_EXPIRED_TOKEN       = 401;
const qint32 QDROPBOX_ERROR_BAD_OAUTH_REQUEST   = 403;
const qint32 QDROPBOX_ERROR_FILE_NOT_FOUND      = 404;
const qint32 QDROPBOX_ERROR_WRONG_METHOD        = 405;
const qint32 QDROPBOX_ERROR_REQUEST_CAP         = 503;
const qint32 QDROPBOX_ERROR_USER_OVER_QUOTA     = 507;

struct qdropbox_request{
    qdropbox_request_type type;
    QString method;
    QString host;
    int linked;
};

class QTDROPBOXSHARED_EXPORT QDropbox : public QObject
{
    Q_OBJECT
public:
    enum OAuthMethod{
        Plaintext,
        HMACSHA1
    };

    enum Error{
        NoError,
        CommunicationError,
        VersionNotSupported,
        UnknownAuthMethod,
        ResponseToUnknownRequest,
        APIError,
        UnknownQueryMethod,
        BadInput,
        BadOAuthRequest,
        WrongHttpMethod,
        MaxRequestsExeeded,
        UserOverQuota
    };

    explicit QDropbox(QObject *parent = 0);
    explicit QDropbox(QString key, QString sharedSecret,
                      OAuthMethod method = QDropbox::Plaintext,
                      QString url = "api.dropbox.com", QObject *parent = 0);

    void test();

    // get and set
    qint64  error();
    QString errorString();

    void setApiUrl(QString url);
    QString apiUrl();

    void setAuthMethod(OAuthMethod m);
    OAuthMethod authMethod();

    void setApiVersion(QString apiversion);
    QString apiVersion();

    void setKey(QString key);
    QString key();

    void setSharedSecret(QString sharedSecret);
    QString sharedSecret();


    // authentication
    int requestToken();
    int authorize(QString mail, QString password);
    QUrl authorizeLink();
    int requestAccessToken();

    // account info
    int requestAccountInfo();

signals:
    void errorOccured(Error errorcode);
    void tokenExpired();
    void fileNotFound();

    void operationFinished(int requestnr);
    void requestTokenFinished(QString token, QString secret);
    void accessTokenFinished(QString token, QString secret);
    void tokenChanged(QString token, QString secret);

    void accountInfo(QString accountJson);


public slots:

private slots:
    void requestFinished(int nr, QNetworkReply* rply);
    void networkReplyFinished(QNetworkReply* rply);

private:
    QNetworkAccessManager conManager;

    Error   errorState;
    QString errorText;

    QString appKey;
    QString appSharedSecret;

    QUrl        apiurl;
    QString     nonce;
    long        timestamp;
    OAuthMethod oauthMethod;
    QString     version;

    QString oauthToken;
    QString oauthTokenSecret;

    QMap <QNetworkReply*,int>  replynrMap;
    int  lastreply;
    QMap<int,qdropbox_request> requestMap;
    QMap<int,int> delayMap;

    QString mail;
    QString password;


    QString hmacsha1(QByteArray key, QByteArray baseString);
    QString generateNonce(qint32 length);
    QString oAuthSign(QUrl base, QString method = "GET");
    void prepareApiUrl();
    int sendRequest(QUrl request, QString type = "GET", QByteArray postdata = 0, QString host = "");
    void responseTokenRequest(QString response);
    int responseDropboxLogin(QString response, int reqnr);
    void responseAccessToken(QString response);
    QString signatureMethodString();
    void parseToken(QString response);
    void parseAccountInfo(QString response);
};

#endif // QDROPBOX_H
