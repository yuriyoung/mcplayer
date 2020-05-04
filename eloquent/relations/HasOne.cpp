#include "HasOne.h"

HasOne::HasOne(Model *related, Model *parent, const QString &foreignKey, const QString &localKey)
    : HasOneOrMany(related, parent, foreignKey, localKey)
{

}
