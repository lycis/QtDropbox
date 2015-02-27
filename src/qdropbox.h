#ifndef QDROPBOX_H
#define QDROPBOX_H

#include "qtdropbox_global.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QDateTime>
#include <QUrl>
#include <QDomDocument>
#include <QEventLoop>
#include <QUrlQuery>

#ifdef QTDROPBOX_DEBUG
#include <QDebug>
#endif

#include "qtdropbox_global.h"
#include "qdropboxjson.h"
#include "qdropboxaccount.h"
#include "qdropboxfileinfo.h"
#include "qdropboxdeltaresponse.h"

typedef int qdropbox_request_type;

const qdropbox_request_type QDROPBOX_REQ_INVALID = 0x00;
const qdropbox_request_type QDROPBOX_REQ_CONNECT = 0x01;
const qdropbox_request_type QDROPBOX_REQ_RQTOKEN = 0x02;
const qdropbox_request_type QDROPBOX_REQ_AULOGIN = 0x03;
const qdropbox_request_type QDROPBOX_REQ_REDIREC = 0x04;
const qdropbox_request_type QDROPBOX_REQ_ACCTOKN = 0x05;
const qdropbox_request_type QDROPBOX_REQ_ACCINFO = 0x06;
const qdropbox_request_type QDROPBOX_REQ_RQBTOKN = 0x07;
const qdropbox_request_type QDROPBOX_REQ_BACCTOK = 0x08;
const qdropbox_request_type QDROPBOX_REQ_METADAT = 0x09;
const qdropbox_request_type QDROPBOX_REQ_BACCINF = 0x0A;
const qdropbox_request_type QDROPBOX_REQ_BMETADA = 0x0B;
const qdropbox_request_type QDROPBOX_REQ_SHRDLNK = 0x0C;
const qdropbox_request_type QDROPBOX_REQ_BSHRDLN = 0x0D;
const qdropbox_request_type QDROPBOX_REQ_REVISIO = 0x0E;
const qdropbox_request_type QDROPBOX_REQ_BREVISI = 0x0F;
const qdropbox_request_type QDROPBOX_REQ_DELTA   = 0x10;
const qdropbox_request_type QDROPBOX_REQ_BDELTA  = 0x11;

//! Internally used struct to handle network requests sent from QDropbox
/*!
  This structure is used internally by QDropbox. It is used to connect network
  requests that are sent to the Dropbox API server with the asynchronous queries
  made to the QtDropbox API.
 */
struct qdropbox_request{
    qdropbox_request_type type; //!< Type of the request
    QString method;             //!< Used method to send the request (POST/GET)
    QString host;               //!< Host that received the request
    int linked;                 //!< ID of any linked request (for forwarded requests)
};

//! The main entry point of QtDropbox API. Provides various connection facilities and general information.
/*!
  QDropbox provides you with all utilities required to connect to any Dropbox account. For purposes of
  connection this class provides an asynchronous, signal and slot based, interface.

  <h3>Connection to new account</h3>
  If you want to initiate a new connection to an account that did not authorize your application to
  access it you use requestToken() and then you have to call requestAccessToken as soon as the signal
  requestTokenFinished() is emitted.

  If the token you are using is not authorized or is expired the signal tokenExpired() will be emitted.
  In this case you have to prompt the user for reauthorization of your application. A link to the
  authoriziation interface of Dropbox is provided by the function authorizeLink(). This API does not
  automatise the authorization process as this feature is not provided by Dropbox. So you have to
  display the link in a web browser.

  To reconnect to an account on a later use of your application you have to save the token and token
  secret obtained after requestAccessToken(). These values are provided by the functions token() and
  tokenSecret().

  <h3>Connection to authorized account</h3>
  If the account you want to connect to has already authorized your application and you already
  have obtained an authorized token and token secret you will use a shortcut to connect. You have
  to set the token and token secret you obtained by a prior use of the API with the according
  functions setToken() and setTokenSecret(). You do not need to invoke requestToken() or
  requestAccessToken(). These functions are only called at first use or if no token and token secret
  are available.

  Should the token or token secret you are using be already expired the signal tokenExpired()
  will be emitted. In that case you have to prompt the user for reauthorization.

  <h3>Using blocking requests</h3>
  Every function that requests information from the server has a blocking and non-blocking function.
  A blocking request will wait until the server has responded to your query before returning while a
  non-blocking request will return immediately. Usually a blocking function directly returns a result
  and a non-blocking function will emit an according signal as the request has finished.

  \warning The use of a blocking function will reset the current error flag. So after calling a blocking
  function the function error() will return QDropbox::NoError if no error occurred or the error that
  occurred when processing the blocking request.

  \bug HMAC-SHA1 authentication is not working (does not have to be in 1.0)

 */
