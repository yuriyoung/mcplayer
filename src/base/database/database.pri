INCLUDEPATH += $$PWD

include(helpers/helpers.pri)
include(migrations/migrations.pri)

HEADERS += \
    $$PWD/Database.h \
	$$PWD/DatabaseError.h

SOURCES += \
    $$PWD/Database.cpp \
	$$PWD/DatabaseError.cpp
