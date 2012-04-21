#ifndef QDROPBOXACCOUNT_H
#define QDROPBOXACCOUNT_H

#include <QObject>
#include <QUrl>
#include "qdropboxjson.h"

class QTDROPBOXSHARED_EXPORT QDropboxAccount : public QObject
{
    Q_OBJECT
public:
    QDropboxAccount(QObject *parent = 0);
    QDropboxAccount(QDropboxJson *json, QObject *parent = 0);
    QDropboxAccount(QString jsonString, QObject *parent = 0);
    QDropboxAccount(QDropboxAccount& other);

    void setJson(QDropboxJson *json);
    bool isValid();

    QUrl    referralLink();
    QString displayName();
    qint64  uid();
    QString country();
    QString email();
    quint64  quotaShared();
    quint64  quota();
    quint64  quotaNormal();

private:
    bool valid;
    
    QUrl    _referralLink;
    QString _displayName;
    quint64 _uid;
    QString _country;
    QString _email;
    quint64 _quotaShared;
    quint64 _quota;
    quint64 _quotaNormal;
};

#endif // QDROPBOXACCOUNT_H