class QTDROPBOXSHARED_EXPORT QDropbox : public QObject
{
    Q_OBJECT
public:
    //! Method for oAuth authentication
    /*! These methods are used for authentication with the oAuth protocol
        \bug Currently HMAC-SHA1 encoding does not work. (does not have to be in 1.0)
     */
    enum OAuthMethod{
        Plaintext, /*!< Plaintext authentication, HTTPS is automatically used. */
        HMACSHA1 /*!< HMAC-SHA1 encoded authentication */
    };

    //! Error state of QDropbox
    /*!
      This enum is used to determine the current error state of the Dropbox connection.
      If an error occurs it can be access by using the error() function.
     */
    enum Error{
        NoError,                        /*!< No error occured */
        CommunicationError,             /*!< Error while communicating with the server */
        VersionNotSupported,            /*!< The used Dropbox API version is not supported by the server */
        UnknownAuthMethod,              /*!< The used authentication method is not supported */
        ResponseToUnknownRequest,       /*!< QtDropbox API received an unexpected response from the server */
        APIError,                       /*!< The remote server violated the Dropbox REST API protocol by sending wrong data. */
        UnknownQueryMethod,             /*!< Internal error. The API tried to send with a not supported HTTP Query method. */
        BadInput,                       /*!< A wrong input parameter was sent to the Dropbox REST API. Dropbox API error 400*/
        BadOAuthRequest,                /*!< A wrong oAuth request was received by the server (expired time stamp,
                                             bad nonce etc.). Dropbox API error 403 */
        WrongHttpMethod,                /*!< The REST API request used a wrong HTTP method. Dropbox API error 405 */
        MaxRequestsExceeded,            /*!< The maximum amount of requests was exceeded. Dropbox API error 503 */
        UserOverQuota,                  /*!< The user exceeded his or her storage quota. Dropbox API error 507 */
        TokenExpired                    /*!< The access token has expired. Dropbox API error 401*/
    };

    /*!
      This constructor creates an unconfigured instance of QDropbox. The server URL is set to <em>api.dropbpx.com</em>,
      the REST API version 1.0 is used (currently the only one supported) and the authentication method is
      QDropbox::Plaintext.

      You need to set your API key and shared secret by using setKey(QString key) and setSharedSecret(QString sharedSecret).

      \param parent The parent object QDropbox depends on.
     */
    explicit QDropbox(QObject *parent = 0);

    /*!
      This constructor initializes QDropbox with your key and shared secret. The selected authentication method and
      API URL will be set as well.

      \param key API key of your application (provided by Dropbox)
      \param sharedSecret Your app's secret (provided by Dropbox
      \param method Used authentication method
      \param url URL of the API server
      \param parent Parent object of QDropbox

     */
    explicit QDropbox(QString key, QString sharedSecret,
                      OAuthMethod method = QDropbox::Plaintext,
                      QString url = "api.dropbox.com", QObject *parent = 0);

    /*!
      If an error occured you can access the last error code by using this function.
     */
    Error  error();

    /*!
      After an error occured you'll get a description of the last error by using this
      function.
     */
    QString errorString();

    /*!
      Use this function if you want to change the URL of the API server you are
      accessing. This won't usually be necessary as QtDropbox automatically chooses the
      official Dropbox API server according to the request. This is usually
      http://api.dropbox.com

      \param url URL of the API server. Usually this is <em>api.dropbox.com</em>
     */
    void setApiUrl(QString url);

    /*!
      Provides you with the address of the API server.
     */
    QString apiUrl();

    /*!
      This function is used to changed the used authentication method. You can use it
      even if you want to change the authentication method during an already existing
      connection.

      \param m Authentication method.
     */
    void setAuthMethod(OAuthMethod m);

    /*!
      Returns the currently used authentication method.
     */
    OAuthMethod authMethod();

    /*!
      Set the version of the Dropbox API to be used. 1.0 is default. Usually you don't
      need to use this function as currently only version 1.0 is supported by Dropbox.

      \param apiversion Version string of the API
     */
    void setApiVersion(QString apiversion);

    /*!
      Returns the currently used API version.
     */
    QString apiVersion();

