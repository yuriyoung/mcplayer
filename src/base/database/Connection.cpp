#include "Connection.h"
#include "Connection_p.h"
#include "connectors/Connector.h"
#include "query/QueryBuilder.h"
#include "schema/SchemaBuilder.h"
#include "schema/SchemaGrammar.h"

#include <QUuid>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QSharedPointer>
#include <QDebug>

/**
 * @brief Connection::Database
 */
Connection::Connection(const QString &prefix)
    : Connection(*new ConnectionPrivate(this), prefix)
{

}

Connection::Connection(ConnectionPrivate &dd, const QString &prefix)
    : d_ptr(&dd)
{
    d_ptr->tablePrefix = prefix;
}

Connection::Connection(const Connection &other)
{
    Q_D(Connection);
    d->driver = other.driverName();
    d->tablePrefix = other.tablePrefix();
}

Connection::~Connection()
{
    Q_D(Connection);
    qDebug() << "Connection::~Connection()";
    d->connector.reset(nullptr);
}

SchemaBuilder *Connection::schemaBuilder()
{
    Q_D(Connection);
    d->schemaBuilder = QSharedPointer<SchemaBuilder>::create(this);
    return d->schemaBuilder.data();
}

void Connection::setReconnection(Connection::Closure callback)
{
    Q_D(Connection);
    d->reconnection = callback;
}

void Connection::setConnector(Connector *connector)
{
    Q_D(Connection);
    // auto free old connector
    d->connector.reset(connector, &Connector::deleteLater);
    d->pdo = d->connector->connect();
}

bool Connection::reconnect()
{
    Q_D(Connection);
    if(d->reconnection)
    {
        // lazy connection
        return d->reconnection(this);
    }

    qWarning() << "Lost connection and no reconnector available.";

    return false;
}

void Connection::disconnect()
{
    Q_D(Connection);
    d->connector->disconnect();
}

QString Connection::driverName() const
{
    Q_D(const Connection);
    return d->driver;
}

QString Connection::connectionName() const
{
    Q_D(const Connection);
    return d->connector->connectionName();
}

void Connection::setTablePrefix(const QString &prefix)
{
    Q_D(Connection);
    d->tablePrefix = prefix;
    this->schemaGrammar()->setTablePrefix(prefix);
}

QString Connection::tablePrefix() const
{
    Q_D(const Connection);
    return d->tablePrefix;
}

Grammar *Connection::withTablePrefix(Grammar *grammar) const
{
    Q_D(const Connection);
    grammar->setTablePrefix(d->tablePrefix);
    return grammar;
}

void Connection::query()
{
    // TODO: return a query builder(new one with query grammar)
}

QueryBuilder *Connection::table(const QString &table, const QString &as)
{
    Q_UNUSED(table)
    Q_UNUSED(as)

    // TODO: return a query build and set from table with as alias
    QueryBuilder *query = new QueryBuilder(this);
    return &query->from(table);
}

QString Connection::selectOne(const QString &query, const QStringList &bindings)
{
    Q_UNUSED(query)
    Q_UNUSED(bindings)
    return QString();
}

QSqlQuery Connection::select(const QString &query, const QVariantMap &bindings)
{
    QSqlQuery qu;
    qu.prepare(query);

    QMapIterator<QString, QVariant> it(bindings);
    while (it.hasNext())
    {
        it.next();
        qu.bindValue(it.key(), it.value());
    }

    qu.exec(query);
    return qu;
}

bool Connection::insert(const QString &query, const QStringList &bindings)
{
    Q_UNUSED(query)
    Q_UNUSED(bindings)
    return this->statement(query, bindings);
}

int Connection::update(const QString &query, const QStringList &bindings)
{
    Q_UNUSED(query)
    Q_UNUSED(bindings)
    return 0;
}

int Connection::del(const QString &query, const QStringList &bindings)
{
    Q_UNUSED(query)
    Q_UNUSED(bindings)
    return 0;
}

int Connection::statement(const QString &query, const QStringList &bindings)
{
    Q_D(Connection);
    Q_UNUSED(bindings)
    QSqlQuery sql = d->pdo.exec(query);
    if(sql.lastError().type() != QSqlError::NoError)
    {
        qWarning() << sql.lastError().text();
        return -1;
    }

    qDebug() << "execute statement ok: [" << sql.lastQuery() << "]";

    return 0;
}

int Connection::affectingStatement(const QString &query, const QStringList &bindings)
{
    Q_UNUSED(query)
    Q_UNUSED(bindings)
    return 0;
}
