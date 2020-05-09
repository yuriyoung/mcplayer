#include "SQLiteConnection.h"
#include "Connection_p.h"
#include "query/SQLiteQueryGrammar.h"
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

Grammar *SQLiteConnection::createScheamGrammar()
{
    Q_D(SQLiteConnection);
    auto grammar = new SQLiteSchemaGrammar;
    grammar->setTablePrefix(d->tablePrefix);
    return grammar;
}

Grammar *SQLiteConnection::createQueryGrammar()
{
    return this->withTablePrefix(new SQLiteQueryGrammar());
}
