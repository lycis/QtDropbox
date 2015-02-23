#include <QLocale>

#include "dropboxjson.h"

DropboxJson::DropboxJson(QObject *parent) :
    QObject(parent)
{
    _init();
}

DropboxJson::DropboxJson(QString strJson, QObject *parent) :
    QObject(parent)
{
    _init();
    parseString(strJson);
}

DropboxJson::DropboxJson(const DropboxJson &other) :
    QObject(other.parent())
{
    _init();
    parseString(other.strContent());
}

DropboxJson::~DropboxJson()
{
    emptyList();
}

void DropboxJson::_init()
{
    _valid          = false;
	_anonymousArray = false;
}

void DropboxJson::parseString(QString strJson)
{
#ifdef QT_DEBUG
    qDebug() << "parse string = " << strJson << endl;
#endif

    // clear all existing data
    emptyList();

    // basically a json is valid until it is invalidated
    _valid = true;

    if(!strJson.startsWith("{") ||
            !strJson.endsWith("}"))
    {
#ifdef QT_DEBUG
    qDebug() << "string does not start with { " << endl;
#endif

		if(strJson.startsWith("[") && strJson.endsWith("]"))
		{
#ifdef QT_DEBUG
			qDebug() << "JSON is anonymous array" << endl;
#endif
			_anonymousArray = true;
			// fix json to be parseable by the algorithm below
			strJson = "{\"_anonArray\":"+strJson+"}";
		}
		else
		{
            _valid = false;
			return;
		}
    }

    QString buffer   = "";
    QString key      = "";
    QString value    = "";

    bool isKey       = true;
    bool insertValue = false;
    bool isJson      = false;
    bool isArray     = false;
    bool openQuotes  = false;


    for(int i=0; i<strJson.size(); ++i)
    {
        switch(strJson.at(i).toLatin1())
        {
        case '"':
            if(!isKey)
            {
                buffer += "\"";
                openQuotes = !openQuotes;
            }
            continue;
            break;
        case ' ':
            if(!isKey)
                buffer += " ";
            continue;
            break;
        case '}':
	    if(openQuotes)
	      buffer += "}";
            continue;
            break;
        case ':':
            if(!isKey)
            {
                buffer += ":";
                continue;
            }
#ifdef QT_DEBUG
            qDebug() << "key = " << buffer << endl;
#endif

            key    = buffer.trimmed();
            buffer = "";
            isKey  = false;
            break;
        case ',':
            if(openQuotes)
			{
				buffer += ',';
                continue;
			}
#ifdef QT_DEBUG
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
	    if(!openQuotes)
              isJson  = true;
            buffer += '{';
            break;
        case '[':
	    if(!openQuotes)
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
            DropboxJsonEntry e;
			int offset = parseSubJson(strJson, i, &e);
            _valueMap[key] = e;
            key = "";

            // ignore next ,
            i = offset+1;
            buffer      = "";
            isJson      = false;
            insertValue = false;
            isKey       = true;
            //continue;
        }

		 if(isArray)
         {
			 // value is an array value -> parse array
			 bool inString    = false;
			 bool arrayEnd    = false;
             int arrayDepth = 0;
			 int j = i+1;
			 buffer = "[";
			 for(;!arrayEnd && j<strJson.size();++j)
			 {
				 QChar arrC = strJson.at(j);
				 switch(arrC.toLatin1())
				 {
				 case '"': 
					 inString = !inString;
					 buffer += arrC;
					 break;
                 case '[':
                     buffer += "[";
                     arrayDepth += 1;
                     break;
				 case ']':
					 buffer += "]";
                     if(!inString)
                     {
                         if(arrayDepth == 0)
                         {
                            arrayEnd = true;
                         }
                         else
                         {
                            arrayDepth -= 1;
                         }
                     }
					 break;
				 case '\\':
					 if(strJson.at(j+1) == '"') // escaped double quote
					 {
						 buffer += "\"";
						 j++;
					 }
					 else
						 buffer += "\\";
					 break;
				 default:
					 buffer += arrC;
					 break;
				 }
			 }
			 
			 // save array value string (buffer)
			 value       = buffer;
			 buffer      = "";
			 i           = j;
			 insertValue = true;
			 isArray     = false;
			 isKey       = true; // next element is started
		 }

        if(insertValue)
        {
#ifdef QT_DEBUG
            qDebug() << "insert value " << key << " with content = " << value.trimmed() << " and type = " << interpretType(value.trimmed()) << endl;
#endif
            DropboxJsonEntry e;
			QString *valuePointer = new QString(value.trimmed());
			e.value.value = valuePointer;
            e.type        = interpretType(value.trimmed());
            _valueMap[key] = e;

            key   = "";
            value = "";
            insertValue = false;
        }
    }

    // there's some key left
    if(key.compare(""))
    {
#ifdef QT_DEBUG
            qDebug() << "rest value = " << buffer << endl;
#endif
        // but no value in the buffer -> json is invalid because a value is missing
        if(!buffer.compare(""))
        {
            _valid = false;
        }
        else
        {
            // if there's a value left we store it
            DropboxJsonEntry e;
            e.value.value = new QString(buffer.trimmed());
            e.type        = interpretType(buffer.trimmed());
            _valueMap[key] = e;
        }

    }

    return;
}

