#ifndef QDROPBOXJSON_H
#define QDROPBOXJSON_H

#include "qtdropbox_global.h"

#include <QObject>
#include <QMap>
#include <QList>

#ifdef QTDROPBOX_DEBUG
#include <QDebug>
#endif

typedef char qdropboxjson_entry_type;

const qdropboxjson_entry_type QDROPBOXJSON_TYPE_NUM     = 'N';
const qdropboxjson_entry_type QDROPBOXJSON_TYPE_STR     = 'S';
const qdropboxjson_entry_type QDROPBOXJSON_TYPE_JSON    = 'J';
const qdropboxjson_entry_type QDROPBOXJSON_TYPE_ARRAY   = 'A';
const qdropboxjson_entry_type QDROPBOXJSON_TYPE_FLOAT   = 'F';
const qdropboxjson_entry_type QDROPBOXJSON_TYPE_BOOL    = 'B';
const qdropboxjson_entry_type QDROPBOXJSON_TYPE_UINT    = 'U';
const qdropboxjson_entry_type QDROPBOXJSON_TYPE_UNKNOWN = '?';

class QDropboxJson;

union qdropboxjson_value{
    QDropboxJson  *json;
    QString       *value;
};

struct qdropboxjson_entry{
    qdropboxjson_entry_type type;
    qdropboxjson_value      value;
};

class QTDROPBOXSHARED_EXPORT QDropboxJson : public QObject
{
    Q_OBJECT
public:
    QDropboxJson(QObject *parent = 0);
    QDropboxJson(QString strJson, QObject *parent = 0);
    QDropboxJson(QDropboxJson &other);
    ~QDropboxJson();

    enum DataType{
        NumberType,
        StringType,
        JsonType,
        ArrayType,
        FloatType,
        BoolType,
        UnsignedIntType,
        UnknownType
    };

    void parseString(QString strJson);
    void clear();
    bool isValid();

    bool     hasKey(QString key);
    DataType type(QString key);

    int           getInt(QString key, bool force = false);
    quint32       getUInt(QString key, bool force = false);
    QString       getString(QString key, bool force = false);
    QDropboxJson *getJson(QString key);
    double        getDouble(QString key, bool force = false);
    bool          getBool(QString key, bool force = false);

    QString strContent();
    
private:
    QMap<QString, qdropboxjson_entry> valueMap;
    bool                      valid;

    void emptyList();
    qdropboxjson_entry_type interpretType(QString value);
    QString _strContent;
};

#endif // QDROPBOXJSON_H
