
QT -= gui
QT = core sql
CONFIG += c++11 console
CONFIG -= app_bundle


CONFIG += c++17
TARGET = testQEloquent
DESTDIR = $$PWD/../../bin/
QELOQUENT_BIN_PATH = $$PWD/../../bin
INCLUDEPATH += ../qeloquent
#DEFINES += CATCH_CONFIG_MAIN


DEFINES += QT_DEPRECATED_WARNINGS
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += $$QELOQUENT_BIN_PATH/qeloquent.lib
SOURCES += \
    TestCreatTable.cpp \
    TestInsertRecord.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

SUBDIRS += \
    .

HEADERS += \
#    catch.hpp

