#include "Connection.h"
#include "Connection_p.h"
#include "Grammar.h"
#include "connectors/Connector.h"
#include "query/QueryBuilder.h"
#include "schema/SchemaBuilder.h"

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
    : d_ptr(new ConnectionPrivate(this))
{
    *d_ptr.data() = *other.d_ptr.data();
}

Connection::~Connection()
{
    Q_D(Connection);
    qDebug() << "Connection::~Connection()";
    d->connector.reset(nullptr);
}

QSharedPointer<Grammar> Connection::schemaGrammar()
{
    Q_D(Connection);
    if(!d->schemaGrammar)
        d->schemaGrammar.reset(this->createScheamGrammar(), &Grammar::deleteLater);

    return d->schemaGrammar;
}

QSharedPointer<Grammar> Connection::queryGrammar()
{
    Q_D(Connection);
    if(!d->queryGrammar)
        d->queryGrammar.reset(this->createQueryGrammar(), &Grammar::deleteLater);

    return d->queryGrammar;
}

SchemaBuilder Connection::schemaBuilder() const
{
    // TODO: support other database schema builder
    // SQLiteSchemaBuilder/MySQLSchemaBuilder/...
    return SchemaBuilder(this);
}

QueryBuilder Connection::queryBuilder() const
{
    return QueryBuilder(this);
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
    return d->connector->driverName();
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

QString Connection::selectOne(const QString &query, const QStringList &bindings)
{
    Q_UNUSED(query)
    Q_UNUSED(bindings)
    return QString();
}

QSqlQuery Connection::select(const QString &query, const QVariantMap &bindings)
{
    Q_D(Connection);
    QSqlQuery sqlQuery(d->pdo);
    sqlQuery.prepare(query);
    if(query.contains(QRegExp("=\\s+\\?")))
    {
        foreach (auto &val, bindings)
            sqlQuery.addBindValue(val);
    }

    if(!sqlQuery.exec())
    {
        qWarning() << sqlQuery.lastError().text();
    }

    return sqlQuery;
}

bool Connection::insert(const QString &query, const QVariantMap &bindings)
{
    Q_D(Connection);
    QSqlQuery sqlQuery(d->pdo);
    sqlQuery.prepare(query);
    foreach (auto &val, bindings)
        sqlQuery.addBindValue(val);
    qDebug().noquote()<<query;
    bool ok;
    if(!(ok = sqlQuery.exec()))
    {
        qWarning() << sqlQuery.lastError().text();
    }

    return ok;
    //    return this->statement(query, bindings);
}

bool Connection::insert(const QString &query, const QList<QVariantMap> &bindings)
{
    Q_D(Connection);

    QSqlQuery sqlQuery(d->pdo);
    sqlQuery.prepare(query);

    // batch binding
    foreach (auto &record, bindings)
    {
        foreach (auto &val, record)
            sqlQuery.addBindValue(val);
    }

    bool ok;
    if(!(ok = sqlQuery.exec()))
    {
        qWarning() << sqlQuery.lastError().text();
    }

    // sqlQuery.lastInsertId();

    return ok;
}

int Connection::update(const QString &query, const QVariantMap &bindings)
{
    Q_D(Connection);
    QSqlQuery sqlQuery(d->pdo);
    sqlQuery.prepare(query);
    if(query.contains(QRegExp("=\\s+\\?")))
    {
        foreach (auto &val, bindings)
            sqlQuery.addBindValue(val);
    }

    if(!sqlQuery.exec())
    {
        qWarning() << sqlQuery.lastError().text();
    }

    return sqlQuery.numRowsAffected();
}

int Connection::del(const QString &query, const QVariantMap &bindings)
{
    Q_D(Connection);
    Q_UNUSED(bindings)

    QSqlQuery sqlQuery(d->pdo);
    sqlQuery.prepare(query);
    if(query.contains(QRegExp("=\\s+\\?")))
    {
        foreach (auto &val, bindings)
            sqlQuery.addBindValue(val);
    }

    if(!sqlQuery.exec())
    {
        qWarning() << sqlQuery.lastError().text();
    }

    return sqlQuery.numRowsAffected();
}

int Connection::statement(const QString &query, const QVariantMap &bindings)
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

int Connection::affectingStatement(const QString &query, const QVariantMap &bindings)
{
    Q_UNUSED(query)
    Q_UNUSED(bindings)
    return 0;
}
