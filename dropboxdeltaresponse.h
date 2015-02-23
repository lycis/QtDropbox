#ifndef QDROPBOXDELTARESPONSE_H
#define QDROPBOXDELTARESPONSE_H

#include <QObject>
#include <QIODevice>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QEvent>

#include "dropbox_global.h"
#include "dropboxjson.h"
#include "dropboxfileinfo.h"

//! Type for a mapping from file paths to file metadata info.
typedef QMap<QString, QSharedPointer<DropboxFileInfo> > QDropboxDeltaEntryMap;

//! Response from a /delta call.
/*!
  This structure is used to carry the (multi-part) response from a call to the delta API.
 */
class DropboxDeltaResponse
{
public:
    //! Constructs a blank QDropboxDeltaResponse object.
    DropboxDeltaResponse();

    //! Constructs a QDropboxDeltaResponse object from a JSON response.
    DropboxDeltaResponse(QString response);

    //! Retrieves the string-to-metadata map.
    /*!
      This is a mapping from file paths to metadata (QDropboxFileInfo) entries.

      \note The values in the mapping are allowed to be 'null' QSharedPointer objects,
            which represent entries that should be deleted from the local state tracking.
     */
    const QDropboxDeltaEntryMap getEntries() const;

    //! Returns whether the local state tracking mechanism should clear its current state.
    bool shouldReset() const;

    //! Returns the cursor that should be passed to the next delta API call.
    QString getNextCursor() const;

    //! Returns whether or not a subsequent delta API call is part of the same response.
    /*!
      \return if true: make a delta API call with the same cursor and treat it as
              part of the same response;
              if false: wait some time (e.g. 5 minutes) before making another delta call.
      */
    bool hasMore() const;


private:
    QDropboxDeltaEntryMap   _entries;
    bool                    _reset;
    QString                 _cursor;
    bool                    _hasMore;
};

#endif // QDROPBOXDELTA_H