    /*!
      Use this function to set your applications API key if you did not already when
      using the constructor. The API key is provided when you register your application
      with Dropbox.

      \param key API key of your application.
     */
    void setKey(QString key);

    /*!
      Returns the used API key of the application.
    */
    QString key();

    /*!
      Use this function to set your applications API secret if you did not when using
      the constructor. The API secret is provided when you register your application
      with Dopbox.

      \param sharedSecret API secret of your application
     */
    void setSharedSecret(QString sharedSecret);

    /*!
      Returns the used API secret.
     */
    QString sharedSecret();

    /*!
      If you have an already verified and authorized token to communicate with the
      Dropbox API you can set it by using this function. By setting a token you
      do not need to use requestToken() and requestAccessToken() to iniate a
      connection.

      \param t token string
     */
    void setToken(QString t);
    /*!
      Returns the used token. This function may be used to get an authorized token
      after iniating a new connection (e.g. to save it for later use).
     */
    QString token();

    /*!
      If you have an already verified and authorized token and token secret to
      communicate with the Dropbox API you can set the secret by using this
      function. By setting token and secret you do not need to use requestToken()
      and requestAccessToken() to iniaite a connection.

      \param s token secret string
     */
    void setTokenSecret(QString s);
    /*!
      Returns the currently used token secret. This function may be used to get an
      authorized token secret after iniating a new connection (e.g. to save it).
     */
    QString tokenSecret();

    /*!
      Returns the Dropbox API key that is used.
     */
    QString appKey();

    /*!
      Returns the currently used Dropbox API shared secret of your application.
     */
    QString appSharedSecret();

    /*!
      This functions requests a request token that will be valid for the rest of the
      authentication process. When the token is received the signal
      requestTokenFinished(...) will be emitted.

      After the request token was obtained you can continue with the authentication by
      prompting the user to authorize your application.

      It is not necessary to call this function when the user already authenticated
      your application. In this case just provide the token and token secret received
      by using requestAccessToken() to QDropbox.

      \param blocking <i>internal only</i> indidicates if the call should block
     */
    int requestToken(bool blocking = false);

    /*!
      This functions works exactly like requestToken(...) but will block until the
      answer (e.g. the token or an error) has arrived from the server.

      \return <i>true</i> if the token was received successfully or <i>false</i> if an
              error occured
    */
    bool requestTokenAndWait();
    /*!
      This function should do automatic authorization.
      \warning This functions is currently not supported by the Dropbox API. You need
               the user to authenticate by using the URL provided by authorizeLink().
     */
    int authorize(QString mail, QString password);
    /*!
      Returns an URL the user will have to use to authorize the connection to your
      application. You may use that link in connection with QDesktopServices::openUrl(...)
      to open a web browser with the returned URL.
     */
    QUrl authorizeLink();

    /*!
      This function should be invoked after the user authorized your application. It
      retrieves an access token from the Dropbox API that you'll have to use to access
      Dropbox services.

      \param blocking <i>internal only</i> indidicates if the call should block
     */
    int requestAccessToken(bool blocking = false);

    /*!
      This functions works exactly like requestAccessToken(...) but blocks until the answer
      from the server was received.

      \return <i>true</i> if the access token could be requested without error or <i>false</i>
              if an error occured.
    */
    bool requestAccessTokenAndWait();

    /*!
      By using this function the account information of the connected user will be
      retrieved. When the account information was obtained the signal QDropbox::accountInfoReceived()
      will be emitted.

      \param blocking <i>internal only</i> indidicates if the call should block
     */
    void requestAccountInfo(bool blocking = false);

    /*!
      Works exactly like accountInfo() but blocks until the data was received from the server.
      It returns an instance of QDropboxAccount containing the requested data. You do not have
      to react on the accountInfoReceived() signal when using this function.
     */
    QDropboxAccount requestAccountInfoAndWait();

    /*!
      This function is public for internal QtDropbox API use. It is used to sign
      requests to the Dropbox API and thus is required by most other QtDropbox
      classes for their requests.

      \param base Complete unsigned request URL
      \param method Request method (currently only POST or GET)
     */
    QString oAuthSign(QUrl base, QString method = "GET");

    /*!
      Returns the authentication method as string.
     */
    QString signatureMethodString();

    /*!
      This functions generates and returns a nonce with the given length. The
      generated nonce is a random hex based string.

      \param length Length of the nonce.
     */
    static QString generateNonce(qint32 length);

