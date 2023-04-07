#include "SchemaBuilder.h"
#include "Blueprint.h"
#include "Connection.h"

#include <QFile>
#include <QDebug>

class SchemaBuilderPrivate
{
    Q_DECLARE_PUBLIC(SchemaBuilder)
public:
    SchemaBuilderPrivate(SchemaBuilder *q)
        : q_ptr(q)
    {
    }

    void build(Blueprint *blueprint);

    SchemaBuilder *q_ptr = nullptr;
    Connection *connection = nullptr;
    int defaultStringLength = 255;
};

void SchemaBuilderPrivate::build(Blueprint *blueprint)
{
    blueprint->build(connection, connection->schemaGrammar().get());
}

/**
 * @brief SchemaBuilder::SchemaBuilder
 * @param connection
 */
SchemaBuilder::SchemaBuilder(const Connection *connection)
    : d_ptr(new SchemaBuilderPrivate(this))
{
    qDebug() << "SchemaBuilder::SchemaBuilder()";
    d_ptr->connection = const_cast<Connection*>(connection);
}

SchemaBuilder::SchemaBuilder(const SchemaBuilder &other)
    : d_ptr(new SchemaBuilderPrivate(this))
{
    *d_ptr.data() = *other.d_ptr.data();
}

SchemaBuilder &SchemaBuilder::operator=(const SchemaBuilder &other)
{
    *d_ptr.data() = *other.d_ptr.data();
    return *this;
}

SchemaBuilder::~SchemaBuilder()
{
    qDebug() << "SchemaBuilder::~SchemaBuilder()";
}

void SchemaBuilder::create(const QString &table, Blueprint::Closure fun)
{
    Q_D(SchemaBuilder);
    QSharedPointer<Blueprint> blueprint(new Blueprint(table, fun));
    blueprint->create();

    d->build(blueprint.data());
}

void SchemaBuilder::drop(const QString &table)
{
    Q_D(SchemaBuilder);
    QSharedPointer<Blueprint> blueprint(new Blueprint(table, [](Blueprint *blueprint)
    {
        blueprint->drop();
    }));

    d->build(blueprint.data());
}

void SchemaBuilder::dropIfExists(const QString &table)
{
    Q_D(SchemaBuilder);
    QSharedPointer<Blueprint> blueprint(new Blueprint(table, [](Blueprint *blueprint)
    {
        blueprint->dropIfExists();
    }));

    d->build(blueprint.data());
}

void SchemaBuilder::dropAllTables()
{
    Q_D(SchemaBuilder);
    if(QString("QSQLITE").compare(d->connection->driverName(), Qt::CaseInsensitive) != 0)
    {
        qWarning() << "This database driver does not support dropping all tables.";
        return;
    }

    // TODO: execute drop all table commands
}

void SchemaBuilder::dropAllViews()
{

}

void SchemaBuilder::renameTable(const QString &from, const QString &to)
{
    Q_D(SchemaBuilder);
    QSharedPointer<Blueprint> blueprint(new Blueprint(from, [&from, &to](Blueprint *blueprint)
    {
        blueprint->rename(from, to);
    }));

    d->build(blueprint.data());
}

int SchemaBuilder::defaultStringLength() const
{
    Q_D(const SchemaBuilder);
    return d->defaultStringLength;
}

bool SchemaBuilder::hasTable(const QString &table) const
{
    Q_D(const SchemaBuilder);
    QString tableName = d->connection->tablePrefix() + table;

    // TODO: check the tableName exists or not in current connection
    // ...

    return false;
}
