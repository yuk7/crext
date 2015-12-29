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

    QCommandLineOption co_listpart("lp","List Partitions");
    parser.addOption(co_listpart);

    QCommandLineOption co_setpart("sp","Set partition","Partition name","0");
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

