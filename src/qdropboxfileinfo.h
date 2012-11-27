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
class QTDROPBOXSHARED_EXPORT QDropboxFileInfo : public QObject
{
    Q_OBJECT
public:
    QDropboxFileInfo(QObject *parent = 0);
    QDropboxFileInfo(QDropboxJson* json, QObject *parent = 0);
    QDropboxFileInfo(QDropboxFileInfo &other);

    void copyFrom(QDropboxFileInfo &other);

    QDropboxFileInfo& operator =(QDropboxFileInfo& other);
signals:
    
public slots:

private:
    void dataFromJson(QDropboxJson json);
    void _init();

    QString   _size;
    QString   _revision;
    bool      _thumbExists;
    quint64   _bytes;
    QDateTime _modified;
    QDateTime _clientModified;
    QString   _icon;
    QString   _root;
};

#endif // QDROPBOXFILEINFO_H
