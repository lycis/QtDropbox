#ifndef QDROPBOXFILE_H
#define QDROPBOXFILE_H

#include <QObject>
#include <QIODevice>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QEvent>

#include "qtdropbox_global.h"
#include "qdropboxjson.h"
#include "qdropbox.h"

const QString QDROPBOXFILE_CONTENT_URL = "https://api-content.dropbox.com";

class QTDROPBOXSHARED_EXPORT QDropboxFile : public QIODevice
{
    Q_OBJECT
public:
    QDropboxFile(QObject* parent = 0);
    QDropboxFile(QDropbox* api, QObject* parent = 0);
    QDropboxFile(QString filename, QDropbox* api, QObject* parent = 0);
    ~QDropboxFile();

    bool isSequential();
    bool open(OpenMode mode);
    void close();

    void setApi(QDropbox* dropbox);
    QDropbox* api();

    void setFilename(QString filename);
    QString filename();

    bool flush();
    bool event(QEvent* event);

    void setFlushThreshold(qint64 num);
    qint64 flushThreshold();

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private slots:
    void networkRequestFinished(QNetworkReply* rply);

private:
    QNetworkAccessManager _conManager;

    QByteArray *_buffer;

    QString _token;
    QString _tokenSecret;
    QString _filename;

    QDropbox *_api;


    enum WaitState{
        notWaiting,
        waitForRead,
        waitForWrite
    };

    WaitState _waitMode;

    QEventLoop* _evLoop;

    int     lastErrorCode;
    QString lastErrorMessage;

    qint64 _bufferThreshold;

    void obtainToken();
    void connectSignals();

    bool isMode(QIODevice::OpenMode mode);
    bool getFileContent(QString filename);
    void rplyFileContent(QNetworkReply* rply);
    void startEventLoop();
    void stopEventLoop();

    void _init(QDropbox *api, QString filename, qint64 bufferTh);
};

#endif // QDROPBOXFILE_H
