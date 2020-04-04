#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QSqlDatabase>
#include <QMap>

/**
 * @brief
 * TODO:
 * - elapsed time query
 * - log query
 * - dispatch event
 * -
 */

class Connector;
class Grammar;
class SchemaBuilder;
class QueryBuilder;

class ConnectionPrivate;
class Connection
{
    Q_DECLARE_PRIVATE(Connection)
public:
    using Closure = std::function<bool(Connection *)>;

    explicit Connection(const QString &prefix = "");
    explicit Connection(const Connection &other);
    virtual ~Connection();

    virtual Grammar *queryGrammar() = 0;
    virtual Grammar *schemaGrammar() = 0;

    virtual SchemaBuilder *schemaBuilder();

    // lazy connection callback
    void setReconnection(Closure callback);
    void setConnector(Connector *connector);

    virtual bool reconnect();
    void disconnect();

    QString driverName() const;
    QString connectionName() const;

    void setTablePrefix(const QString &prefix);
    QString  tablePrefix() const;
    Grammar *withTablePrefix(Grammar *grammar) const;

    // TODO: return a query builder(new one with query grammar)
    void query();
    // TODO: return a query builder
    QueryBuilder *table(const QString &table, const QString &as = {});

    QString selectOne(const QString &query, const QStringList &bindings = QStringList());
    QSqlQuery select(const QString &query, const QVariantMap &bindings = QVariantMap());
    bool insert(const QString &query, const QStringList &bindings = QStringList());
    int update(const QString &query, const QStringList &bindings = QStringList());
    int del(const QString &query, const QStringList &bindings = QStringList());
    // execute a sql
    int statement(const QString &query, const QStringList &bindings = QStringList());
    int affectingStatement(const QString &query, const QStringList &bindings = QStringList());

protected:
    explicit Connection(ConnectionPrivate &dd, const QString &prefix = "");
    QScopedPointer<ConnectionPrivate> d_ptr;
};

#endif // CONNECTION_H
