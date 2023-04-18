#ifndef HASONEORMANY_P_H
#define HASONEORMANY_P_H

#include "Relation_p.h"

class HasOneOrManyPrivate : public RelationPrivate
{
public:
    HasOneOrManyPrivate(Relation *q);

    QString foreignKey;
    QString localKey;
};

#endif // HASONEORMANY_P_H
