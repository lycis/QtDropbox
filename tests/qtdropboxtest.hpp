#ifndef QDROPBOXJSONTEST_H
#define QDROPBOXJSONTEST_H

#include <QtTest>
#include "qtdropbox.h"

class QtDropboxTest : public QObject
{
    Q_OBJECT

public:
   QtDropboxTest();

private Q_SLOTS:
    void jsonCase1();
    void jsonCase2();
    void jsonCase3();
    void jsonCase4();
    void jsonCase5();
    void jsonCase6();
    void jsonCase7();
    void jsonCase8();
    void jsonCase9();
    void jsonCase10();
};

#endif // QDROPBOXJSONTEST_H
