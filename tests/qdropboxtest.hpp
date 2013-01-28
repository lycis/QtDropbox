#ifndef QDROPBOXTEST_HPP
#define QDROPBOXTEST_HPP

#include <QtTest>
#include "qtdropbox.h"

class QDropboxTest : public QObject
{
    Q_OBJECT

public:
   QDropboxTest();

private Q_SLOTS:
    void testCase1();
};

#endif // QDROPBOXTEST_HPP
