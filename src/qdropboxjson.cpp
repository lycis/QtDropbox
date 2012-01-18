#include "qdropboxjson.h"

QDropboxJson::QDropboxJson(QObject *parent) :
    QObject(parent)
{
    valid = false;
}

QDropboxJson::QDropboxJson(QString strJson, QObject *parent) :
    QObject(parent)
{
    valid = false;
    parseString(strJson);
}

QDropboxJson::~QDropboxJson()
{
    emptyList();
}

void QDropboxJson::parseString(QString strJson)
{
#ifdef QTDROPBOX_DEBUG
    qDebug() << "parse string = " << strJson << endl;
#endif

    // clear all existing data
    emptyList();

    // basically a json is valid until it is invalidated
    valid = true;

    if(!strJson.startsWith("{") ||
            !strJson.endsWith("}"))
    {
#ifdef QTDROPBOX_DEBUG
    qDebug() << "string does not start with { " << endl;
#endif
        valid = false;
        return;
    }

    QString buffer   = "";
    QString key      = "";
    QString value    = "";

    bool isKey       = true;
    bool insertValue = false;
    bool isJson      = false;
    bool isArray     = false;

    QDropboxJson *jsonValue;

    for(int i=0; i<strJson.size(); ++i)
    {
        switch(strJson.at(i).toAscii())
        {
        case '"':
            if(!isKey)
                buffer += "\"";
            continue;
            break;
        case ' ':
            if(!isKey)
                buffer += " ";
            continue;
            break;
        case '}':
            continue;
            break;
        case ':':
            if(!isKey)
            {
                buffer += ":";
                continue;
            }
#ifdef QTDROPBOX_DEBUG
            qDebug() << "key = " << buffer << endl;
#endif

            key    = buffer.trimmed();
            buffer = "";
            isKey  = false;
            break;
        case ',':
#ifdef QTDROPBOX_DEBUG
            qDebug() << "value = " << buffer << endl;
#endif
            value       = buffer.trimmed();
            buffer      = "";
            isKey       = true;
            insertValue = true;
            break;
        case '{':
            if(i == 0)
                continue;
            isJson  = true;
            buffer += '{';
            break;
        case '[':
            isArray = true;
            buffer += '[';
            break;
        default:
            buffer += strJson.at(i);
            break;
        }

        if(isJson)
        {
            // create new json object with data until }
            int openBrackets = 1;
            buffer = "";

            int j;
            for(j=i+1; openBrackets > 0 && j < strJson.size(); ++j)
            {
                if(strJson.at(j).toAscii() == '{')
                    openBrackets++;
                else if(strJson.at(j).toAscii() == '}')
                    openBrackets--;
            }

            buffer = strJson.mid(i, j-i);
#ifdef QTDROPBOX_DEBUG
            qDebug() << "brackets = " << openBrackets << endl;
            qDebug() << "json data(" << i << ":" << j-i << ") = " << buffer << endl;
#endif
            jsonValue = new QDropboxJson;
            jsonValue->parseString(buffer);

            // invalid sub json means invalid json
            if(!jsonValue->isValid())
            {
#ifdef QTDROPBOX_DEBUG
                qDebug() << "subjson invalid!" << endl;
                #endif
                valid = false;
                return;
            }

            if(isArray)
            {
                // @TODO
            }

#ifdef QTDROPBOX_DEBUG
            qDebug() << "subjson " << key << " = " << jsonValue << endl;
#endif
            // insert new
            qdropboxjson_entry e;
            e.value.json = jsonValue;
            e.type       = QDROPBOXJSON_TYPE_JSON;
            valueMap[key] = e;

            key = "";
            jsonValue = 0;

            qDebug() << "jsonValue = " << e.value.json << endl;

            // ignore next ,
            i = j+1;
            buffer      = "";
            isJson      = false;
            insertValue = false;
            isKey       = true;
            //continue;
        }

        if(insertValue)
        {
#ifdef QTDROPBOX_DEBUG
            qDebug() << "insert value " << key << endl;
#endif
            QString *valuePointer = new QString(value.trimmed());
            qdropboxjson_entry e;
            e.value.value = valuePointer;
            e.type        = interpretType(value.trimmed());
            valueMap[key] = e;

            key   = "";
            value = "";
            insertValue = false;
        }
    }

    // there's some key left
    if(key.compare(""))
    {
#ifdef QTDROPBOX_DEBUG
            qDebug() << "rest value = " << buffer << endl;
#endif
        // but no value in the buffer -> json is invalid because a value is missing
        if(!buffer.compare(""))
        {
            valid = false;
        }
        else
        {
            // if there's a value left we store it
            qdropboxjson_entry e;
            e.value.value = new QString(buffer.trimmed());
            e.type        = interpretType(buffer.trimmed());
            valueMap[key] = e;
        }

    }

    return;
}