    /*!
      Get the file metadata for a file speciified by the filename. When the Dropbox
      API server answeres the request the signal QDropbox::metadataReceived() will be
      emitted.

      \param file The absoulte path of the file (e.g. <i>/dropbox/test.txt</i>)
      \param blocking <i>internal only</i> indidicates if the call should block
    */
    void requestMetadata(QString file, bool blocking = false);

    /*!
      Works exactly like QDropbox::requestMetadata() but blocks until the metadata
      was received from the Dropbox server and returns an instance of QDropboxFileInfo
      that contains the metadata of the requested file.

      \param file The absoulte path of the file (e.g. <i>/dropbox/test.txt</i>)
     */
    QDropboxFileInfo requestMetadataAndWait(QString file);

    /*!
     * \brief Creates and returns a Dropbox link to files or folders users can use to view a preview of the file in a web browser.
     * \param path from the file i.e. /dropbox/hello.txt
     * \param blocking
     */
    void requestSharedLink(QString file, bool blocking = false);

    /*!
    * \brief Works exactly like QDropbox::requestSharedLink() but blocks until link
    * was receivied from the Dropbox Server.
    * \param path from the file i.e. /dropbox/hello.txt
    * \return Url to the file
    */
    QUrl requestSharedLinkAndWait(QString file);
    
    /*!
      Resets the last error. Use this when you reacted on an error to delete the error flag.
    */
    void clearError();

	/*!
	  Requests the latest revisions of a file. When the request is answered by the Dropbox server
	  the signal QDropbox::revisionsReceived() will be emitted.

	  \param file The absoulte path of the file (e.g. <i>/dropbox/test.txt</i>)
	  \param max Defines the maximum amount of revisions to be requested.
	  \param blocking <i>internal only</i> indidicates if the call should block
	 */
	void requestRevisions(QString file, int max = 10, bool blocking = false);

	/*!
	  Works exactly like QDropbox::requestRevisions but blocks until the list of revisisions was
	  received.

	  \param file The absoulte path of the file (e.g. <i>/dropbox/test.txt</i>)
	  \param max Defines the maximum amount of revisions to be requested.
	 */
	QList<QDropboxFileInfo> requestRevisionsAndWait(QString file, int max = 10);


    /*!
      \brief Produces a list of delta entries. When the request is answered by the Dropbox server
      the signal QDropbox::deltaEntriesReceived() will be emitted.

      \param cursor A string used to keep track of current delta state.
      \param path_prefix If non-empty, only include entries with given prefix.

     */
    void requestDelta(QString cursor, QString path_prefix, bool blocking = false);

    /*!
      \brief Works exactly like QDropbox::requestDelta but blocks until the list of delta
      entries was received.

      \param cursor A string used to keep track of current delta state.
      \param path_prefix If non-empty, only includes entries with given prefix.

      \return a QDropboxDeltaResponse representing the API response.

     */
     QDropboxDeltaResponse requestDeltaAndWait(QString cursor, QString path_prefix);

	 /*!
	   \brief Provides information about a request.

	   This function can be used if you wish to obtain further information regarding a request.
	   It provides technical information for requests so it is mostly about debugging information.

	   Requesting information about a request number that does not exist will return invalid information.

	   Requesting information on a request that has been finished already will return an invalid record.

	   \param rqnr number of the request
	 */
	 qdropbox_request requestInfo(int rqnr);

	 /*!
		\brief For debugging: Save finished requests so information can be requested on them.

		This function is for debugging errors. When the setting is changed to true records of already
		finished requests to Dropbox will be saved. Usually they are deleted as soon as they are
		processed. Saving them will allow you to use requestInfo(...) on already finished requests.

		Activating this setting may have an impact about long-time performance and used memory.

		Old records will not be deleted when the setting is turned off!

		\param save set to true if you want to persist request information
	 */
	 void setSaveFinishedRequests(bool save);

	 /*!
		\brief Indicates if information about finished requests is to be persisted.
	 */
	 bool saveFinishedRequests();

signals:
    /*!
      This signal is emitted whenever an error occurs. The error is passed
      as parameter to the slot. To retrieve descriptive information about
      the error use errorString().

      \param errorcode The occured error.
     */
    void errorOccured(QDropbox::Error errorcode);
    /*!
      Emitted when the used token is expired. Reauthorize the user connection
      by prompting the URL provided by authorizeUrl() to your user to reauthorize.
     */
    void tokenExpired();
    /*!
      Should never be emitted by QDropbox as there is no functionality that accesses
      files in QDropbox but all implemented in QDropboxFile.
     */
    void fileNotFound();

