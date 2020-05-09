#ifndef HASONE_H
#define HASONE_H

#include "HasOneOrMany.h"
#include "../Model.h"

class HasOne : public HasOneOrMany
{
public:
    HasOne(Model *related, Model *parent, const QString &foreignKey, const QString &localKey);
};

#endif // HASONE_H
