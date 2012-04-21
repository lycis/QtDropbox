#include "qdropboxaccount.h"

QDropboxAccount::QDropboxAccount(QObject *parent) :
    QObject(parent)
{
    valid = false;
}

QDropboxAccount::QDropboxAccount(QDropboxJson *json, QObject *parent) :
    QObject(parent)
{
    valid = false;
    setJson(json);
}

QDropboxAccount::QDropboxAccount(QString jsonString, QObject *parent) :
    QObject(parent)
{
    valid = false;
    QDropboxJson json(jsonString);
    setJson(&json);
}

QDropboxAccount::QDropboxAccount(QDropboxAccount& other) :
    QObject(other.parent())
{
    qDebug() << "creating account from account" << endl;
    qDebug() << "taken reflink: " << other.referralLink().toString() << endl;
    _referralLink = other.referralLink();
    _displayName  = other.displayName();
    _uid          = other.uid();
    _country      = other.country();
    _email        = other.email();
    _quotaShared  = other.quotaShared();
    _quota    = other.quota();
    _quotaNormal  = other.quotaNormal();
}

void QDropboxAccount::setJson(QDropboxJson *json)
{
    if(!json->isValid())
    {
        valid = false;
        return;
    }

    if(!json->hasKey("referral_link") ||
       !json->hasKey("display_name")  ||
       !json->hasKey("uid") ||
       !json->hasKey("country") ||
       !json->hasKey("quota_info") ||
       !json->hasKey("email"))
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "json invalid 1" << endl;
#endif
        valid = false;
        return;
    }

    QDropboxJson* quota = json->getJson("quota_info");
    if(!quota->hasKey("shared") ||
       !quota->hasKey("quota") ||
       !quota->hasKey("normal"))
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "json invalid 2" << endl;
#endif
        valid = false;
        return;
    }

    _referralLink.setUrl(json->getString("referral_link"), QUrl::StrictMode);
    _displayName = json->getString("display_name");
    _uid         = json->getInt("uid");
    _country     = json->getString("country");
    _email       = json->getString("email");

    _quotaShared = quota->getUInt("shared", true);
    _quota       = quota->getUInt("quota", true);
    _quotaNormal = quota->getUInt("normal", true);

    valid = true;

#ifdef QTDROPBOX_DEBUG
    qDebug() << "== account data ==" << endl;
    qDebug() << "reflink: " << _referralLink << endl;
    qDebug() << "displayname: " << _displayName << endl;
    qDebug() << "uid: " << _uid << endl;
    qDebug() << "country: " << _country << endl;
    qDebug() << "email: " << _email << endl;
    qDebug() << "quotaShared: " << _quotaShared << endl;
    qDebug() << "quotaNormal: " << _quotaNormal << endl;
    qDebug() << "quotaUsed: " << _quota << endl;
    qDebug() << "== account data end ==" << endl;
#endif
    return;
}

bool QDropboxAccount::isValid()
{
    return valid;
}

QUrl QDropboxAccount::referralLink()
{
    return _referralLink;
}

QString QDropboxAccount::displayName()
{
    return _displayName;
}

qint64 QDropboxAccount::uid()
{
    return _uid;
}

QString QDropboxAccount::country()
{
    return _country;
}

QString QDropboxAccount::email()
{
    return _email;
}

quint64 QDropboxAccount::quotaShared()
{
    return _quotaShared;
}

quint64 QDropboxAccount::quota()
{
    return _quota;
}

quint64 QDropboxAccount::quotaNormal()
{
    return _quotaNormal;
}
