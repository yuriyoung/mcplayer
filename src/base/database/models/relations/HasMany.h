#ifndef HASMANY_H
#define HASMANY_H

#include "HasOneOrMany.h"

class HasMany : public HasOneOrMany
{
public:
    HasMany(Model *related, Model *parent, const QString &foreignKey, const QString &localKey);
    ~HasMany();
};

#endif // HASMANY_H
