#ifndef HASONEORMANY_H
#define HASONEORMANY_H

#include "Relation.h"

class HasOneOrManyPrivate;
class HasOneOrMany : public Relation
{
    Q_DECLARE_PRIVATE(HasOneOrMany)
public:
    HasOneOrMany(Model *related, Model *parent, const QString &foreignKey, const QString &localKey);
    HasOneOrMany(EloquentBuilder *builder, Model *parent, const QString &foreignKey, const QString &localKey);
    ~HasOneOrMany();
};

#endif // HASONEORMANY_H
