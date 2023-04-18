#ifndef SCHEMABUILDER_H
#define SCHEMABUILDER_H

#include "Blueprint.h"
#include <QObject>

class Connection;
class SchemaBuilderPrivate;
class SchemaBuilder
{
    Q_DECLARE_PRIVATE(SchemaBuilder)
public:
    SchemaBuilder(const Connection *connection);
    SchemaBuilder(const SchemaBuilder &other);
    SchemaBuilder& operator=(const SchemaBuilder &other);
    virtual ~SchemaBuilder();

    // create a new table
    void create(const QString &table, Blueprint::Closure fun);

    void drop(const QString &table);
    void dropIfExists(const QString &table);

    virtual void dropAllTables();
    virtual void dropAllViews();

    void renameTable(const QString &from, const QString &to);

    int defaultStringLength() const;
    bool hasTable(const QString &table) const;

protected:
    QScopedPointer<SchemaBuilderPrivate> d_ptr;
};

#endif // SCHEMABUILDER_H
