#include "HasRelationship.h"
#include "Model.h"

//#include "relations/Relation.h"

class HasRelationshipPrivate
{
public:
    HasRelationshipPrivate(Model *model, HasRelationship *q)
        : q_ptr(q), model(model)
    {
    }

    HasRelationship *q_ptr = nullptr;
    Model *model = nullptr;
};

HasRelationship::HasRelationship(Model *model)
    : d_ptr(new HasRelationshipPrivate(model, this))
{

}

HasRelationship::~HasRelationship()
{

}

Relation HasRelationship::hasOne(Model *related, const QString &foreignKey, const QString localKey) const
{
    Q_D(const HasRelationship);
    const QString fk = foreignKey.isEmpty() ? d->model->foreignKey() : foreignKey;
    const QString lk = localKey.isEmpty() ? d->model->primaryKey() : localKey;

    return HasOne(related, d->model, fk, lk);
}

Relation HasRelationship::hasMany(Model *related, const QString &foreignKey, const QString localKey) const
{
    Q_D(const HasRelationship);
    const QString fk = foreignKey.isEmpty() ? d->model->foreignKey() : foreignKey;
    const QString lk = localKey.isEmpty() ? d->model->primaryKey() : localKey;

    return HasMany(related, d->model, fk, lk);
}

Relation HasRelationship::hasOneThrough() const
{
    return Relation();
}

Relation HasRelationship::hasManyThrough() const
{
    return Relation();
}

Relation HasRelationship::belongsTo(const QString &foreignKey,
                                    const QString localKey, const QString &relation) const
{
    return Relation();
}

Relation HasRelationship::belongsToMany() const
{
    return Relation();
}

Relation HasRelationship::morphOne() const
{
    return Relation();
}

Relation HasRelationship::morphTo() const
{
    return Relation();
}

Relation HasRelationship::morphMany() const
{
    return Relation();
}

Relation HasRelationship::morphToMany() const
{
    return Relation();
}

Relation HasRelationship::morphedByMany() const
{
    return Relation();
}

Model *HasRelationship::model() const
{
    Q_D(const HasRelationship);
    return d->model;
}
