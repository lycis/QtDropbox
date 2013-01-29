#include <QTest>
#include <QTextStream>

#include "qdropboxtest.hpp"
#include "qdropboxjsontest.hpp"

int main()
{
    QDropboxTest dropboxTest;
    QDropboxJsonTest jsonTest;

    int failed = 0;
    failed += QTest::qExec(&dropboxTest);
    failed += QTest::qExec(&jsonTest);

    QTextStream outstr(stdout);

    outstr << endl << "************************************" << endl;
    outstr << " Test ";
    if(failed != 0)
        outstr << " FAILED!!!!" << endl;
    else
        outstr << "finished without error." << endl;
    outstr << "************************************" << endl;
    outstr.flush();

    return 0;
}
