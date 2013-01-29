#include "qdropboxjsontest.hpp"

QDropboxJsonTest::QDropboxJsonTest()
{
}

/*!
 * \brief Simple string read
 * JSON represents an object with single string value. The test
 * tries to read the string value.
 */
void QDropboxJsonTest::testCase1()
{
    QDropboxJson json("{\"string\":\"asdf\"}");
    QVERIFY2(json.isValid(), "validity");
    QVERIFY2(json.getString("string").compare("asdf") == 0, "string value does not match");
}

/*!
 * \brief Simple int read
 * JSON represents an object with a single integer value. The test
 * tries to read that value.
 */
void QDropboxJsonTest::testCase2()
{
    QDropboxJson json("{\"int\":1234}");
    QVERIFY2(json.isValid(), "validity");
    QVERIFY2(json.getInt("int") == 1234, "integer value does not match");
}

/*!
 * \brief Invalidity check
 * JSON is invalid. Test confirms invalidity of the JSON.
 */
void QDropboxJsonTest::testCase3()
{
    QDropboxJson json("{\"test\":\"foo\"");
    QVERIFY2(!json.isValid(), "invalidity not confirmed");
}

/*!
 * \brief Simple boolean read
 * JSON contains a single boolean value. Test accesses this value.
 */
void QDropboxJsonTest::testCase4()
{
    QDropboxJson json("{\"bool\":true}");
    QVERIFY2(json.isValid(), "validity");
    QVERIFY2(json.getBool("bool"), "boolean value does not match");
}

/*!
 * \brief Simple floating point read
 * JSON contains a single double value. Test reads it.
 */
void QDropboxJsonTest::testCase5()
{
    QDropboxJson json("{\"double\":14.323667}");
    QVERIFY2(json.isValid(), "validity");
    QVERIFY2(json.getDouble("double"), "double value does not match");
}

/*!
 * \brief Subjson read
 * JSON contains a subjson that is read, but not evaluated.
 */
void QDropboxJsonTest::testCase6()
{
    QDropboxJson json("{\"json\": {\"string\":\"abcd\"}}");
    QVERIFY2(json.isValid(), "validity");

    QDropboxJson* subjson = json.getJson("json");

    QVERIFY2(subjson!=NULL, "subjson is null");
    QVERIFY2(subjson->isValid(), "subjson invalid");
}
