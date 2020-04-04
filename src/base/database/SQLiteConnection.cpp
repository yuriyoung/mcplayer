#include "SQLiteConnection.h"
#include "Connection_p.h"
#include "query/QueryGrammar.h"
#include "schema/SQLiteSchemaGrammar.h"

class SQLiteConnectionPrivate : public ConnectionPrivate
{
public:
    SQLiteConnectionPrivate(Connection *q) : ConnectionPrivate(q) { }
};

SQLiteConnection::SQLiteConnection(const QString &prefix)
    : Connection(*new SQLiteConnectionPrivate(this), prefix)
{

}

Grammar *SQLiteConnection::queryGrammar()
{
    // TODO: create SqliteQueryGrammar

    return new QueryGrammar();
}

Grammar *SQLiteConnection::schemaGrammar()
{
/*
    Q_D(SQLiteConnection);
    auto grammar = new SQLiteSchemaGrammar;
    grammar->setTablePrefix(d->tablePrefix);
    return grammar;
*/

    // be auto free (see SchemaBuilder::build())
    return this->withTablePrefix(new SQLiteSchemaGrammar);
}
