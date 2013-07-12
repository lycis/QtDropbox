#ifndef QDROPBOXFILEINFO_H
#define QDROPBOXFILEINFO_H

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QList>

#ifdef QTDROPBOX_DEBUG
#include <QDebug>
#endif

#include "qdropboxjson.h"

//! Provides information and metadata about files and directories
/*!
  This class is a more specialised version of QDropboxJson. It provides access to 
  the metadata of a file or directory that is stored on the Dropbox.

  To obtain metadata information about any kind of file stored on the Dropbox you
  have to use QDropbox::metadata() or QDropboxFile::metadata(). Those functions
  return an instance of this class that contains the required information. If an
  error occured while obtaining the metadata the functon isValid() will return
  <i>false</i>.
  
  <b>Traversing the Dropbox file system</b>
  Walking through the filetree on Dropbox is possible by using the isDir() and contents()
  functions. The function contents() provides you with the metadata of all the files and
  directories in a directory. Due to a limitation of the Dropbox REST API these metadata
  do not contain contents of subdirectories. Calling contents() on metadata that you
  retrieved by using a previous contents() call will return an empty list. You have to
  query the metadata of a subdirectory again by using QDropbox::requestMetadata() or 
  QDropbox::requestMetadataAndWait().

  \bug modified() and clientModified() are currently not working due to a bug in 
  QDropboxJson
 */
class QTDROPBOXSHARED_EXPORT QDropboxFileInfo : public QDropboxJson
{
    Q_OBJECT
public:

	/*!
	  Creates an empty instance of QDropboxFileInfo.
	  \warning internal use only
	  \param parent parent QObject
	*/
    QDropboxFileInfo(QObject *parent = 0);

	/*!
	  Creates an instance of QDropboxFileInfo based on the data provided
	  in the JSON in string representation.

	  \param jsonStr metadata JSON in string representation
	  \param parent pointer to the parent QObject
	*/
    QDropboxFileInfo(QString jsonStr, QObject *parent = 0);

	/*!
	   Creates a copy of an other QDropboxFileInfo instance.

	   \param other original instance
	 */
    QDropboxFileInfo(const QDropboxFileInfo &other);

	/*!
	  Default destructor. Takes care of cleaning up when the object is destroyed.
	*/
	~QDropboxFileInfo();
	
	/*!
	  Copies the values from an other QDropboxFileInfo instance to the
	  current instance.

	  \param other original instance
	*/
    void copyFrom(const QDropboxFileInfo &other);

	/*!
	  Works exactly like copyFrom() only as an operator.

	  \param other original instance
	*/
    QDropboxFileInfo& operator=(const QDropboxFileInfo& other);

	/*!
	  Human readable file size.
	*/
    QString   size() const;

	/*!
	  Current revision number.
	 */
    quint64   revision() const;

	/*!
	  Indicates whether a thumbnail is available.
	 */
    bool      thumbExists()  const;

	/*!
	  File size in bytes.
	 */
    quint64   bytes()  const;

	/*!
	  Timestamp of last modification.
	  \bug Currently not working
	 */
    QDateTime modified();

	/*!
	  Timestamp of desktop client upload.
	 */
    QDateTime clientModified();

	/*!
	  Icon name.
	 */
    QString   icon() const;

	/*!
	  Root directors. Can be either <i>/dropbox</i> or <i>/sandbox</i>
	*/
    QString   root() const;

	/*!
	  Full canonical path of the file.
	*/
    QString   path()  const;

	/*!
	  Indicates whether the selected item is a directory.
	*/
    bool      isDir()  const;

	/*!
	  Mime-Type of the item.
	*/
    QString   mimeType()  const;

	/*!
	  Indiciates that the item was deleted from the server.
	*/
    bool      isDeleted()  const;

	/*!
	  Current revision as hash string. Use this for e.g. change check.
	*/
    QString   revisionHash()  const;
	
	/*!
	  Returns the content of a directory.
	  This function will return a list with length 0 (zero) if the item is no
	  directory.
	*/
	QList<QDropboxFileInfo> contents() const;

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
	QList<QDropboxFileInfo>* _content;
};

#endif // QDROPBOXFILEINFO_H
