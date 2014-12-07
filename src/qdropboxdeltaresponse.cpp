#include "qdropboxdeltaresponse.h"
#include "qdropboxjson.h"

QDropboxDeltaResponse::QDropboxDeltaResponse()
{

}

QDropboxDeltaResponse::QDropboxDeltaResponse(QString response)
{
    QDropboxJson js(response);

    this->reset = js.getBool("reset");
    this->cursor = js.getString("cursor");
    this->has_more = js.getBool("has_more");

    QStringList entriesList = js.getArray("entries");

    for(QStringList::iterator i = entriesList.begin();
        i != entriesList.end();
        i++)
    {
        QDropboxJson s(*i);
        QStringList pair = js.getArray();

        QSharedPointer<QDropboxFileInfo> val(
                    new QDropboxFileInfo(
                            pair.value(1)
                        )
                    );
        this->entries.insert(pair.value(0), val);
    }
}

const QDropboxDeltaEntryMap QDropboxDeltaResponse::getEntries() const
{
    return this->entries;
}

bool QDropboxDeltaResponse::shouldReset() const
{
    return this->reset;
}

QString QDropboxDeltaResponse::getNextCursor() const
{
    return this->cursor;
}


bool QDropboxDeltaResponse::hasMore() const
{
    return this->has_more;
}
