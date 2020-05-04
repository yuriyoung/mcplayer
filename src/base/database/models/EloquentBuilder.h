#ifndef ELOQUENTBUILDER_H
#define ELOQUENTBUILDER_H

#include <QObject>

class Model;
class QueryBuilder;

class EloquentBuilderPrivate;
class EloquentBuilder
{
    Q_DECLARE_PRIVATE(EloquentBuilder)
    QScopedPointer<EloquentBuilderPrivate> d_ptr;
public:
    EloquentBuilder(const QueryBuilder &query, const Model *model);
    EloquentBuilder(const EloquentBuilder &other);
    ~EloquentBuilder();

    EloquentBuilder &where(const QString &column, const QVariant &value);

    void setQuery(const QueryBuilder &query);
    QueryBuilder query() const;

    void setModel(Model *model);
    Model *model() const;
};

#endif // ELOQUENTBUILDER_H
