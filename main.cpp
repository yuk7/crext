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

    Ext2Read *app;
    app = new Ext2Read();
    log_init();

    QCommandLineParser parser;
    parser.setApplicationDescription("crext is command-line based ext image/partition reader.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption co_openfd("o","Open File/Devices","FilePath|dev");
    parser.addOption(co_openfd);

    QCommandLineOption co_listparts("ip","List Partition indexes");
    parser.addOption(co_listparts);

    QCommandLineOption co_setpart("sp","Set partition","Partition index","0");
    parser.addOption(co_setpart);


    QCommandLineOption co_cmdls("ls","List information files","-d|-l");
    parser.addOption(co_cmdls);

    QCommandLineOption co_cmdcopy("cp","copy file/directory");
    parser.addOption(co_cmdcopy);

    parser.process(a);


    if(parser.isSet(co_openfd))
    {
        QString openfdopt = parser.value(co_openfd);
        if(openfdopt != "dev")
        {
            int result;
            result = app->add_loopback(openfdopt.toUtf8());
            cout << result << endl;
                if(result <= 0)
                {
                    cout << "Open image file failed.";
                    LOG("No valid Ext2 Partitions found in the disk image.");
                    return 1;
                }
        }
        else
        {
            cout << "device open selected." << endl;

        }
    }
    else
    {

        cout << "ERROR: Please set Open File/Devices Option" <<endl << endl;
        cout << "[crext --help] to show help" <<endl;
        return 1;
    }

    if(parser.isSet(co_listparts)){


    }




    return 0;
}

