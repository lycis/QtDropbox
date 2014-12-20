#include "qtdropboxtest.hpp"

typedef QMap<QString, QSharedPointer<QDropboxFileInfo> > QDropboxFileInfoMap;

QtDropboxTest::QtDropboxTest()
{
}


/*!
 * \brief QDropboxJson: Simple string read
 * JSON represents an object with single string value. The test
 * tries to read the string value.
 */
void QtDropboxTest::jsonCase1()
{
    QDropboxJson json("{\"string\":\"asdf\"}");
    QVERIFY2(json.isValid(), "json validity");
    QVERIFY2(json.getString("string").compare("asdf") == 0, "string value does not match");
}

/*!
 * \brief QDropboxJson: Simple int read
 * JSON represents an object with a single integer value. The test
 * tries to read that value.
 */
void QtDropboxTest::jsonCase2()
{
    QDropboxJson json("{\"int\":1234}");
    QVERIFY2(json.isValid(), "json validity");
    QVERIFY2(json.getInt("int") == 1234, "integer value does not match");
}

/*!
 * \brief QDropboxJson: Injson validity check
 * JSON is invalid. Test confirms invalidity of the JSON.
 */
void QtDropboxTest::jsonCase3()
{
    QDropboxJson json("{\"test\":\"foo\"");
    QVERIFY2(!json.isValid(), "injson validity not confirmed");
}

/*!
 * \brief QDropboxJson: Simple boolean read
 * JSON contains a single boolean value. Test accesses this value.
 */
void QtDropboxTest::jsonCase4()
{
    QDropboxJson json("{\"bool\":true}");
    QVERIFY2(json.isValid(), "json validity");
    QVERIFY2(json.getBool("bool"), "boolean value does not match");
}

/*!
 * \brief QDropboxJson: Simple floating point read
 * JSON contains a single double value. Test reads it.
 */
void QtDropboxTest::jsonCase5()
{
    QDropboxJson json("{\"double\":14.323667}");
    QVERIFY2(json.isValid(), "json validity");
    QVERIFY2(json.getDouble("double"), "double value does not match");
}

/*!
 * \brief QDropboxJson: Subjson read
 * JSON contains a subjson that is read, but not evaluated.
 */
void QtDropboxTest::jsonCase6()
{
    QDropboxJson json("{\"json\": {\"string\":\"abcd\"}}");
    QVERIFY2(json.isValid(), "json validity");

    QDropboxJson* subjson = json.getJson("json");

    QVERIFY2(subjson!=NULL, "subjson is null");
    QVERIFY2(subjson->isValid(), "subjson invalid");
}

/*!
 * \brief QDropboxJson: Simple unsigned integer read.
 * JSON contains single unsigned integer that is read.
 */
void QtDropboxTest::jsonCase7()
{
    QDropboxJson json("{\"uint\":4294967295}");
    QVERIFY2(json.isValid(), "json validity");
    QVERIFY2(json.getUInt("uint") == 4294967295, "unsigned int value does not match");
}

/**
 * @brief QDropboxJson: Test if clear works correctly
 */
void QtDropboxTest::jsonCase8()
{
    QDropboxJson json("{\"uint\":4294967295}");
    QVERIFY2(json.isValid(), "json validity");
    json.clear();
    QVERIFY2(json.getUInt("uint") == 0, "internal list not cleared");
    QVERIFY2(json.strContent().isEmpty(), "json string is not cleared");
}

/**
 * @brief QDropboxJson: Test if array interpretation and access are working.
 */
void QtDropboxTest::jsonCase9()
{
    QDropboxJson json("{\"array\": [1, \"test\", true, 7.3]}");
    QVERIFY2(json.isValid(), "json validity");

    QStringList l = json.getArray("array");
    QVERIFY2(l.size() == 4, "array list has wrong size");
    QVERIFY2(l.at(0).compare("1") == 0, "int element not correctly formatted");
    QVERIFY2(l.at(1).compare("test") == 0, "string element not correctly formatted");
    QVERIFY2(l.at(2).compare("true") == 0, "boolean element not correctly formatted");
    QVERIFY2(l.at(3).compare("7.3") == 0, "double element not correctly formatted");
}

/**
 * @brief QDropboxJson: Test if json in array is accessible.
 */
