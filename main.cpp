#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <iostream>
#include <iomanip>
#include <time.h>
#include "lvm.h"
#include "ext2read.h"
#include "ext2fs.h"


string mode_str(uint16_t mode);
string time_str(uint32_t time,const char *format);
bool copy_dir(Ext2File *srcfile,QString &destdir);
bool copy_file(Ext2File *srcfile,QString &destfile);
bool show_progress(int now,int max,QString str);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("crext");
    QCoreApplication::setApplicationVersion("1.1.0a");

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

    QCommandLineOption co_cmd(QStringList() << "c" << "cmd","Command","ls|lsl|cp|size|mode|time");
    parser.addOption(co_cmd);

    parser.addPositionalArgument("ePath", "Source Path(Ext Partition)");
    parser.addPositionalArgument("lPath","Destination Path(Host's File System)");

    parser.process(a);

    QStringList pargs = parser.positionalArguments();

    if(argv[1] == NULL)
        parser.showHelp(0);

    if(!(parser.isSet(co_listpart) | parser.isSet(co_cmd)))
    {
        cout << "bad parameter" << endl;
        cout << "List partition or Command option required." << endl << endl;
        parser.showHelp(1);
    }

    if(parser.isSet(co_listpart) && (parser.isSet(co_setpart) | parser.isSet(co_cmd) | (pargs.size() > 0)))
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
    QString optepath = "";
    QString optlpath = "";
    if(pargs.size() >= 1)
        optepath = pargs.at(0);
    if(pargs.size() >= 2)
        optlpath = pargs.at(1);
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
        if(EXT2_S_ISDIR(setefile->inode.i_mode))
        {
            ext2dirent *lsdirent;
            lsdirent = setpart->open_dir(setefile);

            while(setefile = setpart->read_dir(lsdirent))
            {
                cout << setefile->file_name.c_str() << endl;
            }
        }
        else
        {
            cout << setefile->file_name.c_str() << endl;
        }
        return 0;
    }

    if(optcmd == "lsl")
    {
        QStringList listfname;
        QStringList listfsize;
        QStringList listfmode;
        QStringList listfdate;

        int fsize_l_max = 0;
        int fdate_l_max = 0;

        if(EXT2_S_ISDIR(setefile->inode.i_mode))
        {
            ext2dirent *lsdirent;
            lsdirent = setpart->open_dir(setefile);

            while(setefile = setpart->read_dir(lsdirent))
            {
                listfname << QString(setefile->file_name.c_str());

                QString fsize = QString(QString::number(int(setefile->file_size)));
                listfsize << fsize;
                if (fsize_l_max < fsize.size())
                    fsize_l_max = fsize.size();

                listfmode << QString(mode_str(setefile->inode.i_mode).c_str());

                QString fdate = QString(time_str(setefile->inode.i_atime,"%Y-%m-%d %I:%M").c_str());
                listfdate << fdate;
                if (fdate_l_max < fdate.size())
                    fdate_l_max = fdate.size();
            }
        }
        else
        {
            listfname << QString(setefile->file_name.c_str());

            QString fsize = QString(QString::number(int(setefile->file_size)));
            listfsize << fsize;
            if (fsize_l_max < fsize.size())
                fsize_l_max = fsize.size();

            listfmode << QString(mode_str(setefile->inode.i_mode).c_str());

            QString fdate = QString(time_str(setefile->inode.i_atime,"%Y-%m-%d %I:%M").c_str());
            listfdate << fdate;
            if (fdate_l_max < fdate.size())
                fdate_l_max = fdate.size();
        }

        for(int i = 0; i < listfname.size(); i++)
        {
            cout << listfmode.at(i).toLocal8Bit().constData() << "  "
                 << setw(fsize_l_max) << listfsize.at(i).toLocal8Bit().constData() << "  "
                 << setw(fdate_l_max) <<listfdate.at(i).toLocal8Bit().constData() << "  "
                 << listfname.at(i).toLocal8Bit().constData() << endl;
        }
        return 0;
    }

    if(optcmd == "cp")
    {
        if(pargs.size() > 0)
        {
            if(EXT2_S_ISDIR(setefile->inode.i_mode))
            {
                copy_dir(setefile,optlpath);
            }
            else
            {
                copy_file(setefile,optlpath);
            }
        }
        return 0;
    }

    if(optcmd == "size")
    {
        cout << int(setefile->file_size) << endl;
        return 0;
    }

    if(optcmd == "mode")
    {
        cout << mode_str(setefile->inode.i_mode) << endl;
        return 0;
    }
    if(optcmd == "time")
    {
        cout << time_str(setefile->inode.i_atime,"%Y-%m-%d %I:%M:%S") << endl;
        return 0;
    }


    cout << "bad parameter" << endl;
    cout << "The specified command option does not exist." << endl << endl;
    parser.showHelp(1);

    return 0;
}


