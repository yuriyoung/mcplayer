#include "HasMany.h"

HasMany::HasMany(Model *related, Model *parent, const QString &foreignKey, const QString &localKey)
    : HasOneOrMany(related, parent, foreignKey, localKey)
{

}

HasMany::~HasMany()
{

}
