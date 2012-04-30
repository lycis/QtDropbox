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
class QTDROPBOXSHARED_EXPORT QDropboxAccount : public QObject
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
      Creates an instance with data based on the data of the given JSON.

      \param json Pointer to the QDropboxJson that contains the data.
      \param parent Parent QObject.
     */
    QDropboxAccount(QDropboxJson *json, QObject *parent = 0);

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
    QDropboxAccount(QDropboxAccount& other);

    /*!
      Interprets the data contained in the QDropboxJson and replaces the
      currently held data of the object by those from the JSON. This may
      invalidate QDropboxAccount if any error occurs during this process.

      \param json Pointer to the QDropboxJson containing the data.
     */
    void setJson(QDropboxJson *json);

    /*!
      Use this function to verify if the instance of QDropboxAccount contains
      valid data. If it returns false the data are not complete or even wrong.
     */
    bool isValid();

    /*!
      Returns the referal link of the user.
     */
    QUrl    referralLink();

    /*!
      Returns the display name of the account.
     */
    QString displayName();

    /*!
      Returns the Dropbox UID of the account.
     */
    qint64  uid();

    /*!
      Returns the country the account is associated to.
     */
    QString country();

    /*!
      Returns the E-Mail address the owner of the account uses.
     */
    QString email();

    /*!
      Returns the user's used quota in shared folders in bytes.
     */
    quint64  quotaShared();

    /*!
      Returns the user's total quota of allocated bytes.
     */
    quint64  quota();

    /*!
      Returns the user's quota outside of shared folders in bytes.
     */
    quint64  quotaNormal();

    /*!
      Overloaded operator to copy a QDropboxAccount by using =. Internally
      copyFrom() is called.
     */
    QDropboxAccount& operator =(QDropboxAccount&);

    /*!
      This function is used to copy the data from an other QDropboxAccount.
     */
    void copyFrom(QDropboxAccount& a);

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
