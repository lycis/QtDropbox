#ifndef QDROPBOXJSONTEST_H
#define QDROPBOXJSONTEST_H

#include <QtTest>
#include "qtdropbox.h"

class QDropboxJsonTest : public QObject
{
    Q_OBJECT

public:
   QDropboxJsonTest();

private Q_SLOTS:
    void testCase1();
    
};

#endif // QDROPBOXJSONTEST_H
