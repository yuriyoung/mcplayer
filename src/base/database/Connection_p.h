#ifndef CONNECTION_P_H
#define CONNECTION_P_H

#include "Connection.h"

#include <QObject>
#include <QSharedPointer>

class QSqlDatabase;
class Grammar;
class SchemaBuilder;
class QueryBuilder;

class ConnectionPrivate
{
public:
    ConnectionPrivate(Connection *q) : q_ptr(q) {}
    virtual ~ConnectionPrivate() {}

    Connection *q_ptr = nullptr;
//    QSharedPointer<SchemaBuilder> schemaBuilder = nullptr;
//    QSharedPointer<QueryBuilder> queryBuilder = nullptr;
    QSharedPointer<Grammar> schemaGrammar = nullptr;
    QSharedPointer<Grammar> queryGrammar = nullptr;

    QSqlDatabase pdo;
    Connection::Closure reconnection = nullptr;
    QSharedPointer<Connector> connector = nullptr;
    QString tablePrefix = ""; // The table prefix for the database table.
};

#endif // CONNECTION_P_H
