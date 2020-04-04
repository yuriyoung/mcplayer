#ifndef CONNECTION_P_H
#define CONNECTION_P_H

#include "Connection.h"

#include <QObject>
#include <QSharedPointer>

class QSqlDatabase;
class Grammar;
class SchemaBuilder;

class ConnectionPrivate
{
public:
    ConnectionPrivate(Connection *q) : q_ptr(q) {}
    virtual ~ConnectionPrivate() {}

    Connection *q_ptr = nullptr;
    QSharedPointer<SchemaBuilder> schemaBuilder = nullptr;
    QSharedPointer<Grammar> grammar = nullptr;

    QSqlDatabase pdo;
    Connection::Closure reconnection = nullptr;
    QSharedPointer<Connector> connector = nullptr;
    QString driver;
    QString tablePrefix = ""; // The table prefix for the database table.
};

#endif // CONNECTION_P_H
