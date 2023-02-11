# -------------------------------------------------
# Project created by QtCreator 2010-05-06T17:43:10
# -------------------------------------------------
TARGET = QtNeonBtl
TEMPLATE = app
SOURCES += main.cpp \
    emubase/pit8253.cpp \
    mainwindow.cpp \
    Common.cpp \
    emubase/Processor.cpp \
    emubase/Floppy.cpp \
    emubase/Disasm.cpp \
    emubase/Board.cpp \
    emubase/Hard.cpp \
    Emulator.cpp \
    qscreen.cpp \
    qkeyboardview.cpp \
    Settings.cpp \
    qdebugview.cpp \
    qdisasmview.cpp \
    qconsoleview.cpp \
    qmemoryview.cpp \
    qsoundout.cpp \
    qscripting.cpp \
    UnitTests.cpp \
    qdialogs.cpp
HEADERS += mainwindow.h \
    stdafx.h \
    Common.h \
    emubase/Processor.h \
    emubase/Emubase.h \
    emubase/Defines.h \
    emubase/Board.h \
    Emulator.h \
    qscreen.h \
    qkeyboardview.h \
    main.h \
    qdebugview.h \
    qdisasmview.h \
    qconsoleview.h \
    qmemoryview.h \
    qsoundout.h \
    qscripting.h \
    UnitTests.h \
    qdialogs.h
FORMS += mainwindow.ui
RESOURCES += QtNeonBtl.qrc
QT += widgets
QT += script
QT += testlib 
QT += multimedia
DEFINES -= UNICODE _UNICODE
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
