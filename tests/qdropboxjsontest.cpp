#include "qdropboxjsontest.hpp"

QDropboxJsonTest::QDropboxJsonTest()
{
}

void QDropboxJsonTest::testCase1()
{
    QDropboxJson json("{\"string\":\"asdf\"}");
    QVERIFY2(json.isValid(), "validity");
    QVERIFY2(json.getString("string").compare("asdf") == 0, "simple string read");
}

void QDropboxJsonTest::testCase2()
{
    QDropboxJson json("{\"int\":1234}");
    QVERIFY2(json.isValid(), "validity");
    QVERIFY2(json.getInt("int") == 1234, "simple int read");
}

void QDropboxJsonTest::testCase3()
{
    QDropboxJson json("{\"test\":\"foo\"");
    QVERIFY2(!json.isValid(), "invalidity not confirmed");
}
