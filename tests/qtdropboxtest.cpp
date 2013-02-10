#include "qtdropboxtest.hpp"

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
                      "\"array\": [1, 3.5, {\"arraykey\": \"arrayvalue\"}]}";
    QDropboxJson json(jsonStr);
    QVERIFY2(json.isValid(), "json validity");

    QString jsonContent = json.strContent();
    QDropboxJson json2(jsonContent);
    QString j2c = json2.strContent();

    int compare = json.compare(json2);

    QVERIFY2(compare == 0, "string content of json is incorrect or compare is broken");
}

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

QTEST_MAIN(QtDropboxTest)
