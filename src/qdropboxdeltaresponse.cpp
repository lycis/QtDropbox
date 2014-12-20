#include "qdropboxdeltaresponse.h"
#include "qdropboxjson.h"

QDropboxDeltaResponse::QDropboxDeltaResponse()
{

}

QDropboxDeltaResponse::QDropboxDeltaResponse(QString response)
{
    QDropboxJson js(response);

    this->_reset = js.getBool("reset");
    this->_cursor = js.getString("cursor");
    this->_has_more = js.getBool("has_more");

    QStringList entriesList = js.getArray("entries");

    for(QStringList::iterator i = entriesList.begin();
        i != entriesList.end();
        i++)
    {
        QDropboxJson s(*i);
        QStringList pair = s.getArray();

        QSharedPointer<QDropboxFileInfo> val(
                    new QDropboxFileInfo(
                            pair.value(1)
                        )
                    );
        this->_entries.insert(pair.value(0), val);
    }
}

const QDropboxDeltaEntryMap QDropboxDeltaResponse::getEntries() const
{
    return this->_entries;
}

bool QDropboxDeltaResponse::shouldReset() const
{
    return this->_reset;
}

QString QDropboxDeltaResponse::getNextCursor() const
{
    return this->_cursor;
}


bool QDropboxDeltaResponse::hasMore() const
{
    return this->_has_more;
}
