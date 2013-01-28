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