void DropboxJson::clear()
{
    emptyList();
    return;
}

bool DropboxJson::isValid()
{
    return _valid;
}

bool DropboxJson::hasKey(QString key)
{
    return _valueMap.contains(key);
}

DropboxJson::DataType DropboxJson::type(QString key)
{
    if(!_valueMap.contains(key))
        return UnknownType;

    DropboxJsonEntryType t;
    t = _valueMap.value(key).type;

    switch(t)
    {
    case DROPBOXJSON_TYPE_NUM:
        return NumberType;
    case DROPBOXJSON_TYPE_STR:
        return StringType;
    case DROPBOXJSON_TYPE_JSON:
        return JsonType;
    case DROPBOXJSON_TYPE_ARRAY:
        return ArrayType;
    case DROPBOXJSON_TYPE_FLOAT:
        return FloatType;
    case DROPBOXJSON_TYPE_BOOL:
        return BoolType;
    case DROPBOXJSON_TYPE_UINT:
        return UnsignedIntType;
    default:
        return UnknownType;
    }
    return UnknownType;
}

qint64 DropboxJson::getInt(QString key, bool force)
{
    if(!_valueMap.contains(key))
        return 0;

    DropboxJsonEntry e;
    e = _valueMap.value(key);

    if(!force && e.type != DROPBOXJSON_TYPE_NUM)
        return 0;

    return e.value.value->toInt();
}

void DropboxJson::setInt(QString key, qint64 value)
{
    if(_valueMap.contains(key)){
        _valueMap[key].value.value->setNum(value);
    }else{
        DropboxJsonEntry e;
        QString *valuePointer = new QString();
        valuePointer->setNum(value);
        e.value.value = valuePointer;
        e.type        = DROPBOXJSON_TYPE_NUM;
        _valueMap[key] = e;
    }
}

quint64 DropboxJson::getUInt(QString key, bool force)
{
    if(!_valueMap.contains(key))
        return 0;

    DropboxJsonEntry e;
    e = _valueMap.value(key);

    if(!force && e.type != DROPBOXJSON_TYPE_UINT)
        return 0;

    return e.value.value->toUInt();
}

void DropboxJson::setUInt(QString key, quint64 value)
{
    if(_valueMap.contains(key)){
        _valueMap[key].value.value->setNum(value);
    }else{
        DropboxJsonEntry e;
        QString *valuePointer = new QString();
        valuePointer->setNum(value);
        e.value.value = valuePointer;
        e.type        = DROPBOXJSON_TYPE_UINT;
        _valueMap[key] = e;
    }
}

QString DropboxJson::getString(QString key, bool force)
{
    if(!_valueMap.contains(key))
        return "";

    DropboxJsonEntry e;
    e = _valueMap.value(key);

    if(!force && e.type != DROPBOXJSON_TYPE_STR)
        return "";

	QString value = e.value.value->mid(1, e.value.value->size()-2);
    return value;
}

void DropboxJson::setString(QString key, QString value)
{
    if(_valueMap.contains(key)){
        *(_valueMap[key].value.value) = value;
    }else{
        DropboxJsonEntry e;
        QString *valuePointer = new QString(value);
        e.value.value = valuePointer;
        e.type        = DROPBOXJSON_TYPE_STR;
        _valueMap[key] = e;
    }
}

DropboxJson* DropboxJson::getJson(QString key)
{
    if(!_valueMap.contains(key))
        return NULL;

    DropboxJsonEntry e;
    e = _valueMap.value(key);

    if(e.type != DROPBOXJSON_TYPE_JSON)
        return NULL;


    return e.value.json;
}

