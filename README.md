# crext

This tool is command-line ext partition/img file reader for windows.

**Attention:This tool is test/alpha version.**

**Command-line options and many functions will be changed.**

##Usage
```dos
crext [options]

Options:
  -f, --fopen <ImgFilePath>  Open Image File
  -l, --lp                   List Partitions
  -s, --sp <Partition name>  Set Partition
  -c, --cmd <ls|cp>          Command
  -e, --epath <ExtPath>      Path in Ext Partition
  -p, --lpath <LocalPath>    Path in Local
```

###Example

####List Partition Tables
```dos
>crext -l

/dev/sda4
/dev/sda5
```

####List Files in Directory Disk Image File
/bin directory in system.img
```dos
>crext -f system.img -c ls -e /bin

aapt            1               418148
adb             1               121072
akmd            1               38184
alsaucm_test            1               9600
am              1               210
amix            1               5364
aplay           1               13784
app_process             7               13
app_process32           1               13668
...
```

####Copy file from ext partition in HDD
*Reading disk is required an Administrator.*

open/dev/sda5 partition & copy /boot/grub/grub.cfg to C:\grub.cfg
```dos
>crext -s /dev/sda5 -c cp -e /boot/grub/grub.cfg -p C:\grub.cfg
```

###License
This program licensed under GPL v2.