void QtDropboxTest::jsonCase10()
{
    QDropboxJson json("{\"jsonarray\":[{\"key\":\"value\"}]}");
    QVERIFY2(json.isValid(), "json validity");

    QStringList l = json.getArray("jsonarray");
    QVERIFY2(l.size() == 1, "array list has wrong size");

    QDropboxJson arrayJson(l.at(0));
    QVERIFY2(arrayJson.isValid(), "json from array is invalid");
    QVERIFY2(arrayJson.getString("key").compare("value") == 0, "json from array contains wrong value");
}

/**
 * @brief QDropboxJson: Checks if compare() is working by doing a self-comparison.
 */
void QtDropboxTest::jsonCase11()
{
    QString jsonStr = "{\"int\": 1, \"string\": \"test\", \"bool\": true, \"json\": {\"key\": \"value\"}, "
                      "\"array\": [1, 3.5, {\"arraykey\": \"arrayvalue\"}]}";
    QDropboxJson json(jsonStr);
    QVERIFY2(json.isValid(), "json validity");
    QVERIFY2(json.compare(json) == 0, "comparing the same json resulted in negative comparison");
}

/**
 * @brief QDropboxJson: Test whether strContent() returns the correct JSON
 * The test case creates a JSON and another JSON that is based on the return value of strContent() of
 * the first JSON. Both JSONs are compared afterwards and expected to be equal.
 */
void QtDropboxTest::jsonCase12()
{
    QString jsonStr = "{\"int\": 1, \"string\": \"test\", \"bool\": true, \"json\": {\"key\": \"value\"}, "
                      "\"array\": [1, 3.5, {\"arraykey\": \"arrayvalue\"}], \"timestamp\": \"Sat, 21 Aug 2010 22:31:20 +0000\"}";
    QDropboxJson json(jsonStr);
    QVERIFY2(json.isValid(), "json validity");

    QString jsonContent = json.strContent();
    QDropboxJson json2(jsonContent);
    QString j2c = json2.strContent();

    int compare = json.compare(json2);

    QVERIFY2(compare == 0, "string content of json is incorrect or compare is broken");
}

/**
 * @brief QDropboxJson: Setter functions
 * The test verifies if the setter functions are working correctly by setting a value and
 * reading it afterwards.
 */
void QtDropboxTest::jsonCase13()
{
    QDropboxJson json;
    json.setInt("testInt", 10);
    QVERIFY2(json.getInt("testInt") == 10, "setInt of json is incorrect");

    json.setUInt("testUInt", 10);
    QVERIFY2(json.getUInt("testUInt") == 10, "setUInt of json is incorrect");

    json.setDouble("testDouble", 10.0);
    QVERIFY2(json.getDouble("testDouble") == 10.0, "setDouble of json is incorrect");

    json.setBool("testBool", true);
    QVERIFY2(json.getBool("testBool"), "setBool of json is incorrect");

    json.setString("testString", "10");
    QVERIFY2(json.getString("testString").compare("10"), "setString of json is incorrect");

    QDateTime time = QDateTime::currentDateTime();
    json.setTimestamp("testTimestamp", time);
    QVERIFY2(json.getTimestamp("testTimestamp").daysTo(time) == 0, "setTimestamp of json is incorrect");
}

/**
 * @brief QDropboxJson: [] in strings
 * Verify that square brackets in strings are working correctly.
 */
void QtDropboxTest::jsonCase14()
{
    QDropboxJson json("{\"string\": \"[asdf]abcd\"}");  
    QVERIFY2(json.isValid(), "json could not be parsed");
    QVERIFY2(json.getString("string").compare("[asdf]abcd") == 0, "square brackets in string not parsed correctly");
}

/**
 * @brief QDropboxJson: {} in strings
 * Verify that curly brackets within a string are parsed correctly
 */
void QtDropboxTest::jsonCase15()
{
    QDropboxJson json("{\"string\": \"{asdf}abcd\"}");  
    QVERIFY2(json.isValid(), "json could not be parsed");
    QVERIFY2(json.getString("string").compare("{asdf}abcd") == 0, 
	     QString("curly brackets in string not parsed correctly [%1]").arg(json.getString("string")).toStdString().c_str());
}

/**
 * @brief QDropbox: Plaintext Connection
 * This test connects to Dropbox and sends a dummy request to check that the connection in
 * Plaintext mode. The request is not processed any further! <b>You are required to authorize
 * the application for access! The Authorization URI will be printed to you and manual interaction
 * is required to pass this test!</b>
 */