void DropboxJson::setJson(QString key, DropboxJson value)
{
    if(_valueMap.contains(key)){
        *(_valueMap[key].value.json) = value;
    }else{
        DropboxJsonEntry e;
        DropboxJson *valuePointer = new DropboxJson(value);
        e.value.json = valuePointer;
        e.type        = DROPBOXJSON_TYPE_JSON;
        _valueMap[key] = e;
    }
}

double DropboxJson::getDouble(QString key, bool force)
{
    if(!_valueMap.contains(key))
        return 0.0f;

    DropboxJsonEntry e;
    e = _valueMap.value(key);

    if(!force && e.type != DROPBOXJSON_TYPE_FLOAT)
        return 0.0f;

    return e.value.value->toDouble();
}

void DropboxJson::setDouble(QString key, double value)
{
    if(_valueMap.contains(key)){
        _valueMap[key].value.value->setNum(value);
    }else{
        DropboxJsonEntry e;
        QString *valuePointer = new QString();
        valuePointer->setNum(value);
        e.value.value = valuePointer;
        e.type        = DROPBOXJSON_TYPE_FLOAT;
        _valueMap[key] = e;
    }
}

bool DropboxJson::getBool(QString key, bool force)
{
    if(!_valueMap.contains(key))
        return false;

    DropboxJsonEntry e;
    e = _valueMap.value(key);

    if(!force && e.type != DROPBOXJSON_TYPE_BOOL)
        return false;

    if(!e.value.value->compare("false"))
        return false;

    return true;
}

void DropboxJson::setBool(QString key, bool value)
{
    if(_valueMap.contains(key)){
        *(_valueMap[key].value.value) = value ? "true" : "false";
    }else{
        DropboxJsonEntry e;
        QString *valuePointer = new QString(value ? "true" : "false");
        e.value.value = valuePointer;
        e.type        = DROPBOXJSON_TYPE_BOOL;
        _valueMap[key] = e;
    }
}

QDateTime DropboxJson::getTimestamp(QString key, bool force)
{
    if(!_valueMap.contains(key))
		return QDateTime();

    DropboxJsonEntry e;
    e = _valueMap.value(key);

    if(!force && e.type != DROPBOXJSON_TYPE_STR)
		return QDateTime();

    const QString dtFormat = "dd MMM yyyy HH:mm:ss";

    QDateTime res = QLocale(QLocale::English).toDateTime(e.value.value->mid(6, dtFormat.size()), dtFormat);
    res.setTimeSpec(Qt::UTC);

    return res;
}

void DropboxJson::setTimestamp(QString key, QDateTime value)
{
    const QString dtFormat = "ddd, dd MMM yyyy hh:mm:ss '+0000'";

    value = value.toUTC();

    if(_valueMap.contains(key)){
        *(_valueMap[key].value.value) = value.toString(dtFormat);
    }else{
        DropboxJsonEntry e;
        QString *valuePointer = new QString(QLocale{QLocale::English}.toString(value, dtFormat));
        e.value.value = valuePointer;
        e.value.value = valuePointer;
        e.type        = DROPBOXJSON_TYPE_STR;
        _valueMap[key] = e;
    }
}

QString DropboxJson::strContent() const
{
    if(_valueMap.size() == 0)
		return "";

    QString content = "{";
    QList<QString> keys = _valueMap.keys();
	for(int i=0; i<keys.size(); ++i)
	{
		QString value = "";
        DropboxJsonEntry e = _valueMap.value(keys.at(i));

        if(e.type != DROPBOXJSON_TYPE_JSON)
			value = *e.value.value;
		else
			value = e.value.json->strContent();

		content.append(QString("\"%1\": %2").arg(keys.at(i)).arg(value));
		if(i != keys.size()-1)
			content.append(", ");
	}
	content.append("}");
	return content;
}

void DropboxJson::emptyList()
{
    QList<QString> keys = _valueMap.keys();
    for(qint32 i=0; i<keys.size(); ++i)
    {
        DropboxJsonEntry e = _valueMap.value(keys.at(i));
        if(e.type == DROPBOXJSON_TYPE_JSON)
            delete e.value.json;
        else
            delete e.value.value;
        _valueMap.remove(keys.at(i));
    }
    _valueMap.empty();
    return;
}

