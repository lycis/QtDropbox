#ifndef QDROPBOXFILEINFO_H
#define QDROPBOXFILEINFO_H

#include <QObject>
#include <QDateTime>
#include <QString>
#include "qdropboxjson.h"

//! Provides information and metadata about files and directories
/*!
  \todo implement!
 */
class QTDROPBOXSHARED_EXPORT QDropboxFileInfo : public QDropboxJson
{
    Q_OBJECT
public:
    QDropboxFileInfo(QObject *parent = 0);
    QDropboxFileInfo(QString jsonStr, QObject *parent = 0);
    QDropboxFileInfo(QDropboxFileInfo &other);

    void copyFrom(QDropboxFileInfo &other);

    QDropboxFileInfo& operator =(QDropboxFileInfo& other);

	QString   size();
	quint64   revision();
	bool      thumbExists();
	quint64   bytes();
	QDateTime modified();
	QDateTime clientModified();
	QString   icon();
	QString   root();
	QString   path();
	bool      isDir();
	QString   mimeType();
	bool      isDeleted();
	QString   revisionHash();

signals:
    
public slots:

private:
    void dataFromJson();
    void _init();

    QString   _size;
    quint64   _revision;
    bool      _thumbExists;
    quint64   _bytes;
    QDateTime _modified;
    QDateTime _clientModified;
    QString   _icon;
    QString   _root;
	QString   _path;
	bool      _isDir;
	QString   _mimeType;
	bool      _isDeleted;
	QString   _revisionHash;
};

#endif // QDROPBOXFILEINFO_H
