#ifndef CONNECTION_H
#define CONNECTION_H
#include "QEloquentMacro.h"
#include "query/QueryBuilder.h"

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

class ConnectionPrivate;
class QELOQUENT_EXPORT Connection
{
    Q_DECLARE_PRIVATE(Connection)
public:
    using Closure = std::function<bool(Connection *)>;

    explicit Connection(const QString &prefix = "");
    explicit Connection(const Connection &other);
    virtual ~Connection();

    virtual QSharedPointer<Grammar> schemaGrammar();
    virtual QSharedPointer<Grammar> queryGrammar();

    virtual SchemaBuilder schemaBuilder() const;
    virtual QueryBuilder queryBuilder() const;

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

    QString selectOne(const QString &query, const QStringList &bindings = QStringList());
    QSqlQuery select(const QString &query, const QVariantMap &bindings = QVariantMap());
    bool insert(const QString &query, const QVariantMap &bindings = QVariantMap());
    bool insert(const QString &query, const QList<QVariantMap> &bindings = QList<QVariantMap>());
    int update(const QString &query, const QVariantMap &bindings = QVariantMap());
    int del(const QString &query, const QVariantMap &bindings = QVariantMap());
    // execute a sql
    int statement(const QString &query, const QVariantMap &bindings = QVariantMap());
    int affectingStatement(const QString &query, const QVariantMap &bindings = QVariantMap());

protected:
    virtual Grammar *createScheamGrammar() = 0;
    virtual Grammar *createQueryGrammar() = 0;

    explicit Connection(ConnectionPrivate &dd, const QString &prefix = "");
    QScopedPointer<ConnectionPrivate> d_ptr;
};

#endif // CONNECTION_H
