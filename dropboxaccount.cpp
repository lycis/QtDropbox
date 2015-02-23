#include "dropboxaccount.h"

DropboxAccount::DropboxAccount(QObject *parent) :
    DropboxJson(parent)
{
    _quotaShared = 0;
    _quota       = 0;
    _quotaNormal = 0;
    _uid         = 0;
}

DropboxAccount::DropboxAccount(QString jsonString, QObject *parent) :
    DropboxJson(jsonString, parent)
{
	_init();
}

DropboxAccount::DropboxAccount(const DropboxAccount& other) :
    DropboxJson()
{
    copyFrom(other);
}

void DropboxAccount::_init()
{
    if(!isValid())
    {
        _valid = false;
        return;
    }

    if(!hasKey("referral_link") ||
       !hasKey("display_name")  ||
       !hasKey("uid") ||
       !hasKey("country") ||
       !hasKey("quota_info") ||
       !hasKey("email"))
    {
#ifdef QT_DEBUG
        qDebug() << "json invalid 1" << endl;
#endif
        _valid = false;
        return;
    }

    DropboxJson* quota = getJson("quota_info");
    if(!quota->hasKey("shared") ||
       !quota->hasKey("quota") ||
       !quota->hasKey("normal"))
    {
#ifdef QT_DEBUG
        qDebug() << "json invalid 2" << endl;
#endif
        _valid = false;
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

    _valid = true;

#ifdef QT_DEBUG
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

QUrl DropboxAccount::referralLink()  const
{
    return _referralLink;
}

QString DropboxAccount::displayName()  const
{
    return _displayName;
}

qint64 DropboxAccount::uid()  const
{
    return _uid;
}

QString DropboxAccount::country()  const
{
    return _country;
}

QString DropboxAccount::email()  const
{
    return _email;
}

quint64 DropboxAccount::quotaShared()  const
{
    return _quotaShared;
}

quint64 DropboxAccount::quota()  const
{
    return _quota;
}

quint64 DropboxAccount::quotaNormal()  const
{
    return _quotaNormal;
}

DropboxAccount &DropboxAccount::operator =(DropboxAccount &a)
{
    copyFrom(a);
    return *this;
}

void DropboxAccount::copyFrom(const DropboxAccount &other)
{
    this->setParent(other.parent());
#ifdef QT_DEBUG
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
