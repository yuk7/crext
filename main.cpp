#include <QCoreApplication>
#include <QCommandLineParser>
#include <iostream>
#include <iomanip>
#include "lvm.h"
#include "ext2read.h"
#include "ext2fs.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("crext");
    QCoreApplication::setApplicationVersion("dev-1.0");

    log_init();
    Ext2Read *app;
    app = new Ext2Read();


    QCommandLineParser parser;
    parser.setApplicationDescription("crext is command-line based ext image/partition reader.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption co_openfd(QStringList() << "o" << "open","Open Image File","ImgFilePath");
    parser.addOption(co_openfd);

    QCommandLineOption co_listpart(QStringList() << "l" << "lp","List Partitions");
    parser.addOption(co_listpart);

    QCommandLineOption co_setpart(QStringList() << "s" << "sp","Set partition","Partition name","0");
    parser.addOption(co_setpart);

    QCommandLineOption co_cmd(QStringList() << "c" << "cmd","Command","ls|cp");
    parser.addOption(co_cmd);

    QCommandLineOption co_epth(QStringList() <<"e" << "epath","Path in Ext Partition","ExtPath");
    parser.addOption(co_epth);

    QCommandLineOption co_lpath(QStringList() << "p" << "lpath","Path in Local","LocalPath");
    parser.addOption(co_lpath);



    parser.process(a);


    if(parser.isSet(co_openfd))
    {
        QString openfdopt = parser.value(co_openfd);

        int result;
        result = app->add_loopback(openfdopt.toUtf8());
        if(result <= 0)
        {
            cout << "Open image file failed.";
            LOG("No valid Ext2 Partitions found in the disk image.");
            return 1;
        }
    }

    if(parser.isSet(co_listpart)){
        Ext2Partition *temp;
        list<Ext2Partition *> parts;
        list<Ext2Partition *>::iterator i;

        parts = app->get_partitions();
        for(i = parts.begin(); i != parts.end(); i++)
        {
            temp = (*i);
            cout << temp->get_linux_name().c_str() << endl;

        }
    }




    return 0;
}

