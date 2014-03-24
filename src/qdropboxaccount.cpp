#include "qdropboxaccount.h"

QDropboxAccount::QDropboxAccount(QObject *parent) :
    QDropboxJson(parent)
{
    _quotaShared = 0;
    _quota       = 0;
    _quotaNormal = 0;
    _uid         = 0;
}

QDropboxAccount::QDropboxAccount(QString jsonString, QObject *parent) :
    QDropboxJson(jsonString, parent)
{
	_init();
}

QDropboxAccount::QDropboxAccount(const QDropboxAccount& other) :
    QDropboxJson()
{
    copyFrom(other);
}

void QDropboxAccount::_init()
{
    if(!isValid())
    {
        valid = false;
        return;
    }

    if(!hasKey("referral_link") ||
       !hasKey("display_name")  ||
       !hasKey("uid") ||
       !hasKey("country") ||
       !hasKey("quota_info") ||
       !hasKey("email"))
    {
#ifdef QTDROPBOX_DEBUG
        qDebug() << "json invalid 1" << endl;
#endif
        valid = false;
        return;
    }

    QDropboxJson* quota = getJson("quota_info");
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

    _referralLink.setUrl(getString("referral_link"), QUrl::StrictMode);
    _displayName = getString("display_name");
    _uid         = getInt("uid");
    _country     = getString("country");
    _email       = getString("email");

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

QUrl QDropboxAccount::referralLink()  const
{
    return _referralLink;
}

QString QDropboxAccount::displayName()  const
{
    return _displayName;
}

qint64 QDropboxAccount::uid()  const
{
    return _uid;
}

QString QDropboxAccount::country()  const
{
    return _country;
}

QString QDropboxAccount::email()  const
{
    return _email;
}

quint64 QDropboxAccount::quotaShared()  const
{
    return _quotaShared;
}

quint64 QDropboxAccount::quota()  const
{
    return _quota;
}

quint64 QDropboxAccount::quotaNormal()  const
{
    return _quotaNormal;
}

QDropboxAccount &QDropboxAccount::operator =(QDropboxAccount &a)
{
    copyFrom(a);
    return *this;
}

void QDropboxAccount::copyFrom(const QDropboxAccount &other)
{
    this->setParent(other.parent());
#ifdef QTDROPBOX_DEBUG
    qDebug() << "creating account from account" << endl;
    qDebug() << "taken reflink: " << other.referralLink().toString() << endl;
#endif
    _referralLink = other.referralLink();
    _displayName  = other.displayName();
    _uid          = other.uid();
    _country      = other.country();
    _email        = other.email();
    _quotaShared  = other.quotaShared();
    _quota    = other.quota();
    _quotaNormal  = other.quotaNormal();
}
