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

    QCommandLineOption co_openf(QStringList() << "f" << "fopen","Open Image File","ImgFilePath");
    parser.addOption(co_openf);

    QCommandLineOption co_listpart(QStringList() << "l" << "lp","List Partitions");
    parser.addOption(co_listpart);

    QCommandLineOption co_setpart(QStringList() << "s" << "sp","Set Partition","Partition name","0");
    parser.addOption(co_setpart);

    QCommandLineOption co_cmd(QStringList() << "c" << "cmd","Command","ls|cp");
    parser.addOption(co_cmd);

    QCommandLineOption co_epath(QStringList() <<"e" << "epath","Path in Ext Partition","ExtPath");
    parser.addOption(co_epath);

    QCommandLineOption co_lpath(QStringList() << "p" << "lpath","Path in Local","LocalPath");
    parser.addOption(co_lpath);

    parser.process(a);

    if(argv[1] == NULL)
        parser.showHelp(0);

    if(!(parser.isSet(co_listpart) | parser.isSet(co_cmd)))
    {
        cout << "bad parameter" << endl;
        cout << "List partition or Command option required." << endl << endl;
        parser.showHelp(1);
    }

    if(parser.isSet(co_listpart) && (parser.isSet(co_setpart) | parser.isSet(co_cmd) | parser.isSet(co_epath) | parser.isSet(co_lpath)))
    {
        cout << "bad parameter" << endl;
        cout << "List partitions option cannot use with Other options" << endl << endl;
        parser.showHelp(1);
    }


    if(parser.isSet(co_openf))
    {
        QString openfopt = parser.value(co_openf);

        int result;
        result = app->add_loopback(openfopt.toUtf8());
        if(result <= 0)
        {
            cout << "Open image file failed.";
            LOG("No valid Ext2 Partitions found in the disk image.");
            return 1;
        }
    }

    list<Ext2Partition *> parts;
    parts = app->get_partitions();

    if(parts.size() <= 0)
    {
        cout << "ERR:No partitions detected." << endl;
        cout << "Reading disk is required an Administrator. (not required for image file)" <<endl <<endl;
        cout << "*Please make sure ext partitions exists." <<endl;
        cout << "*Please make sure you are running this application as an Administrator." <<endl;
        return 1;
    }

    if(parser.isSet(co_listpart)){
        Ext2Partition *lptemp;
        list<Ext2Partition *>::iterator lpi;
        bool bl = true;
        for(lpi = parts.begin(); lpi != parts.end(); lpi++)
        {
            lptemp = (*lpi);
            cout << lptemp->get_linux_name().c_str() << endl;
        }
        return 0;
    }

    Ext2Partition *setpart;
    Ext2Partition *sptemp;
    bool spsetd = false;
    list<Ext2Partition *>::iterator spi;
    QString optsetpart = parser.value(co_setpart);
    for(spi = parts.begin(); spi != parts.end(); spi++)
    {
        sptemp = (*spi);
        if(optsetpart == "0")
        {
            if(!strstr(sptemp->get_linux_name().c_str(),"/dev/sd"))
            {
                setpart = sptemp;
                spsetd = true;
            }
        }
        else
        {
            if(strstr(sptemp->get_linux_name().c_str(),optsetpart.toUtf8().data()))
            {
                setpart = sptemp;
                spsetd = true;
            }
        }
    }
    if(spsetd == false)
    {
        cout << "ERR:can't set partition." << endl;
        cout << "*Please make sure ext partitions name exist." <<endl;
        return 1;
    }


    QString optcmd = parser.value(co_cmd);
    QString optepath = parser.value(co_epath);
    QString optlpath = parser.value(co_lpath);
    QStringList epathlist = optepath.split("/", QString::SkipEmptyParts);
    Ext2File *ptr;
    Ext2File *setefile;
    ext2dirent *dirent;

    ptr = setpart->get_root();
    dirent = setpart->open_dir(ptr);

    setefile = ptr;
    for(int i = 0; i < epathlist.size(); i++)
    {
        bool efsetd = false;
        while((ptr = setpart->read_dir(dirent)) != NULL)
        {
            if(strcmp(ptr->file_name.c_str(),epathlist.at(i).toLocal8Bit().constData()) == 0)
            {
                setefile = ptr;
                efsetd = true;
            }
        }
        if(efsetd == false)
        {
            cout << "ERR:Ext Path Not found" << endl;
            cout << "*Please make sure path in selected ext partition exist." <<endl;
            return 1;
        }
        else
        {
            dirent = setpart->open_dir(setefile);
        }
    }




    if(optcmd == "ls")
    {
        ext2dirent *lsdirent;
        lsdirent = setpart->open_dir(setefile);

        while(setefile = setpart->read_dir(lsdirent))
        {
            cout << setefile->file_name.c_str() << "\t\t" << (int)setefile->file_type << "\t\t" << setefile->file_size << endl;
        }
    }

    if(optcmd == "cp")
    {

    }



    return 0;
}

