QT -= gui
QT += sql

CONFIG += c++17
CONFIG -= app_bundle
TEMPLATE = lib
CONFIG  += dll
DESTDIR = $$PWD/../../bin/
DEFINES += QELOQUENT_LIBRARY
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Connection.cpp \
        ConnectionProvider.cpp \
        Database.cpp \
        MySqlConnection.cpp \
        SQLiteConnection.cpp \
        connectors/SQLiteConnector.cpp \
        eloquent/HasRelationship.cpp \
        eloquent/User.cpp \
        query/SQLiteQueryGrammar.cpp \
        schema/SQLiteSchemaGrammar.cpp \
        schema\ColumnDefinition.cpp \
        connectors/Connector.cpp \
        Grammar.cpp \
        connectors/MySqlConnector.cpp \
        eloquent/Collection.cpp \
        eloquent/EloquentBuilder.cpp \
        eloquent/Model.cpp \
        eloquent/relations/BelongsTo.cpp \
        eloquent/relations/BelongsToMany.cpp \
        eloquent/relations/HasMany.cpp \
        eloquent/relations/HasOne.cpp \
        eloquent/relations/HasOneOrMany.cpp \
        eloquent/relations/Pivot.cpp \
        eloquent/relations/Relation.cpp \
        main.cpp \
        query/Clause.cpp \
        query/QueryBuilder.cpp \
        query/QueryGrammar.cpp \
        schema/Blueprint.cpp \
        schema/Command.cpp \
        schema/SchemaBuilder.cpp \
        schema/SchemaGrammar.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Connection.h \
    ConnectionProvider.h \
    Connection_p.h \
    Database.h \
    MySqlConnection.h \
    QEloquentMacro.h \
    SQLiteConnection.h \
    connectors/SQLiteConnector.h \
    eloquent/HasRelationship.h \
    eloquent/User.h \
    eloquent/relations/HasOneOrMany_p.h \
    eloquent/relations/Relation_p.h \
    query/QueryGrammar_p.h \
    query/SQLiteQueryGrammar.h \
    schema/SQLiteSchemaGrammar.h \
    schema\ColumnDefinition.h \
    Grammar_p.h \
    connectors/Connector.h \
    Grammar.h \
    connectors/MySqlConnector.h \
    eloquent/Collection.h \
    eloquent/EloquentBuilder.h \
    eloquent/Model.h \
    eloquent/relations/BelongsTo.h \
    eloquent/relations/BelongsToMany.h \
    eloquent/relations/HasMany.h \
    eloquent/relations/HasOne.h \
    eloquent/relations/HasOneOrMany.h \
    eloquent/relations/Pivot.h \
    eloquent/relations/Relation.h \
    query/Clause.h \
    query/QueryBuilder.h \
    query/QueryGrammar.h \
    schema/Blueprint.h \
    schema/Command.h \
    schema/SchemaBuilder.h \
    schema/SchemaGrammar.h \
    schema/SchemaGrammar_p.h \
    support/sfinae.h \
    support/array_helper.h \
    support/string_helper.h

RESOURCES += \
    res.qrc