string mode_str(uint16_t mode)
{
    string str = "";

    if(EXT2_S_ISREG(mode))
        str += "-";
    if(EXT2_S_ISDIR(mode))
        str += "d";
    if(EXT2_S_ISLINK(mode))
        str += "|";

    if(mode & EXT2_S_IRUSR)
        str += "r";
    else
        str += "-";
    if(mode & EXT2_S_IWUSR)
        str += "w";
    else
        str += "-";
    if(mode & EXT2_S_IXUSR)
        str += "x";
    else
        str += "-";
    if(mode & EXT2_S_IRGRP)
        str += "r";
    else
        str += "-";
    if(mode & EXT2_S_IWGRP)
        str += "w";
    else
        str += "-";
    if(mode & EXT2_S_IXGRP)
        str += "x";
    else
        str += "-";
    if(mode & EXT2_S_IROTH)
        str += "r";
    else
        str += "-";
    if(mode & EXT2_S_IWOTH)
        str += "w";
    else
        str += "-";
    if(mode & EXT2_S_IXOTH)
        str += "x";
    else
        str += "-";

    return str;
}

string time_str(uint32_t time,const char *format)
{
    char str[256];
    time_t timet = time;
    struct tm *tm;
    tm = localtime(&timet);
    strftime(str, 255, format, tm);

    return string(str);
}

bool copy_dir(Ext2File *srcfile,QString &destdir)
{
    Ext2Partition *part;
    ext2dirent *dirent;
    part = srcfile->partition;
    dirent = part->open_dir(srcfile);

    QDir().mkdir(destdir);

    while(srcfile = part->read_dir(dirent))
    {
        QString cdestpath = destdir + QString("/") +QString(srcfile->file_name.c_str());
        if(EXT2_S_ISDIR(srcfile->inode.i_mode))
        {
            copy_dir(srcfile,cdestpath);
        }
        else
        {
            copy_file(srcfile,cdestpath);
        }
    }
    return true;
}



bool copy_file(Ext2File *srcfile,QString &destfile)
{
    if(destfile.toStdString().substr(destfile.size() - 1) == "/" |
            destfile.toStdString().substr(destfile.size() - 1) == "\\")
    {
        destfile = destfile + QString(srcfile->file_name.c_str());
    }


    lloff_t blocks, blkindex;
    QString qsrc;
    QFile *filesav;
    int extra;
    int ret;
    int blksize = srcfile->partition->get_blocksize();

    char *buffer;
    buffer = new char [blksize];


    filesav = new QFile(destfile);
    if (!filesav->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        LOG("Error creating file %s.\n", srcfile->file_name.c_str());
        return false;
    }

    blocks = srcfile->file_size / blksize;
    for(blkindex = 0; blkindex < blocks; blkindex++)
    {
        ret = srcfile->partition->read_data_block(&srcfile->inode, blkindex, buffer);
        if(ret < 0)
        {
            filesav->close();
            return false;
        }
        filesav->write(buffer, blksize);
        show_progress(blkindex,blocks,destfile);
    }

    extra = srcfile->file_size % blksize;
    if(extra)
    {
        ret = srcfile->partition->read_data_block(&srcfile->inode, blkindex, buffer);
        if(ret < 0)
        {
            filesav->close();
            return false;
        }
        filesav->write(buffer, extra);
    }
    filesav->close();
    show_progress(1,1,destfile);
    cout << endl;
    return true;
}

bool show_progress(int now,int max,QString str)
{
    int iprog = int(double(now)/double(max) *100);
    string progstr = "[";
    for( int i = 0;i < (iprog/5);i++ )
    progstr += "#";

    for( int i = (iprog/5);i < 20;i++ )
    progstr += " ";

    progstr +="]";

    cout << progstr << " " << setw(3) << iprog << "%  " << str.toStdString() << "\r" << flush;
}