void QDropboxJson::clear()
{
    emptyList();
    return;
}

bool QDropboxJson::isValid()
{
    return valid;
}

bool QDropboxJson::hasKey(QString key)
{
    return valueMap.contains(key);
}

QDropboxJson::DataType QDropboxJson::type(QString key)
{
    if(!valueMap.contains(key))
        return UnknownType;

    qdropboxjson_entry_type t;
    t = valueMap.value(key).type;

    switch(t)
    {
    case QDROPBOXJSON_TYPE_NUM:
        return NumberType;
    case QDROPBOXJSON_TYPE_STR:
        return StringType;
    case QDROPBOXJSON_TYPE_JSON:
        return JsonType;
    case QDROPBOXJSON_TYPE_ARRAY:
        return ArrayType;
    case QDROPBOXJSON_TYPE_FLOAT:
        return FloatType;
    case QDROPBOXJSON_TYPE_BOOL:
        return BoolType;
    case QDROPBOXJSON_TYPE_UINT:
        return UnsignedIntType;
    default:
        return UnknownType;
    }
    return UnknownType;
}

int QDropboxJson::getInt(QString key, bool force)
{
    if(!valueMap.contains(key))
        return 0;

    qdropboxjson_entry e;
    e = valueMap.value(key);

    if(!force && e.type != QDROPBOXJSON_TYPE_NUM)
        return 0;

    return e.value.value->toInt();
}

quint32 QDropboxJson::getUInt(QString key, bool force)
{
    if(!valueMap.contains(key))
        return 0;

    qdropboxjson_entry e;
    e = valueMap.value(key);

    if(!force && e.type != QDROPBOXJSON_TYPE_UINT)
        return 0;

    return e.value.value->toUInt();
}

QString QDropboxJson::getString(QString key, bool force)
{
    if(!valueMap.contains(key))
        return "";

    qdropboxjson_entry e;
    e = valueMap.value(key);

    if(!force && e.type != QDROPBOXJSON_TYPE_STR)
        return "";

    return e.value.value->mid(1, e.value.value->size()-2);
}

QDropboxJson* QDropboxJson::getJson(QString key)
{
    if(!valueMap.contains(key))
        return 0;

    qdropboxjson_entry e;
    e = valueMap.value(key);

    if(e.type != QDROPBOXJSON_TYPE_JSON)
        return 0;


    return e.value.json;
}

double QDropboxJson::getDouble(QString key, bool force)
{
    if(!valueMap.contains(key))
        return 0.0f;

    qdropboxjson_entry e;
    e = valueMap.value(key);

    if(!force && e.type != QDROPBOXJSON_TYPE_FLOAT)
        return 0.0f;

    return e.value.value->toDouble();
}

bool QDropboxJson::getBool(QString key, bool force)
{
    if(!valueMap.contains(key))
        return false;

    qdropboxjson_entry e;
    e = valueMap.value(key);

    if(!force && e.type != QDROPBOXJSON_TYPE_BOOL)
        return false;

    if(!e.value.value->compare("false"))
        return false;

    return true;
}

void QDropboxJson::emptyList()
{
    QList<QString> keys = valueMap.keys();
    for(qint32 i; i<keys.size(); ++i)
    {
        qdropboxjson_entry e = valueMap.value(keys.at(i));
        if(e.type == QDROPBOXJSON_TYPE_JSON)
            delete e.value.json;
        else
            delete e.value.value;
    }
    valueMap.empty();
    return;
}

qdropboxjson_entry_type QDropboxJson::interpretType(QString value)
{
    // check for string
    if(value.startsWith("\"") && value.endsWith("\""))
        return QDROPBOXJSON_TYPE_STR;

    // check for integer
    bool ok;
    value.toInt(&ok);
    if(ok)
        return QDROPBOXJSON_TYPE_NUM;

    // check for uint
    value.toUInt(&ok);
    if(ok)
        return QDROPBOXJSON_TYPE_UINT;

    // check for bool
    if(!value.compare("true") || !value.compare("false"))
        return QDROPBOXJSON_TYPE_BOOL;

    // check for array
    if(value.startsWith("[") && value.endsWith("]"))
        return QDROPBOXJSON_TYPE_ARRAY;

    value.toDouble(&ok);
    if(ok)
        return QDROPBOXJSON_TYPE_FLOAT;

    return QDROPBOXJSON_TYPE_UNKNOWN;
}
