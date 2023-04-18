#include "HasOneOrMany.h"
#include "HasOneOrMany_p.h"

HasOneOrManyPrivate::HasOneOrManyPrivate(Relation *q)
    : RelationPrivate(q)
{

}

HasOneOrMany::HasOneOrMany(Model *related, Model *parent, const QString &foreignKey, const QString &localKey)
    : Relation(*new HasOneOrManyPrivate(this), related, parent)
{
    Q_D(HasOneOrMany);
    d->foreignKey = foreignKey;
    d->localKey = localKey;
}

HasOneOrMany::HasOneOrMany(EloquentBuilder *builder, Model *parent, const QString &foreignKey, const QString &localKey)
    : Relation(*new HasOneOrManyPrivate(this), builder, parent)
{
    Q_D(HasOneOrMany);
    d->foreignKey = foreignKey;
    d->localKey = localKey;
}

HasOneOrMany::~HasOneOrMany()
{

}
