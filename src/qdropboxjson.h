#ifndef QDROPBOXJSON_H
#define QDROPBOXJSON_H

#include "qtdropbox_global.h"

#include <QObject>
#include <QMap>
#include <QList>
#include <QDateTime>
#include <QStringList>

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

//! Keeps values of a JSON
union qdropboxjson_value{
    QDropboxJson  *json; //!< Used to store subjsons (JSON in JSON)
    QString       *value; //!< used to store a real value, all values are converted from QString
};

//! Keeps keys of a JSON
struct qdropboxjson_entry{
    qdropboxjson_entry_type type; //!< Datatype of value
    qdropboxjson_value      value; //!< Reference to the value struct
};

//! Used to store JSON data that is returned from Dropbox.
/*!
  Most of the communication with Dropbox is handled by using JSON data structures. JSON is
  originally method of complex data description used for JavaScript and PHP and thus it is
  designed to work with typeless languages. QDropboxJson provides an interface that maps
  the mixed type values of a JSON to native C++ data types as good as possible.

  A JSON is usually passed as string and can be parsed by either passing that string to the
  constructor or using parseString(). If any error occurs the QDropboxJson will be marked as
  invalid (see isValid()).

  The data of a valid QDropboxJson can be accessed by using one of the get-functions. If the
  value you want to access is not mapped to the datatype you requested an empty value will be
  returned. You can always set a force flag. If you do the returned value will be converted but
  may return nonsense data. Use this flag with care and only if you know what you're doing.

  \warning Currently arrays in JSONs are not supported.
  \todo Implemement setter functions and toString() for JSON generation (altough not necessary it
        would be a nice feature)
 */
class QTDROPBOXSHARED_EXPORT QDropboxJson : public QObject
{
    Q_OBJECT
public:
    /*!
      Creates an empty JSON object.

      \param parent Pointer to the parent QObject.
     */
    QDropboxJson(QObject *parent = 0);

    /*!
      This constructor interprets the given string as JSON.

      \param strJson JSON as string.
      \param parent Parent QObject.
     */
    QDropboxJson(QString strJson, QObject *parent = 0);

    /*!
      Copies the data of another QDropboxJSon.

      \param other The QDropboxJson to be copied.
     */
    QDropboxJson(const QDropboxJson &other);

    /*!
      Cleans up the JSON on destruction.
     */
    ~QDropboxJson();

    /*!
      This enum is used to categorize the data type of JSON values.
     */
    enum DataType{
        NumberType, //!< Number based type (interpreted as qint64)
        StringType, //!< String based type of variable length
        JsonType,   //!< A subjson
        ArrayType,  //!< Array data type (currently not supported!)
        FloatType,  //!< Floating point based datatype
        BoolType,   //!< Boolean based types.
        UnsignedIntType, //!< Number based type unsigned (only applied if NumberType does not match)
        UnknownType //!< Data type could not be identified.
    };

    /*!
      Interprets a string as JSON - or at least tries to. If this is not possible
      the QDropboxJson will be invalidated.

      \parem strJson JSON in string representation.
     */
    void parseString(QString strJson);

    /*!
      Drops all stored JSON data.
     */
    void clear();

    /*!
      Returns true if the QDropboxJson contains valid data from a JSON. If an error occurs
      during the parsing of a JSON string this function will return false.
     */
    bool isValid();

    /*!
      Returns true if the QDropboxJson contains the given key.

      \param key The requested key.
     */
    bool hasKey(QString key);

    /*!
      Returns the data type of the value mapped to the key.
      \param key The key to be checked.
     */
    DataType type(QString key);

    /*!
      Returns a stored integer value identified by the given key. If the key does
      not map 0 is returned. If the force flag is set the check of the data type
      is omitted and it is tried to convert the value regardless of the real data type.
     */
    qint64 getInt(QString key, bool force = false);

    void setInt(QString key, qint64 value);

    /*!
      Returns a stored unsigned integer value identified by the given key. If the key does
      not map 0 is returned. If the force flag is set the check of the data type
      is omitted and it is tried to convert the value regardless of the real data type.
     */
    quint64       getUInt(QString key, bool force = false);

    void setUInt(QString key, quint64 value);

    /*!
      Returns a stored string value identified by the given key. If the key does
      not map an empty QString is returned. If the force flag is set the check of the data type
      is omitted and it is tried to convert the value regardless of the real data type.
     */
    QString       getString(QString key, bool force = false);

    void setString(QString key, QString value);

    /*!
      Returns a sub JSON identified by the given key. If the key does not map to a
      JSON a NULL pointer will be returned. It is not possible to force a conversion.
     */
    QDropboxJson *getJson(QString key);

    void setJson(QString key, QDropboxJson value);

    /*!
      Returns a stored floating point value identified by the given key. If the key does
      not map 0.0 is returned. If the force flag is set the check of the data type
      is omitted and it is tried to convert the value regardless of the real data type.
     */
    double        getDouble(QString key, bool force = false);

    void setDouble(QString key, double value);

    /*!
      Returns a stored boolean value identified by the given key. If the key does
      not map false is returned. If the force flag is set the check of the data type
      is omitted and it is tried to convert the value regardless of the real data type.
     */
    bool          getBool(QString key, bool force = false);

    void setBool(QString key, bool value);

    /*!
      Returns the stored JSON's string representation.
     */
    QString strContent() const;

	/*!
	  Returns a stored string values as QDateTime timestamp. The timestamp will be invalid
	  if the string could not be converted.
	*/
    QDateTime getTimestamp(QString key, bool force = false);

    void setTimestamp(QString key, QDateTime value);

	/*!
	  Returnes a stored array as a list of string items. If the key does not exist or is not
	  stored as array the function returns an empty list. If you need the items in a specific
	  data type you have to do equivalent casting your self!
	*/
	QStringList getArray(QString key, bool force = false);

	/*!
	  Returns the content of a stored array as a list of string items <i>if the JSON contains
	  an anynmous array</i> (see also isAnonymousArray()). As with getArray(QString key, bool force = false)
	  you have to parse the content of the array your self.
	*/
	QStringList getArray();

	/**!
	  Overloaded operator to copy a QDropboxJson.
	*/
    QDropboxJson& operator =(QDropboxJson&);

	/**!
	  A JSON may be an anonymous array like this:
	  \code
	  [
	    "a": "valueA",
		"b": "valueB"
	  ]
	  \endcode

	  Use this function to identify a JSON that is an anonymous array.
	  \returns <code>true</code> if the JSON is an anonymous array.
	*/
	bool isAnonymousArray();

	/**!
	  Compares two JSON objects if they are the same.
	  This means that they have the same keys with the same values.

	  \param other the JSON you wish to compare to
	  \returns 0 if the JSON objects are equals
	*/
	int compare(const QDropboxJson& other);
    
protected:
		bool valid;

private:
    QMap<QString, qdropboxjson_entry> valueMap;
	bool _anonymousArray;

    void emptyList();
    qdropboxjson_entry_type interpretType(QString value);
	int parseSubJson(QString str, int start, qdropboxjson_entry *jsonEntry);
	void _init();
};

#endif // QDROPBOXJSON_H
