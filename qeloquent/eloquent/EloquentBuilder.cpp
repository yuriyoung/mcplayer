#include "EloquentBuilder.h"
#include "Model.h"
#include "query/QueryBuilder.h"

class EloquentBuilderPrivate
{
    Q_DECLARE_PUBLIC(EloquentBuilder)
public:
    EloquentBuilderPrivate(EloquentBuilder *q) : q_ptr(q)
    {
    }

    EloquentBuilder *q_ptr = nullptr;
    QueryBuilder query;
    Model *model = nullptr;
};

EloquentBuilder::EloquentBuilder(const QueryBuilder &query, const Model *model)
    : d_ptr(new EloquentBuilderPrivate(this))
{
    d_ptr->query = query;
    d_ptr->model = const_cast<Model*>(model);
}

EloquentBuilder::EloquentBuilder(const EloquentBuilder &other)
    : d_ptr(new EloquentBuilderPrivate(this))
{
    *d_ptr.data() = *other.d_ptr.data();
}

EloquentBuilder::~EloquentBuilder()
{

}

EloquentBuilder &EloquentBuilder::where(const QString &column, const QVariant &value)
{
    Q_D(EloquentBuilder);
    d->query.where(column, "=", value);
    return *this;
}

void EloquentBuilder::setQuery(const QueryBuilder &query)
{
    Q_D(EloquentBuilder);
    d->query = query;
}

QueryBuilder EloquentBuilder::query() const
{
    Q_D(const EloquentBuilder);
    return d->query;
}

void EloquentBuilder::setModel(Model *model)
{
    Q_D(EloquentBuilder);
    d->model = model;
    d->query.from(model->table());
}

Model *EloquentBuilder::model() const
{
    Q_D(const EloquentBuilder);
    return d->model;
}
