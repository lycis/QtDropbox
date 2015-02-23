#include "dropboxdeltaresponse.h"
#include "dropboxjson.h"

DropboxDeltaResponse::DropboxDeltaResponse()
{

}

DropboxDeltaResponse::DropboxDeltaResponse(QString response)
{
    DropboxJson js(response);

    this->_reset = js.getBool("reset");
    this->_cursor = js.getString("cursor");
    this->_hasMore = js.getBool("has_more");

    QStringList entriesList = js.getArray("entries");

    for(QStringList::iterator i = entriesList.begin();
        i != entriesList.end();
        i++)
    {
        DropboxJson s(*i);
        QStringList pair = s.getArray();

        QSharedPointer<DropboxFileInfo> val(
                    new DropboxFileInfo(
                            pair.value(1)
                        )
                    );
        this->_entries.insert(pair.value(0), val);
    }
}

const QDropboxDeltaEntryMap DropboxDeltaResponse::getEntries() const
{
    return this->_entries;
}

bool DropboxDeltaResponse::shouldReset() const
{
    return this->_reset;
}

QString DropboxDeltaResponse::getNextCursor() const
{
    return this->_cursor;
}


bool DropboxDeltaResponse::hasMore() const
{
    return this->_hasMore;
}