    /*!
      QDropbox uses an operation based asynchronous interface for reacting to messages.
      This signal is emitted whenever a request to the Dropbox API is finished.

      \param requestnr Number of the finished request.
     */
    void operationFinished(int requestnr);

	/*!
	  When an asynchronous operation (actually any operation) that requests or transfers
	  information from or to Dropbox is started this signal is emitted. The passed
	  request number can be used to link operations with the operationFinished(...) signal.

	  \param requestnr number of the started request.
	*/
	void operationStarted(int requestnr);

    /*!
      This signal is emitted when the function requestToken() is finished and a
      token and token scret (valid for authorization only) is received.

      \param token Temporary token
      \param secret Temporary token secret
     */
    void requestTokenFinished(QString token, QString secret);
    /*!
      This signal is emitted when the function requestAccessToken() is finished and
      a valid and authorized token used for the connection was received.

      \param token Token used for the connection to Dropbox
      \param secret Secret used for the connection to Dropbox
     */
    void accessTokenFinished(QString token, QString secret);
    /*!
      Emitted whenever the token changes.

      \param token New token.
      \param secret New secret.
     */
    void tokenChanged(QString token, QString secret);

    /*!
      Emitted when account information was received. Only relevant for non-blocking
      use of accountInfo().

      \param accountJson JSON that contains the account information data.
     */
    void accountInfoReceived(QString accountJson);

    /*!
      Emitted when metadata information about a file or directory was received. This will
      only be relevant for non-blocking use of metadata(...);

      \param metadataJson JSON string that contains the metadata information
    */
    void metadataReceived(QString metadataJson);

    /*!
    Emmited when shared link was received. Only relevant for non-blocking use of sharedLink()
    \param sharedLinkJson string than contains the share link information.
    */
    void sharedLinkReceived(QString sharedLink);

	/*!
	  Emitted when revisions of a file were received. Only relevant for non-blocking use
	  of requestRevisions().
	*/
	void revisionsReceived(QString revisionJson);

    /*!
      Emitted when a delta response is received.
    */
    void deltaReceived(QString deltaJson);

public slots:

private slots:
    void requestFinished(int nr, QNetworkReply* rply);
    void networkReplyFinished(QNetworkReply* rply);

private:
    enum {
        SHA1_DIGEST_LENGTH      = 20,
        SHA1_BLOCK_SIZE         = 64,
        HMAC_BUF_LEN            = 4096
    } ;

    QNetworkAccessManager conManager;

    Error   errorState;
    QString errorText;

    QString _appKey;
    QString _appSharedSecret;

    QUrl        apiurl;
    QString     nonce;
    long        timestamp;
    OAuthMethod oauthMethod;
    QString     _version;

    QString oauthToken;
    QString oauthTokenSecret;

    QMap <QNetworkReply*,int>  replynrMap;
    int  lastreply;
    QMap<int,qdropbox_request> requestMap;
    QMap<int,int> delayMap;

    QString mail;
    QString password;

    // for blocked functions
    QEventLoop *_evLoop;
    void startEventLoop();
    void stopEventLoop();

    // temporary memory
    QDropboxJson _tempJson;

    QDropboxAccount _account;

	// indicates wether finished request shall be saved for debugging
	// mind the possible performance impact!
	bool _saveFinishedRequests;

    QString hmacsha1(QString key, QString baseString);
    void prepareApiUrl();
    int  sendRequest(QUrl request, QString type = "GET", QByteArray postdata = 0, QString host = "");
    void responseTokenRequest(QString response);
    void responseBlockedTokenRequest(QString response);
    int  responseDropboxLogin(QString response, int reqnr);
    void responseAccessToken(QString response);
    void responseBlockingAccessToken(QString response);
    void parseToken(QString response);
    void parseAccountInfo(QString response);
    void parseSharedLink(QString response);
    void checkReleaseEventLoop(int reqnr);
    void parseMetadata(QString response);
    void parseBlockingAccountInfo(QString response);
    void parseBlockingMetadata(QString response);
    void parseBlockingSharedLink(QString response);
	void parseRevisions(QString response);
	void parseBlockingRevisions(QString response);
    void parseDelta(QString response);
    void parseBlockingDelta(QString response);
	void removeRequestFromMap(int rqnr);
};

#endif // QDROPBOX_H
