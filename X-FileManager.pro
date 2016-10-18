#-------------------------------------------------
#
# Project created by QtCreator 2016-09-22T22:24:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = X-FileManager
TEMPLATE = app

RC_ICONS = image/system_file_manager_128px_1172140_easyicon.net.ico

QMAKE_CXXFLAGS_DEBUG += -O0

SOURCES += main.cpp\
        dialog.cpp \
    processdialog.cpp \
    myLibrary/qsfiles.cpp \
    myLibrary/filecopyspeedcalculates.cpp

HEADERS  += dialog.h \
    processdialog.h \
    myLibrary/qsfiles.h \
    myLibrary/filecopyspeedcalculates.h

FORMS    += dialog.ui \
    processdialog.ui

RESOURCES += \
    image/images.qrc
