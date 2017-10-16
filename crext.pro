QT += core
QT -= gui

TARGET = crext
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ext2fs.cpp \
    ext2read.cpp \
    lvm.cpp \
    log.c \
    platform_dos.c \
    platform_unix.c \
    platform_win32.c

HEADERS += \
    ext2fs.h \
    ext2read.h \
    lvm.h \
    partition.h \
    parttypes.h \
    platform.h \
    gpt.h