void QtDropboxTest::dropboxCase1()
{
    QDropbox dropbox(APP_KEY, APP_SECRET);
    QVERIFY2(connectDropbox(&dropbox, QDropbox::Plaintext), "connection error");
    QDropboxAccount accInf = dropbox.requestAccountInfoAndWait();
    QVERIFY2(dropbox.error() == QDropbox::NoError, "error on request");
    return;
}

/**
 * @brief QDropbox: delta
 * This test connects to Dropbox and tests the delta API.
 *
 * <b>You are required to authorize
 * the application for access! The Authorization URI will be printed to you and manual interaction
 * is required to pass this test!</b>
 */
void QtDropboxTest::dropboxCase2()
{
    QTextStream strout(stdout);
    QDropbox dropbox(APP_KEY, APP_SECRET);
    QVERIFY2(connectDropbox(&dropbox, QDropbox::Plaintext), "connection error");

    QString cursor = "";
    bool hasMore = true;
    QDropboxFileInfoMap file_cache;

    strout << "requesting delta...\n";
    do
    {
        QDropboxDeltaResponse r = dropbox.requestDeltaAndWait(cursor, "");
        cursor = r.getNextCursor();
        hasMore = r.hasMore();

        const QDropboxDeltaEntryMap entries = r.getEntries();
        for(QDropboxDeltaEntryMap::const_iterator i = entries.begin(); i != entries.end(); i++)
        {
            if(i.value().isNull())
            {
                file_cache.remove(i.key());
            }
            else
            {
                strout << "inserting file " << i.key() << "\n";
                file_cache.insert(i.key(), i.value());
            }
        }

    } while (hasMore);
    strout << "next cursor: " << cursor << "\n";
    for(QDropboxFileInfoMap::const_iterator i = file_cache.begin(); i != file_cache.end(); i++)
    {
        strout << "file " << i.key() << " last modified " << i.value()->clientModified().toString() << "\n";
    }

    return;
}

/**
 * @brief Prompt the user for authorization.
 */
void QtDropboxTest::authorizeApplication(QDropbox* d)
{
    QTextStream strout(stdout);
    QTextStream strin(stdin);

    strout << "##########################################" << endl;
    strout << "# You need to grant this test access to  #" << endl;
    strout << "# your Dropbox!                          #" << endl;
    strout << "#                                        #" << endl;
    strout << "# Go to the following URL to do so.      #" << endl;
    strout << "##########################################" << endl << endl;

    strout << "URL: " << d->authorizeLink().toString() << endl;
    QDesktopServices::openUrl(d->authorizeLink());
    strout << "Press ENTER after you authorized the application!";
    strout.flush();
    strin.readLine();
    strout << endl;
    d->requestAccessTokenAndWait();
}

/**
 * @brief Connect a QDropbox to the Dropbox service
 * @param d QDropbox object to be connected
 * @param m Authentication Method
 * @return <code>true</code> on success
 */
bool QtDropboxTest::connectDropbox(QDropbox *d, QDropbox::OAuthMethod m)
{
    QFile tokenFile("tokens");

    if(tokenFile.exists()) // reuse old tokens
    {
        if(tokenFile.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QTextStream instream(&tokenFile);
            QString token = instream.readLine().trimmed();
            QString secret = instream.readLine().trimmed();
            if(!token.isEmpty() && !secret.isEmpty())
            {
                d->setToken(token);
                d->setTokenSecret(secret);
                tokenFile.close();
                return true;
            }
        }
        tokenFile.close();
    }

    // acquire new token
    if(!d->requestTokenAndWait())
    {
        qCritical() << "error on token request";
        return false;
    }

    d->setAuthMethod(m);
    if(!d->requestAccessTokenAndWait())
    {
        int i = 0;
        for(;i<3; ++i) // we try three times
        {
            if(d->error() != QDropbox::TokenExpired)
                break;
            authorizeApplication(d);
        }

       if(i>3)
       {
           qCritical() <<  "too many tries for authentication";
           return false;
       }

        if(d->error() != QDropbox::NoError)
        {
           qCritical() << "Error: " << d->error() << " - " << d->errorString() << endl;
           return false;
        }
    }

    if(!tokenFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
        return true;

    QTextStream outstream(&tokenFile);
    outstream << d->token() << endl;
    outstream << d->tokenSecret() << endl;
    tokenFile.close();
    return true;
}

QTEST_MAIN(QtDropboxTest)