DropboxJsonEntryType DropboxJson::interpretType(QString value)
{
    // check for string
    if(value.startsWith("\"") && value.endsWith("\""))
        return DROPBOXJSON_TYPE_STR;

    // check for integer
    bool ok;
    value.toInt(&ok);
    if(ok)
        return DROPBOXJSON_TYPE_NUM;

    // check for uint
    value.toUInt(&ok);
    if(ok)
        return DROPBOXJSON_TYPE_UINT;

    // check for bool
    if(!value.compare("true") || !value.compare("false"))
        return DROPBOXJSON_TYPE_BOOL;

    // check for array
    if(value.startsWith("[") && value.endsWith("]"))
        return DROPBOXJSON_TYPE_ARRAY;

    value.toDouble(&ok);
    if(ok)
        return DROPBOXJSON_TYPE_FLOAT;

    return DROPBOXJSON_TYPE_UNKNOWN;
}

DropboxJson& DropboxJson::operator=(DropboxJson& other)
{
	/*!< \todo use toString() */
	parseString(other.strContent());
	return *this;
}

QStringList DropboxJson::getArray(QString key, bool force)
{
	QStringList list;
    if(!_valueMap.contains(key))
        return list;

    DropboxJsonEntry e;
    e = _valueMap.value(key);

    if(!force && e.type != DROPBOXJSON_TYPE_ARRAY)
        return list;

	QString arrayStr = e.value.value->mid(1, e.value.value->length()-2);
	QString buffer = "";
	bool inString = false;
	int  inJson   = 0;
    int  inArray  = 0;
	for(int i=0; i<arrayStr.size(); ++i)
	{
		QChar c = arrayStr.at(i);
        if( ((c != ',' && c != ' ') || inString || inJson || inArray) &&
            (c != '"' || inJson > 0 || inArray > 0))
			buffer += c;
		switch(c.toLatin1())
		{
		case '"':
			if(i > 0 && arrayStr.at(i-1).toLatin1() == '\\')
			{
				buffer += c;
				break;
			}
			else
				inString = !inString;
			break;
        case '{':
			inJson++;
			break;
		case '}':
			inJson--;
			break;
        case '[':
            inArray++;
            break;
        case ']':
            inArray--;
            break;
		case ',':
            if(inJson == 0 && inArray == 0 && !inString)
			{
				list.append(buffer);
				buffer = "";
			}
			break;
		}
	}

	if(!buffer.isEmpty())
		list.append(buffer);

    return list;
}

int DropboxJson::parseSubJson(QString strJson, int start, DropboxJsonEntry *jsonEntry)
{
	int openBrackets = 1;
	QString buffer = "";
    DropboxJson* jsonValue = NULL;

	int j;
	for(j=start+1; openBrackets > 0 && j < strJson.size(); ++j)
	{
		if(strJson.at(j).toLatin1() == '{')
			openBrackets++;
		else if(strJson.at(j).toLatin1() == '}')
			openBrackets--;
	}

	buffer = strJson.mid(start, j-start);
#ifdef QT_DEBUG
	qDebug() << "brackets = " << openBrackets << endl;
	qDebug() << "json data(" << start << ":" << j-start << ") = " << buffer << endl;
#endif
    jsonValue = new DropboxJson();
	jsonValue->parseString(buffer);

	// invalid sub json means invalid json
	if(!jsonValue->isValid())
	{
#ifdef QT_DEBUG
		qDebug() << "subjson invalid!" << endl;
#endif
        _valid = false;
		return j;
	}

	// insert new
	jsonEntry->value.json = jsonValue;
    jsonEntry->type       = DROPBOXJSON_TYPE_JSON;
	return j;
}

bool DropboxJson::isAnonymousArray()
{
	return _anonymousArray;
}

QStringList DropboxJson::getArray()
{
	if(!isAnonymousArray())
		return QStringList();

	return getArray("_anonArray");
}

int DropboxJson::compare(const DropboxJson& other)
{
    if(_valueMap.size() != other._valueMap.size())
		return 1;

    QMap<QString, DropboxJsonEntry> yourMap = other._valueMap;

    QList<QString> keys = _valueMap.keys();
	for(int i=0; i<keys.size(); ++i)
	{
		QString key = keys.at(i);
		if(!yourMap.contains(key))
			return 1;

        DropboxJsonEntry myEntry = _valueMap.value(key);
        DropboxJsonEntry yourEntry = yourMap.value(key);
		
		if(myEntry.type != yourEntry.type)
			return 1;

        if(myEntry.type == DROPBOXJSON_TYPE_JSON)
		{
			if(myEntry.value.json->compare(*yourEntry.value.json) != 0)
				return 1;
		}
		else
		{
			if(myEntry.value.value->compare(yourEntry.value.value) != 0)
				return 1;
		}
	}

	return 0;
}
