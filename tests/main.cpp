#include <QTest>
#include "qdropboxtest.hpp"
#include "qdropboxjsontest.hpp"

int main()
{
    QDropboxTest dropboxTest;
    QDropboxJsonTest jsonTest;

    QTest::qExec(&dropboxTest);
    QTest::qExec(&jsonTest);
    return 0;
}
