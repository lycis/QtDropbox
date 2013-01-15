#ifndef QDROPBOXACCOUNT_H
#define QDROPBOXACCOUNT_H

#include <QObject>
#include <QUrl>
#include "qdropboxjson.h"

//! Stores information about a user account
/*!
  This class is used to store user account information retrieved by using
  QDropbox::accountInfo(). The stored data directly correspond to the
  Dropbox API request account_info.

  QDropboxAccount interprets given data based on a QDropboxJson. If the data
  could be interpreted and hence is valid the resulting object will be valid.
  If any error occurs while interpreting the data the resultung QDropboxAccount
  object will be invalid. This can checked by using isValid().

  See https://www.dropbox.com/developers/reference/api#account-info for details.

 */
class QTDROPBOXSHARED_EXPORT QDropboxAccount : public QDropboxJson
{
    Q_OBJECT
public:
    /*!
      Creates an empty instance of the object. It is automatically invalid
      and does not contain useful data.

      \param parent Parent QObject.
     */
    QDropboxAccount(QObject *parent = 0);

    /*!
      This constructor creates an object based on the data contained in the
      given string that is in valid JSON format.

      \param jsonString JSON data in string representation
      \param parent Parent QObject.
     */
    QDropboxAccount(QString jsonString, QObject *parent = 0);

    /*!
      Use this constructor to create a copy of an other QDropboxAccount.

      \param other Original QDropboxAccount
     */
    QDropboxAccount(const QDropboxAccount& other);

    /*!
      Returns the referal link of the user.
     */
    QUrl    referralLink()  const;

    /*!
      Returns the display name of the account.
     */
    QString displayName()  const;

    /*!
      Returns the Dropbox UID of the account.
     */
    qint64  uid()  const;

    /*!
      Returns the country the account is associated to.
     */
    QString country()  const;

    /*!
      Returns the E-Mail address the owner of the account uses.
     */
    QString email()  const;

    /*!
      Returns the user's used quota in shared folders in bytes.
     */
    quint64  quotaShared()  const;

    /*!
      Returns the user's total quota of allocated bytes.
     */
    quint64  quota()  const;

    /*!
      Returns the user's quota outside of shared folders in bytes.
     */
    quint64  quotaNormal()  const;

    /*!
      Overloaded operator to copy a QDropboxAccount by using =. Internally
      copyFrom() is called.
     */
    QDropboxAccount& operator =(QDropboxAccount&);

    /*!
      This function is used to copy the data from an other QDropboxAccount.
     */
    void copyFrom(const QDropboxAccount& a);

private:  
    QUrl    _referralLink;
    QString _displayName;
    quint64 _uid;
    QString _country;
    QString _email;
    quint64 _quotaShared;
    quint64 _quota;
    quint64 _quotaNormal;

	void _init();
};

#endif // QDROPBOXACCOUNT_H
