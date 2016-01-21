# crext
[Download](https://github.com/yuk7/crext/releases/latest)

This tool is command-line ext partition/img file reader for windows.

**Attention:This tool is test/alpha version.**

**Command-line options and many functions will be changed.**

##Usage
```dos
crext [options] ePath lPath

Options:
  -?, -h, --help                        Displays this help.
  -v, --version                         Displays version information.
  -f, --fopen <ImgFilePath>             Open Image File
  -l, --lp                              List Partitions
  -s, --sp <Partition name>             Set Partition
  -c, --cmd <ls|lsl|cp|size|mode|time>  Command

Arguments:
  ePath                                 Source Path(Ext Partition)
  lPath                                 Destination Path(Host's File System)
```

###Example

####List Partition Tables
```dos
>crext -l
/dev/sda4
/dev/sda5
```

####List Files in Disk Image File
/bin directory in system.img
```dos
>crext -f system.img -c ls /bin
aapt
adb
akmd
alsaucm_test
am
amix
aplay
app_process
app_process32
...
```
####List Files and detail in Disk Image File
/bin directory in system.img
```dos
>crext -f system.img -c lsl /bin
-rwxr-xr-x   418148  2015-09-19 12:25  aapt
-rwxr-xr-x   121072  2015-09-19 12:25  adb
-rwxr-xr-x    38184  2015-09-19 12:25  akmd
-rwxr-xr-x     9600  2015-09-19 12:25  alsaucm_test
-rwxr-xr-x      210  2015-09-19 12:25  am
-rwxr-xr-x     5364  2015-09-19 12:25  amix
-rwxr-xr-x    13784  2015-09-19 12:25  aplay
|rwxr-xr-x       13  2015-09-19 12:25  app_process
-rwxr-xr-x    13668  2015-09-19 12:25  app_process32
...
```

####Copy file from ext partition in HDD
*Reading disk is required an Administrator.*

open /dev/sda5 partition & copy /boot/grub/grub.cfg to C:\grub.cfg
```dos
>crext -s /dev/sda5 -c cp /boot/grub/grub.cfg C:\grub.cfg
[####################] 100%  C:\grub.cfg
```

####Show file details in HDD
*Reading disk is required an Administrator.*

open /dev/sda5 partition & show /bin/bash mode
```dos
>crext -s /dev/sda5 -c lsl /bin/bash
-rwxr-xr-x
```

show /bin/bash access time
```dos
>crext -s /dev/sda5 -c time /bin/bash
2016-01-14 02:52:14
```

show /bin/bash size
```dos
>crext -s /dev/sda5 -c time /bin/bash
1037464
```



###License
This program licensed under GPL v2.
