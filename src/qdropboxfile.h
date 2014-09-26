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
#include "qdropboxfileinfo.h"

const QString QDROPBOXFILE_CONTENT_URL = "https://api-content.dropbox.com";

//! Allows access to files stored on Dropbox
/*!
  QDropboxFile allows you to access files that are stored on Dropbox. You can
  use this class as any QIODevice, very similar to the default QFile class. It is
  usable in connection with QTextStream and QDataStream to access the file contents.

  When accessing files on Dropbox remember to use valid Dropbox paths. Such a path
  begins with either /dropbox/ or /sandbox/ depending on the access level of your
  application.

  It is important to know that QDropboxFile buffers the content of the remote file
  locally when using open(). This means that the file content is not automatically
  updated if it changed on the Dropbox server which in return means that you may not
  always have the most current version of the file content.

  \todo implement utilities for revision access (get a list of revisions and get actual
        revisions)

 */
class QTDROPBOXSHARED_EXPORT QDropboxFile : public QIODevice
{
    Q_OBJECT
public:
    /*!
      Default constructor. Use setApi() and setFilename() to access Dropbox.

      \param parent Parent QObject
     */
    QDropboxFile(QObject* parent = 0);

    /*!
      Creates an instance of QDropboxFile that may connect to Dropbox if the passed
      QDropbox is already connected. Use setFilename() before you try to access any
      file.

      \param api Pointer to a QDropbox that is connected to an account.
      \param parent Parent QObject
     */
    QDropboxFile(QDropbox* api, QObject* parent = 0);

    /*!
      Creates an instance of QDropboxFile that may access a file on Dropbox.

      \param filename Dropbox path of the file you want to access.
      \param api A QDropbox that is connected to an user account.
      \param parent Parent QObject
     */
    QDropboxFile(QString filename, QDropbox* api, QObject* parent = 0);

    /*!
      This deconstructor cleans up on destruction of the object.
     */
    ~QDropboxFile();

    /*!
      QDropboxFile is currently implemented as sequential device. That will
      change in time.
     */
    bool isSequential() const;

    /*!
      Fetches the file content from the Dropbox server and buffers it locally. Depending
      on the OpenMode read or write access will be granted.

      \param mode The access mode of the file. Equivalent to QIODevice.
     */
    bool open(OpenMode mode);

    /*!
      Closes the file buffer. If the file was opened with QIODevice::WriteOnly (or
      QIODevice::ReadWrite) the file content buffer will be flushed and written to
      the file.
     */
    void close();

    /*!
      Sets the QDropbox instance that is used to access Dropbox.

      \param dropbox Pointer to the QDropbox object
     */
    void setApi(QDropbox* dropbox);

    /*!
      Returns a pointer to the QDropbox instance that is used to connect to Dropbox.
     */
    QDropbox* api();

    /*!
      Set the name of the file you want to access. Remember to use correct Dropbox path
      beginning with either /dropbox/ or /sandbox/.

      \param filename Path of the file.
     */
    void setFilename(QString filename);

    /*!
      Returns the path of the file that is accessed by this instance.
     */
    QString filename();

    /*!
      Writes the content of the buffer to the file (only if the file is opened in
      write mode).
     */
    bool flush();

    /*!
      Reimplemented from QIODEvice.
     */
    bool event(QEvent* event);

    /*!
      Usually the file content is automatically flushed whenever the internal buffer
      has more than 1024 new byte or on using close(). If you want QDropboxFile to
      automatically flush earlier than those 1024 byte use this function to reduce
      this threshold.

      \param num QDropboxFile will automatically flush the file buffer when there are
                 more than num new byte of data.
     */
    void setFlushThreshold(qint64 num);

    /*!
      Returns the current flush threshold setting.
     */
    qint64 flushThreshold();

    /*!
      By default an already existing file will be overwritten. If you don't want to
      let this happen use this function to set the overwrite flag to false. If a file
      with the same name already exists it will be automatically renamed by Dropbox to
      something like "file (1).txt".

      \param overwrite Overwrite flag
     */
    void setOverwrite(bool overwrite);

    /*!
      Returns the current state of the overwrite flag.
     */
    bool overwrite();

	/*!
	  Return the metadata of the file as a QDropboxFileInfo object.
	*/
	QDropboxFileInfo metadata();

	/*!
	  Check if the file has changed on the dropbox while it was opened locally.
	  This function will return false if the file was not previously opened and an error
	  occured during the retrieval of the file metadata. Hence it is safer to open the file
	  first and then check hasChanged()

	  \returns <i>true</i> if the file has changed or <i>false</i> if it has not.
	*/
	bool hasChanged();	

	/*!
	  Gets and returns all available revisions of the file.
	  \param max When defined the function will only list up to the specified amount of revisions.
	  \returns A list of the latest revisions of the file.
	*/
	QList<QDropboxFileInfo> revisions(int max = 10);

	/*!
	  Reimplemented from QIODevice::seek().
	  Foreward to the given (byte) position in the file. Unlike QFile::seek() this function does
	  not seek beyond the file end. When seeking beyond the end of a file this function stops beyond
	  the last byte of the current content and returns <code>false</code>.
	*/
	bool seek(qint64 pos);

	/*!
	  Reimplemented from QIODevice::pos().
	  Returns the current position in the file.
	*/
    qint64 pos() const;

	/*!
	  Reimplemented from QIODevice::reset().
	  Seeks to the beginning of the file. See seek().
	*/
	bool reset();

public slots:
    void abort();

signals:
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void uploadProgress(qint64 bytesReceived, qint64 bytesTotal);

    void operationAborted();

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
    qint64 _currentThreshold;

    bool _overwrite;

	int _position;

	QDropboxFileInfo *_metadata;

    void obtainToken();
    void connectSignals();

    bool isMode(QIODevice::OpenMode mode);
    bool getFileContent(QString filename);
    void rplyFileContent(QNetworkReply* rply);
    void rplyFileWrite(QNetworkReply* rply);
    void startEventLoop();
    void stopEventLoop();
    bool putFile();
	void obtainMetadata();

    void _init(QDropbox *api, QString filename, qint64 bufferTh);
};

#endif // QDROPBOXFILE_H
